#ifndef _LOAD_TEXTURE_H_
#define _LOAD_TEXTURE_H_

#include <oslib/oslib.h>
#define MAX_LOAD_TEXTURE 200

OSL_IMAGE *load_texture(const char* file, int x, int y, int w, int h, \
        int pf, int loc);
OSL_IMAGE *load_texture_config(dictionary *d, const char *name);

#endif
