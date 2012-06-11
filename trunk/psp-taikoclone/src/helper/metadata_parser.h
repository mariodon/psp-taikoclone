#ifndef _METADATA_PARSER_H_
#define _METADATA_PARSER_H_

typedef struct {
    char *name;
    char *val_type;
    int offset;
    char *def_val;
} metadata_def_t;

// save default value to (dest + offset)
int metadata_get_default(metadata_def_t *def, void *dest);
// try parse a line, if metadata name == def name, then save data
int metadata_parse_line(char *line, metadata_def_t *def, void *dest);

#endif
