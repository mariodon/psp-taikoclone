#include "animation.h"
#include "frame.h"

anime_t *anime_create_empty()
{
	anime_t *ani = (anime_t *)malloc(sizeof(anime_t));
	if (ani == NULL) {
		oslFatalError("Can't malloc for animation!");
	}

	ani->time = 0;
	ani->framerate = ANIME_FRAMERATE_DEFAULT;
	ani->stop_callback = NULL;
	ani->ani_seq = NULL;
	ani->ani_path = NULL;
	ani->ani_pal = NULL;
	ani->ani_scale = NULL;
	ani->frame = frame_create(NULL);
}

// anime_t file layout:
// offset	variable
// +0x0		framerate		
anime_t *anime_from_file(const char *file)
{
	SceUID fd = -1;
	anime_t *ret_ani = NULL;
	
	// open animation file
	fd = SceIoOpen(file, PSP_O_RDONLY, 0777);
	if (fd < 0) {
		oslFatalError("can't open file %s", file);
	}
	// create empty anime_t struct to fill
	ret_ani = anime_create_empty();
	// 
}

void anime_update(anime_t *ani, float step)
{
	assert(ani != NULL)
	ani->time += step;
	
	int frame = (int)(ani->time * ani->framerate);
	bool stop = TRUE;
	anime_func_t *func;
	
	int i;
	for (i = 0; i < 4; ++ i) {
		func = ani->ani_funcs[i];
		if (func == NULL || func->is_stopped) {
			continue;
		}
		// check if func should be stopped
		if (! func->is_loopped && frame >= func->total_frame - 1) {
			func->is_stopped = TRUE;
		}
		anime_eval_func(func, frame, ani->frame);
		// at least one func is not dead, so go on
		stop = FALSE;
	}
	
	// callback
	if (stop && ani->callback != NULL) {
		ani->callback(ani);
	}
}

void anime_eval_func(anime_func_t *func, int frame, frame_t *res)
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
	key = bisearch_key_points(frame, func->keys, func->num_keys)
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
		ret->palette = func->value.palette;
		break;
	}
	
	return;
}

frame_t *anime_get_frame(anime_t *ani)
{
	return ani->frame;
}

inline void anime_set_func(anime_t *ani, anime_func_t *func)
{
	ani->funcs[ani->type] = func;
}

inline anime_set_callback(anime_t *ani, anime_callback_t callback)
{
	ani->callback = callback;
}

//binary search for a right [key1, key2] section and return key1
anime_key_t *bisearch_key_points(int frame, anime_key_t **keys, 
	int num_keys) {
	// empty array
	if (num_keys == NULL) {
		return NULL;
	}
	// only one element
	if (num_keys == 1) {
		return keys[0];
	}
	// begin binary search
	int left = 0;
	int right = num_keys - 1;
	anime_key_t *key1, *key2;
	while (left < right) {
		key1 = keys[left];
		key2 = key1 + 1;
		if (key1->frame > frame) {
			right = (left + right) >> 1;
		} else if (key2->frame < frame) {
			left = (left + right) >> 1;
		} else {
			return key1;
		}
	}
	// not found
	return NULL;
}