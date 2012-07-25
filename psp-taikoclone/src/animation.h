#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include <osl/oslib.h>
#include "const.h"

#define ANIME_CTRL_IMG		0x1
#define ANIME_CTRL_POS		0x2
#define ANIME_CTRL_SCALE	0x4
#define ANIME_CTRL_PALETTE	0x8

typedef struct {
	// switches for different animation type
	int enables;
	bool is_loop;
	bool is_stopped;
	bool is_used;	// a flag for reuse
	float framerate;
	float time;
	
	// frame by frame animation, TODO:mipmap??
	OSL_IMAGE **imgs;
	int imgs_count;
	// pos animation
	int pos_interp;
	int pos_count;
	int *pos_x, *pos_y;
	// scale animation
	int scale_interp;
	int scale_count;
	float *scale_x, *scale_y;
	// palette animation, only work when all imgs are in the same texture sheet
	// used to implement glow effect, plane alpha, colorize effect, etc.
	OSL_PALETTE **palettes;
	int palettes_count;
	
	// current frame
	frame_t *current_frame;
	// callback
	void (*stopped_callback)(anime_control_t *self);
} anime_control_t;

typedef struct {
	OSL_IMAGE **images;
	int num_images;	
} anime_control_images_t;

#endif
