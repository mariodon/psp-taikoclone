#ifndef __TEXTURES_H__
#define __TEXTURES_H__
#include <oslib/oslib.h>
#include "helper/dictionary.h"

bool textures_init();
void textures_cache(char *key, char *file, int pf, int loc);
void textures_cache_cfg(dictionary *cfg, char *key);
OSL_IMAGE *textures_get(const char *key);
OSL_IMAGE *textures_shared_copy(const char *key, int x, int y, int w, int h);
bool texturs_destroy();

#endif
