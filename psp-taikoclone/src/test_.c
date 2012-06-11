#include "aalib/pspaalib.h"
#include <oslib/oslib.h>

PSP_MODULE_INFO("Taiko No Tatsujin", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(-64*15);

int main()
{
    AalibInit();
    AalibLoad("tja/elpyscongroo.ogg", PSPAALIB_CHANNEL_OGG_1, TRUE);
    AalibPlay(PSPAALIB_CHANNEL_OGG_1, TRUE);

    while (1) {
    }
}
