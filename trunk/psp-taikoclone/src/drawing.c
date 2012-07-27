#include <math.h>
#include "helper/iniparser.h"
#include "load_texture.h"
#include "const.h"
#include <time.h>
#include "tjaparser.h"
#include "note.h"
#include "aalib/pspaalib.h"

//------------------------------------------------------------------------------
// includes and variables for test code
//------------------------------------------------------------------------------
#if TEST_ANIMATION

#include "animation.h"
static anime_t *anime_note_fly;

#endif

//------------------------------------------------------------------------------
// global variables
//------------------------------------------------------------------------------
static dictionary *tex_conf;

//preloaded textures
static bool is_texture_preloaded = FALSE;
OSL_IMAGE *bg, *note_bg, *hit_circle, *donchan;
OSL_IMAGE *taiko;
OSL_IMAGE *soulbar_empty;
OSL_IMAGE *soulbar_full;
OSL_IMAGE *taiko_flash[4];

static OSL_IMAGE *note_tex[MAX_NOTE][4];

float format_t(float t)
{
    if (t < 0)
        t += 1.0;
    else if (t > 1)
        t -= 1.0;
    return t;
}

float calc_color_comp(float t, float p, float q)
{
    if (t < 1.0 / 6)
        return p + ((q - p) * 6.0 * t);
    else if (t < 0.5)
        return q;
    else if (t < 2.0 / 3)
        return p + ((q - p) * 6.0 * (2.0 / 3 - t));
    else
        return p;
}

void hsl_to_rgb(float h, float s, float l, float *r, float *g, float *b)
{
    float p, q, hk;
    float tr, tg, tb;

    if (s == 0) {
        *r = *g = *b = l;
    } else {
        if (l < 0.5)
            q = l * (1 + s);
        else
            q = l + s - l * s;
        p = 2 * l - q;
        hk = h / 360.0;
        tr = format_t(hk + 1.0 / 3);
        tg = format_t(hk);
        tb = format_t(hk - 1.0 / 3);
        *r = calc_color_comp(tr, p, q);
        *g = calc_color_comp(tg, p, q);
        *b = calc_color_comp(tb, p, q);
    }
}

void colorize_palette(OSL_IMAGE *img)
{
    OSL_PALETTE *palette = img->palette;
    int i;
    int r, g, b, a;
    float rf, gf, bf;
    float lum, h, s, l;
    float lightness;
    
    h = 63;
    s = 0.5;
    lightness = 52;

    for (i = 0; i < palette->nElements; ++ i) {
		r = (((u32*)palette->data)[i]) & 0xff;
		g = ((((u32*)palette->data)[i]) >> 8) & 0xff;
		b = ((((u32*)palette->data)[i]) >> 16) & 0xff;
    	a = ((((u32*)palette->data)[i]) >> 24) & 0xff;        
        lum = r * 0.2126 + g * 0.7152 + b * 0.0722;
        if (lightness < 0)
            lum *= (lightness + 100) / 100.0;
        else
            lum = lum + lightness * (255 - lum) / 100.0;
        l = lum / 255.0;

        hsl_to_rgb(h, s, l, &rf, &gf, &bf);

        r = (int)(255 * rf);
        g = (int)(255 * gf);
        b = (int)(255 * bf);
        ((u32*)palette->data)[i] = (a << 24) + (b << 16) + (g << 8) + r;
    }
}
/*
 * preload textures.
 * */
void init_drawing()
{

    if (is_texture_preloaded) {
        return;
    }

    tex_conf = iniparser_load("config/texture.ini");
    bg = load_texture_config(tex_conf, "bg");

    donchan = load_texture_config(tex_conf, "donchan");
    colorize_palette(donchan);

    note_bg = load_texture_config(tex_conf, "note_bg");
    hit_circle = load_texture_config(tex_conf, "hit_circle");
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
    memset(note_tex, -1, sizeof(note_tex)); // set NULL
    note_tex[NOTE_DON][0] = load_texture_config(tex_conf, "note_don");
    note_tex[NOTE_KATSU][0] = load_texture_config(tex_conf, "note_katsu");
    note_tex[NOTE_LDON][0] = load_texture_config(tex_conf, "note_ldon");
    note_tex[NOTE_LKATSU][0] = load_texture_config(tex_conf, "note_lkatsu");    
    note_tex[NOTE_BARLINE][0] = load_texture_config(tex_conf, "note_barline");
    note_tex[NOTE_BARLINE][1] = load_texture_config(tex_conf, "note_barline_yellow");

    //==================debug======================
    printf("load yellow barline texture %p\n", note_tex[NOTE_BARLINE][1]);

    note_tex[NOTE_YELLOW][0] = load_texture_config(tex_conf, "note_yellowh");
    note_tex[NOTE_YELLOW][1] = load_texture_config(tex_conf, "note_yellowb");
    note_tex[NOTE_YELLOW][2] = load_texture_config(tex_conf, "note_yellowt");

    note_tex[NOTE_LYELLOW][0] = load_texture_config(tex_conf, "note_lyellowh");
    note_tex[NOTE_LYELLOW][1] = load_texture_config(tex_conf, "note_lyellowb");
    note_tex[NOTE_LYELLOW][2] = load_texture_config(tex_conf, "note_lyellowt");

    note_tex[NOTE_BALLOON][0] = load_texture_config(tex_conf, "note_ballonh");
    note_tex[NOTE_BALLOON][1] = load_texture_config(tex_conf, "note_ballont");        

    is_texture_preloaded = TRUE;
	
	#if TEST_ANIMATION
    anime_note_fly = anime_from_file("ani/note_fly_don.ani");
	
	// create and set up animation here
	#endif
}

void draw_bg_upper(OSL_IMAGE *img)
{
    int i;
    for (i = 0; i * img->sizeX < SCREEN_WIDTH; ++ i) {
        oslDrawImageSimpleXY(img, i * img->sizeX, 0);
    }
}

void draw_bg_note(OSL_IMAGE *img)
{
    img->stretchX = SCREEN_WIDTH;
    oslDrawImageXY(img, 0, 78);
}

void draw_note(note_t *note, int x, int y)
{
    int x2;
    barline_t *note_barline;

    switch (note->type) {
        case NOTE_DON:
        case NOTE_KATSU:
        case NOTE_LDON:
        case NOTE_LKATSU:
            oslDrawImageSimpleXY(note_tex[note->type][0], x, y);
            break;
        case NOTE_BARLINE:
            note_barline = (barline_t *)note;
            oslDrawImageSimpleXY(note_tex[note->type][note_barline->is_branch], x, y);            
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
    draw_bg_upper(bg);        
    oslDrawImageSimple(donchan);
    draw_bg_note(note_bg);

    oslDrawImageSimpleXY(hit_circle, NOTE_FIT_X, NOTE_Y);
    //
//    oslDrawImage(soulbar_empty);

	#if TEST_ANIMATION
	// animate and draw animation here
	#endif
}

void drawing_after_note()
{
    oslDrawImageSimple(taiko);
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
