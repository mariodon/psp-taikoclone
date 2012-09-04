#ifndef __FRAME_FACTORY_H__
#define __FRAME_FACTORY_H__
#include "frame.h"

void frame_factory_init(const char *tex_cfg_file);


frame_cfg_t *frame_factory_read_cfg(SceUID fd);
frame_t *frame_factory_get(const char *tex_name);
frame_t *frame_factory_from_cfg(frame_cfg_t *cfg);
frame_t *frame_factory_from_cfg_file(const char *cfg_file);

void frame_factory_destroy(void);

#endif
