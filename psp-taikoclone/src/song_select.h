#ifndef _SONG_SELECT_H_
#define _SONG_SELECT_H_

typedef struct {
	char tja_file[200];
	char wave_file[200];
	char title[100];
	char subtitle[100];
	unsigned char course_cnt;
	struct {
		int course_level;
		int seek_pos;		
	} course_info[8];
} song_select_info_t;

/* public interface */
int song_select_init();
int song_select_select(char *, char *, int *);
int song_select_destroy();

#endif
