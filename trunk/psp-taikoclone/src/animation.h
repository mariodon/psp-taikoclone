#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include "const.h"
#include "frame.h"

//------------------------------------------------------------------------------
// definitions
//------------------------------------------------------------------------------

#define ANIME_PLAY_STATUS_PAUSED    0
#define ANIME_PLAY_STATUS_PLAYING   1
#define ANIME_PLAY_STATUS_STOPPED   2

typedef struct {
    int status;
    int speed;
    int frame_time;
    bool loop;

    int cur_frame;
    int time_passed;
    int num_frame;
    frame_t *frames[0];
} anime_t;

typedef struct {
    int len;    /* number of frame last */
    bool lerp;  /* linear lerp between this frame and next? */
    frame_cfg_t *cfg;   /* how to config that texture */
} anime_key_cfg_t;

typedef struct {
    int key_count;
    int play_speed;
    bool loop;
    anime_key_cfg_t keys[0];
} anime_cfg_t;

void anime_play(anime_t *ani);
void anime_pause(anime_t *ani);
void anime_stop(anime_t *ani);

void anime_update(anime_t *ani, int time);
void anime_draw(anime_t *ani, int x, int y);
void anime_goto_frame(anime_t *ani, int idx);

anime_t *anime_create_from_cfg(anime_cfg_t *cfg);
anime_t *anime_create_from_file(const char *filename);

#endif
