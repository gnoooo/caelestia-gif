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
    // allocate config struct
    Config *cfg = malloc(sizeof(Config));
    if (!cfg) {
        // allocation failed
        fprintf(stderr, "Failed to allocate memory for Config\n");
        return NULL;
    }

    // set default values
    cfg->thumb_size = THUMB_SIZE;
    cfg->has_magick = 0;
    cfg->is_kitty = 0;

    // get environment variables 
    // CAELESTIA_GIFS_FOLDER might not be set, so we handle that case by using HOME
    const char *CAELESTIA_GIFS_FOLDER = getenv("CAELESTIA_GIFS_FOLDER");
    const char *HOME = getenv("HOME");

    if (!HOME) {
        // HOME is required for default paths (generally should always be set in Unix-like systems)
        fprintf(stderr, "HOME environment variable not set\n");
        free(cfg);
        return NULL;
    }

    // determine root session gifs folder
    char *sessiongifs_folder = NULL;
    if (CAELESTIA_GIFS_FOLDER) {
        // if CAELESTIA_GIFS_FOLDER is set, use it
        sessiongifs_folder = CAELESTIA_GIFS_FOLDER ? strdup(CAELESTIA_GIFS_FOLDER) : NULL;
    } else {
        // otherwise, use default: $HOME/Pictures
        const char *defaultparts[] = {HOME, "/Pictures"};
        sessiongifs_folder = alloc_concat(defaultparts, sizeof(defaultparts) / sizeof(defaultparts[0]));
        if (!sessiongifs_folder) {
            // allocation failed
            fprintf(stderr, "Error: Failed to allocate memory for session gif folder path\n");
            free(cfg);
            return NULL;
        }
    }

    // construct full paths (to allocate the exact size of memory needed)
    const char *gifdirparts[] = {sessiongifs_folder, "/CaelestiaGifs/sessionGif"};
    const char *currentdirparts[] = {sessiongifs_folder, "/CaelestiaGifs/sessionGif/current"};
    const char *thumbcacheparts[] = {HOME, "/.cache/caelestia_gifs_thumb"};
    const char *thumbsessionparts[] = {HOME, "/.cache/caelestia_gifs_thumb/sessionGif"};

    // dynamic allocation of paths
    cfg->gif_dir = alloc_concat(gifdirparts, sizeof(gifdirparts) / sizeof(gifdirparts[0]));
    cfg->current_dir = alloc_concat(currentdirparts, sizeof(currentdirparts) / sizeof(currentdirparts[0]));
    cfg->thumb_cache_dir = alloc_concat(thumbcacheparts, sizeof(thumbcacheparts) / sizeof(thumbcacheparts[0]));
    cfg->thumb_session_dir = alloc_concat(thumbsessionparts, sizeof(thumbsessionparts) / sizeof(thumbsessionparts[0]));

    // free temp variable
    free(sessiongifs_folder);

    if (!cfg->gif_dir || !cfg->current_dir || !cfg->thumb_cache_dir || !cfg->thumb_session_dir) {
        // allocation failed
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

