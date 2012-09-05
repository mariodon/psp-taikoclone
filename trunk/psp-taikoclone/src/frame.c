#include <assert.h>
#include <oslib/oslib.h>
#include "frame.h"

/*
 * create a simple frame from a image file, with the right pixel format
 *  and location. */
frame_t *frame_create_simple(const char *filename, int pixel_format, \
    int location)
{
    frame_t *obj = NULL;

    obj = (frame_t *)malloc(sizeof(frame_t));
    if (obj == NULL) {
        return NULL;
    }

    printf("ok??? %s %d %d\n", filename, pixel_format, location);
    obj->enables = FRAME_ATTR_NONE;
    obj->img = oslLoadImageFile((char *)filename, location, pixel_format);
    if (obj->img == NULL) {
        free(obj);
        return NULL;
    }
    obj->alpha = 1.0;
    obj->x = obj->y = 0;
    return obj;
}

frame_t *frame_copy(frame_t *old)
{
    frame_t *obj = NULL;

    obj = (frame_t *)malloc(sizeof(frame_t));
    if (obj == NULL) {
        return NULL;
    }

    obj->enables = old->enables;
    obj->img = oslCreateImageTile(old->img, old->img->offsetX0, old->img->offsetY0, old->img->offsetX1, old->img->offsetY1);
    obj->alpha = old->alpha;
    obj->x = old->x;
    obj->y = old->y;

    return obj;
}

/*
 * config a already created frame to give it a lot of attributes, but share
 * the same image data.
 * */
void frame_config(frame_t *obj, frame_cfg_t *cfg)
{
    // check enables
    obj->enables = FRAME_ATTR_NONE;
    if (cfg->alpha != 1.0) {
        obj->enables |= FRAME_ATTR_ALPHA;
    }
    if (cfg->size_palette != 0 \
            && osl_pixelWidth[obj->img->pixelFormat] <= 8
            && cfg->size_palette == (1 << osl_pixelWidth[obj->img->pixelFormat])) {
        obj->enables |= FRAME_ATTR_PALETTE;
    }
    if (cfg->scale_x != 1.0 || cfg->scale_y != 1.0) {
        obj->enables |= FRAME_ATTR_SCALE;
    }
    if (cfg->angle != 0) {
        obj->enables |= FRAME_ATTR_ROTATE;
    }
    // config sub image
    obj->img->offsetX0 = cfg->sx;
    obj->img->offsetY0 = cfg->sy;
    obj->img->offsetX1 = cfg->sx + cfg->w - 1;
    obj->img->offsetY1 = cfg->sy + cfg->h - 1;
    obj->img->stretchX = cfg->w * cfg->scale_x;
    obj->img->stretchY = cfg->h * cfg->scale_y;
    obj->x = cfg->x - cfg->center_x * cfg->scale_x;
    obj->y = cfg->y - cfg->center_y * cfg->scale_y;

    // config rotation
    obj->img->centerX = cfg->center_x;
    obj->img->centerY = cfg->center_y;
    obj->img->angle = cfg->angle;

    // config palette
    if (obj->enables & FRAME_ATTR_PALETTE) {
        OSL_PALETTE *palette = NULL;
        int i;
        palette = oslCreatePalette(cfg->size_palette, OSL_PF_8888);
        for (i = 0; i < cfg->size_palette; ++ i) {
            ((u32*)palette->data)[i] = cfg->palette[i];
        }
        oslUncachePalette(palette);
        obj->img->palette = palette;
    }
}


/* draw a frame, with the status applied.
 * This is the most simple implementation.
 */
void frame_draw(frame_t *frame, int x, int y)
{
    if (frame->enables & FRAME_ATTR_ALPHA) {
        oslSetAlpha(OSL_FX_ALPHA, (int)(frame->alpha * 255));
    } else {
        oslSetAlpha(OSL_FX_DEFAULT, 0);
    }

    if (frame->enables & FRAME_ATTR_ROTATE) {
        oslDrawImageXY(frame->img, x + frame->x, y + frame->y);
    } else {
        oslDrawImageSimpleXY(frame->img, x + frame->x, y + frame->y);
    }
}

void frame_destroy(frame_t *frame)
{
	if (frame != NULL) {
		if (frame->img != NULL) {
            if (frame->enables & FRAME_ATTR_PALETTE) {
                oslDeletePalette(frame->img->palette);
            }
			oslDeleteImage(frame->img);
			frame->img = NULL;
		}
		free(frame);
	}
}

inline int frame_get_width(frame_t *frame)
{
	return frame->img->stretchX;
}

inline int frame_get_height(frame_t *frame)
{
	return frame->img->stretchY;
}

frame_cfg_t *frame_cfg_lerp(frame_cfg_t *fcfg1, frame_cfg_t *fcfg2, float f)
{
    frame_cfg_t *ret;

    if (fcfg1->size != fcfg2->size || strcmp(fcfg1->tex_name, fcfg2->tex_name)) {
        return NULL;
    }

    ret = malloc(sizeof(fcfg1->size));
    if (ret == NULL) {
        return ret;
    }
 
    memcpy(ret, fcfg1, fcfg1->size);

    if (f <= 0) {
        f = 0;
    } else if (f >= 1) {
        f = 1;
    }

    strcpy(ret->tex_name, fcfg1->tex_name);
    ret->x = (fcfg2->x - fcfg1->x) * f + fcfg1->x;
    ret->y = (fcfg2->y - fcfg1->y) * f + fcfg1->y;
    ret->scale_x = (fcfg2->scale_x - fcfg1->scale_x) * f + fcfg1->scale_x;
    ret->scale_y = (fcfg2->scale_y - fcfg1->scale_y) * f + fcfg1->scale_y;
    ret->angle = (fcfg2->angle - fcfg1->angle) * f + fcfg1->angle;
    ret->alpha = (fcfg2->alpha - fcfg1->alpha) * f + fcfg1->alpha;
    int i;
    for (i = 0; i < ret->size_palette; ++ i) {
        ret->palette[i] = (fcfg2->palette[i] - fcfg1->palette[i]) * f + fcfg1->palette[i];
    }

    return ret;
}

void frame_cfg_destroy(frame_cfg_t *cfg)
{
    free(cfg);
}
