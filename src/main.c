#include "config.h"
#include "utils.h"
#include "filesystem.h"
#include "terminal.h"
#include "thumbnails.h"
#include "ui.h"
#include "gif_operations.h"
#include "args.h"
#include "postinstall.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // parse command-line arguments
    Args args = args_parse(argc, argv);
    
    // handle immediate actions (help, version, init)
    if (args.show_help) {
        if (args.session_mode) {
            args_print_help_session();
        } else if (args.media_mode) {
            args_print_help_media();
        } else {
            args_print_help_main();
        }
        return 0;
    }
    
    if (args.show_version) {
        args_print_version();
        return 0;
    }
    
    if (args.init_mode) {
        postinstall();
        return 0;
    }
    
    // validate that a mode was selected
    if (!args.session_mode && !args.media_mode && !args.cli_mode) {
        fprintf(stderr, "Error: No mode selected. Use -h for help.\n");
        return 1;
    }
    
    // initialize configuration
    Config *cfg = config_init();
    if (!cfg) {
        fprintf(stderr, "Error: Failed to initialize configuration\n");
        return 1;
    }
    
    // detect system capabilities
    config_set_has_magick(cfg, has_cmd("magick"));
    config_set_is_kitty(cfg, is_term_kitty());
    
    // ensure required directories exist
    if (ensure_dir(config_get_current_dir(cfg)) != 0) {
        fprintf(stderr, "Warning: Could not create current directory\n");
    }
    
    if (ensure_dir(config_get_gif_dir(cfg)) != 0) {
        fprintf(stderr, "Error: Could not create GIF directory\n");
        config_free(cfg);
        return 1;
    }
    
    // create thumbnail directories if using Kitty
    if (config_is_kitty(cfg)) {
        ensure_dir(config_get_thumb_cache_dir(cfg));
        ensure_dir(config_get_thumb_session_dir(cfg));
    }
    
    // generate thumbnails if Kitty and ImageMagick are available
    if (config_is_kitty(cfg) && config_has_magick(cfg)) {
        thumbnails_generate(cfg, args.regenerate);
    } else if (config_is_kitty(cfg) && !config_has_magick(cfg)) {
        printf("Note: ImageMagick not found, thumbnails will not be generated\n");
    }
    
    // scan for GIFs
    char **gifs = NULL;
    int ngifs = scan_gifs(config_get_gif_dir(cfg), &gifs);
    
    if (ngifs == 0) {
        fprintf(stderr, "No GIFs found in %s\n", config_get_gif_dir(cfg));
        printf("Please add GIF files to this directory.\n");
        config_free(cfg);
        return 1;
    }
    
    printf("Found %d GIF(s)\n", ngifs);
    
    // scan for thumbnails (if Kitty)
    char **thumbs = NULL;
    int nthumbs = 0;
    
    // only scan for thumbnails if in Kitty terminal
    if (config_is_kitty(cfg)) {
        nthumbs = scan_gifs(config_get_thumb_session_dir(cfg), &thumbs);
        if (nthumbs > 0) {
            printf("Found %d thumbnail(s)\n", nthumbs);
        }
    }
    
    // run the appropriate mode
    int result = 0;
    
    if (args.session_mode) {
        // initialize terminal for interactive mode
        terminal_init();
        terminal_setup_signals();
        
        // run interactive session selector
        int selected = ui_session_loop(cfg, gifs, thumbs, ngifs, nthumbs);
        
        // restore terminal
        terminal_restore();
        
        if (selected >= 0) {
            printf("Applied GIF: %s\n", ui_get_basename(gifs[selected]));
            result = 0;
        } else {
            printf("Selection cancelled\n");
            result = 0;
        }
    }
    else if (args.media_mode) {
        fprintf(stderr, "Error: Media mode not implemented yet\n");
        result = 1;
    }
    else if (args.cli_mode) {
        fprintf(stderr, "Error: CLI mode not implemented yet\n");
        result = 1;
    }
    
    // cleanup
    free_gif_list(gifs, ngifs);
    if (thumbs) {
        free_gif_list(thumbs, nthumbs);
    }
    config_free(cfg);
    
    return result;
}

