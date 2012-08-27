#ifndef __FRAME_FACTORY_H__
#define __FRAME_FACTORY_H__
#include "frame.h"

void frame_factory_init(const char *tex_cfg_file);

frame_t *frame_factory_get(const char *tex_name);
frame_t *frame_factory_get_cfged(const char *tex_name, frame_cfg_t *cfg);

void frame_factory_destroy(void);

#endif
