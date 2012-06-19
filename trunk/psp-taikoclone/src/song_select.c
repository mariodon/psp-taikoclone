#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <oslib/oslib.h>
#include "helper/opendir.h"
#include "helper/libminiconv.h"
#include "song_select.h"
#include "ui/song_select_ui.h"

int inited = 0;
char *file_list[256];
int file_list_len;
static char tja_path[512+1];

static song_select_info_t *song_select_info = NULL;

int song_select_init(char *root)
{
	int i;
	int fumen_num;
	
    if (inited) {
        song_select_destroy();
    }

    strcpy(tja_path, root);
    file_list_len = song_select_get_list(tja_path, file_list, 256);

	fumen_num = song_select_get_list2("fumen.lst", &song_select_info);
	if (fumen_num > 0) {
		for (i = 0; i < fumen_num; ++ i) {
			printf("%s\n", song_select_info[i].title);
		}
	}

    init_song_select_ui((void *)file_list, file_list_len); 
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
		printf("not enough memory for fumen info\n");
		return -1;
	}
	
	if (sceIoRead(fd, (void *)(*ret), mem_size) != mem_size) {
		printf("read error\n");
		return -1;
	}
	
	return fumen_count;
}

int song_select_get_list(char *directory, char **ret, int max_len)
{
    SceIoDirent dir;
    SceUID fd;
    char *file_list[256];
    int file_list_max_len = max_len > 256 ? 256 : max_len;
    int file_list_len;

    fd = sceIoDopen(directory);
    if (fd < 0) {
        printf("can't open tja folder, errcode=%d\n", fd);
        return 0;
    }

    file_list_len = 0;
    memset(&dir, 0, sizeof(dir));
    while (file_list_len <= file_list_max_len && sceIoDread(fd, &dir) > 0) {
        if (stricmp(dir.d_name+strlen(dir.d_name)-4, ".tja") == 0) {

            file_list[file_list_len] = malloc(strlen(dir.d_name)+1);
            strcpy(file_list[file_list_len], dir.d_name);

            file_list_len ++;

        }
        memset(&dir, 0, sizeof(dir));
    }

    memcpy(ret, file_list, sizeof(char *) * file_list_len);
    sceIoDclose(fd);    

    return file_list_len;
}

void song_select_free_list(char **list, int list_len)
{
    int i;
    for (i = 0; i < list_len; ++ i) {
        free(list[i]);
    }
    return;
}

int song_select_select(char *path, char *file, int *course_idx)
{
    OSL_CONTROLLER *pad;
    char *filename = NULL;

    if (!inited) {
        printf("song select not inited!\n");
        return 0;
    }

    while (filename == NULL) {
        pad = oslReadKeys();
        if (pad->pressed.start) {
            return 0;
        }

        filename = song_select_ui_handle_input(pad);
        update_song_select_ui();
    }

    //TODO: correct this
    strcpy(path, tja_path);
    strcpy(file, filename);
    *course_idx = 0;

    return 1;
}

int song_select_destroy()
{
    if (!inited) {
        return 0;
    }

    if (file_list_len > 0) {
        song_select_free_list(file_list, file_list_len);
    }
    inited = 0;
    return 1;
}
