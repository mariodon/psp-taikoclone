#include "helper/iniparser.h"
#include "load_texture.h"
#include "taiko_flash.h"
#include "const.h"
#include <time.h>
#include "tjaparser.h"
#include "aalib/pspaalib.h"

dictionary *tex_conf;

//preloaded textures
OSL_IMAGE *bg, *bg2;
OSL_IMAGE *taiko;
OSL_IMAGE *soulbar_empty;
OSL_IMAGE *soulbar_full;
OSL_IMAGE *taiko_flash[4];
OSL_IMAGE *note_tex[MAX_NOTE];
OSL_IMAGE *yellow_tex[3];
OSL_IMAGE *lyellow_tex[3];
OSL_IMAGE *balloon_tex[2];
OSL_IMAGE *barline_tex;

#define NOTE_NUM 8 
#define NOTE_DIST 30 
#define NOTE_SPEED 0.71 
int pos[NOTE_NUM];

note_t *note_list;

void init_drawing(note_t *note)
{
    int i;
    tex_conf = iniparser_load("config/texture.ini");
    bg = load_texture_config(tex_conf, "bg");
    taiko = load_texture_config(tex_conf, "taiko");

    // init taiko flash
    taiko_flash[0] = load_texture_config(tex_conf, "taiko_lred");
    taiko_flash[1] = load_texture_config(tex_conf, "taiko_lblue");
    taiko_flash[2] = load_texture_config(tex_conf, "taiko_rred");
    taiko_flash[3] = load_texture_config(tex_conf, "taiko_rblue");
    init_taiko_flash(taiko_flash);

    // init soul bar
    soulbar_empty = load_texture_config(tex_conf, "soulbar_empty");
    soulbar_full = load_texture_config(tex_conf, "soulbar_full");

    // init notes
    note_tex[NOTE_DON] = load_texture_config(tex_conf, "note_don");
    note_tex[NOTE_DON]->flags &= (~OSL_IMAGE_AUTOSTRIP);
    note_tex[NOTE_KATSU] = load_texture_config(tex_conf, "note_katsu");
    note_tex[NOTE_LDON] = load_texture_config(tex_conf, "note_ldon");
    note_tex[NOTE_LKATSU] = load_texture_config(tex_conf, "note_lkatsu");    
    
    //yellow texture
    yellow_tex[0] = load_texture_config(tex_conf, "note_yellowh");
    yellow_tex[1] = load_texture_config(tex_conf, "note_yellowb");
    yellow_tex[2] = load_texture_config(tex_conf, "note_yellowt");

    //big yellow texture
    lyellow_tex[0] = load_texture_config(tex_conf, "note_lyellowh");
    lyellow_tex[1] = load_texture_config(tex_conf, "note_lyellowb");
    lyellow_tex[2] = load_texture_config(tex_conf, "note_lyellowt");

    balloon_tex[0] = load_texture_config(tex_conf, "note_ballonh");
    balloon_tex[1] = load_texture_config(tex_conf, "note_ballont");    

    barline_tex = load_texture_config(tex_conf, "note_barline");
    note_list = note;
}


void draw_image_tiles(OSL_IMAGE *img, int start_x, int start_y, int end_x, int end_y)
{
    int x, y;
    for (y = start_y; y < end_y; y += img->sizeY) {
        for (x = start_x; x < end_x; x += img->sizeX) {
            oslDrawImageSimpleXY(img, x, y);
        }
    }
}

void drawing()
{
    int i, x;
    oslDrawImage(bg);
    oslDrawImage(soulbar_empty);

    //oslDrawImage(taiko_flash0);
     //   oslDrawImage(taiko_flash1);

    //        oslDrawImage(taiko_flash2);
      //          oslDrawImage(taiko_flash3);
 
}

void draw_yellow(OSL_IMAGE **, int, int, int);
int prev_disapper = -1;

void update_drawing(float play_pos, int auto_play)
{
    int i, x;
    note_t *p, *tmpp;
    note_t *prev;
    note_t *head;
    branch_start_t *pbs;
    float elapse_time;
    int id = 0;
    int played = FALSE;
    int offset = 0;
    int bx;
    static int last_yellow = 0;
    int cn = 0;
    static int hold_until_end = FALSE;

    //update taiko flash 
    play_pos += offset; 
    elapse_time = 1000.0 / 60;

    prev = NULL;
    head = note_list;
    for (p = note_list; p != NULL; prev = p, p = (note_t *)(p->next)) {
        cn ++;
        x = 106 + (p->offset - play_pos) * p->speed;        
        //build prev link
        if (p->prev == NULL) {
            p->prev = prev;
        }        
        // handle branch
        if (p->type == NOTE_BRANCH_START) {
            p->type = NOTE_DUMMY;
            printf("\n[Branch %d]\n", id);
            pbs = (branch_start_t *)p;
            if (id == 0) {
                tmpp = pbs->next;
                pbs->next = pbs->fumen_e;
                pbs->fumen_e_ed->next = tmpp;
            } else if (id == 1) {
                tmpp = pbs->next;
                pbs->next = pbs->fumen_n;
                pbs->fumen_n_ed->next = tmpp;                
            } else if (id == 2) {
                tmpp = pbs->next;
                pbs->next = pbs->fumen_m;
                pbs->fumen_m_ed->next = tmpp;
            }
        } else if (p->type == NOTE_YELLOW || p->type == NOTE_LYELLOW || p->type == NOTE_BALLOON || p->type == NOTE_PHOTATO) {
            hold_until_end = TRUE;
        } else if (p->type == NOTE_END && x <= 490) {
            hold_until_end = FALSE;
        }       
        // if out of screen, then we get all the note we need for this update.
        // special case: lasting note
        if (p->next == NULL) {
            break;
        }

        if (cn < 10) {
            continue;
        }

        if (x > 490 && (p->type == NOTE_END || ! hold_until_end)) {
            break;
        }

    }

    printf("cn=%d\n", cn);
    for (; p != NULL && p != head->prev; p = (note_t *)p->prev) {
        x = 106 + (p->offset - play_pos) * p->speed;

        if (p->type == NOTE_DON || p->type == NOTE_KATSU || p->type == NOTE_LDON || p->type == NOTE_LKATSU) {
            oslDrawImageSimpleXY(note_tex[p->type], x, 105);            
        } else if (p->type == NOTE_END) {
            note_t *start_note = ((end_t *)p)->start_note;
            int start_x = 106 + (start_note->offset - play_pos) * start_note->speed;
            if (start_note->type == NOTE_YELLOW) {
                draw_yellow(yellow_tex, start_x, x, 105);
            } else if (start_note->type == NOTE_LYELLOW) {
                draw_yellow(lyellow_tex, start_x, x, 105);
            }
        } else if (p->type == NOTE_BALLOON) {
            draw_balloon(balloon_tex, x, 105);
        } else if (p->type == NOTE_BARLINE) {
            oslDrawImageSimpleXY(barline_tex, x, 105);
        }


        if (x < 106 && (p->type == NOTE_YELLOW || p->type == NOTE_LYELLOW)) {
            last_yellow = 0;
        }
    
        if (p->type == NOTE_END && x >= 106) {
            note_t *start_note = ((end_t *)p)->start_note;
            int start_x = 106 + (start_note->offset - play_pos) * start_note->speed;

            if (auto_play && !played && (start_note->type == NOTE_YELLOW || start_note->type == NOTE_LYELLOW) && start_x < 106) {
                if (!last_yellow) {
                    AalibRewind(PSPAALIB_CHANNEL_WAV_1);            
                    AalibPlay(PSPAALIB_CHANNEL_WAV_1);                
                    played = TRUE;                
                }
                last_yellow = (last_yellow + 1) % 3;
            }
        } else if (x < 106) {


            if (auto_play && ! played) {
                if (p->type == NOTE_DON || p->type == NOTE_LDON) {
                    AalibRewind(PSPAALIB_CHANNEL_WAV_1);            
                    AalibPlay(PSPAALIB_CHANNEL_WAV_1);                
                    played = TRUE;
                } else if (p->type == NOTE_KATSU || p->type == NOTE_LKATSU) {
                    AalibRewind(PSPAALIB_CHANNEL_WAV_2);            
                    AalibPlay(PSPAALIB_CHANNEL_WAV_2);                
                    played = TRUE;
                }
            }
            note_list = (note_t *)(p->next);
        }        
    }

    oslDrawImage(taiko);    
    update_taiko_flash(elapse_time);    
}

void drawing_notes()
{
}

void draw_yellow(OSL_IMAGE **textures, int x1, int x2, int y)
{
    int draw_x1 = (x1 < 0 ? 0 : x1);
    int draw_x2 = (x2 >= 480 ? 479 : x2); 
    int draw_length =  draw_x2 - draw_x1 + 1;
    textures[1]->stretchX = draw_length;
    oslDrawImageXY(textures[1], draw_x1, y);
    if (x2-textures[2]->centerX < 480 && x2-textures[2]->centerX+textures[2]->sizeX >= 0) {
        oslDrawImageSimpleXY(textures[2], x2, y);
    }
    if (x1-textures[0]->centerX < 480 && x1-textures[0]->centerX+textures[2]->sizeX >= 0) {    
        oslDrawImageSimpleXY(textures[0], x1, y);
    }
}

void draw_balloon(OSL_IMAGE **textures, int x, int y)
{
    oslDrawImageSimpleXY(textures[0], x, y);
    oslDrawImageSimpleXY(textures[1], x, y);
}
