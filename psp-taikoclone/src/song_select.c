#include <oslib/oslib.h>
#include <stdlib.h>
#include "song_select.h"
#include "ui/song_select_ui.h"

int inited = 0;
char *file_list[256];
int file_list_len;
char tja_path[512+1];

int song_select_init(char *root)
{
    if (inited) {
        song_select_destroy();
    }

    strcpy(tja_path, root);

    file_list_len = song_select_get_list(tja_path, file_list, 256);

    init_song_select_ui((void *)file_list, file_list_len); 
    inited = 1;
    
    printf("song select init ok!\n");
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

int song_select_free_list(char **list, int list_len)
{
    int i;
    for (i = 0; i < list_len; ++ i) {
        free(list[i]);
    }
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
