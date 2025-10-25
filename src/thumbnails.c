#include "thumbnails.h"
#include "config.h"
#include "utils.h"

#include "filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>

/**
 * @brief Generate a single thumbnail using ImageMagick
 * 
 * @param gifpath Path to source GIF
 * @param thumbpath Path to output thumbnail
 * @param thumb_size Size of thumbnail (width and height)
 * @return int 0 on success, -1 on error
 */
static int generate_single_thumbnail(const char *gifpath, const char *thumbpath, int thumb_size) {
    if (!gifpath || !thumbpath) { 
        return -1;
    }

    // convert thumb_size to string
    char thumb_size_str[16];
    snprintf(thumb_size_str, sizeof(thumb_size_str), "%d", thumb_size);

    // build command
    const char *parts[] = {
        "magick \"", gifpath,
        "\" -coalesce -resize ", thumb_size_str, "x", thumb_size_str,
        " \"", thumbpath, "\" >/dev/null 2>&1"
    };
    
    char *cmd = alloc_concat(parts, 8);
    if (!cmd) {
        fprintf(stderr, "Error: Failed to allocate memory for magick command\n");
        return -1;
    }

    int result = run_cmd(cmd);
    free(cmd);

    return result;
}

/**
 * @brief Check if a thumbnail needs regeneration
 */
int thumbnails_needs_regeneration(const char *gifpath, const char *thumbpath) {
    if (!gifpath || !thumbpath) {
        return 0;
    }

    struct stat st_thumb, st_gif;

    // if the thumbnail does not exist, we need to regenerate
    if (stat(thumbpath, &st_thumb) != 0) {
        return 1;
    }

    // if the gif does not exist, we cannot regenerate
    if (stat(gifpath, &st_gif) != 0) {
        return 0;
    }

    // compare modification times
    return (st_gif.st_mtime > st_thumb.st_mtime);
}

/**
 * @brief Check if a thumbnail exists for a given GIF
 */
int thumbnails_exists(const char *gifpath, const Config *cfg) {
    if (!gifpath || !cfg) {
        return 0;
    }

    char *thumbpath = thumb_path_for(gifpath, config_get_thumb_session_dir(cfg));
    if (!thumbpath) {
        return 0;
    }

    int exists = (access(thumbpath, F_OK) == 0);
    free(thumbpath);

    return exists;
}

/**
 * @brief Generate missing thumbnails for all GIFs in directory
 */
void thumbnails_generate(const Config *cfg, int clean_thumbs) {
    if (!cfg) {
        fprintf(stderr, "Error: NULL config passed to thumbnails_generate\n");
        return;
    }

    const char *gif_dir = config_get_gif_dir(cfg);
    const char *thumb_cache_dir = config_get_thumb_cache_dir(cfg);
    const char *thumb_session_dir = config_get_thumb_session_dir(cfg);
    int thumb_size = config_get_thumb_size(cfg);
    int has_magick = config_has_magick(cfg);

    // check if ImageMagick is available
    if (!has_magick) {
        fprintf(stderr, "Warning: ImageMagick not found, thumbnails will not be generated\n");
        return;
    }

    // create thumbnail directories if they don't exist
    if (ensure_dir(thumb_cache_dir) != 0 || ensure_dir(thumb_session_dir) != 0) {
        fprintf(stderr, "Warning: cannot create thumb dir %s: %s\n", 
                thumb_session_dir, strerror(errno));
        return;
    }

    // clean existing thumbnails if requested
    if (clean_thumbs) {
        char cmd[MAX_PATH_LEN * 2];
        snprintf(cmd, sizeof(cmd), "rm -f \"%s\"/*.gif 2>/dev/null || true", thumb_session_dir);
        run_cmd(cmd);
        printf("Cleaned existing thumbnails\n");
    }

    // scan GIF directory
    DIR *d = opendir(gif_dir);
    if (!d) {
        fprintf(stderr, "Warning: cannot open GIF directory %s: %s\n", 
                gif_dir, strerror(errno));
        return;
    }

    struct dirent *ent;
    int generated_count = 0;
    int regenerated_count = 0;

    while ((ent = readdir(d))) {
        // filter only regular files
        if (ent->d_type != DT_REG && ent->d_type != DT_UNKNOWN) {
            continue;
        }

        const char *name = ent->d_name;
        size_t len = strlen(name);

        // filter only .gif files
        if (len < 5 || strcasecmp(name + len - 4, ".gif") != 0) {
            continue;
        }

        // build full paths
        const char *gifparts[] = {gif_dir, "/", name};
        char *gifpath = alloc_concat(gifparts, 3);
        
        const char *thumbparts[] = {thumb_session_dir, "/", name};
        char *thumbpath = alloc_concat(thumbparts, 3);

        if (!gifpath || !thumbpath) {
            fprintf(stderr, "Error: Failed to allocate memory for paths\n");
            free(gifpath);
            free(thumbpath);
            continue;
        }

        // check if thumbnail exists
        if (access(thumbpath, F_OK) == 0) {
            // if it exists, check if it needs regeneration
            if (thumbnails_needs_regeneration(gifpath, thumbpath)) {
                if (generate_single_thumbnail(gifpath, thumbpath, thumb_size) == 0) {
                    printf("Regenerated thumbnail for %s\n", name);
                    regenerated_count++;
                }
            }
        } else {
            // thumbnail does not exist, generate it
            if (generate_single_thumbnail(gifpath, thumbpath, thumb_size) == 0) {
                printf("Generated thumbnail for %s\n", name);
                generated_count++;
            }
        }

        free(gifpath);
        free(thumbpath);
    }

    closedir(d);

    // delete orphaned thumbnails
    DIR *td = opendir(thumb_session_dir);
    if (!td) {
        if (generated_count > 0 || regenerated_count > 0) {
            printf("Summary: %d generated, %d regenerated\n", 
                   generated_count, regenerated_count);
        }
        return;
    }

    int removed_count = 0;

    while ((ent = readdir(td))) {
        // ignore hidden files
        if (ent->d_name[0] == '.') continue;

        size_t len = strlen(ent->d_name);
        if (len < 5 || strcasecmp(ent->d_name + len - 4, ".gif") != 0) {
            continue;
        }

        // check if corresponding GIF exists
        const char *src_parts[] = {gif_dir, "/", ent->d_name};
        char *srcpath = alloc_concat(src_parts, 3);
        
        if (!srcpath) {
            fprintf(stderr, "Error: Failed to allocate memory for source path\n");
            continue;
        }

        // if source GIF does not exist, remove thumbnail
        if (access(srcpath, F_OK) != 0) {
            const char *tpath_parts[] = {thumb_session_dir, "/", ent->d_name};
            char *tpath = alloc_concat(tpath_parts, 3);
            
            if (tpath) {
                unlink(tpath);
                printf("Removed orphan thumbnail %s\n", ent->d_name);
                removed_count++;
                free(tpath);
            }
        }

        free(srcpath);
    }

    closedir(td);

    // print summary
    if (generated_count > 0 || regenerated_count > 0 || removed_count > 0) {
        printf("Summary: %d generated, %d regenerated, %d removed\n",
               generated_count, regenerated_count, removed_count);
    }
}


