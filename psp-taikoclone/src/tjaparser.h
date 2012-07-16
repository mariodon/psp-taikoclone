#ifndef _TJAPARSER_H_
#define _TJAPARSER_H_

#include <stdio.h>
#include "const.h"
#include "helper/metadata_parser.h"

#define MAX_LINE_WIDTH  512
#define FIELD_OFFSET(s, f) ((int)(&((s *)0)->f))

//the distance between 2 1/16 notes, when scroll = 1
#define NOTE_MARGIN     26

/* standard note*/
#define NOTE_EMPTY      0
#define NOTE_DON        1
#define NOTE_KATSU      2
#define NOTE_LDON       3
#define NOTE_LKATSU     4
#define NOTE_YELLOW     5
#define NOTE_LYELLOW    6
#define NOTE_BALLOON    7
#define NOTE_END        8
#define NOTE_PHOTATO    9
/* special note */
#define NOTE_SECTION    10
#define NOTE_BRANCH_START   11
#define NOTE_DUMMY      12  //used as a place holder
#define NOTE_LEVELHOLD  13
#define NOTE_BRANCH_END 14
#define NOTE_BARLINE    15 //yet another visible object
#define NOTE_FREED      16
#define MAX_NOTE        17

/* max num of line a bar can spread.
 * example:
 * 0000
 * #DELAY -0.01
 * 1111,
 *
 * This bar spread for 3 lines. */
#define MAX_DEFERRED_LINE 10

int tjaparser_load(cccUCS2 *file);
int tjaparser_seek_course(int idx);
int tjaparser_unload();

typedef struct {
    char *title;
    char *subtitle;
    float bpm;
    float offset;
    float demostart;
    char *wave;
    float songvol;
    float sevol;
} tja_header_t;

extern metadata_def_t tja_header_defs[];

int tjaparser_read_tja_header(tja_header_t *ret);

typedef struct {
    char *course;
    int level;
    int *scoreinit;
    int scorediff;
    int *balloon;
} course_header_t;

extern metadata_def_t course_header_defs[];

int tjaparser_read_course_header(int idx, course_header_t *ret);

typedef struct {
    int type;
    float offset;
    float speed;
    void *next;
    void *prev;
    int ggt;
} note_t;

typedef struct {
    int type;
    float offset;
    float speed;
    void *next;
    void *prev;
    int ggt;
    /* extra */
    float offset2;
    int hit_count;
} balloon_t;

typedef struct {
    int type;
    float offset;
    float speed;
    void *next;
    void *prev;
    int ggt;    
    /* extra */
    float offset2;
} yellow_t;

typedef struct {
    int type;
    float offset;
    float speed;
    void *next;
    void *prev;    
    int ggt;    
    /* extra */
    float offset2;
    int hit_count;
} potato_t;

typedef struct {
    int type;
    float offset;
    float speed;
    void *next;
    void *prev;    
    int ggt;    
    /* extra */
    int is_branch;
} barline_t;

typedef struct {
    int type;
    float offset;
    float speed;
    void *next;
    void *prev;    
    int ggt;    
    /* extra */
    char cond;
    float x, y;
    note_t *fumen_e; //Kurouto - Expert
    note_t *fumen_n; //Futsu - Normal
    note_t *fumen_m; //Tatsujin - Master
} branch_start_t;

typedef struct {
    int type;
    float offset;
    float speed;
    void *next;
    void *prev;    
    int ggt;    
    /* extra */
} branch_end_t;

typedef struct {
    int type;
    float offset;
    float speed;
    void *next;
    void *prev;    
    int ggt;
    /* extra */
} section_t;

typedef struct {
    int type;
    float offset;
    float speed;
    void *next;
    void *prev;    
    int ggt;
    /* extra */
} dummy_t;

int tjaparser_parse_course(int idx, note_t **entry);


SceUID sceIoOpenUCS2(const cccUCS2 *filename, int flags, SceMode mode);

// parse data for a fumen(Normal, Expert, Master)
typedef struct {
    float bpm;
    float scroll;
    float measure;
    float offset;
    bool barline_on;
    bool is_ggt;
    int balloon_idx;
    note_t *head, *tail;
    yellow_t *lasting_note;

    bool first_bar_after_branch;
} parse_data_t;

#endif
