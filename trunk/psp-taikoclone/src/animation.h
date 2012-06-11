#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include <osl/oslib.h>

#define ANIME_CTRL_MOVE_LINEAR       0
#define ANIME_CTRL_MOVE_CUSUTOM      1
#define ANIME_CTRL_SEQUENTIAL_PICS   2
#define ANIME_CTRL_ALPHA_LINEAR      3
#define ANIME_CTRL_COMBINED          4

struct {
    OSL_IMAGE **texture_array;
    float x, y;
    float alpha;
} ANIME_DATA_T;

struct {
    float fps;
    float t;
    int is_loop;
    int type;
    int (*update)(float elapse_time);
} ANIME_CTRL_COMMON_T;

struct {
    ANIME_CTRL_COMMON_T comm;
    /* custom */
    float startX, startY;
    float endX, endY;
    float velocityX, velocityY;
} ANIME_CTRL_MOVE_LINEAR_T;

struct {
    ANIME_CTRL_COMMON_T comm;
    /* custom */
    float originX, originY;
    float *points;
} ANIME_CTRL_MOVE_CUSTOME_T;

struct {
    ANIME_CTRL_COMMON_T comm;
    /* custom */
} ANIME_CTRL_SEQUENTIAL_PICS_T;

struct {
    ANIME_CTRL_COMMON_T comm;
    /* custom */
    void *sub_animations;
    ANIME_JOIN_T *anime_joins;
} ANIME_CTRL_COMBINED_T;

struct {
    void *animation;
    int join_type;
} ANIME_JOIN_T;
#endif
