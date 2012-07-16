#ifndef _NOTE_H_
#define _NOTE_H_
#include <oslib/oslib.h>
#include "const.h"
#include "tjaparser.h"

typedef struct {
    unsigned char left_don, right_don;
    unsigned char left_katsu, right_katsu;
    unsigned char big_don, big_katsu;
} play_input_t;

typedef struct {
    unsigned short bad;
    unsigned short good;
    unsigned short great;
} judge_level_t;

int note_init(note_t *note_N, note_t *note_E, note_t *note_M);
int note_update(float play_pos, int auto_play, OSL_CONTROLLER *pad);
int note_destroy();

#define NOTE_APPEAR_X   (SCREEN_WIDTH - 1)
#define NOTE_DISAPPEAR_X 80
#define NOTE_Y 109 
#define NOTE_FIT_X  104

//TODO: change this to taiko pic's right

#endif
