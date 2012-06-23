#include <math.h>
#include "helper/iniparser.h"
#include "load_texture.h"
#include "taiko_flash.h"
#include "const.h"
#include <time.h>
#include "tjaparser.h"
#include "note.h"
#include "aalib/pspaalib.h"

static judge_level_t judge = {217, 150, 50};
static dictionary *tex_conf;

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
    note_tex[NOTE_KATSU] = load_texture_config(tex_conf, "note_katsu");
    note_tex[NOTE_LDON] = load_texture_config(tex_conf, "note_ldon");
    note_tex[NOTE_LKATSU] = load_texture_config(tex_conf, "note_lkatsu");    
    note_tex[NOTE_BARLINE] = load_texture_config(tex_conf, "note_barline");
    
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
}

void draw_yellow(OSL_IMAGE **, int, int, int);
int prev_disapper = -1;


int oslImageNeedDraw(OSL_IMAGE *img, int x)
{
    return (x - img->centerX < SCREEN_WIDTH) && (x + img->sizeX - img->centerX >= 0);
}

int need_update(note_t *p, float play_pos)
{
    int x, x2;


    x = 106 + (p->offset - play_pos) * p->speed;

    switch (p->type) {
        case NOTE_DON:
        case NOTE_KATSU:
        case NOTE_LDON:
        case NOTE_LKATSU:
        case NOTE_BARLINE:
            return oslImageNeedDraw(note_tex[p->type], x);

        case NOTE_YELLOW:
            x2 = 106 + (((yellow_t *)p)->offset2 - play_pos) * p->speed;
            return !(x - yellow_tex[0]->centerX >= SCREEN_WIDTH || x2 + yellow_tex[2]->sizeX - yellow_tex[2]->centerX < 0);

        case NOTE_LYELLOW:
            x2 = 106 + (((yellow_t *)p)->offset2 - play_pos) * p->speed;
            return !(x - lyellow_tex[0]->centerX >= SCREEN_WIDTH || x2 + lyellow_tex[2]->sizeX - lyellow_tex[2]->centerX < 0);

        case NOTE_BALLOON: /* derive ? */
            x2 = 106 + (((yellow_t *)p)->offset2 - play_pos) * p->speed;
            return !(x - balloon_tex[0]->centerX >= SCREEN_WIDTH || x2 + balloon_tex[1]->sizeX - balloon_tex[1]->centerX < 0);

        default:
            return (x < SCREEN_WIDTH && x >= 0);
    }
}

void update_note(note_t *p, float play_pos)
{
    int x, x2;

    x = 106 + (p->offset - play_pos) * p->speed;

    switch (p->type) {
        case NOTE_DON:
        case NOTE_KATSU:
        case NOTE_LDON:
        case NOTE_LKATSU:
        case NOTE_BARLINE:
            oslDrawImageSimpleXY(note_tex[p->type], x, 105);
            return;

        case NOTE_YELLOW:
            x2 = 106 + (((yellow_t *)p)->offset2 - play_pos) * p->speed;
            draw_yellow(yellow_tex, x, x2, 105);
            return;

        case NOTE_LYELLOW:
            x2 = 106 + (((yellow_t *)p)->offset2 - play_pos) * p->speed;
            draw_yellow(lyellow_tex, x, x2, 105);
            return;

        case NOTE_BALLOON:
            x2 = 106 + (((yellow_t *)p)->offset2 - play_pos) * p->speed;
            if (x >= 106) {
                draw_balloon(balloon_tex, x, 105);
            } else if (x2 >= 106) {
                draw_balloon(balloon_tex, 106, 105);
            } else {
                draw_balloon(balloon_tex, x2, 105);
            }
            return;

        default:
            return;
    }
}

void update_drawing(float play_pos, int auto_play, OSL_CONTROLLER *pad)
{
    int i, x, x2, t_delta;
    note_t *p, *tmpp;
    note_t *prev;
    static note_t *head = NULL, *tail = NULL, *cur_hit_obj=NULL;
    branch_start_t *pbs;
    float elapse_time;
    int id = 0;
    int played = FALSE;
    int offset = 0;
    int bx;
    static int last_yellow = 0;
    int cn = 0;

    int hit_off = FALSE;
    int hit_over = FALSE;
    int hit_ok = FALSE;

    play_input_t input;

    //update taiko flash 
    play_pos += offset; 
    elapse_time = 1000.0 / 60;

    /* [head, tail], init update section */
    if (head == NULL) {
        head = tail = note_list;
        head->prev = tail->prev = NULL;
    }

    //printf("before move tail forward, (%p, %p)\n", head, tail);
    /* start from tail to see if we need update more notes. */
    for (p = tail, prev = tail->prev; p != NULL;  prev = p, p = (note_t *)(p->next)) {

        /* special~ influence note flow */
        if (p->type == NOTE_BRANCH_START) {
            if (need_update(p, play_pos)) {
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
                tail = (note_t *)p;
                cn = 0;
            } else {
                break;
            }
        }

        if (need_update(p, play_pos)) {
            tail = (note_t *)p;
            cn = 0;
        } else {
            cn ++;
            if (cn >= 5) {
                break;
            }
        }

        // work out reverse link
        p->prev = prev;

        //find the first hit obj
        if (cur_hit_obj == NULL && (p->type == NOTE_DON \
            || p->type == NOTE_LDON || p->type == NOTE_KATSU \
            || p->type == NOTE_LKATSU || p->type == NOTE_YELLOW \
            || p->type == NOTE_LYELLOW || p->type == NOTE_BALLOON)) {
            cur_hit_obj = p;
        }
    }

    memset(&input, 0, sizeof(input));
    if (! auto_play) {
        input.left_don = (pad->pressed.right || pad->pressed.down);
        input.right_don = (pad->pressed.square || pad->pressed.cross);
        input.left_katsu = (pad->pressed.up || pad->pressed.left);
        input.right_katsu = (pad->pressed.triangle || pad->pressed.circle);
        input.big_don = (input.left_don && input.right_don);
        input.big_katsu = (input.left_katsu && input.right_katsu);
    }

    /* generate auto play input */
    if (cur_hit_obj != NULL) {
        t_delta = cur_hit_obj->offset - play_pos;
        x = 106 + t_delta * cur_hit_obj->speed;        
        switch(cur_hit_obj->type) {
            case NOTE_DON:
            case NOTE_LDON:
            case NOTE_KATSU:
            case NOTE_LKATSU:
                hit_over = (hit_over || -t_delta > judge.bad);
                if (hit_over) {
                    break;
                }
                if (auto_play && x <= 106) {
                    if (cur_hit_obj->type == NOTE_DON) {
                        input.left_don = 1;
                    } 
                    if (cur_hit_obj->type == NOTE_LDON) {
                        input.left_don = input.right_don = 1;
                    }
                    if (cur_hit_obj->type == NOTE_KATSU) {
                        input.left_katsu = 1;
                    }
                    if (cur_hit_obj->type == NOTE_LKATSU) {
                        input.right_katsu = input.left_katsu = 1;
                    }                    
                }

                if (((input.left_don || input.right_don) && (cur_hit_obj->type == NOTE_DON || cur_hit_obj->type == NOTE_LDON)) \
                       || ((input.left_katsu || input.right_katsu) && (cur_hit_obj->type == NOTE_KATSU || cur_hit_obj->type == NOTE_LKATSU))) {
                    if (abs(t_delta) <= judge.bad) {
                        hit_ok = TRUE;
                        hit_off = TRUE;
                        hit_over = TRUE;
                    }
                }
                break;
            
            case NOTE_YELLOW:
            case NOTE_LYELLOW:
                x2 = 106 + (((yellow_t *)cur_hit_obj)->offset2 - play_pos) * head->speed;        
                if (auto_play && x <= 106 && x2 >= 106) {
                    if (!last_yellow) {
                        input.left_don = 1;
                    }
                    last_yellow = (last_yellow + 1) % 3;
                }

                if (x <= 106 && x2 >= 106 && (input.left_don || input.right_don || input.right_katsu || input.left_katsu)) {
                    hit_ok = TRUE;
                } else if (x2 < 106) {
                    hit_over = TRUE;
                }
                break;
            case NOTE_BALLOON:
                x2 = 106 + (((yellow_t *)cur_hit_obj)->offset2 - play_pos) * head->speed;        
                if (auto_play && x<= 106 && x2 >= 106) {
                    if (!last_yellow) {
                        input.left_don = 1;
                    }
                    last_yellow = (last_yellow + 1) % 3;
                }

                if (x <= 106 && x2 >= 106 && (input.left_don || input.right_don)) {
                    (((balloon_t *)cur_hit_obj)->hit_count) -= input.left_don + input.right_don;
                    hit_ok = TRUE;
                    if ((((balloon_t *)cur_hit_obj)->hit_count) <= 0) {
                        hit_over = hit_off = TRUE;
                    }
                } else if (x2 < 106) {
                    hit_over = TRUE;
                }
                break;
            default:
                break;
        }
    }

    if (input.left_don || input.right_don) {
        AalibRewind(PSPAALIB_CHANNEL_WAV_1);
        AalibPlay(PSPAALIB_CHANNEL_WAV_1);
    }
    if (input.left_katsu || input.right_katsu) {
        AalibRewind(PSPAALIB_CHANNEL_WAV_2);
        AalibPlay(PSPAALIB_CHANNEL_WAV_2);
    }

    if (hit_off) {
        if (cur_hit_obj->type == NOTE_BALLOON) {
            AalibRewind(PSPAALIB_CHANNEL_WAV_3);
            AalibPlay(PSPAALIB_CHANNEL_WAV_3);
        }
        cur_hit_obj->type = NOTE_DUMMY;
    }

    if (hit_over) {
        for (p = cur_hit_obj->next; tail == NULL ? p != tail : p != tail->next; p = (note_t *)p->next) {
            if (p->type == NOTE_DON \
                || p->type == NOTE_LDON || p->type == NOTE_KATSU \
                || p->type == NOTE_LKATSU || p->type == NOTE_YELLOW \
                || p->type == NOTE_LYELLOW || p->type == NOTE_BALLOON) {
                cur_hit_obj = p;
                hit_over = FALSE;
                break;
            }
        }
        // not find
        if (hit_over) {
            cur_hit_obj = NULL;
        }
    }

    //printf("after move tail forward, (%p, %p)\n", head, tail);    
    /* start from head to see if anyone doesn't need update any more*/
    for (p = head; p != tail; p = (note_t *)(p->next)) {

        if (!need_update(p, play_pos)) {
            head = p->next;
        } else {
            break;
        }
    }

    //printf("after move head forward, (%p, %p)\n", head, tail);

    /* start from tail to head to do a full update & render */
    for (p = tail; p != head->prev; p = (note_t *)p->prev) {
        if (need_update(p, play_pos)) {
            update_note(p, play_pos);
        }
        //printf("update %d ", p->type);
    }

    //printf("\n");
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
