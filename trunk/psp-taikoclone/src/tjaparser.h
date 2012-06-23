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
#define MAX_NOTE        16

int tjaparser_load(cccUCS2 *file);
int tjaparser_seek_course(int idx);

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
    int visible;
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
    note_t *fumen_e, *fumen_e_ed; //Futsu - easy
    note_t *fumen_n, *fumen_n_ed; //Kurouto - normal
    note_t *fumen_m, *fumen_m_ed; //Tatsujin - mad
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

typedef struct {
    int type;
    float offset;
    float speed;
    void *next;
    void *prev;
    int ggt;
    /* extra */
    note_t *start_note; 
} end_t;
int tjaparser_parse_course(int idx, note_t **entry);
int tjaparser_go_branch(int id, note_t *start);

SceUID sceIoOpenUCS2(const cccUCS2 *filename, int flags, SceMode mode);

#endif
