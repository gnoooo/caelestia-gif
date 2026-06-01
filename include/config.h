#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <limits.h>

#define MAX_PATH_LEN (PATH_MAX + 256)
#define THUMB_SIZE 48

#define ENV_CAELESTIA_GIFS_FOLDER "CAELESTIA_GIFS_FOLDER"
#define ENV_CAELESTIA_GIFS_FOLDER_DEFAULT "/Pictures/CaelestiaGifs" // by default, HOME/...

typedef struct {
    char *gif_dir;
    char *thumb_cache_dir;
    char *thumb_session_dir;
    char *current_dir;
    int thumb_size;
    int has_magick;
    int is_kitty;
} Config;


Config* config_init(void);
void config_free(Config *cfg);

const char* config_get_gif_dir(const Config *cfg);
const char* config_get_thumb_cache_dir(const Config *cfg);
const char* config_get_thumb_session_dir(const Config *cfg);
const char* config_get_current_dir(const Config *cfg);
int config_get_thumb_size(const Config *cfg);
int config_has_magick(const Config *cfg);
int config_is_kitty(const Config *cfg);

void config_set_has_magick(Config *cfg, int v);
void config_set_is_kitty(Config *cfg, int v);

#endif // __CONFIG_H__
