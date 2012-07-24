#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include <osl/oslib.h>
#include "const.h"

// base struct
typedef struct {
	float duration;
	bool is_loop;
	bool is_stopped;
	void (*draw)();
	void (*reset)();
	void (*play)();
} anime_t;

// frame by frame animation
typedef struct {
	float duration;
	bool is_loop;
	bool is_stopped;
	void (*draw)();
	void (*reset)();
	void (*play)();	
} anime_frames_t;

#endif
