#ifndef _SONG_SELECT_H_
#define _SONG_SELECT_H_

/* public interface */
int song_select_init();
int song_select_select(char *, char *, int *);
int song_select_destroy();

#endif
