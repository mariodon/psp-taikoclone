#include <assert.h>
#include "animation.h"
#include "frame.h"
#include "frame_factory.h"

anime_t *anime_create_from_cfg(anime_cfg_t *cfg)
{
    int num_frame;
    int i, j, k;

    num_frame = 0;
    for (i = 0; i < cfg->key_count; ++ i) {
        num_frame += cfg->keys[i].len;
    }

    anime_t *ani = (anime_t *)malloc(sizeof(anime_t) + sizeof(frame_t *) * num_frame);
    if (ani == NULL) {
        return NULL;
    }
    memset(ani, 0, sizeof(anime_t) + sizeof(frame_t *) * num_frame);

    ani->speed = cfg->play_speed;
    ani->status = ANIME_PLAY_STATUS_STOPPED;
    ani->loop = cfg->loop;

    ani->num_frame = num_frame;
    ani->cur_frame = 0;
    ani->time_passed = 0;

    k = 0;
    for (i = 0; i < cfg->key_count; ++ i) {
        if (ani->frames[k] == NULL) {
            ani->frames[k] = frame_factory_from_cfg(cfg->keys[i].cfg);
        }
        // do not need lerp? 
        // is the last key frame? 
        // copy from current key frame.
        if (! cfg->keys[i].lerp || i + 1 >= cfg->key_count) {
            for (j = 1; j < cfg->keys[i].len; ++ j) {
                ani->frames[k+j] = ani->frames[k+j-1];
            }
        // lerp between current and next key frame.
        } else {
            for (j = 1; j < cfg->keys[i].len; ++ j) {
                frame_cfg_t *lerp_frame_cfg = frame_cfg_lerp(cfg->keys[i].cfg, cfg->keys[i + 1].cfg, (float)j / cfg->keys[i].len);
                ani->frames[k+j] = frame_factory_from_cfg(lerp_frame_cfg);
                printf("after lerp, frame x = %d\n", ani->frames[k + j]->x);
                frame_cfg_destroy(lerp_frame_cfg);
            }
        }
        k += cfg->keys[i].len;
    }

    return ani;
}

anime_t *anime_create_from_file(const char *filename)
{
    SceUID fd = -1;
    anime_cfg_t *cfg = NULL;
    int key_count;
    
	fd = sceIoOpen(filename, PSP_O_RDONLY, 0777);
	if (fd < 0) {
        printf("can't open ani file %s\n", filename);
        goto error;
	}

    if (sceIoRead(fd, &key_count, sizeof(int)) != sizeof(int)) {
        goto error;
    }

    cfg = (anime_cfg_t *)malloc(sizeof(anime_cfg_t) + sizeof(anime_key_cfg_t) * key_count);
    if (cfg == NULL) {
        printf("can't alloc memory for animation.\n");
        goto error;
    }
    memset(cfg, 0, sizeof(cfg));

    if (sceIoRead(fd, (void *)cfg + sizeof(int), sizeof(anime_cfg_t) - sizeof(int)) != sizeof(anime_cfg_t) - sizeof(int)) {
        printf("can't read ani basic config\n");
        goto error;
    }
    cfg->key_count = key_count;

    //printf("%d %d %d\n", cfg->key_count, cfg->play_speed, cfg->loop);
    int i, cfg_size;
    for (i = 0; i < key_count; ++ i) {
        sceIoRead(fd, &cfg->keys[i].len, sizeof(int));
        sceIoRead(fd, &cfg->keys[i].lerp, sizeof(int));
        //printf("%d %d\n", cfg->keys[i].len, cfg->keys[i].lerp);

        cfg->keys[i].cfg = frame_factory_read_cfg(fd);
        if (cfg->keys[i].cfg == NULL) {
            printf("can't build animation key cfg\n");
            goto error;
        }
    }

    return anime_create_from_cfg(cfg);

error:
    if (fd >= 0) {
        sceIoClose(fd);
    }
    if (cfg != NULL) {
        free(cfg);
    }
    return NULL;
}

inline void anime_update(anime_t *ani, int time)
{
    if (ani->status != ANIME_PLAY_STATUS_PLAYING) {
        return;
    }

    ani->time_passed += time;
    while (ani->time_passed >= ani->speed) {
        ++ ani->cur_frame;
        ani->time_passed -= ani->speed;
    }
    if (ani->cur_frame >= ani->num_frame) {
        ani->cur_frame -= ani->num_frame;
        if (! ani->loop) {
            ani->status = ANIME_PLAY_STATUS_STOPPED;
        }        
    }
    //printf("frame = %d, addr = %p, x = %d\n", ani->cur_frame, ani->frames[ani->cur_frame], ani->frames[ani->cur_frame]->x);
    return;
}

inline void anime_goto_frame(anime_t *ani, int id)
{
    ani->cur_frame = id;
    ani->time_passed = 0;
}

inline void anime_draw(anime_t *ani, int x, int y)
{
    frame_draw(ani->frames[ani->cur_frame], x, y);
}

inline void anime_play(anime_t *ani)
{
    switch (ani->status) {
        case ANIME_PLAY_STATUS_PLAYING:
            return;
        case ANIME_PLAY_STATUS_PAUSED:
            ani->status = ANIME_PLAY_STATUS_PLAYING;
            return;
        case ANIME_PLAY_STATUS_STOPPED:
            ani->status = ANIME_PLAY_STATUS_PLAYING;
            ani->cur_frame = 0;
            ani->time_passed = 0;
            return;
    }
    return;
}

inline void anime_pause(anime_t *ani)
{
    ani->status = ANIME_PLAY_STATUS_PAUSED;
}

inline void anime_stop(anime_t *ani)
{
    ani->status = ANIME_PLAY_STATUS_STOPPED;
}


