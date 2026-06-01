#ifndef __UTILS_H__
#define __UTILS_H__

#include <stddef.h>

char* alloc_concat(const char **strings, size_t n);

int has_cmd(const char *name);
int run_cmd(const char *cmd);
int is_term_kitty(void);

#endif // __UTILS_H__
