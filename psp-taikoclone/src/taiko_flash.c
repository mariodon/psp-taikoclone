//show taiko flash
#include "taiko_flash.h"

#define MAX_FLASH_TIME 150

OSL_IMAGE **flash_tex;
float t[4];

void init_taiko_flash(OSL_IMAGE **v)
{
    flash_tex = v;
    memset(t, 0, sizeof(t));
}

void refresh_taiko_flash(int color, int dir)
{
   t[dir*2+color] = MAX_FLASH_TIME;
}

void update_taiko_flash(float elapse_time)
{
    int i;
    for (i = 0; i < 4; ++i) {
        t[i] -= elapse_time;
        if (t[i] < 0)
            t[i] = 0;
        if (t[i] > 0) {
            oslDrawImage(flash_tex[i]);
        }
    }
}
