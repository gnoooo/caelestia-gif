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
    const char *CAELESTIA_GIFS_FOLDER = getenv(ENV_CAELESTIA_GIFS_FOLDER);
    const char *CAELESTIA_THUMB_DIR   = getenv(ENV_CAELESTIA_THUMB_DIR);
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
        sessiongifs_folder = strdup(CAELESTIA_GIFS_FOLDER);
    } else {
        // otherwise, use default: $HOME/Pictures
        const char *defaultparts[] = {HOME, ENV_CAELESTIA_GIFS_FOLDER_DEFAULT}; // defined in config.h
        sessiongifs_folder = alloc_concat(defaultparts, sizeof(defaultparts) / sizeof(defaultparts[0]));
        if (!sessiongifs_folder) {
            // allocation failed
            fprintf(stderr, "Error: Failed to allocate memory for session gif folder path\n");
            free(cfg);
            return NULL;
        }
    }

    // determine thumbnail cache base directory
    char *thumb_base = NULL;
    if (CAELESTIA_THUMB_DIR) {
        thumb_base = strdup(CAELESTIA_THUMB_DIR);
    } else {
        const char *defaultthumb[] = {HOME, ENV_CAELESTIA_THUMB_DIR_DEFAULT};
        thumb_base = alloc_concat(defaultthumb, 2);
    }
    if (!thumb_base) {
        fprintf(stderr, "Error: Failed to allocate memory for thumbnail cache path\n");
        free(sessiongifs_folder);
        free(cfg);
        return NULL;
    }

    // construct full paths (to allocate the exact size of memory needed)
    const char *gifdirparts[]        = {sessiongifs_folder, "/sessionGif"};
    const char *mediagifdirparts[]   = {sessiongifs_folder, "/mediaGif"};
    const char *currentdirparts[]    = {sessiongifs_folder, "/.current"};
    const char *thumbcacheparts[]    = {thumb_base, ""};
    const char *thumbsessionparts[]  = {thumb_base, "/sessionGif"};
    const char *thumbmediaparts[]    = {thumb_base, "/mediaGif"};

    // dynamic allocation of paths
    cfg->gif_dir           = alloc_concat(gifdirparts,       sizeof(gifdirparts)       / sizeof(gifdirparts[0]));
    cfg->media_gif_dir     = alloc_concat(mediagifdirparts,  sizeof(mediagifdirparts)  / sizeof(mediagifdirparts[0]));
    cfg->current_dir       = alloc_concat(currentdirparts,   sizeof(currentdirparts)   / sizeof(currentdirparts[0]));
    cfg->thumb_cache_dir   = alloc_concat(thumbcacheparts,   sizeof(thumbcacheparts)   / sizeof(thumbcacheparts[0]));
    cfg->thumb_session_dir = alloc_concat(thumbsessionparts, sizeof(thumbsessionparts) / sizeof(thumbsessionparts[0]));
    cfg->thumb_media_dir   = alloc_concat(thumbmediaparts,   sizeof(thumbmediaparts)   / sizeof(thumbmediaparts[0]));

    // free temp variables
    free(sessiongifs_folder);
    free(thumb_base);

    if (!cfg->gif_dir || !cfg->media_gif_dir || !cfg->current_dir ||
        !cfg->thumb_cache_dir || !cfg->thumb_session_dir || !cfg->thumb_media_dir) {
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
    free(cfg->media_gif_dir);
    free(cfg->current_dir);
    free(cfg->thumb_cache_dir);
    free(cfg->thumb_session_dir);
    free(cfg->thumb_media_dir);
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

const char* config_get_media_gif_dir(const Config *cfg) {
    return cfg ? cfg->media_gif_dir : NULL;
}

const char* config_get_thumb_cache_dir(const Config *cfg) {
    return cfg ? cfg->thumb_cache_dir : NULL;
}

const char* config_get_thumb_session_dir(const Config *cfg) {
    return cfg ? cfg->thumb_session_dir : NULL;
}

const char* config_get_thumb_media_dir(const Config *cfg) {
    return cfg ? cfg->thumb_media_dir : NULL;
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

