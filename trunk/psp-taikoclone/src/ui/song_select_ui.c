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

char **data;
int item_count = 22;
int scroll_idx = 0;
int selected_idx = 0;
int item_per_page = 8;
int item_height = 272 / 8;
int is_dirty = 1;
OSL_FONT *fnt;

// a simple and tmp version
void *song_select_ui_handle_input(OSL_CONTROLLER *pad)
{

    if (pad->pressed.up) {
        -- selected_idx;
        if (selected_idx < 0) {
            selected_idx = 0;
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
        if (selected_idx >= scroll_idx + item_per_page) {
            scroll_idx = selected_idx - item_per_page + 1;
        }
        is_dirty = 1;
        return NULL;
    } else if (pad->pressed.circle) {
        return (void *)data[selected_idx];
    }
    return NULL;
}

void refresh(char **data, int scroll_idx, int selected_idx)
{
    int i;

    oslStartDrawing();
    oslClearScreen(RGB(0, 0, 0));

    oslIntraFontSetStyle(fnt, 0.8, 0xffffffff, 0x000000ff, 0);
    for (i = 0; i < item_per_page && i < item_count; ++ i) {
        if (i+scroll_idx == selected_idx) {
            oslIntraFontSetStyle(fnt, 0.8, 0xff0000ff, 0x000000ff, 0);
        }

        oslDrawString(0, i * item_height, data[scroll_idx+i]);

        if (i+scroll_idx == selected_idx) {
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
    int i;
    item_count = data_length;
    data = (char **)_data;
    for (i = 0; i < item_count; ++ i) {
        printf("%s\n", data[i]);
    }

    fnt = oslLoadIntraFontFile("flash0:/font/jpn0.pgf", INTRAFONT_STRING_GBK | INTRAFONT_CACHE_LARGE);
    oslSetFont(fnt);

    is_dirty = 1;
}
