#include <math.h>
#include "helper/iniparser.h"
#include "load_texture.h"
#include "taiko_flash.h"
#include "const.h"
#include <time.h>
#include "tjaparser.h"
#include "note.h"
#include "aalib/pspaalib.h"

static dictionary *tex_conf;

//preloaded textures
static bool is_texture_preloaded = FALSE;
OSL_IMAGE *bg, *bg2;
OSL_IMAGE *taiko;
OSL_IMAGE *soulbar_empty;
OSL_IMAGE *soulbar_full;
OSL_IMAGE *taiko_flash[4];

static OSL_IMAGE *note_tex[MAX_NOTE][4];

/*
 * preload textures.
 * */
void init_drawing()
{
    int i;
    OSL_IMAGE *p;

    if (is_texture_preloaded) {
        return;
    }

    tex_conf = iniparser_load("config/texture.ini");
    bg = load_texture_config(tex_conf, "bg");
    taiko = load_texture_config(tex_conf, "taiko");

    // init taiko flash
    taiko_flash[0] = load_texture_config(tex_conf, "taiko_lred");
    taiko_flash[1] = load_texture_config(tex_conf, "taiko_lblue");
    taiko_flash[2] = load_texture_config(tex_conf, "taiko_rred");
    taiko_flash[3] = load_texture_config(tex_conf, "taiko_rblue");

    // init soul bar
    soulbar_empty = load_texture_config(tex_conf, "soulbar_empty");
    soulbar_full = load_texture_config(tex_conf, "soulbar_full");

    // init notes
    memset(note_tex, NULL, sizeof(note_tex));
    note_tex[NOTE_DON][0] = load_texture_config(tex_conf, "note_don");
    note_tex[NOTE_KATSU][0] = load_texture_config(tex_conf, "note_katsu");
    note_tex[NOTE_LDON][0] = load_texture_config(tex_conf, "note_ldon");
    note_tex[NOTE_LKATSU][0] = load_texture_config(tex_conf, "note_lkatsu");    
    note_tex[NOTE_BARLINE][0] = load_texture_config(tex_conf, "note_barline");

    note_tex[NOTE_YELLOW][0] = load_texture_config(tex_conf, "note_yellowh");
    note_tex[NOTE_YELLOW][1] = load_texture_config(tex_conf, "note_yellowb");
    note_tex[NOTE_YELLOW][2] = load_texture_config(tex_conf, "note_yellowt");

    note_tex[NOTE_LYELLOW][0] = load_texture_config(tex_conf, "note_lyellowh");
    note_tex[NOTE_LYELLOW][1] = load_texture_config(tex_conf, "note_lyellowb");
    note_tex[NOTE_LYELLOW][2] = load_texture_config(tex_conf, "note_lyellowt");

    note_tex[NOTE_BALLOON][0] = load_texture_config(tex_conf, "note_ballonh");
    note_tex[NOTE_BALLOON][1] = load_texture_config(tex_conf, "note_ballont");        

    is_texture_preloaded = TRUE;

}

void draw_note(note_t *note, int x, int y)
{
    int x2;

    switch (note->type) {
        case NOTE_DON:
        case NOTE_KATSU:
        case NOTE_LDON:
        case NOTE_LKATSU:
        case NOTE_BARLINE:
            oslDrawImageSimpleXY(note_tex[note->type][0], x, y);
            break;
        case NOTE_BALLOON:
            draw_balloon(note_tex[note->type], x, y);
            break;
        case NOTE_YELLOW:
        case NOTE_LYELLOW:
            x2 = x + (((yellow_t *)note)->offset2 - note->offset) * note->speed;
            draw_yellow(note_tex[note->type], x, x2, y);
            break;
        default:
            break;
    }
}

void get_note_left_right(note_t *note, int x, int *left, int *right)
{
    int x2;
    switch (note->type) {
        case NOTE_DON:
        case NOTE_KATSU:
        case NOTE_LDON:
        case NOTE_LKATSU:
        case NOTE_BARLINE:
            *left = x - note_tex[note->type][0]->centerX;
            *right = x + note_tex[note->type][0]->sizeX - note_tex[note->type][0]->centerX;
            break;
        case NOTE_BALLOON:
            *left = x - note_tex[note->type][0]->centerX;
            *right = x + note_tex[note->type][1]->sizeX - note_tex[note->type][1]->centerX;
            break;
        case NOTE_YELLOW:
        case NOTE_LYELLOW:
            x2 = x + (((yellow_t *)note)->offset2 - note->offset) * note->speed;
            *left = x - note_tex[note->type][0]->centerX;
            *right = x2 + note_tex[note->type][2]->sizeX - note_tex[note->type][2]->centerX;
            break;
        default:
            *left = *right = x;
            break;
    }
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
    oslDrawImage(bg);
    oslDrawImage(soulbar_empty);
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
