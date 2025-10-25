#include "terminal.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>

static struct termios original_termios;
static int terminal_initialized = 0;
static int is_kitty_terminal = 0;

/**
 * @brief Initialize terminal for interactive mode
 */
void terminal_init(void) {
    if (terminal_initialized) {
        return;
    }

    // detect if running in Kitty terminal
    is_kitty_terminal = is_term_kitty();

    // save original terminal settings
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // fix TERM if needed (xterm-kitty can cause problems)
    char *term = getenv("TERM");
    if (term && strcmp(term, "xterm-kitty") == 0) {
        if (system("infocmp xterm-kitty > /dev/null 2>&1") != 0) {
            setenv("TERM", "xterm-256color", 1);
        }
    }

    // save screen (alternate screen buffer)
    system("tput smcup");

    // hide cursor
    terminal_hide_cursor();

    terminal_initialized = 1;
}

/**
 * @brief Restore terminal to original state
 */
void terminal_restore(void) {
    if (!terminal_initialized) {
        return;
    }

    // clean Kitty images if possible
    if (is_kitty_terminal) {
        terminal_clear_kitty_images();
    }

    // restore screen
    system("tput rmcup");

    // restore termios
    if (tcsetattr(STDIN_FILENO, TCSANOW, &original_termios) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }

    // show cursor
    terminal_show_cursor();

    fflush(stdout);
    terminal_initialized = 0;
}

/**
 * @brief Get terminal size
 */
TermSize terminal_get_size(void) {
    struct winsize ws;
    TermSize size = {24, 80}; // default size

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        size.rows = ws.ws_row;
        size.cols = ws.ws_col;
    }

    return size;
}

/**
 * @brief Get a single character input without echo
 */
int terminal_getch(void) {
    struct termios oldt, newt;
    char ch;

    // get current terminal settings
    tcgetattr(STDIN_FILENO, &oldt);

    // copy settings to modify
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // disable canonical mode and echo

    // apply new settings
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // read a single character
    ch = getchar();

    // restore original terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}

/**
 * @brief Clear the entire screen
 */
void terminal_clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

/**
 * @brief Move cursor to specified position
 */
void terminal_move_cursor(int row, int col) {
    printf("\033[%d;%dH", row, col);
    fflush(stdout);
}

/**
 * @brief Clear current line
 */
void terminal_clear_line(void) {
    printf("\033[K");
    fflush(stdout);
}

/**
 * @brief Show cursor
 */
void terminal_show_cursor(void) {
    printf("\033[?25h");
    fflush(stdout);
}

/**
 * @brief Hide cursor
 */
void terminal_hide_cursor(void) {
    printf("\033[?25l");
    fflush(stdout);
}

/**
 * @brief Show image in terminal using Kitty graphics protocol
 */
void terminal_show_image_kitty(const char *path, int x, int y) {
    if (!path) return;

    printf("\033[s");  // Save cursor position
    fflush(stdout);

    // convert int to string
    char xbuf[16], ybuf[16];
    snprintf(xbuf, sizeof(xbuf), "%d", x);
    snprintf(ybuf, sizeof(ybuf), "%d", y);
    const char *cmdparts[] = {
        "kitty +kitten icat --no-trailing-newline --silent --place 0x0@",
        xbuf,
        "x",
        ybuf, 
        " \"",
        path,
        "\""
    };
    char *cmd = alloc_concat(cmdparts, sizeof(cmdparts)/sizeof(cmdparts[0]));
    run_cmd(cmd);
    free(cmd);

    printf("\033[u");  // Restore cursor position
    fflush(stdout);
}

/**
 * @brief Clear all Kitty graphics
 */
void terminal_clear_kitty_images(void) {
    run_cmd("kitty +kitten icat --clear");
}

/**
 * @brief Cleanup function for exit/signals
 */
void terminal_cleanup(void) {
    terminal_restore();
    exit(0);
}

/**
 * @brief Signal handler wrapper
 */
static void signal_handler(int signo) {
    (void)signo;  // Unused
    terminal_cleanup();
}

/**
 * @brief Setup signal handlers for clean exit
 * 
 * Call this after terminal_init() to ensure proper cleanup on Ctrl+C
 */
void terminal_setup_signals(void) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    atexit(terminal_cleanup);
}
