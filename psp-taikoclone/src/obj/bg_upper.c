#include "bg_upper.h"
#include "drawing.h"

void bg_upper_update(float elapse_time, void *_data)
{
	bg_upper_data_t *data = (bg_upper_data_t *)data;
	
	data->offset += elapse_time * data->scroll_speed;
	return;
}

void bg_upper_render(void *_data)
{
	bg_upper_data_t *data = (bg_upper_data_t *)_data;
}