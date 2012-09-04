#ifndef __FRAME_H__
#define __FRAME_H__
#include <oslib/oslib.h>
#include "const.h"

#define FRAME_ATTR_NONE     0
#define FRAME_ATTR_SCALE    1
#define FRAME_ATTR_ROTATE   2
#define FRAME_ATTR_ALPHA    4
#define FRAME_ATTR_PALETTE  8
#define FRAME_ATTR_TINT     16

#define FRAME_EFFECT_NONE   0
#define FRAME_EFFECT_TINT   1

/*
 * osl_img as internal implementation.
 */

typedef struct {
    OSL_IMAGE *img;              /* image data */
    int alpha;                /* plane alpha */
    int x, y;                   /* position */
    int enables;
} frame_t;

/* a simple config of a frame. */
typedef struct {
    int size;
    char tex_name[MAX_TEXTURE_NAME+1];
    int x, y;
    int sx, sy, w, h;
    int center_x, center_y;
    float scale_x, scale_y;
    int angle;
    float alpha;
    int size_palette;
    int palette[0];
} frame_cfg_t;

frame_t *frame_create_simple(const char *filename, int pixel_format, \
    int location);
frame_t *frame_copy(frame_t *frame);
void frame_config(frame_t *frame, frame_cfg_t *cfg);
void frame_draw(frame_t *frame, int x, int y);
void frame_destroy(frame_t *frame);

int frame_get_width(frame_t *frame);
int frame_get_height(frame_t *frame);

frame_cfg_t *frame_cfg_lerp(frame_cfg_t *fcfg1, frame_cfg_t *fcfg2, float f);
void frame_cfg_destroy(frame_cfg_t *cfg);

#endif
