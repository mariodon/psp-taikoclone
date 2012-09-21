#include <oslib/oslib.h>
#include "animation.h"
#include "frame_factory.h"
#include "drawing.h"

PSP_MODULE_INFO("Taiko No Tatsujin", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(-64*15);

int main()
{
    scePowerSetCpuClockFrequency(333);

    oslInit(0);
    oslInitGfx(OSL_PF_5551, 1);

    frame_factory_init("config/textures.ini");
    drawing_init();

    clock_t t0, t1;
    while (! osl_quit) {
        t0 = clock();
        drawing_update();

        oslStartDrawing();
            oslClearScreen(RGB(0, 0, 0));
            drawing_draw();
        oslEndDrawing();
        oslSyncFrameEx(0, 0, 0);
        t1 = clock();
//        printf("time = %d\n", t1 - t0);
    }
}
