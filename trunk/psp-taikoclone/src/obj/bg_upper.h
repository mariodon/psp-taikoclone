#ifndef __BG_UPPER_H__
#define __BG_UPPER_H__

#define BG_UPPER_TYPE_NORMAL	0
#define BG_UPPER_TYPE_CLEAR		1
#define BG_UPPER_TYPE_HOT		2
#define BG_UPPER_TYPE_MISS		3

typedef struct {
	const OSL_IMAGE *bg_upper_normal;
	const OSL_IMAGE *bg_upper_miss;
	const OSL_IMAGE *bg_upper_clear;
	const OSL_IMAGE *bg_upper_full;
	const int scroll_speed;
} bg_upper_def_t;

typedef struct {
	int offset;
	const bg_upper_def_t const *def;
} bg_upper_data_t;

void bg_upper_update(float elapse_time, void *data)
void bg_upper_render(void *data);
void bg_upper_set_type(int type, void *data);

#endif