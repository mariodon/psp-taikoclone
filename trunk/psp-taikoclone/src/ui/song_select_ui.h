#ifndef _SONG_SELECT_UI_H_
#define _SONG_SELECT_UI_H_
#include <oslib/oslib.h>

void *song_select_ui_handle_input(OSL_CONTROLLER *pad);
void update_song_select_ui();
void init_song_select_ui(void *data, int data_length);
#endif
