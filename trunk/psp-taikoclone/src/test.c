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
#include "note.h"

PSP_MODULE_INFO("Taiko No Tatsujin", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(-64*15); 

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

void print_str_as_hex(char *str)
{
    char *p;
    puts("");
    for (p = str; *p != '\0'; ++ p) {
        printf("\\x%x", (unsigned char)(*p));
    }
    puts("");
    return;
}

/*
 * preload sound. can be used in menu and game
 * */
void init_hitsound() {
    cccUCS2 sound_file[MAX_FILENAME_UCS2];
    int len;

    len = cccGBKtoUCS2(sound_file, MAX_FILENAME_UCS2-1, "snd/dong.wav");
    sound_file[len] = 0;
    AalibLoad(sound_file, PSPAALIB_CHANNEL_WAV_1, TRUE, FALSE);
    len = cccGBKtoUCS2(sound_file, MAX_FILENAME_UCS2-1, "snd/ka.wav");
    sound_file[len] = 0;    
    AalibLoad(sound_file, PSPAALIB_CHANNEL_WAV_2, TRUE, FALSE);    
    len = cccGBKtoUCS2(sound_file, MAX_FILENAME_UCS2-1, "snd/balloon.wav");
    sound_file[len] = 0;    
    AalibLoad(sound_file, PSPAALIB_CHANNEL_WAV_3, TRUE, FALSE);
}

/*
 * load a bgm.
 *
 * @return channel id on success, <0 on error */
int load_bgm(cccUCS2 *wave_file, char *ext) {
    int bgm_channel;

    if (strcmp(".mp3", ext) == 0) {
        bgm_channel = PSPAALIB_CHANNEL_SCEMP3_1;
    } else if (strcmp(".ogg", ext) == 0) {
        bgm_channel = PSPAALIB_CHANNEL_OGG_1;
    } else if (strcmp(".wav", ext) == 0) {
        bgm_channel = PSPAALIB_CHANNEL_WAV_4;
    } else {
        return -1;
    }

            int len;
        char bytes[MAX_FILENAME];
        len=cccUCS2toGBK(bytes, MAX_FILENAME-1, wave_file);
        bytes[len] = '\0';
    printf("Loading music ... %s\n", bytes);
    if (AalibLoad(wave_file, bgm_channel, FALSE, TRUE)) {

        oslFatalError("loading music %s failed\n", bytes);
        return -1;            
    }
    printf("loading music ok!\n");

    return bgm_channel;
}

void unload_bgm(int channel) {
    AalibUnload(channel);
}

bool parse_fumen(cccUCS2 *tja_file, int course_idx, tja_header_t *header,
	note_t **note_N, note_t **note_E, note_t **note_M)
{
    if (tjaparser_load(tja_file) == 0) {
        oslFatalError("can't open tjafile");
        return NULL;
    }
    if (tjaparser_read_tja_header(header) == 0) {
        oslFatalError("can't read tja header");        
        return NULL;
    }

    printf("parsing fumen 0\n");
    if (tjaparser_parse_course(course_idx, note_N, note_E, note_M) == 0) {
        oslFatalError("parsing fumen %s failed\n", tja_file);        
    }
    printf("parsing fumen ok! %p\n", *note_N);

    tjaparser_unload();
    
    return TRUE;
}

int main(int argc, char *argv[])
{
    int bgm_channel;
    //OSL_IMAGE *bg;
    int frame = 0;

    int fps = 60;
    float tpf = 1000.0 / fps;
    
    clock_t tbeg, tend;
    clock_t t1 = -1, t2;

    int music_started = 0;

    note_t *note_N, *note_E, *note_M;
    float time_passed;

    OSL_CONTROLLER *pad;

    float eps = 1e-4;
    float delta = 0;
    float play_pos = 0;
    int selecting = TRUE; 

    //sceIoChdir("ms0:/PSP/GAME/TAIKOC");
    /* selected fumen info */
    cccUCS2 tja_file[MAX_FILENAME_UCS2];
    cccUCS2 wave_file[MAX_FILENAME_UCS2];
    int course_idx;

    /* real time info display */
    float fumen_offset;
    int offset = 5; /* offset fix up */
    int auto_play = TRUE;

    tja_header_t tja_header;

    /* debug process */
    //char *debug_fumen = "\x82\xed\x82\xf1\x82\xc9\x82\xe1\x81[\x83\x8f\x81[\x83\x8b\x83h.tja";
    char *debug_fumen = NULL; //file_list[file_list_len-2];    

    bool fumen_over;
    bool music_over;
    //sceIoChdir("ms0:/PSP/GAME/TAIKOC");

    scePowerSetCpuClockFrequency(222);

    oslInit(0);
    oslInitGfx(OSL_PF_8888, 1);
    oslInitConsole();    
    oslIntraFontInit(INTRAFONT_CACHE_LARGE);

    AalibInit();

    //preload images needed for drawing
    init_drawing();
    init_hitsound();


    OSL_FONT *jpn0 = oslLoadIntraFontFile("flash0:/font/jpn0.pgf", INTRAFONT_STRING_GBK | INTRAFONT_CACHE_LARGE);
    //oslIntraFontSetStyle(jpn0, 1.0f,0x0000000, 0xFFFFFFFF, INTRAFONT_ALIGN_LEFT);

    oslSetFont(jpn0);
    
    //print_pf_value();

    /* main loop */
    while (!osl_quit)
    {
        if (selecting) {
            
            if (FALSE && debug_fumen != NULL) {
            } else {
                song_select_init(tja_path); 
                if (!song_select_select(tja_file, wave_file, &course_idx)) {
                    break;
                }
                song_select_destroy();

            }
            selecting = FALSE;
            music_started = FALSE;

            oslSetFont(jpn0);

        	parse_fumen(tja_file, course_idx, &tja_header,
            	&note_N, &note_E, &note_M);

			if (note_N == NULL) {
				oslFatalError("can't parse fumen!");
			}
			
            printf("selected %s\n", tja_header.title);
            //TODO: fix ext problem
            bgm_channel = load_bgm(wave_file, (tja_header.wave+strlen(tja_header.wave)-4));
            if (bgm_channel < 0) {
                oslFatalError("can't load bgm!");
            }
            note_init(note_N, note_E, note_M);

            fumen_over = music_over = FALSE;
            // record basic info for display
            fumen_offset = note_N->offset;

            // current time
            frame = 0;
            time_passed = note_N->offset - 480 / note_N->speed;
            printf("note_offset! %f\n", time_passed);
            if (time_passed < 0) {
                time_passed = -((int)(-time_passed / tpf) + 2) * tpf;
            } else {
                time_passed = -2 * tpf;
            }      
            printf("time_passed init %f\n", time_passed);            
            tbeg = clock();
            continue;
        }       

        if (!music_started && eps + time_passed >= 0) {
            music_started = 1;
            AalibPlay(bgm_channel);
            //AalibSetAutoloop(bgm_channel, TRUE);
            AalibSetVolume(bgm_channel, (AalibVolume){1.0, 1.0});           
            printf("when music start, time_passed = %f\n", time_passed);
            t1 = clock();
            printf("clock = %d\n", clock());
        }

        //if (t1 < 0 && AalibGetStatus(bgm_channel) != PSPAALIB_STATUS_STOPPED) {
        //    t1 = clock();
        //    printf("music thread move to play status at %f\n", t1);
       // }
        if (!music_over && t1 > 0 && AalibGetStatus(bgm_channel) == PSPAALIB_STATUS_STOPPED) {
            music_over = TRUE;
        }
        pad = oslReadKeys();

        if (pad->pressed.L) {
            offset -= 1;
        }

        if (pad->pressed.select) {
            auto_play = !auto_play;
        }
        if (pad->pressed.R) {
            offset += 1;
        }

        if (pad->pressed.start || (music_over && fumen_over)) {
            //TODO: disable destroy note atm.
            note_destroy();
            printf("note_destroy!\n");
            AalibUnload(bgm_channel);
            selecting = TRUE;
            continue;
        }

        oslStartDrawing();
        oslClearScreen(RGB(0,0,0));
        drawing();

        fumen_over = !note_update(time_passed+offset, auto_play, pad);

        drawing_after_note();
        // draw debug info
        oslPrintf_xy(200, 150, "offset=%.3f(%c%d)", fumen_offset+offset, \
                offset > 0 ? '+' : '\0', offset); 
        //TODO:oslDrawStringf(200, 130, test1);
        
        oslEndDrawing();
        oslSyncFrameEx(0, 0, 0);
        //oslSwapBuffers();

        ++ frame;
        //if (1) {
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
