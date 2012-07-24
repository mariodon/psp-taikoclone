#ifndef __FRAME_H__
#define __FRAME_H__
#include <oslib/oslib.h>

/* This contains all the information to describe a frame of an animation.*/
typedef struct {
    OSL_IMAGE *osl_img;         /* image data */
    int x, y;                   /* pos */
    int center_x, center_y;     /* anchor point */
    float scale_x, scale_y;     /* scale */
    float alpha;                /* alpha */
    OSL_PALETTE *osl_palette;   /* palette if indexed. */
} frame_t;
#endif
