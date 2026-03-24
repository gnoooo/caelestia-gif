#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

int ensure_dir(const char *path);
int scan_gifs(const char *dir, char ***out_list);
int scan_thumbs(const char *dir, char ***out_list);
char* thumb_path_for(const char *gifpath, const char *thumb_session_dir);
void open_gif(const char *gifname, const char *gif_dir);
void free_gif_list(char **list, int count);

#endif // __FILESYSTEM_H__
