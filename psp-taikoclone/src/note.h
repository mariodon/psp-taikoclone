#ifndef _NOTE_H_
#define _NOTE_H_
#include <oslib/oslib.h>

struct {
    /* common field*/
    OSL_IMAGE **textures;
    int (*handle_input)(note_t *note, float play_pos, OSL_CONTROLLER *pad);
    int (*can_be_hit)(note_t *note, float play_pos);
    int (*can_be_seen)(note_t *note, float play_pos);
    int (*get_left)(note_t *note);
    int (*get_right)(note_t *note);
    /* dynamic field for the first note */
} note_routine_t;

int can_be_seen_don(note_t *note, float play_pos)
{
}

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
#endif
