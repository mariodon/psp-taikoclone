#include "../song_select.h"
#include "song_select_ui.h"
#include "ui_utils.h"

void draw_song_select_bg(OSL_IMAGE *img, int start_x, int start_y)
{
    int real_start_x = start_x;
    int real_start_y = start_y;
    while (real_start_x + img->sizeX < 0) {
        real_start_x += img->sizeX;
    }

    draw_image_tiles(img, real_start_x, real_start_y, SCREEN_WIDTH, \
            real_start_y + img->sizeY);
}

void draw_unselected_song_item_frame(OSL_IMAGE *shd_img, OSL_IMAGE *bg_img,\
        int x, int y)
{
    oslDrawImageSimpleXY(shd_img, x, y);
    oslDrawImageSimpleXY(bg_img, x, y);
}

void draw_over_song_item_frame(OSL_IMAGE *shd_img, OSL_IMAGE *bg_l_img, \
        OSL_IMAGE *bg_r_img, OSL_IMAGE *bg_c_img, int x, int y)
{
    oslDrawImageSimpleXY(shd_img, x, y);

    oslDrawImageSimpleXY(bg_l_img, x, y);

    bg_c_img->stretchX = shd_img->sizeX - bg_l_img->sizeX - bg_r_img->sizeX;
    oslDrawImageXY(bg_c_img, x+bg_l_img->sizeX, y);

    oslDrawImageSimpleXY(bg_r_img, x+shd_img->sizeX-bg_l_img->x, y);
}

void draw_selected_song_item_frame(OSL_IMAGE *shd_img, OSL_IMAGE *bg_l_img,\
        OSL_IMAGE *bg_r_img, OSL_IMAGE *bg_c_img, int x, int y, float scale)
{
    shd_img->stretchX = shd_img->sizeX * scale;
    oslDrawImageXY(shd_img, x, y);

    oslDrawImageSimpleXY(bg_l_img, x, y);

    bg_c_img->stretchX = shd_img->stretchX - bg_l_img->sizeX \
                         - bg_r_img->sizeX;
    oslDrawImageXY(bg_c_img, x+bg_l_img->sizeX, y);

    oslDrawImageSimpleXY(bg_r_img, x+shd_img->stretchX-bg_l_img->x, y);

}

void draw_unselected_song_idx(OSL_IMAGE *bg_img, \
        OSL_IMAGE **font_imgs, int idx, int x, int y)
{
    int num_count = 0;
    int _tmp_idx;
    int array[10]; //enough

    if (idx < 0 || idx >= 1000000000) {
        printf("invalid song id\n");
        return;
    }

    oslDrawImageSimpleXY(bg_img, x, y);

    _tmp_idx = idx;
    do {
        array[num_count ++] = _tmp_idx % 10;
        _tmp_idx /= 10;
    } while(_tmp_idx !=0);


}

void draw_selected_song_idx(OSL_IMAGE *bg_img, \
        OSL_IMAGE **font_imgs, int idx, \
        int tot, int tot_scale, int tot_alpha,
        int x, int y)
{

}

song_select_info_t *data;
int course_idx = 0;
int item_count = 22;
int scroll_idx = 0;
int selected_idx = 0;
int item_per_page = 8;
int item_height = 272 / 8;
int is_dirty = 1;
OSL_FONT *fnt, *alt_fnt;

// a simple and tmp version
void *song_select_ui_handle_input(OSL_CONTROLLER *pad)
{ 
    int old_selected_idx = selected_idx;

    if (pad->pressed.up) {
        -- selected_idx;
        if (selected_idx < 0) {
            selected_idx = 0;
        }
        if (old_selected_idx != selected_idx) {
            course_idx = 0;
        }
        if (selected_idx < scroll_idx) {
            scroll_idx = selected_idx;
        }
        is_dirty = 1;
        return NULL;
    } else if (pad->pressed.down) {
        ++ selected_idx;
        if (selected_idx >= item_count) {
            selected_idx = item_count - 1;
        }
        if (old_selected_idx != selected_idx) {
            course_idx = 0;
        }        
        if (selected_idx >= scroll_idx + item_per_page) {
            scroll_idx = selected_idx - item_per_page + 1;
        }
        is_dirty = 1;
        return NULL;
    } else if (pad->pressed.L) {
        selected_idx -= item_per_page;
        if (selected_idx < 0) {
            selected_idx = 0;
        }
        if (old_selected_idx != selected_idx) {
            course_idx = 0;
        }        
        if (selected_idx < scroll_idx) {
            scroll_idx = selected_idx;
        }
        is_dirty = 1;
        return NULL;
    } else if (pad->pressed.R) {
        selected_idx += item_per_page;
        if (selected_idx >= item_count) {
            selected_idx = item_count - 1;
        }
        if (old_selected_idx != selected_idx) {
            course_idx = 0;
        }        
        if (selected_idx >= scroll_idx + item_per_page) {
            scroll_idx = selected_idx - item_per_page + 1;
        }
        is_dirty = 1;
        return NULL;
    } else if (pad->pressed.circle) {
        if (data[selected_idx].tja_file[0] == '\0' \
                || data[selected_idx].wave_file[0] == '\0') {
            return NULL;
        }
        data[selected_idx].course_info[7].seek_pos = course_idx;
        return &data[selected_idx];
    } else if (pad->pressed.left) {
        course_idx --;
        if (course_idx < 0) {
            course_idx = 0;
        }
        is_dirty = 1;
    } else if (pad->pressed.right) {
        course_idx ++;
        if (course_idx >= data[selected_idx].course_cnt) {
            course_idx = data[selected_idx].course_cnt-1;
        }
        is_dirty = 1;
    }

    return NULL;
}

void refresh(song_select_info_t *data, int scroll_idx, int selected_idx)
{
    int i;


    oslStartDrawing();
    oslClearScreen(RGB(0, 0, 0));

    oslIntraFontSetStyle(fnt, 0.8, 0xffffffff, 0x000000ff, 0);
    for (i = 0; i < item_per_page && i < item_count; ++ i) {
        if (i+scroll_idx == selected_idx) {
            oslIntraFontSetStyle(fnt, 0.8, 0xff0000ff, 0x000000ff, 0);
        }

        oslDrawStringLimited(0, i * item_height, 400, data[scroll_idx+i].title);

        if (i+scroll_idx == selected_idx) {
            oslDrawStringf(401, i * item_height, "%s\xe2\x98\x86\xc3\x97%d%s", course_idx > 0 ? "\xe2\x86\x90" : " ", data[scroll_idx+i].course_info[course_idx].course_level, course_idx < data[scroll_idx+i].course_cnt-1 ? "\xe2\x86\x92" : " ")
            oslIntraFontSetStyle(fnt, 0.8, 0xffffffff, 0x000000ff, 0);
        }       
    }
    oslEndDrawing();
    oslSyncFrame();

    is_dirty = 0;
}

void update_song_select_ui()
{
    if (is_dirty) {
        refresh(data, scroll_idx, selected_idx);
    }
}

void init_song_select_ui(void *_data, int data_length) 
{
    item_count = data_length;
    data = (song_select_info_t *)_data;

    fnt = oslLoadIntraFontFile("flash0:/font/jpn0.pgf", INTRAFONT_STRING_UTF8 | INTRAFONT_CACHE_LARGE);
    alt_fnt = oslLoadIntraFontFile("flash0:/font/gb3s1518.bwfon", INTRAFONT_STRING_UTF8 | INTRAFONT_CACHE_LARGE);
    intraFontSetAltFont(fnt->intra, alt_fnt->intra);
    oslSetFont(fnt);

    is_dirty = 1;
}
