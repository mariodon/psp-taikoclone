#ifndef __BG_UPPER_H__
#define __BG_UPPER_H__

#define BG_UPPER_TYPE_NORMAL	0
#define BG_UPPER_TYPE_CLEAR		1
#define BG_UPPER_TYPE_HOT		2
#define BG_UPPER_TYPE_MISS		3

typedef struct {
	int offset;
	int scroll_speed;
} bg_upper_data_t;

void bg_upper_update(float elapse_time, void *data)
void bg_upper_render(void *data);
void bg_upper_set_type(int type, void *data);

#endif