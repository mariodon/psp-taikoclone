#ifndef _TAIKO_FLASH_H_
#define _TAIKO_FLASH_H_
#include <oslib/oslib.h>

#define TAIKO_FLASH_LEFT 0
#define TAIKO_FLASH_RIGHT 1
#define TAIKO_FLASH_RED 0
#define TAIKO_FLASH_BLUE 1

void init_taiko_flash(OSL_IMAGE **v);
void refresh_taiko_flash(int color, int dir);
void update_taiko_flash(float elapse_time);

#endif
