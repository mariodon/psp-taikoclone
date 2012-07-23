#include "load_texture.h"
#include "bg_upper.h"

bg_upper_def_t bg_upper_def_t;

bool init_obj_def()
{
	// init bg upper definition
	bg_upper_def_t.bg_upper_normal = NULL;
	bg_upper_def_t.bg_upper_miss = NULL;
	bg_upper_def_t.bg_upper_clear = NULL;
	bg_upper_def_t.bg_upper_full = NULL;
	bg_upper_def_t.scroll_speed = 10;
	
	// init donchan definition
}