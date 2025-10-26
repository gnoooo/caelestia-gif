#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cjson/cJSON.h>
#include <string.h>

#include "color.h"
#include "utils.h"


/*
 * Installation script for the caelestia-sessiongif package
 *
 *
 *  1. If not created yet, create environment variable CAELESTIA_GIFS_FOLDER
 *      by default pointing to $HOME/Pictures/CaelestiaGifs/
 *
 *  2. Backup shell.json
 *      - if shell.json does not exist, return
 *      - If shell.bak.json exists, do nothing (the aim is to keep the first backup only)
 *      - If shell.back.json does not exist, create it as a copy of shell.json
 *    
 *  3. Edit $HOME/.config/caelestia/shell.json
 *      - If the file does not exist, create it with default settings:
 *          {
 *              "paths": {
 *                  "sessionGif": "$HOME/$CAELESTIA_GIFS_FOLDER"
 *               }
 *          }
 *      - If the file exists, search for the "paths" section:
 *          - If "paths" section exists, search for "sessiongif" entry:
 *          - If "sessiongif" entry exists, edit its value to "$HOME/$CAELESTIA_GIFS_FOLDER/sessionGifs/"
 *          - Else add the entry with the value "$HOME/$CAELESTIA_GIFS_FOLDER/sessionGifs/"
 *      - Else add the "paths" section with the "sessiongif" entry and value "$HOME/$CAELESTIA_GIFS_FOLDER/sessiongifs/"
 *
 *  4. Print success message
 */

/**
 * Create the CAELESTIA_GIFS_FOLDER environment variable if it does not exist.
 * The default value is $HOME/Pictures/CaelestiaGifs
 */
int create_env_variable(void) {
    // get HOME env variable
    const char *home = getenv("HOME");
    if (home == NULL) {
        fprintf(stderr, FG_RED "Error: HOME environment variable not set.\n" FG_DEFAULT);
        return -1;
    }

    // check if CAELESTIA_GIFS_FOLDER is set
    const char *gif_folder = getenv("CAELESTIA_GIFS_FOLDER");
    if (gif_folder == NULL) {
        // not set, create it with default value
        const char *pathparts[] = {home, "/Pictures/CaelestiaGifs"};
        char *path = alloc_concat(pathparts, 2);
        setenv("CAELESTIA_GIFS_FOLDER", path, 1);
    }

    return 0;
}

/**
 * Backup the shell.json file if it exists and no backup exists yet.
 */
int backup_shell_json(void) {
    // get HOME env variable
    const char *home = getenv("HOME");
    if (home == NULL) {
        fprintf(stderr, FG_RED "Error: HOME environment variable not set.\n" FG_DEFAULT);
        return -1;
    }

    // shell.json path
    const char *shell_json_pathparts[] = {home, "/.config/caelestia/shell.json"};
    char *shell_json_path = alloc_concat(shell_json_pathparts, 2);
    
    // backup path
    const char *backup_pathparts[] = {home, "/.config/caelestia/shell.bak.json"};
    char *backup_path = alloc_concat(backup_pathparts, 2);
    
    // check if shell.json exists
    FILE *shell_file = fopen(shell_json_path, "r");
    if (shell_file == NULL) {
        // shell.json does not exist, nothing to backup
        printf("shell.json does not exist, skipping backup.\n");
    } else {
        fclose(shell_file);
        FILE *backup_file = fopen(backup_path, "r");
        if (backup_file != NULL) {
            // backup already exists, do nothing
            fclose(backup_file);
            printf("Backup already exists, skipping backup creation.\n");
        } else {
            // create backup
            const char *cmdparts[] = {"cp ", shell_json_path, " ", backup_path};
            char *cmd = alloc_concat(cmdparts, 4);
            
            int result = system(cmd);
            if (result != 0) {
                fprintf(stderr, FG_RED "Error creating backup of shell.json.\n" FG_DEFAULT);
                return -1;
            }
            printf("Backup created successfully.\n");
        }
    }
    return 0;
}

/**
 * Substitute environment variables in the source string.
 * Currently supports only $HOME.
 *
 * @param dest Destination buffer
 * @param size Size of the destination buffer
 * @param src Source string
 */
void sub_variables(char *dest, size_t size, const char *src) {
    // get HOME env variable 
    const char *home = getenv("HOME");

    // simple substitution logic
    char buffer[1024] = {0};
    const char *p = src;
    char *b = buffer;

    // substitute $HOME
    while (*p && (long unsigned int)(b - buffer) < sizeof(buffer) - 1) {
        if (strncmp(p, "$HOME", 5) == 0) {
            strncpy(b, home, sizeof(buffer) - (b - buffer) - 1);
            b += strlen(home);
            p += 5;
        } else {
            *b++ = *p++;
        }
    }
    *b = '\0';
    strncpy(dest, buffer, size);
}

/**
 * Edit the shell.json file to set the sessionGif path.
 * If the file does not exist, create it with default values.
 */
int edit_shell_json(void) {
    // get shell.json path
    const char *shell_json_pathparts[] = {getenv("HOME"), "/.config/caelestia/shell.json"};
    char *shell_json_path = alloc_concat(shell_json_pathparts, 2);

    // get HOME env variable
    const char *home = getenv("HOME");
    if (!home) { 
        fprintf(stderr, FG_RED "Error: HOME env not set\n" FG_DEFAULT); 
        return -1; 
    }
    const char *cae = getenv("CAELESTIA_GIFS_FOLDER");

    // open shell.json
    FILE *fp = fopen(shell_json_path, "r");
    if (!fp) {
        // file does not exist: create default JSON
        cJSON *root = cJSON_CreateObject();
        cJSON *paths = cJSON_CreateObject();
        cJSON_AddItemToObject(root, "paths", paths);

        // determine base directory
        char *base_dir = NULL;
        if (cae) {
            // env variable is set, use it
            base_dir = strdup(cae);
        } else {
            // env variable not set, use default
            const char *default_parts[] = {home, "/Pictures/CaelestiaGifs"};
            base_dir = alloc_concat(default_parts, 2);
        }

        if (!base_dir) {
            fprintf(stderr, FG_RED "Memory allocation failed.\n" FG_DEFAULT);
            cJSON_Delete(root);
            return -1;
        }

        const char *path_parts[] = {base_dir, "/sessionGif"};
        char *session_gif_path = alloc_concat(path_parts, 2);

        if (!session_gif_path) {
            fprintf(stderr, FG_RED "Memory allocation failed.\n" FG_DEFAULT);
            free(base_dir);
            cJSON_Delete(root);
            return -1;
        }

        // add the final revolved path to JSON
        cJSON_AddStringToObject(paths, "sessionGif", session_gif_path);

        // write to file
        char *string = cJSON_Print(root);
        fp = fopen(shell_json_path, "w");
        if (fp) {
            fputs(string, fp);
            fclose(fp);
        }

        // cleanup
        free(base_dir);
        free(session_gif_path);
        cJSON_free(string);
        cJSON_Delete(root);
        return 0;
    }

    // file exists
    fseek(fp, 0, SEEK_END);
    long unsigned int fsize = ftell(fp);
    rewind(fp);

    char *buffer = malloc(fsize + 1);
    if (!buffer) {
        fprintf(stderr, FG_RED "Memory allocation failed.\n" FG_DEFAULT);
        fclose(fp);
        return -1;
    }

    size_t len = fread(buffer, 1, fsize, fp);
    buffer[len] = '\0';
    fclose(fp);

    if (len != fsize) {
        fprintf(stderr, FG_YELLOW "Warning: expected %ld bytes, read %zu bytes\n" FG_DEFAULT, fsize, len);
    }

    cJSON *json = cJSON_Parse(buffer);
    if (!json) {
        const char *err = cJSON_GetErrorPtr();
        fprintf(stderr, FG_RED "Error parsing JSON before: %.40s\n" FG_DEFAULT, err ? err : "(unknown location)");
        free(buffer);
        return -1;
    }

    free(buffer);

    cJSON *paths = cJSON_GetObjectItem(json, "paths");
    if (!paths) {
        // create paths object
        paths = cJSON_CreateObject();
        cJSON_AddItemToObject(json, "paths", paths);
    }


    char *base_dir = NULL;
    if (cae) {
        // env variable is set, use it
        base_dir = strdup(cae);
    } else if (home) {
        // env variable not set, use default
        const char *default_parts[] = {home, "/Pictures/CaelestiaGifs/"};
        base_dir = alloc_concat(default_parts, 2);
    } else {
        fprintf(stderr, FG_RED "Error: HOME env not set\n" FG_DEFAULT);
        cJSON_Delete(json);
        return -1;
    }

    if (!base_dir) {
        fprintf(stderr, FG_RED "Memory allocation failed.\n" FG_DEFAULT);
        cJSON_Delete(json);
        return -1;
    }

    const char *path_parts[] = {base_dir, "/.current/session.gif"};
    char *session_gif_path = alloc_concat(path_parts, 2);

    if (!session_gif_path) {
        fprintf(stderr, FG_RED "Memory allocation failed for session gif path.\n" FG_DEFAULT);
        free(base_dir);
        cJSON_Delete(json);
        return -1;
    }

    cJSON *sessiongif = cJSON_GetObjectItem(paths, "sessionGif");
    if (sessiongif) {
        cJSON_ReplaceItemInObject(paths, "sessionGif", cJSON_CreateString(session_gif_path));
    } else {
        cJSON_AddStringToObject(paths, "sessionGif", session_gif_path);
    }

    free(base_dir);

    // save
    char *string = cJSON_Print(json);
    fp = fopen(shell_json_path, "w");
    if (fp){
        fputs(string, fp);
        fclose(fp);
    } else {
        fprintf(stderr, FG_RED "Error opening shell.json for writing.\n" FG_DEFAULT);
        cJSON_free(string);
        cJSON_Delete(json);
        return -1;
    }

    cJSON_free(string);
    cJSON_Delete(json);
    return 0;
}

int postinstall(void) {
    const char *home = getenv("HOME");
    if (home == NULL) {
        fprintf(stderr, FG_RED "Error: HOME environment variable not set.\n" FG_DEFAULT);
        return EXIT_FAILURE;
    }

    if (access("/usr/bin/caelestia-gif", X_OK) != 0) {
        fprintf(stderr, FG_RED "Error: caelestia-gif executable not found in /usr/bin/. Please install caelestia-sessiongif package first.\n" FG_DEFAULT);
        return EXIT_FAILURE;
    }

    if (create_env_variable() != 0) {
        return EXIT_FAILURE;
    }
    if (backup_shell_json() != 0) {
        return EXIT_FAILURE;
    }
    if (edit_shell_json() != 0) {
        return EXIT_FAILURE;
    }


    // Further steps would be implemented here...

    printf("Installation completed successfully.\n");
    return EXIT_SUCCESS;
}
