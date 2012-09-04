#include <oslib/oslib.h>
#include "frame_factory.h"
#include "helper/dictionary.h"
#include "helper/iniparser.h"

static dictionary *frame_cache = NULL;
static dictionary *texture_cfg = NULL;

void frame_factory_init(const char *tex_cfg_file)
{
    if (frame_cache == NULL) {
        return;
    }

    frame_cache = dictionary_new(0);
    if (frame_cache == NULL) {
        oslFatalError("Not enough memory! %s %d", __FILE__, __LINE__);
    }

    dictionary *texture_cfg = iniparser_load(tex_cfg_file);
    if (texture_cfg == NULL) {
        oslFatalError("can't load texture config!");
    }

    return;
}

frame_t *frame_factory_get(const char *key)
{
    frame_t *frame;
    char *addr_str;

    addr_str = dictionary_get(frame_cache, (char *)key, "0x0");
    frame = (frame_t *)(strtol(addr_str, NULL, 0));
    if (frame != 0) {
        return frame;
    }

    // try to load frame using config from config cache
    static char key_buf[MAX_TEXTURE_NAME+4+1];
    char *file;
    int pf, loc;
    int namelen = strlen(key);

    if (strlen(key) > MAX_TEXTURE_NAME) {
        oslFatalError("config key too long!");
    }
    strcpy(key_buf, key);

    strcat(key_buf, ":f");
    file = iniparser_getstring(texture_cfg, key_buf, NULL);

    key_buf[namelen] = '\0';
    strcat(key_buf, ":pf");
    pf = iniparser_getint(texture_cfg, key_buf, OSL_IN_VRAM);

    key_buf[namelen] = '\0';
    strcat(key_buf, ":loc");
    loc = iniparser_getint(texture_cfg, key_buf, TAIKO_PF);

    frame = frame_create_simple(file, pf, loc);
    return frame;
}

frame_t *frame_factory_from_cfg(frame_cfg_t *cfg)
{
    frame_t *frame = NULL;

    frame = frame_factory_get(cfg->tex_name);
    if (frame == NULL) {
        return NULL;
    }

    frame_config(frame, cfg);

    return frame;
}

frame_cfg_t *frame_factory_read_cfg(SceUID fd)
{
    int bytes = 0;
    int cfg_size = 0;
    frame_cfg_t *cfg = NULL;

    if (fd == -1) {
        goto error;
    }
    bytes = sceIoRead(fd, &cfg_size, sizeof(cfg_size));
    if (bytes != sizeof(cfg_size)) {
        goto error;
    }
    cfg = (frame_cfg_t *)malloc(cfg_size);
    if (cfg == NULL) {
        goto error;
    }
    sceIoLseek(fd, SEEK_CUR, -sizeof(cfg_size));
    bytes = sceIoRead(fd, cfg, cfg_size);
    if (bytes != cfg_size) {
        goto error;
    }
    return cfg;
error:
    if (cfg != NULL) {
        free(cfg);
    }
    return NULL;    
}

frame_t *frame_factory_from_cfg_file(const char *cfg_file)
{
    SceUID fd = -1;
    frame_cfg_t *cfg = NULL;
    frame_t *ret = NULL;

    fd = sceIoOpen(cfg_file, PSP_O_RDONLY, 0777);
    if (fd < 0) {
        goto cleanup;
    }
    cfg = frame_factory_read_cfg(fd);
    if (cfg == NULL) {
        goto cleanup;
    }
    ret = frame_factory_from_cfg(cfg);
cleanup:
    if (fd >= 0) {
        sceIoClose(fd);
    }
    if (cfg != NULL) {
        free(cfg);
    }
    return ret;
}

void frame_factory_destroy(void)
{
    if (texture_cfg != NULL) {
        iniparser_freedict(texture_cfg);
        texture_cfg = NULL;
    }

    if (frame_cache != NULL) {
        int i;
        frame_t *frame;
        for (i = 0; i < frame_cache->size; ++ i) {
            if (frame_cache->key[i] != NULL) {
                frame = frame_factory_get(frame_cache->key[i]);
                frame_destroy(frame);
            }
        }
        dictionary_del(frame_cache);
        frame_cache = NULL;
    }

    return;
}
