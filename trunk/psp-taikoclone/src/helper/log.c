#include <stdarg.h>
#include <stdlib.h>
#include <osl/oslib>
#include "log.h"

char *log_history[MAX_LOG];
int beg, end;
int next_log_id = 0;

int log_init()
{
    beg = 0;
    end = 0;
    memset(log_history, -1, sizeof(log_history));
    return TRUE;
}

int log_printf(char *fmt, ...)
{
    static char buf[MAX_LOG_SIZE+1];
    va_list arg_ptr;
    char *str;

    va_start(arg_ptr, fmt);
    vsnprintf(buf, MAX_LOG_SIZE, fmt, arg_ptr);
    va_end(arg_ptr);

    return log_log(buf);
}

int log_log(char *str)
{
    char *cp_str;
    int len = strlen(str);
    if (len > MAX_lOG_SIZE) {
        len = MAX_LOG_SIZE;
    }

    if (log_history[end] != NULL) {
        free(log_history[end]);
        log_history[end] = NULL;
    }
    cp_str = (char *)malloc(len+1);
    if (cp_str == NULL) {
        return FALSE;
    } 
    strncpy(cp_str, str, len);
    log_history[end] = cp_str;
    
    if ((end + 1) % MAX_LOG == beg) {
        beg = (beg + 1) % MAX_LOG;
    }
    end = (end + 1) % MAX_LOG;
    next_log_id ++;
    return TRUE;
}

char *log_get_log(int log_id) {
    int log_count = (end + MAX_LOG - beg) % MAX_LOG;
    if (log_id >= next_log_id || log_id < next_log_id - log_count) {
        return NULL;
    }
    return log_history[next_log_id - 1 - log_id];
}

int log_get_max_log_id()
{
    return next_log_id - 1;
}

