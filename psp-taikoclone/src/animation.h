#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include <osl/oslib.h>
#include "const.h"

//------------------------------------------------------------------------------
// definitions
//------------------------------------------------------------------------------

#define ANIME_FUNC_SEQUENCE		0x0
#define ANIME_FUNC_SCALE		0x1
#define ANIME_FUNC_PATH			0x2
#define ANIME_FUNC_PALETTE		0x3

#define ANIME_INTERP_NONE		0x0
#define ANIME_INTERP_LINEAR		0x1
#define ANIME_INTERP_PARABOLIC	0x2

#define ANIME_FRAMERATE			30

typedef void (*anime_callback_t)(anime_t *);

typedef struct {
	int frame;			//key value at which frame
	union {
		OSL_IMAGE *img;
		float scale[2];
		int pos[2];
		OSL_PALETTE *palette;
	} value;
} anime_key_t;

typedef struct {
	int type;		// which type of animation it applies to
	int interp;		// which interp method to use to interp key points
	bool is_loopped;
	int num_keys;

	int total_frame;
	bool is_stopped;
	anime_key_t keys[0];
} anime_func_t;

typedef struct {
	/* general control information */
	float time;
	float framerate;
	anime_callback_t callback;
	/* anime component */
	anime_func_t *ani_funcs[4];
	/* anime result frame */
	frame_t *frame;
} anime_t;


//------------------------------------------------------------------------------
// functions
//------------------------------------------------------------------------------
anime_t *anime_create_empty();
#endif
