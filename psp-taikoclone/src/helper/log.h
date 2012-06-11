#ifndef _LOG_H_
#define _LOG_H_

/* constants */
#define MAX_LOG 100
#define MAX_LOG_SIZE 256

/* functions */
int log_init();
int log_log(char *str);
int log_printf(char *fmt, ...);

#endif
