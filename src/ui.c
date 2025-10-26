#include "ui.h"
#include "config.h"
#include "terminal.h"
#include "gif_operations.h"
#include "color.h"

#include <stdio.h>
#include <string.h>

#define CTITLE FG_CYAN BG_DEFAULT STYLE_BOLD
#define CSUBTITLE FG_GREEN BG_DEFAULT STYLE_ITALIC
#define CNUMBER FG_YELLOW BG_DEFAULT STYLE_BOLD
#define CSELECTED FG_BLACK BG_BLUE STYLE_BOLD
#define CNORMAL FG_DEFAULT BG_DEFAULT STYLE_NORMAL

/**
 * @brief Extract basename from path
 */
const char* ui_get_basename(const char *path) {
    if (!path) return "";
    
    const char *base = strrchr(path, '/');
    if (base) {
        return base + 1;
    }
    return path;
}


/**
 * @brief Draw header lines at top of screen
 */
int ui_draw_header(const char **lines, int n_lines, int margin) {
    int current_row = margin + 1;
    
    for (int i = 0; i < n_lines; i++) {
        terminal_move_cursor(current_row++, 1);
        for (int m = 0; m < margin; m++) {
            printf(" ");
        }
        printf("%s\n", lines[i]);
    }

    return current_row;
}

/**
 * @brief Initialize UI state
 */
static void ui_state_init(UIState *state, char **items, int count, int lines_per_item, int available_rows) {
    state->items = items;
    state->count = count;
    state->selected = 0;
    state->top = 0;
    state->prev_selected = 0;
    state->prev_top = -1;
    state->max_visible = available_rows / lines_per_item;
    state->need_full_redraw = 1;
}

/**
 * @brief Update viewport based on selection
 */
static void ui_update_viewport(UIState *state) {
    int current_page = state->selected / state->max_visible;
    int new_top = current_page * state->max_visible;
    
    // adjust if viewport would show beyond items
    if (new_top + state->max_visible > state->count) {
        new_top = state->count - state->max_visible;
        if (new_top < 0) new_top = 0;
    }
    
    state->top = new_top;
}

/**
 * @brief Draw a single item in the list
 */
static void ui_draw_item(const char *basename, int index, int is_selected, int is_kitty, int text_row, int margin) {
    (void)margin;
    (void)text_row;

    //terminal_move_cursor(text_row, 1);
    
    // cursor indicator
    if (is_selected) {
        printf(" > " CNUMBER "%d" CNORMAL, index + 1);
    } else {
        printf("   %d", index + 1);
    }
    
    // spacing for Kitty thumbnail
    if (is_kitty) {
        printf("         ");
    } else {
        printf(" ");
    }
    
    // item name with selection highlight
    if (is_selected) {
        printf(CSELECTED "%s" CNORMAL, basename);
    } else {
        printf(CNORMAL "%s", basename);
    }
}

/**
 * @brief Full screen redraw
 */
static void ui_full_redraw(
        UIState *state, 
        const char **header_lines, 
        int n_header_lines, 
        int margin, 
        int is_kitty, 
        int lines_per_item
    ) {
    // clear screen
    terminal_clear_screen();
    
    if (is_kitty) {
        terminal_clear_kitty_images();
    }
    
    // draw header
    int current_row = ui_draw_header(header_lines, n_header_lines, margin);
    
    int items_start_row = current_row;
    
    // draw visible items
    int visible_count = 0;
    for (int i=state->top; i<state->count && visible_count<state->max_visible; i++, visible_count++) {
        const char *basename = ui_get_basename(state->items[i]);
        int item_base_row = items_start_row + (visible_count * lines_per_item);
       
        int text_row = item_base_row+1 + (is_kitty ? 1 : 0);
        terminal_move_cursor(text_row, 1);
        ui_draw_item(basename, i, (i == state->selected), is_kitty, text_row, margin);

        // show thumbnail if Kitty
        if (is_kitty) {
            terminal_show_image_kitty(state->items[i], margin + 5, item_base_row);
        }
    }
    
    fflush(stdout);
    state->need_full_redraw = 0;
    state->prev_top = state->top;
}

/**
 * @brief Incremental redraw (only selection changed)
 */
static void ui_incremental_redraw(
        UIState *state, 
        int margin, 
        int n_header_lines, 
        int is_kitty, 
        int lines_per_item
    ) {

    int items_start_row = margin + n_header_lines + 2;
    
    // redraw previous selection (deselected)
    if (state->prev_selected >= state->top && 
        state->prev_selected < state->top + state->max_visible) {
        
        int text_row = items_start_row + 
                      ((state->prev_selected - state->top) * lines_per_item) + 
                      (is_kitty ? 1 : 0);
        
        terminal_move_cursor(text_row, 1);
        terminal_clear_line();
        
        const char *basename = ui_get_basename(state->items[state->prev_selected]);
        ui_draw_item(basename, state->prev_selected, 0, is_kitty, text_row, margin);
    }
    
    // redraw current selection (selected)
    if (state->selected >= state->top && 
        state->selected < state->top + state->max_visible) {
        
        int text_row = items_start_row + 
                      ((state->selected - state->top) * lines_per_item) + 
                      (is_kitty ? 1 : 0);
        
        terminal_move_cursor(text_row, 1);
        terminal_clear_line();
        
        const char *basename = ui_get_basename(state->items[state->selected]);
        ui_draw_item(basename, state->selected, 1, is_kitty, text_row, margin);
    }
    
    fflush(stdout);
}

/**
 * @brief Handle keyboard input
 * 
 * @return int 0 to continue, 1 to quit, 2 to select, 3 to open
 */
static int ui_handle_input(UIState *state, char **gifs, const Config *cfg) {
    (void)gifs;
    (void)cfg;

    int c = terminal_getch();
    
    if (c == 'q') {
        return 1;  // quit
    } else if (c == 27) {  // ESC sequence
        int c2 = terminal_getch();
        if (c2 == '[') {
            int c3 = terminal_getch();
            if (c3 == 'A' && state->selected > 0) {
                state->selected--;  // up arrow
            } else if (c3 == 'B' && state->selected < state->count - 1) {
                state->selected++;  // down arrow
            }
        }
    } else if (c == 'o') {
        return 3;  // open file
    } else if (c == '\n' || c == '\r') {
        return 2;  // select and apply
    }
    
    return 0;  // continue
}

/**
 * @brief Run interactive session selection loop
 */
int ui_session_loop(
        const Config *cfg, 
        char **gifs, 
        char **thumbs, 
        int ngifs, 
        int nthumbs,
        const char *typemode
    ) {
    if (!cfg || !gifs || ngifs == 0) return -1;
    
    // determine display list (thumbnails if available, otherwise GIFs)
    char **display_list = nthumbs > 0 ? thumbs : gifs;
    int display_count = nthumbs > 0 ? nthumbs : ngifs;
    
    int is_kitty = config_is_kitty(cfg);
    int margin = 1;
    
    // header lines
    const char *header_lines[] = {
        CTITLE "Available GIFs:" CNORMAL,
        CSUBTITLE "Use Up/Down to navigate, Enter to select, o to open, q to quit." CNORMAL
    };
    int n_header_lines = 2;
    
    // calculate layout
    TermSize term_size = terminal_get_size();
    int lines_per_item = is_kitty ? 4 : 2;
    int available_rows = term_size.rows - n_header_lines - margin - 2;
    
    // initialize UI state
    UIState state;
    ui_state_init(&state, display_list, display_count, lines_per_item, available_rows);
    
    // main loop
    while (1) {
        // update viewport
        ui_update_viewport(&state);
        
        // determine redraw type
        int viewport_changed = (state.top != state.prev_top);
        
        if (viewport_changed || state.need_full_redraw) {
            ui_full_redraw(&state, header_lines, n_header_lines, margin, 
                          is_kitty, lines_per_item);
        } else {
            ui_incremental_redraw(&state, margin, n_header_lines, is_kitty, 
                                 lines_per_item);
        }
        
        state.prev_selected = state.selected;
        
        // handle input
        int action = ui_handle_input(&state, gifs, cfg);
        
        if (action == 1) {
            // quit
            return -1;
        } else if (action == 2) {
            // select and apply
            gif_apply(gifs[state.selected], config_get_current_dir(cfg), typemode);
            return state.selected;
        } else if (action == 3) {
            // open GIF with default application
            const char *basename = ui_get_basename(gifs[state.selected]);
            gif_open(basename, config_get_gif_dir(cfg));
        }
    }
    
    return -1;
}


