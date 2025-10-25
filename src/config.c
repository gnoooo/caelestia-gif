#include "config.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Initialize configuration and paths
 *
 * @return Config* allocated configuration struct (must be freed with config_free)
 */
Config* config_init(void) {
    Config *cfg = malloc(sizeof(Config));
    if (!cfg) {
        fprintf(stderr, "Failed to allocate memory for Config\n");
        return NULL;
    }

    cfg->thumb_size = THUMB_SIZE;
    cfg->has_magick = 0;
    cfg->is_kitty = 0;

    const char *CAELESTIA_GIFS_FOLDER = getenv("CAELESTIA_GIFS_FOLDER");
    const char *HOME = getenv("HOME");

    if (!HOME) {
        fprintf(stderr, "HOME environment variable not set\n");
        free(cfg);
        return NULL;
    }

    // determine root session gifs folder
    const char *sessiongifs_folder;
    char *default_path = NULL;
    if (CAELESTIA_GIFS_FOLDER) {
        sessiongifs_folder = CAELESTIA_GIFS_FOLDER;
    } else {
        const char *defaultparts[] = {HOME, "/Pictures"};
        default_path = alloc_concat(defaultparts, sizeof(defaultparts) / sizeof(defaultparts[0]));
        if (!default_path) {
            fprintf(stderr, "Error: Failed to allocate memory for default path\n");
            free(cfg);
            return NULL;
        }
        sessiongifs_folder = default_path;
    }

    const char *gifdirparts[] = {sessiongifs_folder, "/CaelestiaGifs/sessionGif"};
    const char *currentdirparts[] = {sessiongifs_folder, "/CaelestiaGifs/sessionGif/current"};
    const char *thumbcacheparts[] = {HOME, "/.cache/caelestia_gifs_thumb"};
    const char *thumbsessionparts[] = {HOME, "/.cache/caelestia_gifs_thumb/sessionGif"};

    cfg->gif_dir = alloc_concat(gifdirparts, sizeof(gifdirparts) / sizeof(gifdirparts[0]));
    cfg->current_dir = alloc_concat(currentdirparts, sizeof(currentdirparts) / sizeof(currentdirparts[0]));
    cfg->thumb_cache_dir = alloc_concat(thumbcacheparts, sizeof(thumbcacheparts) / sizeof(thumbcacheparts[0]));
    cfg->thumb_session_dir = alloc_concat(thumbsessionparts, sizeof(thumbsessionparts) / sizeof(thumbsessionparts[0]));

    free(default_path);

    if (!cfg->gif_dir || !cfg->current_dir || !cfg->thumb_cache_dir || !cfg->thumb_session_dir) {
        fprintf(stderr, "Error: Failed to allocate memory for config paths\n");
        config_free(cfg);
        return NULL;
    }

    return cfg;
}

/**
 * @brief Free configuration struct
 *
 * @param cfg Config struct to free
 */
void config_free(Config *cfg) {
    if (!cfg) return;

    free(cfg->gif_dir);
    free(cfg->current_dir);
    free(cfg->thumb_cache_dir);
    free(cfg->thumb_session_dir);
    free(cfg);
}

// Setters
void config_set_has_magick(Config *cfg, int value) {
    if (cfg) cfg->has_magick = value;
}

void config_set_is_kitty(Config *cfg, int value) {
    if (cfg) cfg->is_kitty = value;
}

// Getters
const char* config_get_gif_dir(const Config *cfg) {
    return cfg ? cfg->gif_dir : NULL;
}

const char* config_get_thumb_cache_dir(const Config *cfg) {
    return cfg ? cfg->thumb_cache_dir : NULL;
}

const char* config_get_thumb_session_dir(const Config *cfg) {
    return cfg ? cfg->thumb_session_dir : NULL;
}

const char* config_get_current_dir(const Config *cfg) {
    return cfg ? cfg->current_dir : NULL;
}

int config_get_thumb_size(const Config *cfg) {
    return cfg ? cfg->thumb_size : THUMB_SIZE;
}

int config_has_magick(const Config *cfg) {
    return cfg ? cfg->has_magick : 0;
}

int config_is_kitty(const Config *cfg) {
    return cfg ? cfg->is_kitty : 0;
}

