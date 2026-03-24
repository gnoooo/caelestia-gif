#ifndef __THUMBNAILS_H__
#define __THUMBNAILS_H__

#include "config.h"

void thumbnails_generate(const Config *cfg, int clean_thumbs);
int thumbnails_exists(const char *gifpath, const Config *cfg);
int thumbnails_needs_regeneration(const char *gifpath, const char *thumbpath);

#endif // __THUMBNAILS_H__
