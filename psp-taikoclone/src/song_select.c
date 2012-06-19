#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <oslib/oslib.h>
#include "helper/opendir.h"
#include "helper/libminiconv.h"
#include "song_select.h"
#include "ui/song_select_ui.h"

int inited = 0;
static song_select_info_t *song_select_info = NULL;

int song_select_init(char *root)
{
	int i;
	int fumen_num;
	
    if (inited) {
        song_select_destroy();
    }

	fumen_num = song_select_get_list2("fumen.lst", &song_select_info);
    if (fumen_num < 0) {
        return -1;
    }

    init_song_select_ui((void *)song_select_info, fumen_num); 
    inited = 1;
    
    printf("song select init ok!\n");
    
    return 0;
}

int song_select_get_list2(char *fumen_lst, song_select_info_t **ret)
{
	SceUID fd;
	int fumen_count;
	int mem_size;
	
	fd = sceIoOpen(fumen_lst, PSP_O_RDONLY, 0777);
	if (fd < 0) {
		printf("can't open fumen list file %s!\n", fumen_lst);
		return -1;
	}
	
	if (sceIoRead(fd, (void *)(&fumen_count), sizeof(int)) != sizeof(int)) {
		printf("read error\n");
		return -1;
	}
	
	mem_size = fumen_count * sizeof(song_select_info_t);
	*ret = malloc(mem_size);
	if (*ret == NULL) {
		printf("not enough memory for fumen info %d\n", mem_size);
		return -1;
	}
	
	if (sceIoRead(fd, (void *)(*ret), mem_size) != mem_size) {
		printf("read error\n");
		return -1;
	}
	
	return fumen_count;
}

int song_select_select(char *tja_file, char *wave_file, int *course_idx)
{
    OSL_CONTROLLER *pad;
    song_select_info_t *info = NULL;
    cccUCS2 buf[300];
    int len;

    if (!inited) {
        printf("song select not inited!\n");
        return 0;
    }

    while (info == NULL) {
        pad = oslReadKeys();
        if (pad->pressed.start) {
            return 0;
        }

        info = song_select_ui_handle_input(pad);
        update_song_select_ui();
    }

    // tja file
    /*
    len = cccUTF8toUCS2(buf, 299, info->tja_file);
    buf[len] = 0;
    len = cccUCS2toSJIS(tja_file, 511, buf);
    tja_file[len] = 0;
    */

    //wave file
    /*
    len = cccUTF8toUCS2(buf, 299, info->wave_file);
    buf[len] = 0;
    len = cccUCS2toSJIS(wave_file, 511, buf);
    wave_file[len] = 0;
    */

    strcpy(tja_file, info->tja_file);
    strcpy(wave_file, info->wave_file);

    // course idx
    *course_idx = info->course_info[7].seek_pos;
    return 1;
}

int song_select_destroy()
{
    if (!inited) {
        return 0;
    }

    if (song_select_info != NULL) {
        free(song_select_info);
        song_select_info = NULL;
    }
    
    inited = 0;
    return 1;
}
