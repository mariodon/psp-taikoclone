#ifndef _DRAWING_H_
#define _DRAWING_H_
#include <oslib/oslib.h>
#include "tjaparser.h"

void drawing_init();
void drawing_draw();
void drawing_update();
void draw_image_tiles(OSL_IMAGE*,int,int,int,int);

void get_note_left_right(note_t *note, int x, int *left, int *right);
void draw_note(note_t *note, int x, int y);
#endif
