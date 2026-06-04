#ifndef __ARGS_H__
#define __ARGS_H__

typedef struct {
    int show_help;        // Display help information
    int show_version;     // Display version
    int init_mode;        // Run postinstall configuration
    int regenerate;       // Regenerate thumbnails
    int no_kitty;         // Disable Kitty terminal integration
    int verbose;          // Enable verbose output
    int session_mode;     // Run in session mode
    int media_mode;       // Run in media mode
    int cli_mode;         // Run in command-line interface mode
    int cli_media_mode;   // CLI: apply to media GIF
    int cli_session_mode; // CLI: apply to session GIF
    int cli_list;         // CLI: list available GIFs
    const char *cli_gif_path; // CLI: path to the GIF to apply
} Args;


Args args_parse(int argc, char *argv[]);
void args_print_help_main(void);
void args_print_help_session(void);
void args_print_help_media(void);
void args_print_help_cli(void);
void args_print_version(void);

#endif // __ARGS_H__
