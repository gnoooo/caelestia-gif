#ifndef __TERMINAL_H__
#define __TERMINAL_H__

typedef struct Terminal {
    int rows;
    int cols;
} TermSize;

void terminal_init(void);
void terminal_restore(void);
TermSize terminal_get_size(void);
int terminal_getch(void);
void terminal_clear_screen(void);
void terminal_move_cursor(int row, int col);
void terminal_clear_line(void);
void terminal_show_cursor(void);
void terminal_hide_cursor(void);
void terminal_show_image_kitty(const char *path, int x, int y);
void terminal_clear_kitty_images(void);
void terminal_cleanup(void);
void terminal_setup_signals(void);

#endif // __TERMINAL_H__
