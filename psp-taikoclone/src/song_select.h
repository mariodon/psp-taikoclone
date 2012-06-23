#ifndef _SONG_SELECT_H_
#define _SONG_SELECT_H_

#include "const.h"
#include <oslib/oslib.h>

typedef struct {
	cccUCS2 tja_file[MAX_FILENAME_UCS2];
	cccUCS2 wave_file[MAX_FILENAME_UCS2];
	char title[MAX_TITLE];
	char subtitle[MAX_SUBTITLE];
	unsigned int course_cnt;
	struct {
		int course_level;
		int seek_pos;		
	} course_info[8];
} song_select_info_t;

/* public interface */
int song_select_init();
int song_select_select(cccUCS2 *, cccUCS2 *, int *);
int song_select_get_list2(char *fumen_lst, song_select_info_t **ret);
int song_select_destroy();

#endif
