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
        if (!path) return -1;
        setenv("CAELESTIA_GIFS_FOLDER", path, 1);
        free(path);
    }

    return 0;
}

/**
 * Backup the shell.json file if it exists and no backup exists yet.
 */
int backup_shell_json(void) {
    const char *home = getenv("HOME");
    if (home == NULL) {
        fprintf(stderr, FG_RED "Error: HOME environment variable not set.\n" FG_DEFAULT);
        return -1;
    }

    const char *shell_json_pathparts[] = {home, "/.config/caelestia/shell.json"};
    char *shell_json_path = alloc_concat(shell_json_pathparts, 2);
    if (!shell_json_path) return -1;

    const char *backup_pathparts[] = {home, "/.config/caelestia/shell.bak.json"};
    char *backup_path = alloc_concat(backup_pathparts, 2);
    if (!backup_path) { free(shell_json_path); return -1; }

    int result = 0;
    FILE *shell_file = fopen(shell_json_path, "r");
    if (shell_file == NULL) {
        printf("shell.json does not exist, skipping backup.\n");
    } else {
        fclose(shell_file);
        FILE *backup_file = fopen(backup_path, "r");
        if (backup_file != NULL) {
            fclose(backup_file);
            printf("Backup already exists, skipping backup creation.\n");
        } else {
            const char *cmdparts[] = {"cp ", shell_json_path, " ", backup_path};
            char *cmd = alloc_concat(cmdparts, 4);
            if (!cmd) {
                result = -1;
            } else {
                if (run_cmd(cmd) != 0) {
                    fprintf(stderr, FG_RED "Error creating backup of shell.json.\n" FG_DEFAULT);
                    result = -1;
                } else {
                    printf("Backup created successfully.\n");
                }
                free(cmd);
            }
        }
    }

    free(shell_json_path);
    free(backup_path);
    return result;
}

/**
 * Edit the shell.json file to set the sessionGif path.
 * If the file does not exist, create it with default values.
 */
int edit_shell_json(void) {
    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, FG_RED "Error: HOME env not set\n" FG_DEFAULT);
        return -1;
    }
    const char *cae = getenv("CAELESTIA_GIFS_FOLDER");

    const char *shell_json_pathparts[] = {home, "/.config/caelestia/shell.json"};
    char *shell_json_path = alloc_concat(shell_json_pathparts, 2);
    if (!shell_json_path) return -1;

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
            free(shell_json_path);
            cJSON_Delete(root);
            return -1;
        }

        const char *path_parts[] = {base_dir, "/sessionGif"};
        char *session_gif_path = alloc_concat(path_parts, 2);

        if (!session_gif_path) {
            fprintf(stderr, FG_RED "Memory allocation failed.\n" FG_DEFAULT);
            free(shell_json_path);
            free(base_dir);
            cJSON_Delete(root);
            return -1;
        }

        // add the final revolved path to JSON
        cJSON_AddStringToObject(paths, "sessionGif", session_gif_path);

        // write to file (convert tabs to 2 spaces for consistent formatting)
        char *string = cJSON_PrintBuffered(root, 4096, cJSON_True);
        if (string) {
            fp = fopen(shell_json_path, "w");
            if (fp) {
                for (char *p = string; *p; p++) {
                    if (*p == '\t') {
                        fputc(' ', fp);
                        fputc(' ', fp);
                    } else {
                        fputc(*p, fp);
                    }
                }
                fclose(fp);
            }
            cJSON_free(string);
        }

        // cleanup
        free(shell_json_path);
        free(base_dir);
        free(session_gif_path);
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
        free(shell_json_path);
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
        free(shell_json_path);
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
        base_dir = strdup(cae);
    } else {
        const char *default_parts[] = {home, "/Pictures/CaelestiaGifs/"};
        base_dir = alloc_concat(default_parts, 2);
    }

    if (!base_dir) {
        fprintf(stderr, FG_RED "Memory allocation failed.\n" FG_DEFAULT);
        free(shell_json_path);
        cJSON_Delete(json);
        return -1;
    }

    const char *path_parts[] = {base_dir, "/.current/session.gif"};
    char *session_gif_path = alloc_concat(path_parts, 2);

    if (!session_gif_path) {
        fprintf(stderr, FG_RED "Memory allocation failed for session gif path.\n" FG_DEFAULT);
        free(shell_json_path);
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
    free(session_gif_path);

    // save to file
    int prebuffer = 65536;
    cJSON_bool format = cJSON_True;

    char *string = cJSON_PrintBuffered(json, prebuffer, format);
    if (string == NULL) {
        fprintf(stderr, "Error: failed to serialize JSON\n");
        free(shell_json_path);
        cJSON_Delete(json);
        return -1;
    }

    fp = fopen(shell_json_path, "w");
    if (!fp) {
        fprintf(stderr, FG_RED "Error opening shell.json for writing.\n" FG_DEFAULT);
        free(shell_json_path);
        cJSON_free(string);
        cJSON_Delete(json);
        return -1;
    }

    for (char *p = string; *p; p++) {
        if (*p == '\t') {
            fputc(' ', fp);
            fputc(' ', fp);
        } else {
            fputc(*p, fp);
        }
    }

    fclose(fp);
    free(shell_json_path);
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
