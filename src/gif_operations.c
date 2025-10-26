#include "gif_operations.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

/**
 * @brief Apply the selected GIF as the current session GIF
 *
 * @param gifpath Path to the selected GIF file
 * @param current_dir Directory where current.gif should be placed
 * @return int 0 on success, -1 on failure
 */
int gif_apply(const char *gifpath, const char *current_dir) {
    if (!gifpath || !current_dir) {
        fprintf(stderr, "Error: Invalid parameters for gif_apply\n");
        return -1;
    }

    // construct path to current.gif
    const char *currentgifparts[] = {current_dir, "/current.gif"};
    char *current_gif = alloc_concat(currentgifparts, 2);
    
    // delete existing current.gif (ignore errors if it doesn't exist)
    unlink(current_gif);

    // copy selected GIF to current.gif
    const char *cmdparts[] = {"cp ", gifpath, " ", current_gif};
    char *cmd = alloc_concat(cmdparts, 4);
    
    int result = run_cmd(cmd);
    if (result != 0) {
        fprintf(stderr, "Error: Failed to copy GIF to current.gif\n");
        return -1;
    }

    // refresh Caelestia
    gif_refresh_caelestia();

    return 0;
}

/**
 * @brief Open a GIF file with the default system application
 *
 * @param gifname Name of the GIF file to open
 * @param gif_dir Directory where the GIF file is located
 */
void gif_open(const char *gifname, const char *gif_dir) {
    if (!gifname || !gif_dir) {
        fprintf(stderr, "Error: Invalid parameters for gif_open\n");
        return;
    }

    // construct full path to GIF
    const char *gifpathparts[] = {gif_dir, "/", gifname};
    char *gifpath = alloc_concat(gifpathparts, 3);

    // open with xdg-open (background process)
    const char *cmdparts[] = {"xdg-open \"", gifpath, "\" >/dev/null 2>&1 &"};
    char *cmd = alloc_concat(cmdparts, 3);
    
    system(cmd);
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
    usleep(500000);  // 0.5s

    // restart caelestia shell daemon
    result = run_cmd("caelestia shell -d >/dev/null 2>&1");
    if (result != 0) {
        fprintf(stderr, "Warning: Failed to restart caelestia shell\n");
        return -1;
    }

    return 0;
}
