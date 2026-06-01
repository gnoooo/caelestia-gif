#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <limits.h>

#define MAX_PATH_LEN (PATH_MAX + 256)

/**
 * @brief Allocate and concatenate an array of strings
 *
 * @param strings Array of strings to concatenate
 * @param n Number of strings
 * @return char* Concatenated string (must be freed by caller)
 */
char* alloc_concat(const char **strings, size_t n) {
    size_t total_len = 1; // for '\0'
    for (size_t i = 0; i < n; i++) {
        total_len += strlen(strings[i]);
    }
    char *buffer = malloc(total_len);
    if (!buffer) return NULL;

    char *p = buffer;
    for (size_t i = 0; i < n; i++) {
        size_t len = strlen(strings[i]);
        memcpy(p, strings[i], len);
        p += len;
    }
    *p = '\0';
    return buffer;
}

/**
 * @brief Check existence of executable
 *
 * @param name Command name to check
 * @return int 1 if exists, 0 if not
 */
int has_cmd(const char *name) {
    char cmd[MAX_PATH_LEN];
    snprintf(cmd, sizeof(cmd), "command -v %s >/dev/null 2>&1", name);
    return system(cmd) == 0;
}

/**
 * @brief Run a shell command and return exit status
 *
 * @param cmd Command to run
 * @return int Exit status of command
 */
int run_cmd(const char *cmd) {
    int rc = system(cmd);
    if(rc == -1) return -1;
    return WEXITSTATUS(rc);
}

/**
 * @brief Check if terminal is Kitty
 *
 * @return int 1 if terminal is Kitty, 0 otherwise
 */
int is_term_kitty() {
    const char *term = getenv("TERM");
    if (term && strstr(term, "kitty")) return 1;
    return 0;
}
