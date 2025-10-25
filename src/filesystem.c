#include "filesystem.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>


/**
 * @brief Ensure that a directory exists, create it if it doesn't.
 *
 * @param path The path to the directory.
 * @return int 0 on success, -1 on failure.
 */
int ensure_dir(const char *path) {
    struct stat st;

    // check if the directory already exists
    if (stat(path, &st) == 0) {
        // if it exists, check if it's a directory
        return S_ISDIR(st.st_mode) ? 0 : -1;
    }

    // create the directory with permissions rwx------
    return mkdir(path, 0700);
}


/**
 * @brief Scan directory for GIFs (return count and fill list). Caller must free list entries.
 *
 * @param dir Directory to scan
 * @param out_list Output list of GIF paths (allocated)
 * @return int Number of GIFs found
 */
int scan_gifs(const char *dir, char ***out_list) {
    DIR *d = opendir(dir);
    if (!d) { 
        *out_list = NULL; 
        return 0; 
    }

    struct dirent *ent; 
    char **list = NULL; 
    size_t cap = 0;
    size_t n = 0;

    while ((ent=readdir(d))) {
        // filter to regular files only
        if (ent->d_type != DT_REG && ent->d_type != DT_UNKNOWN) {
            continue;
        }

        const char *name = ent->d_name; 
        size_t len = strlen(name);

        // filter to .gif files only
        if (len<5 || strcasecmp(name+len-4,".gif")!=0) { 
            continue;
        }

        // ensure capacity
        if (n + 1 > cap) { 
            cap = cap ? cap * 2 : 32; 
            char **newlist = realloc(list, cap * sizeof(char*)); 
            if (!newlist) {
                // allocate error: free previously allocated entries
                for (size_t i = 0; i < n; i++) {
                    free(list[i]);
                }
                free(list);
                closedir(d);
                *out_list = NULL;
                return 0;
            }
            list = newlist;
        }

        // construct full path
        const char *fullpathparts[] = {dir, "/", name};
        char *fullpath=alloc_concat(fullpathparts, sizeof(fullpathparts)/sizeof(fullpathparts[0]));
        if (!fullpath) {
            // allocation error: free previously allocated entries
            for (size_t i = 0; i < n; i++) {
                free(list[i]);
            }
            free(list);
            closedir(d);
            *out_list = NULL;
            return 0;
        }
        list[n++] = fullpath;
    }
    closedir(d); 
    *out_list = list; 
    return (int)n;
}

/**
 * @brief Build thumbnail pathname from gif path
 *
 * @param gifpath Path to the original gif
 * @param out Output buffer for thumbnail path
 * @param outlen Length of output buffer
 * @return void
 */
char* thumb_path_for(const char *gifpath, const char *thumb_session_dir) {
    if (!gifpath || thumb_session_dir) {
        fprintf(stderr, "Error: at thumb_path_for: invalid arguments\n");
        return NULL;
    }

    // extract base name from gifpath
    const char *base = strrchr(gifpath,'/');
    if (base) {
        base++; // skip the '/'
    } else {
        base=gifpath; // no '/', already base name
    }

    // construct thumbnail path
    const char *parts[] = {thumb_session_dir, "/", base};
    return alloc_concat(parts, sizeof(parts)/sizeof(parts[0]));
}

/**
 * @brief Open a GIF file using the default system viewer.
 *
 * @param gifname Name of the GIF file to open.
 */
void open_gif(const char *gifname, const char *gif_dir) {
    if (!gifname) {
        fprintf(stderr, "Error: at open_gif: gifname is NULL\n");
        return;
    }

    const char *gifpathparts[] = {gif_dir, "/", gifname};
    char *gifpath = alloc_concat(gifpathparts, sizeof(gifpathparts)/sizeof(gifpathparts[0]));
    if (!gifpath) {
        fprintf(stderr, "Error: at open_gif: failed to allocate gifpath\n");
        return;
    }
    const char *cmdparts[] = {"xdg-open \"", gifpath, "\" >/dev/null 2>&1 &"};
    char *cmd = alloc_concat(cmdparts, sizeof(cmdparts)/sizeof(cmdparts[0]));
    if (!cmd) {
        fprintf(stderr, "Error: at open_gif: failed to allocate cmd\n");
        free(gifpath);
        return;
    }
    system(cmd);
}

/**
 * @brief Free a list of GIF paths.
 *
 * @param list List of GIF paths.
 * @param count Number of entries in the list.
 */
void free_gif_list(char **list, int count) {
    if (!list) {
        return;
    }
    for (int i = 0; i < count; i++) {
        free(list[i]);
    }
    free(list);
}
