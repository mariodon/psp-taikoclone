#include <oslib/oslib.h>

#include "textures.h"
#include "const.h"
#include "helper/dictionary.h"

/* caching preloaded textures */
static dictionary *cache = NULL;

/* preload all the textures we need for rendering. */
bool textures_init()
{
    // inited and we only init once.
    if (cache != NULL) {
        return TRUE;
    }
    // use an dictionary to cache textures
    cache = dictionary_new(0);
    if (cache == NULL) {
        oslFatalError("Not enough memory! %s %d", __FILE__, __LINE__);
    }
    // load configuration
    dictionary *config = iniparser_load("config/textures.ini");
    if (config == NULL) {
        oslFatalError("can't load texture config!");
    }
    // load all textures (config defines PixelFormat, Location)
    textures_cache_cfg(config, "bg");
    textures_cache_cfg(config, "note_bg");
    textures_cache_cfg(config, "donchan");
    textures_cache_cfg(config, "hit_circle");
    textures_cache_cfg(config, "taiko");
    textures_cache_cfg(config, "taiko_lblue");
    textures_cache_cfg(config, "taiko_rblue");
    textures_cache_cfg(config, "taiko_lred");
    textures_cache_cfg(config, "taiko_rred");
    textures_cache_cfg(config, "soulbar_empty");
    textures_cache_cfg(config, "soulbar_notclear");
    textures_cache_cfg(config, "soulbar_notfull");
    textures_cache_cfg(config, "soulbar_full");
    textures_cache_cfg(config, "taiko_flower");
    textures_cache_cfg(config, "note_don");
    textures_cache_cfg(config, "note_katsu");
    textures_cache_cfg(config, "note_ldon");
    textures_cache_cfg(config, "note_lkatsu");
    textures_cache_cfg(config, "note_yellowh");
    textures_cache_cfg(config, "note_yellowb");
    textures_cache_cfg(config, "note_yellowt");
    textures_cache_cfg(config, "note_lyellowh");
    textures_cache_cfg(config, "note_lyellowb");
    textures_cache_cfg(config, "note_lyellowt");
    textures_cache_cfg(config, "note_balloonh");
    textures_cache_cfg(config, "note_balloont");
    textures_cache_cfg(config, "note_barline");
    textures_cache_cfg(config, "note_barline_yellow");
    // free config mem
    iniparser_freedict(config); 
}

/* retrieve textures by key. */
OSL_IMAGE *textures_get(char *key)
{
    OSL_IMAGE *img;
    char *addr_str;

    addr_str = dictionary_get(cache, (char *)key, "0x0");
    img = (OSL_IMAGE *)(strtol(addr_str, NULL, 0));
    return (img == 0 ? NULL : img);
}

/* cache a texture */
void textures_cache(char *key, char *file, int pf, int loc)
{
    assert(cache != NULL);
    assert(cfg != NULL && key != NULL);

    OSL_IMAGE *img;
    char mem[16];
    char *addr_str;
    int loc, pf;

    addr_str = dictionary_get(cache, key, "0x0");
    img = (OSL_IMAGE *)(strtol(addr_str, NULL, 0));
    if (img == NULL) {
        img = oslLoadImageFile((char *)file, loc, pf);
        if (img == NULL) {
            oslFatalError("Can't load Image %s", file);
        }
        sprintf(mem, "%p", img);
        dictionary_set(cache, (char *)key, mem);
    }
}

void textures_cache_cfg(dictionary *cfg, char *key)
{
    assert(cfg != NULL && key != NULL);

    static char key_buf[50];
    char *file;
    int pf, loc;
    int namelen = strlen(key);
    
    if (strlen(key) + 4 >= 50) {
        oslFatalError("config key too long!");
    }
    strcpy(key_buf, key);

    strcat(key_buf, ":f");
    file = iniparser_getstring(cfg, key_buf, NULL);

    key_buf[namelen] = '\0';
    strcat(key_buf, ":pf");
    pf = iniparser_getint(cfg, key_buf, OSL_IN_VRAM);

    key_buf[namelen] = '\0';
    stract(key_buf, ":loc");
    loc = iniparser_getint(cfg, key_buf, TAIKO_PF);

    textures_cache(key, file, pf, loc);
}

/* create a texture, textures with the same name will share image data,
 * but can have different parameters.
 *
 * which we can later customize.Each of these images can have their 
 * identical state.
 *
 * */
OSL_IMAGE *textures_shared_copy(const char *key, int x, int y, int w, int h)
{
    OSL_IMAGE *orig_img, *ret_img;
    ori_img = textures_get(key);
    ret_img = oslCreateImageTileSize(ori_img, x, y, w, h);
    assert(ori_img != NULL && ret_img != NULL);    
    return ret_img;
}

bool textures_destroy()
{
    //let it go.We hold all image data util the whole program ends.
}
