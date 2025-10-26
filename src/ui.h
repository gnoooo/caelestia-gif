#ifndef __UI_H__
#define __UI_H__

#include "config.h"

typedef struct {
    char **items;          // List of items to display
    int count;             // Number of items
    int selected;          // Currently selected index
    int top;               // First visible item index
    int prev_selected;     // Previously selected index
    int prev_top;          // Previous top index
    int max_visible;       // Maximum visible items on screen
    int need_full_redraw;  // Flag to force full screen redraw
} UIState;

int ui_session_loop(const Config *cfg, char **gifs, char **thumbs, int ngifs, int nthumbs, const char *typemode);
int ui_draw_header(const char **lines, int n_lines, int margin);
const char* ui_get_basename(const char *path);

#endif // __UI_H__
