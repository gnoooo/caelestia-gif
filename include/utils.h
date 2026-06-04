#ifndef __UTILS_H__
#define __UTILS_H__

#include <stddef.h>

#define GREEN  "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE   "\033[0;34m"
#define RED    "\033[0;31m"
#define RESET  "\033[0m"

char* alloc_concat(const char **strings, size_t n);

int has_cmd(const char *name);
int run_cmd(const char *cmd);
int is_term_kitty(void);

#endif // __UTILS_H__
