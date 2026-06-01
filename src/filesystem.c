#include "filesystem.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
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
    if (!path || !*path) {
        fprintf(stderr, "Error: Null or empty path passed to ensure_dir()\n");
        return -1;
    }

    char tmp[PATH_MAX];
    strncpy(tmp, path, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';

    // remove trailing slashes
    size_t len = strlen(tmp);
    while (len > 0 && tmp[len - 1] == '/') {
        tmp[--len] = '\0';
    }

    // iterate and create directories as needed
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (access(tmp, F_OK) != 0) {
                if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                    fprintf(stderr, "Error: Failed to create directory '%s': %s\n", tmp, strerror(errno));
                    return -1;
                }
            }
            *p = '/';
        }
    }

    // create the final directory
    if (access(tmp, F_OK) != 0) {
        if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
            fprintf(stderr, "Error: Failed to create directory '%s': %s\n", tmp, strerror(errno));
            return -1;
        }
    }

    return 0;
}

static int compare_paths(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
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

    struct dirent *ent;  // directory entry pointer
    char **list = NULL;  // list of GIF paths
    size_t cap = 0;      // capacity of the list
    size_t n = 0;        // number of GIFs found

    // iterate over directory entries
    while ((ent=readdir(d))) {
        // filter to regular files only
        if (ent->d_type != DT_REG && ent->d_type != DT_UNKNOWN) {
            continue;
        }

        const char *name = ent->d_name; // file name
        size_t len = strlen(name);      // length of file name

        // filter to .gif files only
        if (len<5 || strcasecmp(name+len-4,".gif")!=0) { 
            continue;
        }

        // ensure capacity
        if (n + 1 > cap) { 
            cap = cap ? cap * 2 : 32; 
            char **newlist = realloc(list, cap * sizeof(char*)); // allocate more space
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

    if (n > 1) {
        qsort(list, n, sizeof(char *), compare_paths);
    }

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
    if (!gifpath || !thumb_session_dir) {
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
