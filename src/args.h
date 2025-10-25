#ifndef __ARGS_H__
#define __ARGS_H__

typedef struct {
    int show_help;     // Display help information
    int show_version;  // Display version
    int init_mode;     // Run postinstall configuration
    int regenerate;    // Regenerate thumbnails
    int session_mode;  // Run in session mode
    int media_mode;    // Run in media mode
    int cli_mode;      // Run in command-line interface mode
} Args;


Args args_parse(int argc, char *argv[]);
void args_print_help_main(void);
void args_print_help_session(void);
void args_print_help_media(void);
void args_print_version(void);

#endif // __ARGS_H__
