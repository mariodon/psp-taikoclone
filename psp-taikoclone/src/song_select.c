#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <oslib/oslib.h>
#include "song_select.h"
#include "ui/song_select_ui.h"
#include "tjaparser.h"
int inited = 0;
static song_select_info_t *song_select_info = NULL;

int song_select_init(char *root)
{
	int fumen_num;
	
    if (inited) {
        song_select_destroy();
    }

	fumen_num = song_select_get_list2("fumen.lst", &song_select_info);
    if (fumen_num < 0) {
        oslFatalError("can't build music list");
        return -1;
    }

    init_song_select_ui((void *)song_select_info, fumen_num); 
    inited = 1;
    
    printf("song select init ok!\n");
    
    return 0;
}

void print_str_as_hex2(char *str)
{
    char *p;
    puts("");
    for (p = str; *p != '\0'; ++ p) {
        printf("\\x%x", (unsigned char)(*p));
    }
    puts("");
    return;
}

int song_select_get_list2(char *fumen_lst, song_select_info_t **ret)
{
	SceUID fd;
	int fumen_count;
	int mem_size;
    	
	fd = sceIoOpen(fumen_lst, PSP_O_RDONLY, 0777);
	if (fd < 0) {
		oslFatalError("can't open fumen list file %s!\n", fumen_lst);
		return -1;
	}
	
	if (sceIoRead(fd, (void *)(&fumen_count), sizeof(int)) != sizeof(int)) {
		oslFatalError("read fumen size error\n");
		return -1;
	}
	
	mem_size = fumen_count * sizeof(song_select_info_t);
	*ret = malloc(mem_size);
	if (*ret == NULL) {
		oslFatalError("not enough memory for fumen info %d\n", mem_size);
		return -1;
	}
	
	if (sceIoRead(fd, (void *)(*ret), mem_size) != mem_size) {
		oslFatalError("read all fumen info error\n");
		return -1;
	}

    //printf("%s\n", (*ret)[3].title);
    sceIoClose(fd);

	return fumen_count;
}

int song_select_select(cccUCS2 *tja_file, cccUCS2 *wave_file, int *course_idx)
{
    OSL_CONTROLLER *pad;
    song_select_info_t *info = NULL;

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

    memcpy(tja_file, info->tja_file, (1+cccStrlenUCS2(info->tja_file)) * sizeof(cccUCS2));
    memcpy(wave_file, info->wave_file, (1+cccStrlenUCS2(info->wave_file)) * sizeof(cccUCS2));

    // course idx
    *course_idx = info->course_info[7].seek_pos;
    printf("selected %s\n", info->title);
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
