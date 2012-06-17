#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "metadata_parser.h"

//input string should be stripped
int metadata_convert(char *str, char *type, void *dest)
{
    printf("convert 1%p\n", str);
    printf("%s\n", str);
    printf("%s\n", type);    
    printf("%p\n", dest); 
    char *p, *q, ch;
    int cnt, *pi;
    if (strcmp(type, "f") == 0) {
        *((float *)dest) = atof(str);
    } else if (strcmp(type, "i") == 0) {
        *((int *)dest) = strtol(str, NULL, 0);
    } else if (strcmp(type, "s") == 0 || strcmp(type, "sjis") == 0) {
        printf("%s\n", str);
        *((char **)dest) = (char *)malloc(strlen(str)+1);
        strcpy(*((char **)dest), str);
    } else if (strcmp(type, "ia") == 0) {
        cnt = 0;
        for (p = str; *p != '\0'; ++ p) {
            if (*p == ',') {
                ++ cnt;
            }
        }

        //printf("parse_ia, pre_cnt = %d\n", cnt);
        pi = *((int **)dest) = (int *)malloc(sizeof(int) * (cnt+1+1)); //extra for sizeof array
        *pi = 0;
        for (p = q = str; ; ++ p) {
            if (*p == ',' || *p == '\0') {
                ch = *p;
                *p = '\0';
                *(pi + *pi + 1) = strtol(q, NULL, 0);
                //printf("convert int %s\n", q);
                (*pi) += 1;
                *p = ch;
                q = p + 1;
            }
            if (*p == '\0') {
                break;
            }
        }

        if (*pi != cnt + 1) {
            printf("WARNING: ia parse error!\n");
        }
    }
    return 1;
}

int metadata_get_default(metadata_def_t *def, void *dest)
{
    printf("def val %s, val_type %s\n", def->def_val, def->val_type);
    return metadata_convert(def->def_val, def->val_type, 
            (void *)((char *)dest + def->offset));
}

int metadata_parse_line(char *line, metadata_def_t *def, void *dest)
{
    int namelen = strlen(def->name);
    char *p;

    if (strncmp(line, def->name, namelen) != 0) {
        return 0;
    }

    for (p = line + namelen; *p != '\0' && *p != ':'; ++ p)
        ;
    if (*p != '\0') { //skip delimeter
        ++ p;
    }

    if (stricmp(def->val_type, "sjis") != 0) {
        for (; *p != '\0' && *p == ' '; ++ p)
            ;
    }

    printf("name:%s\nval:%s\n", def->name, p);
    return metadata_convert(p, def->val_type, 
            (void *)((char *)dest + def->offset));
}
