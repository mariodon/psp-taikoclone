#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include <osl/oslib.h>

/*
 * note: We do not need an OSL_IMAGE * array, neither an OSL_PALETTE * array
 * to implement animation.
 */
typedef struct {
    /* moving */
    int x, y;
    /* scaling */
    float scale_x, scale_y;
    /* alpha */
    float alpha;
    /* sequenced images */
    int frame_idx, num_frame;
    /* different color schema */
    int palette_idx, num_palette;
    /* style */
    int style;
} frame_state_t;

/*
 * The following, accept input(usually, a time delta or in special case ,
 * interactive info) and changes one or more frame_state_t's component 
 * accordingly.
 * Thus create an animation effect.
 */

#endif
