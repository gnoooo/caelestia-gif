#include "gif_operations.h"
#include "utils.h"
#include "filesystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>


int get_gif_delay(const char *gifpath) {
    if (!gifpath) {
        fprintf(stderr, "Error: Invalid parameter for get_gif_delay\n");
        return -1;
    }

    // construct command to get delay using ImageMagick
    const char *cmdparts[] = {
        "identify -format \"%T\n\" \"", gifpath, "\" 2>/dev/null"
    };
    char *cmd = alloc_concat(cmdparts, 3);

    // execute command and read output (only first line)
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        fprintf(stderr, "Error: Failed to run command to get GIF delay\n");
        free(cmd);
        return -1;
    }
    char buffer[16];
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        fprintf(stderr, "Error: No output from command to get GIF delay\n");
        pclose(fp);
        free(cmd);
        return -1;
    }
    pclose(fp);
    free(cmd);
    // parse delay value
    char *end;
    long delay_l = strtol(buffer, &end, 10);
    if (end == buffer || delay_l <= 0 || delay_l > INT_MAX) {
        fprintf(stderr, "Error: Invalid GIF delay value\n");
        return -1;
    }
    return (int)delay_l;
}

/**
 * @brief Apply the selected GIF as the current session GIF
 *
 * @param gifpath Path to the selected GIF file
 * @param base_dir Directory of Caelestia GIFs (root directory)
 * @param type session, media, etc. (finish with .gif)
 * @return int 0 on success, -1 on failure
 */
int gif_apply(const char *gifpath, const char *current_dir, const char *typemode) {
    if (!gifpath || !current_dir || !typemode) {
        fprintf(stderr, "Error: Invalid parameters for gif_apply\n");
        return -1;
    }

     
    if (ensure_dir(current_dir) != 0) {
        fprintf(stderr, "Error: Failed to ensure .current directory exists\n");
        return -1;
    }

    const char *targetparts[] = {current_dir, "/", typemode};
    char *target_path = alloc_concat(targetparts, 3);

    // delete existing current.gif (ignore errors if it doesn't exist)
    unlink(target_path);

    // convert gif with 1.5x speed
    int delay = get_gif_delay(gifpath);
    if (delay <= 0) {
        fprintf(stderr, "Error: Invalid GIF delay retrieved\n");
        free(target_path);
        return -1;
    }
    char new_delay[16];
    snprintf(new_delay, sizeof(new_delay), "%d", (int)(delay * SPEED_RATIO));

    const char *cmdparts[] = {
        "magick ", gifpath, " -coalesce -set delay ", new_delay, " -loop 0 ", target_path
    };
    char *cmd = alloc_concat(cmdparts, 6);
    
    int result = run_cmd(cmd);
    if (result != 0) {
        fprintf(stderr, "Error: Failed to copy %s to %s\n", gifpath, target_path);
        free(target_path);
        free(cmd);
        return -1;
    }

    // refresh Caelestia
    gif_refresh_caelestia();

    free(target_path);
    free(cmd);
    return 0;
}

/**
 * @brief Open a GIF file with the default system application
 *
 * @param gifpath Full path to the GIF file
 */
void gif_open(const char *gifpath) {
    if (!gifpath) {
        fprintf(stderr, "Error: Invalid parameter for gif_open\n");
        return;
    }

    // open with xdg-open (background process)
    const char *cmdparts[] = {"xdg-open \"", gifpath, "\" >/dev/null 2>&1 &"};
    char *cmd = alloc_concat(cmdparts, 3);
    if (!cmd) return;
    system(cmd);
    free(cmd);
}

/**
 * @brief Refresh Caelestia shell to pick up new GIF
 */
int gif_refresh_caelestia(void) {
    // check if caelestia command is available
    if (!has_cmd("caelestia")) {
        // not an error, caelestia might not be installed
        return -1;
    }

    // kill current caelestia shell
    int result = run_cmd("caelestia shell -k");
    if (result != 0) {
        fprintf(stderr, "Warning: Failed to kill caelestia shell\n");
    }

    // wait for caelestia to pick up the change
    // this is a bit hacky, but we need to wait a moment to ensure that caelestia has released the old GIF file before we try to restart the shellq
    usleep(500000);  // 0.5s

    // restart caelestia shell daemon
    result = run_cmd("caelestia shell -d >/dev/null 2>&1");
    if (result != 0) {
        fprintf(stderr, "Warning: Failed to restart caelestia shell\n");
        return -1;
    }

    return 0;
}
