#ifndef __FRAME_H__
#define __FRAME_H__
#include <oslib/oslib.h>

/* This contains all the information to describe a frame of an animation.
 * note: image data should not be shared with other frames! Make them
 * a shared copy of a texture will be an ideal choice.
 */

typedef struct {
    OSL_IMAGE *osl_img;         /* image data */
    int x, y;                   /* pos */
    int center_x, center_y;     /* anchor point */
    float scale_x, scale_y;     /* scale */             /* alpha */
    OSL_PALETTE *palette;   /* palette if indexed. */
} frame_t;

frame_t *frame_create(OSL_IMAGE *osl_img);
void frame_draw(frame_t *frame);
void frame_destroy(frame_t *frame);

#endif
