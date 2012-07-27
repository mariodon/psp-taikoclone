#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include <oslib/oslib.h>
#include "const.h"
#include "frame.h"

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

#define ANIME_FRAMERATE_DEFAULT			30

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
    void (*callback)(void *self);
	/* anime component */
	anime_func_t *ani_funcs[4];
	/* anime result frame */
	frame_t *frame;
} anime_t;

typedef void (*anime_callback_t)(void *);

//------------------------------------------------------------------------------
// functions
//------------------------------------------------------------------------------
anime_t *anime_create_empty();
anime_t *anime_from_file(const char *file);
inline void anime_set_func(anime_t *ani, anime_func_t *func)
{
	ani->ani_funcs[func->type] = func;
}

inline void anime_set_callback(anime_t *ani, anime_callback_t callback)
{
	ani->callback = callback;
}

//------------------------------------------------------------------------------
// sub routines
//------------------------------------------------------------------------------
void anime_eval_func(anime_func_t *func, int frame, frame_t *res);
anime_key_t *bisearch_key_points(int frame, anime_key_t *keys, int num_keys);
#endif
