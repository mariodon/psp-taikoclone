#include <oslib/oslib.h>
#include <time.h>
#include "aalib/pspaalib.h"
#include "const.h"
#include "helper/iniparser.h"
#include "load_texture.h"
#include "drawing.h"
#include "taiko_flash.h"
#include "tjaparser.h"
#include "song_select.h"

PSP_MODULE_INFO("Taiko No Tatsujin", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(-64*15); 

int bgm_channel;
//static char *tja_path = "ms0:/psp/game/taikoc/tja/";
char *tja_path = "tja2/";

void print_pf_value()
{
    printf("OSL_PF_8888: %d\n", OSL_PF_8888);
    printf("OSL_PF_5650: %d\n", OSL_PF_5650);
    printf("OSL_PF_4444: %d\n", OSL_PF_4444);
    printf("OSL_PF_5551: %d\n", OSL_PF_5551);
    printf("OSL_PF_8BIT: %d\n", OSL_PF_8BIT);
    printf("OSL_PF_4BIT: %d\n", OSL_PF_4BIT);    
}

int print_str_as_hex(char *str)
{
    char *p;
    puts("");
    for (p = str; *p != '\0'; ++ p) {
        printf("\\x%x", (unsigned char)(*p));
    }
    puts("");
}

char test1[500], test2[500];

note_t *initFumen(char *root, char *filename)
{
    char buf[512];
    tja_header_t header;
    note_t *ret;
    char len, buf3[512];
    short buf2[512];    
    char *p;

    strcpy(buf, root);
    strcat(buf, filename);
    if (tjaparser_load(buf) == 0) {
        return 0;
    }
    tjaparser_load(buf);
    if (tjaparser_read_tja_header(&header) == 0) {
        return NULL;
    }

    printf("%s\n", header.wave);
    print_str_as_hex(header.wave);

    AalibInit();
    AalibLoad("snd/dong.wav", PSPAALIB_CHANNEL_WAV_1, TRUE, FALSE);
    AalibLoad("snd/ka.wav", PSPAALIB_CHANNEL_WAV_2, TRUE, FALSE);    
    
    if (strcmp(".mp3", header.wave+strlen(header.wave)-4) == 0) {
        bgm_channel = PSPAALIB_CHANNEL_SCEMP3_1;
    } else if (strcmp(".ogg", header.wave+strlen(header.wave)-4) == 0) {
        bgm_channel = PSPAALIB_CHANNEL_OGG_1;
    } else {
        bgm_channel = PSPAALIB_CHANNEL_WAV_3;
    }
    strcpy(buf, root);

    len = cccGBKtoUCS2(buf2, 511, header.wave);
    printf("%d len=", len);
    buf2[len] = 0;

    len = cccUSC2toSJIS(buf3, 511, buf2); 
    printf("%d len=1", len);    
    buf3[len] = 0;
    
    //strcpy(buf3, "\x82\xed\x82\xf1\x82\xc9\x82\xe1\x81[\x83\x8f\x81[\x83\x8b\x83h.ogg");
    //strcat(buf, buf3);
    strcat(buf, header.wave);
    printf("Loading music ...\n");
    if (AalibLoad(buf, bgm_channel, FALSE, TRUE)) {
        printf("loading music %s failed\n", buf);
        return NULL;
    }
    printf("loading music ok!\n");

    printf("parsing fumen 0\n");
    if (tjaparser_parse_course(0, &ret) == 0) {
        return NULL;
    }
    printf("parsing fumen ok!\n");

    strcpy(test1, header.title);
    return ret;
}

int main(int argc, char *argv[])
{
    //OSL_IMAGE *bg;
    int frame = 0;

    int fps = 60;
    float tpf = 1000.0 / fps;
    float tbeg, tend;
    float t1 = -1, t2;

    int music_started = 0;

    note_t *note;
    float time_passed;

    OSL_CONTROLLER *pad;

    float eps = 1e-4;
    float delta = 0;
    float play_pos = 0;
    int selecting = TRUE; 

    /* selected fumen info */
    char *filename = malloc(512+1);
    char *root = malloc(512+1);
    int course_idx;

    /* real time info display */
    float fumen_offset;
    int offset = 17; /* offset fix up */
    int auto_play = TRUE;

    /* debug process */
    //char *debug_fumen = "\x82\xed\x82\xf1\x82\xc9\x82\xe1\x81[\x83\x8f\x81[\x83\x8b\x83h.tja";
    char *debug_fumen = NULL; //file_list[file_list_len-2];    

    scePowerSetCpuClockFrequency(333);

    oslInit(0);
    oslInitGfx(TAIKO_PF, 1);
    oslInitConsole();    
    oslIntraFontInit(INTRAFONT_CACHE_LARGE);

    
    OSL_FONT *jpn0 = oslLoadIntraFontFile("flash0:/font/jpn0.pgf", INTRAFONT_STRING_GBK | INTRAFONT_CACHE_LARGE);
    //oslIntraFontSetStyle(jpn0, 1.0f,0x0000000, 0xFFFFFFFF, INTRAFONT_ALIGN_LEFT);

    oslSetFont(jpn0);
    
    //print_pf_value();

    /* main loop */
    while (!osl_quit)
    {
        if (selecting) {
            
            if (debug_fumen != NULL) {
                strcpy(filename, debug_fumen);
                strcpy(root, tja_path);
                course_idx = 0;
            } else {
                song_select_init(tja_path); 
                if (! song_select_select(root, filename, &course_idx)) {
                    break;
                }
                song_select_destroy();
            }

            printf("selected %s%s: %d\n", root, filename, course_idx);
            selecting = FALSE;

            oslSetFont(jpn0);

            note = initFumen(root, filename);
            init_drawing(note);

            // record basic info for display
            fumen_offset = note->offset;

            // current time
            frame = 0;
            time_passed = note->offset - 480 / note->speed;
            printf("note_offset! %f\n", time_passed);
            if (time_passed < 0) {
                time_passed = -((int)(-time_passed / tpf) + 1) * tpf;
            } else {
                time_passed = 0;
            }      
            printf("time_passed init %f\n", time_passed);            
            tbeg = clock();
            continue;
        }       

        


        if (!music_started) {
            printf("time_passed %f\n", time_passed);
        } else {
            play_pos = (float)AalibGetPlayPosition(bgm_channel);
            delta = time_passed - play_pos;
            //printf("time_passed vs play_time, %f, %f, %f\n", time_passed, play_pos, delta);            
            if (delta > 100) {
                //break;
            }
        }
        if (!music_started && eps + time_passed >= 0) {
            music_started = 1;
            AalibPlay(bgm_channel);
            //AalibSetAutoloop(bgm_channel, TRUE);
            AalibSetVolume(bgm_channel, (AalibVolume){0.3, 0.3});            
            printf("when music start, time_passed = %f\n", time_passed);
            t1 = clock();
            printf("clock = %d\n", clock());
        }

        //if (t1 < 0 && AalibGetStatus(bgm_channel) != PSPAALIB_STATUS_STOPPED) {
        //    t1 = clock();
        //    printf("music thread move to play status at %f\n", t1);
       // }
        if (t1 > 0 && AalibGetStatus(bgm_channel) == PSPAALIB_STATUS_STOPPED) {
            printf("when music stopped %f\n", (float)AalibGetPlayPosition(bgm_channel));
            printf("clock = %f\n", (clock() - t1));            
            break;
        }
        pad = oslReadKeys();
        if (pad->pressed.left || pad->pressed.up) {
            refresh_taiko_flash(TAIKO_FLASH_BLUE, TAIKO_FLASH_LEFT);
            AalibRewind(PSPAALIB_CHANNEL_WAV_2);            
            AalibPlay(PSPAALIB_CHANNEL_WAV_2);
        }
        if (pad->pressed.circle || pad->pressed.triangle) {
            refresh_taiko_flash(TAIKO_FLASH_BLUE, TAIKO_FLASH_RIGHT);
            AalibRewind(PSPAALIB_CHANNEL_WAV_2);
            AalibPlay(PSPAALIB_CHANNEL_WAV_2);
        }
        if (pad->pressed.down || pad->pressed.right) {
            refresh_taiko_flash(TAIKO_FLASH_RED, TAIKO_FLASH_LEFT);
            AalibRewind(PSPAALIB_CHANNEL_WAV_1);            
            AalibPlay(PSPAALIB_CHANNEL_WAV_1);
        }
        if (pad->pressed.cross || pad->pressed.square) {
            refresh_taiko_flash(TAIKO_FLASH_RED, TAIKO_FLASH_RIGHT);
            AalibRewind(PSPAALIB_CHANNEL_WAV_1);                        
            AalibPlay(PSPAALIB_CHANNEL_WAV_1);            
        }


        if (pad->pressed.L) {
            offset -= 1;
        }

        if (pad->pressed.select) {
            auto_play = !auto_play;
        }
        if (pad->pressed.R) {
            offset += 1;
        }

        if (pad->pressed.start) {
            AalibStop(bgm_channel);    
            break;
        }

        oslStartDrawing();
        drawing();

        update_drawing(time_passed+offset, auto_play);

        // draw debug info
        oslPrintf_xy(200, 150, "offset=%.3f(%c%d)", fumen_offset+offset, \
                offset >= 0 ? '+' : '-', offset); 
        oslDrawStringf(200, 130, test1);
        
        oslEndDrawing();
        oslSyncFrameEx(0, 0, 0);
        //oslSwapBuffers();

        ++ frame;
        if (!music_started) {
            time_passed += tpf;
        } else {
            //time_passed += tpf;
            time_passed = (clock() - t1) / 1000.0;
            //time_passed = (float)AalibGetPlayPosition(bgm_channel);
            //printf("play pos get at %f\n", time_passed);
        }
        t2 = clock();
        //time_passed = (t2 - t1) / 1000.0;
        //printf("time_passed %f, clock %f, delta=%f\n", time_passed, (t2-t1)/1000.0, time_passed - (t2-t1)/1000.0);
    }

    tend = clock();
    printf("time = %fs, frame = %d, fps=%f\n", (tend-tbeg)/1000000.0,
            frame, frame / ((tend-tbeg)/1000000.0));
    oslEndGfx();
    oslIntraFontShutdown();
    oslQuit();
    return 0;
}
