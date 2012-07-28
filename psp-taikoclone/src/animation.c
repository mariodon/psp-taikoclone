#include <assert.h>
#include "animation.h"
#include "frame.h"
#include "textures.h"

anime_t *anime_create_empty()
{
	anime_t *ani = (anime_t *)malloc(sizeof(anime_t));
	if (ani == NULL) {
		oslFatalError("Can't malloc for animation!");
	}

	ani->time = 0;
	ani->framerate = ANIME_FRAMERATE_DEFAULT;
	ani->callback = NULL;
    int i;
    for (i = 0; i < 4; ++ i) 
        ani->ani_funcs[i] = NULL;
	ani->frame = frame_create(NULL);
	return ani;
}

// anime_t file layout:
// offset	variable
// 0		framerate
// 4		func_count		
anime_t *anime_from_file(const char *file)
{
	SceUID fd = -1;
	anime_t *ret_ani = NULL;
	int func_count;
	int i, j;
	struct {
		int type;
		int interp;
		bool is_loopped;
		int num_keys;
	} func_data;
	struct {
		int frame;
		int sx, sy, w, h, center_x, center_y;		
		char tex_name[MAX_TEXTURE_NAME+1];
	} image_data;
	struct {
		char r;
		char g;
		char b;
		char a;
	} color_data;
	size_t bytes;
	int palette_size;
	
	// open animation file
	fd = sceIoOpen(file, PSP_O_RDONLY, 0777);
	if (fd < 0) {
		oslFatalError("can't open file %s", file);
	}
	// create empty anime_t struct to fill
	ret_ani = anime_create_empty();
	// fill anime_t struct
	bytes = sizeof(float);
	if (sceIoRead(fd, &(ret_ani->framerate), bytes) != bytes) {
		oslFatalError("corrupt ani file");
	}
	ret_ani->framerate /= 1000.0;
	bytes = sizeof(int);
	if (sceIoRead(fd, &func_count, bytes) != bytes) {
		oslFatalError("corrupt ani file");
	}
	
	anime_func_t *func;
	while (func_count > 0) {
		-- func_count;
		bytes = sizeof(func_data);
		if (sceIoRead(fd, &func_data, bytes) != bytes) {
			oslFatalError("corrupt ani file");
		}
		func = NULL;
		func = (anime_func_t *)malloc(sizeof(anime_func_t) \
			+ func_data.num_keys * sizeof(anime_key_t));
		if (func == NULL) {
			oslFatalError("can't malloc for anime_func_t");
		}
		func->type = func_data.type;
		func->interp = func_data.interp;
		func->is_loopped = func_data.is_loopped;
		func->num_keys = func_data.num_keys;
        printf("reading: %d %d %d %d\n", func->type, func->interp, \
            func->is_loopped, func->num_keys);
		func->is_stopped = FALSE;
		// specially, create instance for image or palette
		if (func->type == ANIME_FUNC_SEQUENCE) {
			bytes = sizeof(image_data);
			for (i = 0; i < func->num_keys; ++ i) {
				if (sceIoRead(fd, &image_data, bytes) != bytes) {
					oslFatalError("corrupt ani file");
				}
				func->keys[i].value.img = \
					textures_shared_copy(image_data.tex_name, \
									image_data.sx, image_data.sy, \
									image_data.w, image_data.h);
				func->keys[i].value.img->centerX = image_data.center_x;
				func->keys[i].value.img->centerY = image_data.center_y;
				func->keys[i].frame = image_data.frame;
			}
		} else if (func->type == ANIME_FUNC_PALETTE) {
			for (i = 0; i < func->num_keys; ++ i) {
				bytes = sizeof(int);
				if (sceIoRead(fd, &(func->keys[i].frame), bytes) != bytes) {
					oslFatalError("corrupt ani file");
				}
				if (sceIoRead(fd, &palette_size, bytes) != bytes) {
					oslFatalError("corrupt ani file");
				}
				assert(palette_size == (1 << 4) || palette_size == (1<<8));
				func->keys[i].value.palette = oslCreatePalette(palette_size, \
					OSL_PF_8888);
				for (j = 0; j < palette_size; ++ j) {
					bytes = sizeof(color_data);
					if (sceIoRead(fd, &color_data, bytes) != bytes) {
						oslFatalError("corrupt ani file");
					}
					((u32*)func->keys[i].value.palette->data)[j] = \
						RGBA(color_data.r, color_data.g, color_data.b, color_data.a);
				}
			   oslUncachePalette(func->keys[i].value.palette);
			}
			
		} else {
			bytes = sizeof(anime_key_t) * func->num_keys;
			printf("sizeof path or scale key %d\n", sizeof(anime_key_t));
			if (sceIoRead(fd, func->keys, bytes) != bytes) {
				oslFatalError("corrupt ani file");
			}
		}
		func->total_frame = func->keys[func->num_keys - 1].frame + 1;
		anime_set_func(ret_ani, func);
	}
	for (i = 0; i < 4; ++ i) {
		printf("%p ", ret_ani->ani_funcs[i]);
	}
	printf("\n");
	return ret_ani;
}

void anime_update(anime_t *ani, float step)
{
	assert(ani != NULL);
	ani->time += step;
	
	int frame = (int)(ani->time * ani->framerate);
	bool stop = TRUE;
	anime_func_t *func;
	printf("frame = %d\n", frame);
	int i;
	for (i = 0; i < 4; ++ i) {
		func = ani->ani_funcs[i];
		if (func == NULL || func->is_stopped) {
			continue;
		}
		// check if func should be stopped
		if (! func->is_loopped && frame >= func->total_frame) {
			func->is_stopped = TRUE;
			continue;
		}
		printf("eval func type %d, total_frame %d\n", func->type, func->total_frame);
		anime_eval_func(func, frame, ani->frame);
		// at least one func is not dead, so go on
		stop = FALSE;
	}
	
	// callback
	if (stop && ani->callback != NULL) {
		ani->callback(ani);
	}
	if (stop) {
		printf("anime_stopped!\n");
	}
}

void anime_eval_func(anime_func_t *func, int frame, frame_t *ret)
{
	int type;
	int interp;
	anime_key_t *key, *key1;
	float f;
	
	// init
	type = func->type;
	interp = func->interp;
	
	// truncate frame
	if (frame >= func->total_frame) {
		if (func->is_loopped) {
			frame %= func->total_frame;
		} else {
			frame = func->total_frame - 1;
		}
	}

	// find key point
	key = bisearch_key_points(frame, func->keys, func->num_keys);
	assert(key != NULL);

	// interp
	switch(type) {
	
	case ANIME_FUNC_SEQUENCE:
		ret->osl_img = key->value.img;
		break;
		
	case ANIME_FUNC_PATH:
		if (interp == ANIME_INTERP_NONE || frame == func->total_frame) {
			ret->x = key->value.pos[0];
			ret->y = key->value.pos[1];
		} else if (interp == ANIME_INTERP_LINEAR) {
			key1 = key + 1;
			f = 1.0f * (frame - key->frame) / (key1->frame - key->frame);
			ret->x = (int)(key->value.pos[0] + f * (key1->value.pos[0] - key->value.pos[0]));
			ret->y = (int)(key->value.pos[1] + f * (key1->value.pos[1] - key->value.pos[1]));
		} /*else if (interp == ANIME_INTERP_PARABOLIC) {
			//the last two point descript the func
			// (x1, y1), ..., (xn, yn), (a, b), (c, 0)
			// y = ax^2 + bx + c
			// provided y is ignored because it can be recaculated
			assert(func->num_keys > 2);
			a = func->keys[func->num_keys - 2][0];
			b = func->keys[func->num_keys - 2][1];
			c = func->keys[func->num_keys - 1][0];
			key1 = key + 1;
			
		}*/
		break;
		
	case ANIME_FUNC_SCALE:
		if (interp == ANIME_INTERP_NONE || frame == func->total_frame) {
			ret->scale_x = key->value.scale[0];
			ret->scale_y = key->value.scale[1];
		} else if (interp == ANIME_INTERP_LINEAR) {
			key1 = key + 1;
			f = 1.0f * (frame - key->frame) / (key1->frame - key->frame);
			ret->scale_x = key->value.scale[0] + f * (key1->value.scale[0] - key->value.scale[0]);
			ret->scale_y = key->value.scale[1] + f * (key1->value.scale[1] - key->value.scale[1]);
		}	
		break;
		
	case ANIME_FUNC_PALETTE:
		ret->palette = key->value.palette;
		break;
	}
	
	return;
}

frame_t *anime_get_frame(anime_t *ani)
{
	return ani->frame;
}

//binary search for a right [key1, key2] section and return key1
anime_key_t *bisearch_key_points(int frame, anime_key_t *keys, 
	int num_keys) {
	// empty array
	if (num_keys == 0) {
		return NULL;
	}
	// only one element
	if (num_keys == 1) {
		return &keys[0];
	}
	// begin binary search
	int left = 0;
	int right = num_keys - 1;
	int mid;
	anime_key_t *key1, *key2;
	while (left < right) {
		mid = (left + right)>>1;
		key1 = &keys[mid];
		key2 = key1 + 1;
		printf("search (%d, %d), (%d, %d)\n", mid, mid + 1, key1->frame, key2->frame);
		if (key1->frame > frame) {
			right = mid;
		} else if (key2->frame < frame) {
			left = mid;
		} else {
			return key1;
		}
	}
	// not found
	return NULL;
}

inline void anime_set_func(anime_t *ani, anime_func_t *func)
{
	ani->ani_funcs[func->type] = func;
}

inline void anime_set_callback(anime_t *ani, anime_callback_t callback)
{
	ani->callback = callback;
}
