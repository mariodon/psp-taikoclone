#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <oslib/oslib.h>
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

//all commands implemented
static char *all_cmd[] = {"#BPMCHANGE", "#SCROLL", "#START", "#END", \
    "#E", "#M", "#N", "#DELAY", "#GOGOSTART", "#GOGOEND", "#BRANCHSTART", \
    "#BRACHEND", "#SECTION", "#LEVELHOLD", "#MEASURE", "BARLINEON", "BARLINEOFF", NULL};
//all commands allowed within a bar
static char *all_cmd_within[] = {"#DELAY", "#SCROLL", "#BPMCHANGE", NULL};

bool tjaparser_is_bar_complete(const char *line);
bool tjaparser_is_note(const char *line);
int tjaparser_handle_command_per_fumen(const char *line, parse_data_t *fumen);
int tjaparser_handle_note_per_fumen(char **lines, int count, parse_data_t *fumen);
int tjaparser_feed_line(const char *line);
bool tjaparser_is_command(const char *line);
int tjaparser_check_command(const char *line, char *cmd);
int tjaparser_handle_command(const char *line);
int tjaparser_handle_note(char **lines, int count);
int tjaparser_add_note(parse_data_t *fumen, note_t *note);
branch_start_t *create_note_branch_start(parse_data_t *fumen, char cond, float x, float y);
dummy_t *create_note_dummy(parse_data_t *fumen, int type);
yellow_t *create_note_yellow(parse_data_t *fumen, int type);
balloon_t *create_note_balloon(parse_data_t *fumen, int type, int hit_count);
barline_t *create_note_barline(parse_data_t *fumen, int is_branch_start);

SceUID fd;

/* line buff */
char buf[MAX_LINE_WIDTH];

/* defer lines for later parsing. */
char *deferred_line[MAX_DEFERRED_LINE];
unsigned char num_deferred_line; 

/* current parsing status of three fumens. */
static parse_data_t N_fumen, E_fumen, M_fumen;
unsigned char num_fumen;
parse_data_t *curr_fumen;
bool branch_started;

/*
 * init parse status with course header
 * */
void init_parse_data(parse_data_t *data, tja_header_t *tja_header)
{
    data->bpm = tja_header->bpm;    
    data->scroll = 1.0f;    
    data->measure = 4.0f;    
    data->offset = -tja_header->offset * 1000.0f;  //unit:ms
    data->barline_on = TRUE;
    data->is_ggt = FALSE;
    data->head = data->tail = NULL;
    data->lasting_note = NULL;
    data->first_bar_after_branch = FALSE;
    data->balloon_idx = 0;
}

/*
 * sync parse status.
 * */
void sync_parse_data(parse_data_t **data_arr, int count)
{
    parse_data_t *ruler = NULL, *data;
    int i;

    // find a ruler
    for (i = 0; i < count; ++ i) {
        if (i == 0 || ruler->offset < data_arr[i]->offset) {
            ruler = data_arr[i];
        }
    }
    assert(ruler != NULL);

    // adjust all fumen to the same ruler
    for (i = 0; i < count; ++ i) {
        data = data_arr[i];
        if (ruler == data) continue;
        data->bpm = ruler->bpm;
        data->scroll = ruler->scroll;
        data->measure = ruler->measure;
        data->offset = ruler->offset;
        data->barline_on = ruler->barline_on;
        data->is_ggt = ruler->is_ggt;
    }
}

void print_str_as_hex3(char *str)
{
    char *p;
    puts("");
    for (p = str; *p != '\0'; ++ p) {
        printf("\\x%x", (unsigned char)(*p));
    }
    puts("");
    return;
}

/*
 * Try to open or create a file for reading or writing using a UCS2-encoded filename.
 *
 * @param filename - Pointer to a UCS2 string holding the name of the file to open.
 * @param flags - Libc styled flags that are or'ed together.
 * @param mode - File access mode. 
 *
 * @return A non-negative integer is a valid fd, anything else an error.
 */
SceUID sceIoOpenUCS2(const cccUCS2 *filename, int flags, SceMode mode)
{
    cccCode filename_encoded[MAX_FILENAME];
    int len;
    SceUID fd;

    printf("orgin filename len = %d\n", cccStrlenUCS2(filename));
    /* try SJIS encoding*/
    len = cccUCS2toSJIS(filename_encoded, MAX_FILENAME - 1, filename);
    filename_encoded[len] = '\0';
    printf("first: %s\n", filename_encoded);
    //print_str_as_hex3(filename_encoded);
    fd = sceIoOpen((const char *)filename_encoded, flags, mode);
    if (fd >= 0) {
        return fd;
    }

    /* try GBK encoding */
    len = cccUCS2toGBK(filename_encoded, MAX_FILENAME - 1, filename);
    filename_encoded[len] = '\0';
    printf("second; %s\n", filename_encoded);
    //print_str_as_hex3(filename_encoded);    
    fd = sceIoOpen((const char *)filename_encoded, flags, mode);
    if (fd >= 0) {
        return fd;
    }

    /* try UTF8 encoding */
    len = cccUCS2toUTF8(filename_encoded, MAX_FILENAME - 1, filename);
    filename_encoded[len] = '\0';
    printf("third; %s\n", filename_encoded);
    //print_str_as_hex3(filename_encoded);    
    fd = sceIoOpen((const char *)filename_encoded, flags, mode);
    if (fd >= 0) {
        return fd;
    }

    return fd;
}

int tjaparser_load(cccUCS2 *filename)
{
    fd = sceIoOpenUCS2(filename, PSP_O_RDONLY, 0777);
    return (fd >= 0);
}

char * tjaparser_fgets(char * buf, int max_len, int fd) {
	int i = 0, bytes = 0;
	while( i < max_len && ( bytes = sceIoRead( fd, buf + i, 1 ) ) == 1 )
	{
		if ( buf[i] == -1 || buf[i] == '\n' )
			break;
		i ++;
	}
	buf[i] = 0;
	if ( bytes != 1 && i == 0 )
		return NULL;
    return buf;
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
    sceIoLseek32(fd, 0, SEEK_SET);
    i = 0;
    while (i < idx) {
        line = tjaparser_fgets(buf, MAX_LINE_WIDTH, fd);
        if (line == NULL) {
            break;
        }
        line = string_strip_inplace(line);
        printf("after strip, line = %s\n", line);
        if (strncmp(line, end_cmd, sizeof(end_cmd)) == 0) {
            ++ i;
        }        
    }

    if (i < idx) {
        return 0;
    }
    return 1;
}

int tjaparser_read_tja_header(tja_header_t *ret)
{
    char *line;
    char *start_cmd = "#START";
    metadata_def_t *metadata_def;

    sceIoLseek32(fd, 0, SEEK_SET);

    printf("begin\n");
    for (metadata_def = &tja_header_defs[0]; metadata_def->name != NULL; ++ metadata_def) {
        metadata_get_default(metadata_def, ret);
    }

    printf("load default header value ok!\n");
    while (1) {
        line = tjaparser_fgets(buf, MAX_LINE_WIDTH, fd);
        if (line == NULL) {
            break;
        }
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

    printf("init header def ok\n");
    while (1) {
        line = tjaparser_fgets(buf, MAX_LINE_WIDTH, fd);
        if (line == NULL) {
            break;
        }
        printf("before strip len=%d\n", strlen(line));
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

int tjaparser_parse_course(int idx, note_t **_N_fumen, note_t **_E_fumen, 
	note_t **_M_fumen)
{
    char *line;
    bool has_started = FALSE;

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

    /* init globals */
    num_deferred_line = 0;
    branch_started = FALSE;
    curr_fumen = NULL;

    /* init fumen status */
    init_parse_data(&N_fumen, &tja_header);
    init_parse_data(&E_fumen, &tja_header);
    init_parse_data(&M_fumen, &tja_header);

    printf("parsing started up ok\n");

    while (TRUE) {

        //format line
        line = tjaparser_fgets(buf, MAX_LINE_WIDTH, fd);
        if (line == NULL) {
            break;
        }
        line = remove_jiro_comment_inplace(line);
        line = string_strip_inplace(line);
        

        //printf("after fix line, %s\n", line);
        if (!has_started && tjaparser_check_command(line, "#START")) {
            has_started = TRUE;
            continue;
        }

        if (!has_started) {
            continue;
        }

        if (tjaparser_check_command(line, "#END")) {
            break;
        }

        if (! tjaparser_feed_line(line)) {
            return FALSE;
        }

    }

    if (branch_started) {
        if (! tjaparser_feed_line("#BRANCHEND")) {
            return FALSE;
        }
        branch_started = FALSE;
    }

	// return values
    *_N_fumen = N_fumen.head;
    *_E_fumen = E_fumen.head;
    *_M_fumen = M_fumen.head;        

    return TRUE;
}

int clear_deferred_line()
{
    int i;
    //clear deferred line buf    
    for (i = 0; i < num_deferred_line; ++ i) { // don't free the last line(not copied)
        free(deferred_line[i]);
    }
    num_deferred_line = 0;    
    return TRUE;
}

int tjaparser_feed_line(const char *line)
{
    bool is_cmd;
    bool is_note;
    bool is_complete_bar;

    is_cmd = tjaparser_is_command(line);
    is_complete_bar = tjaparser_is_bar_complete(line);    
    is_note = (tjaparser_is_note(line) || is_complete_bar);


    //printf("feed line %d %d %d %d\n", is_cmd, is_note, is_complete_bar, num_deferred_line);
    if (! is_cmd && ! is_note) {
        return TRUE;
    }

    //printf("feed ok!\n");
    if (num_deferred_line == 0 && is_cmd) { // single line command
        return tjaparser_handle_command(line);
    }

    if (num_deferred_line == 0 && is_complete_bar) { // single note
        //printf("handle single note\n");
        return tjaparser_handle_note((char **)&line, 1);
    }

    if (!is_complete_bar) { // begin/continue to defer line
        ++ num_deferred_line;
        deferred_line[num_deferred_line - 1] = malloc(strlen(line)+1);
        if (deferred_line[num_deferred_line - 1] == NULL) {
            -- num_deferred_line;
            goto error;
        }
        strcpy(deferred_line[num_deferred_line - 1], line);
    } else { // handle deferred lines
        //printf("handle mixed note and command!\n");
        ++ num_deferred_line;
        deferred_line[num_deferred_line - 1] = line;
        if (!tjaparser_handle_note(deferred_line, num_deferred_line)) {
            -- num_deferred_line;
            goto error;
        }
        -- num_deferred_line;
        clear_deferred_line();
    }
    return TRUE;
error:
    clear_deferred_line();
    return FALSE;
}

int tjaparser_check_command(const char *line, char *cmd) {
    int start_with = (strncmp(line, cmd, strlen(cmd)) == 0);
    const char *p;

    if (!start_with)
        return FALSE;

    p = line + strlen(cmd);
    if (*p == '\0' || isspace(*p))
        return TRUE;

    return FALSE;
}

bool tjaparser_is_command(const char *line)
{
    int i;
    char *p;
    for (i = 0; ; ++ i) {
        p = all_cmd[i];
        if (p == NULL) {
            break;
        }
        if (tjaparser_check_command(line, p)) {
            return TRUE;
        }
    }
    return FALSE;
}

bool tjaparser_is_command_within(const char *line)
{
    int i;
    char *p;
    for (i = 0; ; ++ i) {
        p = all_cmd_within[i];
        if (p == NULL) {
            break;
        }
        if (tjaparser_check_command(line, p)) {
            return TRUE;
        }
    }
    return FALSE;
}

bool tjaparser_is_note(const char *line)
{
    const char *p;
    if (tjaparser_is_command(line)) {
        return FALSE;
    }
    for (p = line; *p != '\0'; ++ p) {
        if (isdigit(*p)) {
            return TRUE;
        }
    }
    return FALSE;
}

bool tjaparser_is_bar_complete(const char *line)
{
    return line[strlen(line) - 1] == ',';
}

int tjaparser_handle_command(const char *line)
{
    char *cmd;
    const char *p;
    float a, b;

    // handle command #BRANCHSTART
    cmd = "#BRANCHSTART";
    if (tjaparser_check_command(line, cmd)) {

        // finish previous branch
        if (branch_started) {
            tjaparser_handle_command("#BRANCHEND");
        }
        assert(! branch_started);

        // parse #BRANCHSTART command
        char cond;
        for (p = line + strlen(cmd); *p != '\0' && isspace(*p); ++ p)
            ;
        cond = *p;
        a = atof(p+1);
        for (++ p; *p != '\0' && *p != ','; ++ p)
            ;
        if (*p == '\0') {
            return FALSE;
        }
        for (++ p; *p != '\0' && *p != ','; ++ p)
            ;
        b = atof(p+1);
        printf("branch command %c,%f,%f\n", cond, a, b);
        
        // create 3 branch_start_t note for 3 branches and add link
        branch_start_t *note_N = create_note_branch_start(&N_fumen, cond, a, b);
        branch_start_t *note_E = create_note_branch_start(&E_fumen, cond, a, b);
        branch_start_t *note_M = create_note_branch_start(&M_fumen, cond, a, b);
        note_N->fumen_e = note_E->fumen_e = note_M->fumen_e = (note_t *)note_E;
        note_N->fumen_n = note_E->fumen_n = note_M->fumen_n = (note_t *)note_N;
        note_N->fumen_m = note_E->fumen_m = note_M->fumen_m = (note_t *)note_M;

        // add note to three branches
        tjaparser_add_note(&N_fumen, (note_t *)note_N);
        tjaparser_add_note(&E_fumen, (note_t *)note_E);
        tjaparser_add_note(&M_fumen, (note_t *)note_M);

        // set flag for a yellow barline
        N_fumen.first_bar_after_branch = TRUE;
        E_fumen.first_bar_after_branch = TRUE;
        M_fumen.first_bar_after_branch = TRUE;

        branch_started = TRUE;

        return TRUE;
    }

    // handle command #BRANCHEND
    cmd = "#BRANCHEND";
    if (tjaparser_check_command(line, cmd)) {
        assert(branch_started);

        // sync parse data, in case some fumen is missing
        parse_data_t *data_arr[3];
        data_arr[0] = &N_fumen;
        data_arr[1] = &N_fumen;
        data_arr[2] = &N_fumen;
        sync_parse_data(data_arr, 3);

        curr_fumen = NULL;
        // mark ended.
        branch_started = FALSE;
        return TRUE;
    }    

    //handle command #N
    cmd = "#N";
    if (tjaparser_check_command(line, cmd)) {
        if (! branch_started) {
            return FALSE;
        }
        printf("N fumen start!\n");
        curr_fumen = &N_fumen;
        return TRUE;
    }

    //handle command #E
    cmd = "#E";
    if (tjaparser_check_command(line, cmd)) {
        if (! branch_started) {
            return FALSE;
        }

        printf("E fumen start!\n");
        curr_fumen = &E_fumen;
        return TRUE;
    }    

    //handle command #M
    cmd = "#M";
    if (tjaparser_check_command(line, cmd)) {
        if (! branch_started) {
            return FALSE;
        }
        printf("M fumen start!\n");
        curr_fumen = &M_fumen;
        return TRUE;        
    }

    //handle command per fumen
    if (!branch_started) {
        if (!tjaparser_handle_command_per_fumen(line, &N_fumen)) return FALSE;
        if (!tjaparser_handle_command_per_fumen(line, &E_fumen)) return FALSE;
        if (!tjaparser_handle_command_per_fumen(line, &M_fumen)) return FALSE;
        return TRUE;
    } else {
        if (curr_fumen == NULL) {
            printf("fumen started, but not decide which fumen, but has a command %s\n", line);
            return FALSE;
        }
        if (!tjaparser_handle_command_per_fumen(line, curr_fumen)) return FALSE;
        return TRUE;
    }
    return TRUE;
}

int tjaparser_handle_command_per_fumen(const char *line, parse_data_t *fumen)
{
    char *cmd;
    char buf2[MAX_LINE_WIDTH];
    char *p;
    float new_bpm = -1;
    float new_measure, a = -1, b = -1;
    float new_scroll;

    cmd = "#BPMCHANGE";
    if (tjaparser_check_command(line, cmd)) {
        new_bpm = atof(line+strlen(cmd));
        if (new_bpm > 0) {
            fumen->bpm = new_bpm;
            return TRUE;
        }
        return FALSE;
    }

    cmd = "#MEASURE";
    if (tjaparser_check_command(line, cmd)) {
        strcpy(buf2, line);
        for (p = buf2+strlen(cmd); *p != '/' && *p != '\0'; ++ p)
            ;
        if (*p != '/') {
            return 0;
        }
        *p = '\0';
        a = strtol(buf2+strlen(cmd), NULL, 0);
        b = strtol(p+1, NULL, 0);
        if (a <= 0 || b <= 0) {
            return 0;
        }
        new_measure = (float)4 * a / b;
        if (new_measure <= 0) {
            return 0;
        }
        //printf("measure %f/%f\n", a, b);
        fumen->measure = new_measure;
        return 1;
    } 

    cmd = "#DELAY";
    if (tjaparser_check_command(line, cmd)) {
        fumen->offset += atof(line+strlen(cmd)) * 1000;
        return 1;
    }

    cmd = "#SCROLL";
    if (tjaparser_check_command(line, cmd)) {
        new_scroll = atof(line+strlen(cmd));
        if (new_scroll > 0) {
            fumen->scroll = new_scroll;
            return 1;
        }
        return 0;
    } 

    cmd = "#GOGOSTART";
    if (tjaparser_check_command(line, cmd)) {        
        fumen->is_ggt = TRUE;
        return 1;
    }

    cmd = "#GOGOEND";
    if (tjaparser_check_command(line, cmd)) {        
        fumen->is_ggt = FALSE;
        return TRUE;
    }

    cmd = "#SECTION";
    if (tjaparser_check_command(line, cmd)) {        
        note_t *note = (note_t *)create_note_dummy(fumen, NOTE_SECTION);
        tjaparser_add_note(fumen, note);
        return 1;
    }

    cmd = "#LEVELHOLD";
    if (tjaparser_check_command(line, cmd)) {                
        note_t *note = (note_t *)create_note_dummy(fumen, NOTE_LEVELHOLD);
        tjaparser_add_note(fumen, note);
        return 1;
    }

    cmd = "#BARLINEOFF";
    if (tjaparser_check_command(line, cmd)) {        
        fumen->barline_on = FALSE;
        return 1;
    }

    cmd = "#BARLINEON";
    if (tjaparser_check_command(line, cmd)) {                
        fumen->barline_on = TRUE;
        return 1;
    }

    return 0;
}

int tjaparser_handle_note(char **lines, int count)
{
    if (!branch_started) {
        if (!tjaparser_handle_note_per_fumen(lines, count, &N_fumen)) return FALSE;
        if (!tjaparser_handle_note_per_fumen(lines, count, &E_fumen)) return FALSE;
        if (!tjaparser_handle_note_per_fumen(lines, count, &M_fumen)) return FALSE;
        printf("handle all fumen over!\n");
        return TRUE;
    } else  {
        if (curr_fumen == NULL) {
            printf("fumen started, but not decide which fumen, but has to handle note!\n");
            return FALSE;
        }
        if (!tjaparser_handle_note_per_fumen(lines, count, curr_fumen)) return FALSE;
        //Note: sync balloon idx
        N_fumen.balloon_idx = E_fumen.balloon_idx = M_fumen.balloon_idx = curr_fumen->balloon_idx;
        return TRUE;        
    }
    return TRUE;
}

int tjaparser_count_total_note(char **lines, int count)
{
    int i;
    char *line, *p;
    int ret = 0;

    for (i = 0; i < count; ++ i) {
        line = lines[i];
        if (tjaparser_is_command(line)) {
            continue;
        }
        for (p = line; *p != '\0'; ++ p) {
            if (isdigit(*p)) {
                ++ ret;
            }
        }
    }

    return ret;
}

int tjaparser_handle_note_per_fumen(char **lines, int count, parse_data_t *fumen) {
    int note_cnt;
    char *p, *line;
    int i;
    note_t *note;
    barline_t *barlinenote = NULL;
    
    int note_type;


    //add barline
    if (fumen->barline_on || fumen->first_bar_after_branch) {
        barlinenote = create_note_barline(fumen, fumen->first_bar_after_branch);
    }
    fumen->first_bar_after_branch = FALSE;

    // handle completely empty bar
    note_cnt = tjaparser_count_total_note(lines, count);
    printf("note_cnt = %d\n",note_cnt);    
    if (note_cnt == 0) {
        if (barlinenote != NULL) {
            tjaparser_add_note(fumen, (note_t *)barlinenote);
        }
        fumen->offset += (1000.0 * 60.0 / fumen->bpm) * fumen->measure;
        return TRUE; 
    }

    // handle command mixed note
    for (i = 0; i < count; ++ i) {
        line = lines[i];
        if (tjaparser_is_command(line)) {
            tjaparser_handle_command_per_fumen(line, fumen);
            continue;
        }
        for (p = line; *p != '\0' && *p != ','; ++ p) {
            if (! isdigit(*p)) {
                continue;
            }
            note_type = *p - '0';
            //special case for lasting note
            if (fumen->lasting_note != NULL) {
                if (note_type == NOTE_END) {
                    fumen->lasting_note->offset2 = fumen->offset;
                    fumen->lasting_note = NULL;
                }
            // create and add normal note
            } else if (note_type != NOTE_EMPTY) {
                if (note_type == NOTE_YELLOW || note_type == NOTE_LYELLOW) {
                    note = (note_t *)create_note_yellow(fumen, note_type);
                    fumen->lasting_note = (yellow_t *)note;
                } else if (note_type == NOTE_BALLOON || note_type == NOTE_PHOTATO) {
                    //TODO: implement photato!
                    note = (note_t *)create_note_balloon(fumen, NOTE_BALLOON, course_header.balloon[++ fumen->balloon_idx]);
                    fumen->lasting_note = (yellow_t *)note;
                } else {            
                    note = (note_t *)create_note_dummy(fumen, note_type);
                }
                tjaparser_add_note(fumen, note);
            }
            if (barlinenote != NULL) {
                tjaparser_add_note(fumen, (note_t *)barlinenote);
                barlinenote = NULL;
            }
            // advance in time
            fumen->offset += (1.0 * fumen->measure / note_cnt) * (1000.0 * 60.0 / fumen->bpm);
        }
        printf("handle %p %s ok\n", fumen, line);
    }

    return TRUE;
}

int tjaparser_add_note(parse_data_t *data, note_t *note)
{

    if (data->tail == NULL) {
        assert(data->head == NULL);

        data->head = data->tail = note;
    } else {
        assert(data->tail != NULL);

        data->tail->next = note;
        data->tail = note;
    }

    return TRUE;
}

int fill_common_field(parse_data_t *fumen, note_t *note)
{
    note->offset = fumen->offset;
    note->speed = fumen->scroll * NOTE_MARGIN / (1000.0 * 60 / (fumen->bpm * 4));
    note->next = NULL;
    note->prev = NULL;
    note->ggt = fumen->is_ggt;
    return TRUE;
}

branch_start_t *create_note_branch_start(parse_data_t *fumen, char cond, float x, float y)
{
    branch_start_t *ret = (branch_start_t *)malloc(sizeof(branch_start_t));

    ret->type = NOTE_BRANCH_START;
    fill_common_field(fumen, (note_t *)ret);

    ret->cond = cond;
    ret->x = x;
    ret->y = y;
    ret->bak_next = NULL;

    return ret;
}

dummy_t *create_note_dummy(parse_data_t *fumen, int type) {
    dummy_t *ret = (dummy_t *)malloc(sizeof(dummy_t));
    ret->type = type;
    fill_common_field(fumen, (note_t *)ret);
    return ret;
}

yellow_t *create_note_yellow(parse_data_t *fumen, int type)
{
    yellow_t *ret = (yellow_t *)malloc(sizeof(yellow_t));

    ret->type = type;
    fill_common_field(fumen, (note_t *)ret);

    return ret;
}

balloon_t *create_note_balloon(parse_data_t *fumen, int type, int hit_count) 
{
    balloon_t *ret = (balloon_t *)malloc(sizeof(balloon_t));

    ret->type = type;
    ret->hit_count = hit_count;
    fill_common_field(fumen, (note_t *)ret);

    return ret;
}

barline_t *create_note_barline(parse_data_t *fumen, int is_branch_start)
{
    barline_t *ret = (barline_t *)malloc(sizeof(barline_t));

    ret->type = NOTE_BARLINE;
    ret->is_branch = is_branch_start;

    fill_common_field(fumen, (note_t *)ret);

    return ret;
}

int tjaparser_unload()
{
    sceIoClose(fd);
    return TRUE;
}
