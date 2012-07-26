#ifndef __OBJ_H__
#define __OBJ_H__

typedef void (*update_callback_t)(float elapse_time);
typedef void (*destroy_callback_t)(void);
typedef void (*init_callback_t)(void *data);

typedef struct {
	init_callback_t init_func;
	update_callback_t update_func;
	destroy_ballback_t destroy_func;
} obj_callback_t;

#endif