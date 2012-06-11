#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "tjaparser.h"
#include "helper/metadata_parser.h"

metadata_def_t course_header_defs[] = {
    {"COURSE", "s", FIELD_OFFSET(course_header_t, course), "Oni"},
    {"LEVEL", "i", FIELD_OFFSET(course_header_t, level), "9"},
    {"SCOREINIT", "ia", FIELD_OFFSET(course_header_t, scoreinit), "100,100"},
    {"SCOREDIFF", "i", FIELD_OFFSET(course_header_t, scorediff), "1000"},
    {"BALLOON", "ia", FIELD_OFFSET(course_header_t, balloon), "0"},
    {NULL, NULL, 0, NULL},
};

metadata_def_t tja_header_defs[] = {
    {"TITLE", "sjis", FIELD_OFFSET(tja_header_t, title), "unkown"},
    {"SUBTITLE", "sjis", FIELD_OFFSET(tja_header_t, subtitle), "unkown"},
    {"BPM", "f", FIELD_OFFSET(tja_header_t, bpm), "0"},
    {"OFFSET", "f", FIELD_OFFSET(tja_header_t, offset), "0"},
    {"DEMOSTART", "f", FIELD_OFFSET(tja_header_t, demostart), "0"},
    {"WAVE", "sjis", FIELD_OFFSET(tja_header_t, wave), ""},
    {"SONGVOL", "f", FIELD_OFFSET(tja_header_t, songvol), "100"},
    {"SEVOL", "f", FIELD_OFFSET(tja_header_t, sevol), "100"},
    {NULL, NULL, 0, NULL},
};

int tjaparser_check_command(char *line, char *cmd);
int tjaparser_handle_command(char *line);
int tjaparser_handle_note(char *line);
int tjaparser_add_note(note_t *note);
int tjaparser_handle_a_bar();
branch_start_t *create_note_branch_start(char cond, float x, float y);
dummy_t *create_note_dummy(int type);
end_t *create_note_end(note_t *start_note);

FILE *fp;
char buf[MAX_LINE_WIDTH];

note_t *last;
// for gathering a bar
note_t *note_buf[MAX_LINE_WIDTH];
int note_buf_len = 0;

// for branches
int branch_started = 0;
note_t **which_branch = NULL;
note_t *last_E, *last_N, *last_M; //pointer for branch link list

// global variables 
float offset; 
float bpm;
float measure;
float scroll;
int ggt;
float delay;
int barlineon;

//backups for branch
float bk_offset;
float bk_bpm;
float bk_measure;
float bk_scroll;
int bk_ggt;
float bk_delay;
int bk_barlineon;

int tjaparser_load(char *file)
{
    fp = fopen(file, "r");
    return fp != NULL;
}

char *string_strip_inplace(char *line)
{
    char *p, *q;
    for (p = line; *p != '\0' && isspace(*p); ++ p)
        ;
    for (q = p + strlen(p) - 1; q > p && isspace(*q); -- q)
        ;
    ++ q;
    *q = '\0';
    return p;
}

char *remove_jiro_comment_inplace(char *line)
{
    char *p;
    for (p = line; *p != '\0' && *(p+1) != '\0'; ++ p)
        if (*p == *(p+1) && *p == '/') {
            *p = '\0';
        }
    return line;
}

int tjaparser_seek_course(int idx)
{
    int i;
    char *end_cmd = "#END";
    char *line;

    printf("SEEK START...\n");
    fseek(fp, 0, SEEK_SET);
    i = 0;
    while (i < idx && !feof(fp)) {
        line = fgets(buf, MAX_LINE_WIDTH, fp);
        if (feof(fp)) {
            return 0;
        }
        line = string_strip_inplace(line);
        printf("after strip, line = %s\n", line);
        if (strncmp(line, end_cmd, sizeof(end_cmd)) == 0) {
            ++ i;
        }        
    }
    if (feof(fp) || i < idx) {
        return 0;
    }
    return 1;
}

int tjaparser_read_tja_header(tja_header_t *ret)
{
    char *line;
    char *start_cmd = "#START";
    metadata_def_t *metadata_def;

    fseek(fp, 0, SEEK_SET);

    printf("begin\n");
    for (metadata_def = &tja_header_defs[0]; metadata_def->name != NULL; ++ metadata_def) {
        printf("at least i'm in\n");
        metadata_get_default(metadata_def, ret);
    }

    printf("load default header value ok!\n");
    while (!feof(fp)) {
        line = fgets(buf, MAX_LINE_WIDTH, fp);
        printf("before strip len=%d\n", strlen(line));
        line = string_strip_inplace(line);
        line = remove_jiro_comment_inplace(line);
        printf("after  strip len=%d\n", strlen(line));        
        if (strncmp(line, start_cmd, sizeof(start_cmd)) == 0) {
            break;
        }
        for (metadata_def = &tja_header_defs[0]; metadata_def->name != NULL; ++ metadata_def) {
            if (metadata_parse_line(line, metadata_def, ret) == 1) {
                break;
            }
        }
    }
    return 1;
} 

//idx -- which course header
int tjaparser_read_course_header(int idx, course_header_t *ret)
{
    char *line;
    char *start_cmd = "#START";
    metadata_def_t *metadata_def;

    if (0 == tjaparser_seek_course(idx)) {
        printf("SEEK FAIL\n");
        return 0;
    }

    printf("SEEK SUCCESS!\n");
    for (metadata_def = &course_header_defs[0]; metadata_def->name != NULL; ++ metadata_def) {
        metadata_get_default(metadata_def, ret);
    }

    while (!feof(fp)) {
        line = fgets(buf, MAX_LINE_WIDTH, fp);
        //printf("before strip len=%d\n", strlen(line));
        line = string_strip_inplace(line);
        line = remove_jiro_comment_inplace(line);
        printf("after  strip len=%d\n", strlen(line));        
        if (strncmp(line, start_cmd, sizeof(start_cmd)) == 0) {
            break;
        }
        for (metadata_def = &course_header_defs[0]; metadata_def->name != NULL; ++ metadata_def) {
            if (metadata_parse_line(line, metadata_def, ret) == 1) {
                break;
            }
        }
    }
    return 1;
}

/* temp data */
tja_header_t tja_header;
course_header_t course_header;

int tjaparser_parse_course(int idx, note_t **entry)
{
    char *line;
    int has_started = 0;

    printf("read header begin\n");
    if (!tjaparser_read_tja_header(&tja_header)) {
        printf("read header failed\n");
        return 0;
    }

    printf("read header ok...\n");
    if (!tjaparser_read_course_header(idx, &course_header)) {
        printf("read course header failed\n");
        return 0;
    }

    printf("read course header ok...\n");    
    if (!tjaparser_seek_course(idx)) {
        printf("seek course failed!\n");
        return 0;
    }

    printf("seek course ok...\n");    
    offset = -tja_header.offset * 1000;  //unit:ms
    bpm = tja_header.bpm;
    measure = 4;
    scroll = 1;
    ggt = 0;
    delay = 0;
    barlineon = 1;

    printf("parsing started up ok\n");
    last = (note_t *)create_note_dummy(NOTE_DUMMY);
    *entry = last;
    while (!feof(fp)) {
        line = fgets(buf, MAX_LINE_WIDTH, fp);
        line = string_strip_inplace(line);
        line = remove_jiro_comment_inplace(line);

        //printf("after fix line, %s\n", line);
        if (!has_started && tjaparser_check_command(line, "#START")) {
            has_started = 1;
            continue;
        }

        if (!has_started) {
            continue;
        }

        if (tjaparser_check_command(line, "#END")) {

            if (branch_started) {
                tjaparser_handle_command("#BRANCHEND");
            }
            break;
        }

        if (tjaparser_handle_command(line) || tjaparser_handle_note(line) == 0) {
            // unhandled line
            continue;
        }
    }

    //remove dummy note
    while (*entry != NULL && (*entry)->type == NOTE_DUMMY && (*entry)->next != NULL) {
        *entry = (note_t *)((*entry)->next);
    }

    return 1;
}

int tjaparser_get_command(char *line, char **cmd, char **params)
{
    return 1;
}

int tjaparser_check_command(char *line, char *cmd) {
    int start_with = (strncmp(line, cmd, strlen(cmd)) == 0);
    char *p;

    if (!start_with)
        return 0;

    p = line + strlen(cmd);
    if (*p == '\0' || isspace(*p))
        return 1;

    return 0;
}

int tjaparser_handle_command(char *line) {
    char *cmd, *p;
    float new_bpm = -1;
    float new_measure, a = -1, b = -1;
    float new_scroll, fval;

    // you can't do any command in the middle of a bar, except #BPMCHANGE and
    // #SCROLL
    if (note_buf_len > 0) {
        if (line[0] == '#' 
            && 0 == tjaparser_check_command(line, "#BPMCHANGE")
            && 0 == tjaparser_check_command(line, "#SCROLL")) {
            printf("[ERROR]can't do cmd %s during a bar.", line);
            return 0;
        }
    }

    cmd = "#BPMCHANGE";
    if (tjaparser_check_command(line, cmd)) {
        new_bpm = atof(line+strlen(cmd));
        if (new_bpm > 0) {
            bpm = new_bpm;
            return 1;
        }
        return 0;
    }

    cmd = "#MEASURE";
    if (tjaparser_check_command(line, cmd)) {
        for (p = line+strlen(cmd); *p != '/' && *p != '\0'; ++ p)
            ;
        if (*p != '/') {
            return 0;
        }
        *p = '\0';
        a = strtol(line+strlen(cmd), NULL, 0);
        b = strtol(p+1, NULL, 0);
        if (a <= 0 || b <= 0) {
            return 0;
        }
        new_measure = (float)4 * a / b;
        if (new_measure <= 0) {
            return 0;
        }
        //printf("measure %f/%f\n", a, b);
        measure = new_measure;
        return 1;
    } 

    cmd = "#DELAY";
    if (tjaparser_check_command(line, cmd)) {
        offset += atof(line+strlen(cmd)) * 1000;
        return 1;
    }

    cmd = "#SCROLL";
    if (tjaparser_check_command(line, cmd)) {
        new_scroll = atof(line+strlen(cmd));
        if (new_scroll > 0) {
            scroll = new_scroll;
            return 1;
        }
        return 0;
    } 

    cmd = "#GOGOSTART";
    if (tjaparser_check_command(line, cmd)) {        
        ggt = 1;
        return 1;
    }

    cmd = "#GOGOEND";
    if (tjaparser_check_command(line, cmd)) {        
        ggt = 0;
        return 0;
    }

    cmd = "#DELAY";
    if (tjaparser_check_command(line, cmd)) {        
        fval = atof(line + strlen(cmd));
        if (fval < 0) {
            return 0;
        }
        delay += fval;
        return 1;
    }

    cmd = "#SECTION";
    if (tjaparser_check_command(line, cmd)) {        
        note_t *note = (note_t *)create_note_dummy(NOTE_SECTION);
        tjaparser_add_note(note);
        return 1;
    }

    cmd = "#BRANCHSTART";
    if (tjaparser_check_command(line, cmd)) {        
        if (branch_started) {
            tjaparser_handle_command("#BRANCHEND");
        }

        char cond;
        for (p = line + strlen(cmd); *p != '\0' && isspace(*p); ++ p)
            ;
        cond = *p;
        a = atof(p+1);
        for (++ p; *p != '\0' && *p != ','; ++ p)
            ;
        if (*p == '\0') {
            return 0;
        }
        for (++ p; *p != '\0' && *p != ','; ++ p)
            ;
        b = atof(p+1);
        printf("branch command %c,%f,%f\n", cond, a, b);
        branch_start_t *note = create_note_branch_start(cond,a,b);
        tjaparser_add_note((note_t *)note);

        branch_started = 1;
        which_branch = NULL;
        last_E = note->fumen_e;
        last_N = note->fumen_n;
        last_M = note->fumen_m;
        return 1;
    }

    cmd = "#BRANCHEND";
    if (tjaparser_check_command(line, cmd)) {        
        if (! branch_started) {
            printf("branch end before branch start\n");
            return 0;
        }

        //finish up the nearest branch start
        branch_started = 0;
        which_branch = NULL;
        ((branch_start_t *)last)->fumen_e_ed = last_E;
        ((branch_start_t *)last)->fumen_n_ed = last_N;
        ((branch_start_t *)last)->fumen_m_ed = last_M;        
        last_E = last_N = last_M = NULL;

        // add branch end note
        note_t *note = (note_t *)create_note_dummy(NOTE_BRANCH_END);
        tjaparser_add_note(note);
        printf("branch end!");
        return 1;
    }

    cmd = "#E";
    if (tjaparser_check_command(line, cmd)) {                
        if (!branch_started) {
            return 0;
        }
        printf("Efumen start\n");
        if (which_branch == NULL) {
            bk_offset = offset;
            bk_bpm = bpm;
            bk_measure = measure;
            bk_scroll = scroll;
            bk_ggt = ggt;
            bk_delay = delay;
            bk_barlineon = barlineon;
        } else {
            offset = bk_offset;
            bpm = bk_bpm;
            measure = bk_measure;
            scroll = bk_scroll;
            ggt = bk_ggt;
            delay = bk_delay;
            barlineon = bk_barlineon;
        }
        which_branch = &last_E;
        return 1;
    }

    cmd = "#N";
    if (tjaparser_check_command(line, cmd)) {        
        if (!branch_started) {
            return 0;
        }
        printf("Nfumen start\n");
        if (which_branch == NULL) {
            bk_offset = offset;
            bk_bpm = bpm;
            bk_measure = measure;
            bk_scroll = scroll;
            bk_ggt = ggt;
            bk_delay = delay;
            bk_barlineon = barlineon;
        } else {
            offset = bk_offset;
            bpm = bk_bpm;
            measure = bk_measure;
            scroll = bk_scroll;
            ggt = bk_ggt;
            delay = bk_delay;
            barlineon = bk_barlineon;
        }        
        which_branch = &last_N;
        return 1;
    }

    cmd = "#M";
    if (tjaparser_check_command(line, cmd)) {                
        if (!branch_started) {
            return 0;
        }
        printf("Mfumen start\n");
        if (which_branch == NULL) {
            bk_offset = offset;
            bk_bpm = bpm;
            bk_measure = measure;
            bk_scroll = scroll;
            bk_ggt = ggt;
            bk_delay = delay;
            bk_barlineon = barlineon;
        } else {
            offset = bk_offset;
            bpm = bk_bpm;
            measure = bk_measure;
            scroll = bk_scroll;
            ggt = bk_ggt;
            delay = bk_delay;
            barlineon = bk_barlineon;
        }        
        which_branch = &last_M;
        return 1;
    }

    cmd = "#LEVELHOLD";
    if (tjaparser_check_command(line, cmd)) {                
        note_t *note = (note_t *)create_note_dummy(NOTE_LEVELHOLD);
        tjaparser_add_note(note);
        return 1;
    }

    cmd = "#BARLINEOFF";
    if (tjaparser_check_command(line, cmd)) {        
        barlineon = 0;
        return 1;
    }

    cmd = "#BARLINEON";
    if (tjaparser_check_command(line, cmd)) {                
        barlineon = 1;
        return 1;
    }

    return 0;
}

int tjaparser_handle_note(char *line) {
    static note_t *lasting_note = NULL;

    char *p;
    note_t *note;
    int note_type;

    for (p = line; *p != '\0' && *p != ','; ++ p) {

        if (! isdigit(*p)) {
            continue;
        }

        note_type = *p - '0';

        //ignore all notes between a lasting note and a end note
        if (lasting_note != NULL && note_type == NOTE_END) {
            note = (note_t *)create_note_end(lasting_note);
            lasting_note = NULL;
        } else {
            note = (note_t *)create_note_dummy(note_type);
        }

        note->offset = 1000.0 * 60 / bpm; //used to score current time per beat.
        note_buf[note_buf_len ++] = note;

        if (note_type == NOTE_YELLOW || note_type == NOTE_LYELLOW || note_type == NOTE_BALLOON || note_type == NOTE_PHOTATO) {
            lasting_note = note;
        }
    }

    if (*p == ',') {
        return tjaparser_handle_a_bar();
    }

    return 0;
}


int tjaparser_handle_a_bar()
{
    int i;
    float bcnt, tpb;
    note_t *note_barline = NULL;

    if (barlineon) {
        note_barline = (note_t *)create_note_dummy(NOTE_BARLINE);
        note_barline->offset = offset;
        note_barline->speed = note_buf[0]->speed;
    }
    //calculate offset for all notes
    bcnt = 1.0 * measure / note_buf_len;

    //add all notes to queue
    printf("[");
    for (i = 0; i < note_buf_len; ++ i) {
        tpb = note_buf[i]->offset;
        note_buf[i]->offset = offset;
        offset += bcnt * tpb;  
        if (note_buf[i]->type == NOTE_EMPTY) { // only add non empty
            free(note_buf[i]);
        } else {
            tjaparser_add_note(note_buf[i]);
            printf("%c,%f\n", note_buf[i]->type+'0', note_buf[i]->offset);
        }
        if (i == 0 && note_barline != NULL) {
            tjaparser_add_note(note_barline);
        }
    }
    printf("]\n");

    //clear up queue
    note_buf_len = 0;
    return 1;
}

int tjaparser_add_note(note_t *note)
{
    note_t **note_to_append;

    if (note == NULL) {
        return 0;
    }

    if (! branch_started) {
        note_to_append = &last;
    } else if (which_branch == NULL) {
        //or add note to all branch
        //printf("[ERROR]don't know which branch to add note %d.\n", note->type);
        last_E->next = last_N->next = last_M->next = note;
        last_E = last_N = last_M = note;
        return 0;
    } else {
        note_to_append = which_branch;
    }

    (*note_to_append)->next = note;
    (*note_to_append) = note;

    // a workaround for now
    // TODO: implement these note
    if (note->type == NOTE_PHOTATO) {
        note->type = NOTE_YELLOW;
    }

    return 1;
}

int fill_common_field(note_t *note)
{
    note->offset = offset;
    note->speed = scroll * NOTE_MARGIN / (1000.0 * 60 / (bpm * 4));
    note->next = NULL;
    note->prev = NULL;
    note->ggt = ggt;
    return 1;
}

branch_start_t *create_note_branch_start(char cond, float x, float y)
{
    branch_start_t *ret = (branch_start_t *)malloc(sizeof(branch_start_t));

    ret->type = NOTE_BRANCH_START;
    fill_common_field((note_t *)ret);

    ret->cond = cond;
    ret->x = x;
    ret->y = y;
    ret->fumen_e = (note_t *)create_note_dummy(NOTE_DUMMY);
    ret->fumen_n = (note_t *)create_note_dummy(NOTE_DUMMY);
    ret->fumen_m = (note_t *)create_note_dummy(NOTE_DUMMY);
    ret->fumen_e_ed = ret->fumen_e;
    ret->fumen_n_ed = ret->fumen_n;
    ret->fumen_m_ed = ret->fumen_m;

    return ret;
}

dummy_t *create_note_dummy(int type) {
    dummy_t *ret = (dummy_t *)malloc(sizeof(dummy_t));
    ret->type = type;
    fill_common_field((note_t *)ret);
    return ret;
}

end_t *create_note_end(note_t *start_note) {
    end_t *ret = (end_t *)malloc(sizeof(end_t));

    ret->type = NOTE_END;
    fill_common_field((note_t *)ret);

    ret->start_note = start_note;
    return ret; 
}

int tjaparser_go_branch(int id, note_t *beg) {
    note_t *p, *tmpp;
    branch_start_t *pbs;
    for (p = beg; p != NULL; p = (note_t *)(p->next)) {
        //printf("%d", p->type);
        if (p->type == NOTE_BARLINE) {
            printf("\n|");
        } else if (p->type >= NOTE_EMPTY && p->type <= NOTE_PHOTATO) {
            printf("%c", p->type+'0');
        } else if (p->type == NOTE_BRANCH_START) {
            printf("\n[Branch %d]\n", id);
            pbs = (branch_start_t *)p;
            if (id == 0) {
                tmpp = pbs->next;
                pbs->next = pbs->fumen_e;
                pbs->fumen_e_ed->next = tmpp;
            } else if (id == 1) {
                tmpp = pbs->next;
                pbs->next = pbs->fumen_n;
                pbs->fumen_n_ed->next = tmpp;                
            } else if (id == 2) {
                tmpp = pbs->next;
                pbs->next = pbs->fumen_m;
                pbs->fumen_m_ed->next = tmpp;
            }
        }
    }  
    printf("\n");
}
