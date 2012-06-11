#include <oslib/oslib.h>
#include "helper/dictionary.h"
#include "load_texture.h"
#include <assert.h>
#include "helper/iniparser.h"
#include "const.h"

static dictionary *cache = NULL; 

//this may be slow, we do preload
OSL_IMAGE *load_texture(const char *file, int x, int y, int w, int h, \
        int pf, int loc) {

   OSL_IMAGE *img, *img_tile;
   char mem[16];
   char *addr_str;

   if (cache == NULL) {
       cache = dictionary_new(0);
       if (cache == NULL) {
           oslFatalError("Not enough memory!");
           return NULL;
       }
   }

   addr_str = dictionary_get(cache, (char *)file, "0x0");
   img = (OSL_IMAGE *)(strtol(addr_str, NULL, 0));
   if (img == NULL) {
       img = oslLoadImageFile((char *)file, loc, pf);
       if (img == NULL) {
           oslFatalError("Can't load Image %s", file);
           return NULL;
       }
       sprintf(mem, "%p", img);
       dictionary_set(cache, (char *)file, mem);
   }

   img_tile = oslCreateImageTileSize(img, x, y, w, h);

   //printf("compare internal data: %p | %p\n", img->data, img_tile->data);
   return img_tile;
}

OSL_IMAGE *load_texture_config(dictionary *d, const char *name)
{
    static char key[50];
    char *f;
    int x, y, w, h, pf, loc;
    int center_x, center_y;
    int pos_x, pos_y;
    int name_len = strlen(name);
    OSL_IMAGE *img;

    strcpy(key, name);

    strcat(key, ":f");
    f = iniparser_getstring(d, key, NULL);

    key[name_len] = 0;
    strcat(key, ":x");
    x = iniparser_getint(d, key, 0);

    key[name_len] = 0;
    strcat(key, ":y");
    y = iniparser_getint(d, key, 0);

    key[name_len] = 0;
    strcat(key, ":w");
    w = iniparser_getint(d, key, 0);

    key[name_len] = 0;
    strcat(key, ":h");
    h = iniparser_getint(d, key, 0);

    key[name_len] = 0;
    strcat(key, ":loc");
    loc = iniparser_getint(d, key, OSL_IN_VRAM);

    key[name_len] = 0;
    strcat(key, ":pos_x");
    pos_x = iniparser_getint(d, key, -1);

    key[name_len] = 0;
    strcat(key, ":pos_y");
    pos_y = iniparser_getint(d, key, -1);

    key[name_len] = 0;
    strcat(key, ":pf");
    pf = iniparser_getint(d, key, TAIKO_PF);

    key[name_len] = 0;
    strcat(key, ":center_x");
    center_x = iniparser_getint(d, key, 0);

    key[name_len] = 0;
    strcat(key, ":center_y");
    center_y = iniparser_getint(d, key, 0);

    if (f == NULL) {
        oslFatalError("config:%s incorrect", name);
        return NULL;
    }

    img = load_texture(f, x, y, w, h, pf, loc);
    if (img != NULL) {
        img->x = pos_x;
        img->y = pos_y;
        img->angle = 0;
        img->centerX = center_x;
        img->centerY = center_y;
    }
    printf("load texture %s|%d, %d, %d, %d, %d\n", f, x, y, w, h, pf);
    //printf("img->centerX=%d, img->centerY=%d\n", img->centerX, img->centerY);
    return img;
}
