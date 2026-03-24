#ifndef __GIF_OPERATIONS_H__
#define __GIF_OPERATIONS_H__

#define SPEED_RATIO 0.80

int gif_apply(const char *gifpath, const char *current_dir, const char *typemode);
void gif_open(const char *gifname, const char *gif_dir);
int gif_refresh_caelestia(void);

#endif // __GIF_OPERATIONS_H__
