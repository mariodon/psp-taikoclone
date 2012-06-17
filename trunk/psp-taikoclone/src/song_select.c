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

int song_select_init(char *root)
{
    if (inited) {
        song_select_destroy();
    }

    strcpy(tja_path, root);
    file_list_len = song_select_get_list(tja_path, file_list, 256);

    //song_select_get_list2(tja_path, file_list, 256);

    init_song_select_ui((void *)file_list, file_list_len); 
    inited = 1;
    
    printf("song select init ok!\n");
}

int song_select_get_list2(char *directory, char **ret, int max_len)
{


    return 0;

    char *fileExt = ".tja";
    int fileExtCount = 1;
    struct opendir_struct dirToAdd;
    int i;
    char *result = opendir_open(&dirToAdd, "ms0:/MUSIC", "ms0:/MUSIC", fileExt, fileExtCount, 0);
    if (result == 0) {
        for (i = 0; i < dirToAdd.number_of_directory_entries; ++ i) {
            printf("%s\n", dirToAdd.directory_entry[i].d_name);
        }
    } else {
        printf("%s what thhe???\n", result);
    }
}

int song_select_get_list(char *directory, char **ret, int max_len)
{
    SceIoDirent dir;
    SceUID fd;
    char *file_list[256];
    char len, buf3[512];
    short buf2[512];
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
/*
            printf("%s\n", dir.d_name);
            int i;
            for (i = 0; i < strlen(dir.d_name); ++ i) {
                printf("\\x%X", (unsigned char)dir.d_name[i]);
            }            
            printf("\n");


            len = cccSJIStoUCS2(buf2, 510, dir.d_name);
            printf("%d len=?\n", len);

            for (i = 0; i < len; ++ i) {
                printf("\\x%x\n", buf2[i]);
            }
            printf("\n");
            buf2[len] = 0;

            len = cccUSC2toSJIS(buf3, 511, buf2); 
            printf("%d len=\n", len);    
            buf3[len] = 0;            

            printf("OK1\n");


            file_list[file_list_len] = malloc(len);
            strcpy(file_list[file_list_len], buf3);
    */

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
