#include <math.h>
#include <time.h>

#include "const.h"
#include "note.h"
#include "animation.h"
#include "frame.h"
#include "frame_factory.h"

//------------------------------------------------------------------------------
// load all resources needed to render the scene
//------------------------------------------------------------------------------
static anime_t *a_bg_upper;             /* bg upper. */
static frame_t *f_bg_note_normal;       /* bg of normal note sheet */
static frame_t *f_bg_note_expert;       /* bg of expert note sheet */
static frame_t *f_bg_note_master;       /* bg of master note sheet */
static frame_t *f_bg_note_ggt;          /* bg of gogotime note sheet */

/* all note in scroll field.*/
static frame_t *f_don;
static frame_t *f_katsu;
static frame_t *f_ldon;
static frame_t *f_lkatsu;
static frame_t *f_balloon_head;
static frame_t *f_balloon_tail;
static frame_t *f_renda_head;
static frame_t *f_renda_body;
static frame_t *f_renda_tail;
static frame_t *f_lrenda_head;
static frame_t *f_lrenda_body;
static frame_t *f_lrenda_tail;
static frame_t *f_barline;
static frame_t *f_barline_yellow;

/* the taiko */
static frame_t *f_taiko;
static anime_t *a_taiko_lred;
static anime_t *a_taiko_lblue;
static anime_t *a_taiko_rred;
static anime_t *a_taiko_rblue;
static anime_t *a_taiko_flower;
static frame_t *f_hit_circle;

/* soul bar */
static frame_t *f_soulbar_bg;
static frame_t *f_soulbar_shadow;
static frame_t *f_soulbar_frame;
static frame_t *f_soul_icon;
static frame_t *f_soul_left;
static frame_t *f_soul_right;

void drawing_init()
{
	a_bg_upper = animation_create_from_file("ani/bg_upper.ani");
}

/* accept a time step, control pad, and some game status.
 * and will draw the game scene.*/
void drawing_update()
{
	animation_update(a_bg_upper, 16.6);
}

void drawing_draw()
{
	int i;
	bool is_ggt;
	unsigned char fumen_level;
	
	/* draw bg upper */
	for (i = 0; i < SCREEN_WIDTH / BG_UPPER_WIDTH + 2; ++ i) {
		animation_draw(a_bg_upper, i * BG_UPPER_WIDTH, 0);
	}
	
	/* draw note bg */
	if (is_ggt) {
		frame_draw(f_bg_note_ggt, 0, 0);
	} else {
		switch (fumen_level) {
		case FUMEN_LEVEL_NORMAL:
			frame_draw(f_bg_note_normal, 0, 0);
			break;
		case FUMEN_LEVEL_EXPERT:
			frame_draw(f_bg_note_expert, 0, 0);
			break;
		case FUMEN_LEVEL_MASTER:
			frame_draw(f_bg_note_master, 0, 0);
			break;
		default:
			printf("[ERROR] unknown fumen level %d\n", fumen_level);
			break;
		}
	}
	
	/* draw soul bar */
	
	/* draw note */
	
	/* draw taiko */
	
	/* draw hit effect */
	
	/* draw note flying animation */
}

void draw_bg_upper(OSL_IMAGE *img)
{
    return;

    int i;
    for (i = 0; i * img->sizeX < SCREEN_WIDTH; ++ i) {
        oslDrawImageSimpleXY(img, i * img->sizeX, 0);
    }
}

void draw_bg_note(OSL_IMAGE *img)
{
    return;

    img->stretchX = SCREEN_WIDTH;
    oslDrawImageXY(img, 0, 78);
}

void draw_note(note_t *note, int x, int y)
{
    return;
/*
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
    */
}

void get_note_left_right(note_t *note, int x, int *left, int *right)
{
    /* TODO: modify this to a common AOI. do not depend on image size. */
    int x2;
    switch (note->type) {
        case NOTE_DON:
        case NOTE_KATSU:
        case NOTE_LDON:
        case NOTE_LKATSU:
        case NOTE_BARLINE:
            *left = x - 32;
            *right = x + 32;
            break;
        case NOTE_BALLOON:
            *left = x - 32;
            *right = x + 32 * 2;
            break;
        case NOTE_YELLOW:
        case NOTE_LYELLOW:
            x2 = x + (((yellow_t *)note)->offset2 - note->offset) * note->speed;
            *left = x - 32;
            *right = x2 + 32;
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

    frame_draw(f_bg_upper, 0, 0);    
    return;

    /*
    draw_bg_upper(bg);        
    
    oslDrawImageSimple(donchan);
    draw_bg_note(note_bg);

    oslDrawImageSimpleXY(hit_circle, NOTE_FIT_X, NOTE_Y);
    */
}

void drawing_after_note()
{
    return;

    /*
    oslDrawImageSimple(taiko);
    */
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
