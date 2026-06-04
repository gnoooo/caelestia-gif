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
#include <string.h>

int main(int argc, char *argv[]) {
    // parse command-line arguments
    Args args = args_parse(argc, argv);
    
    // handle immediate actions (help, version, init)
    if (args.show_help) {
        if (args.session_mode) {
            args_print_help_session();
        } else if (args.media_mode) {
            args_print_help_media();
        } else if (args.cli_mode) {
            args_print_help_cli();
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
    if (!args.no_kitty) config_set_is_kitty(cfg, is_term_kitty());
    
    // select active directories based on mode
    const char *active_gif_dir   = (args.media_mode || args.cli_media_mode)
                                   ? config_get_media_gif_dir(cfg)
                                   : config_get_gif_dir(cfg);
    const char *active_thumb_dir = (args.media_mode || args.cli_media_mode)
                                   ? config_get_thumb_media_dir(cfg)
                                   : config_get_thumb_session_dir(cfg);

    // CLI mode: apply a GIF directly without TUI
    if (args.cli_mode) {
        const char *typemode = args.cli_media_mode ? "media.gif" : "session.gif";

        // -l alone: list available GIFs
        if (args.cli_list && !args.cli_gif_path) {
            char **gifs = NULL;
            int ngifs = scan_gifs(active_gif_dir, &gifs);
            if (ngifs == 0) {
                printf("No GIFs found in %s\n", active_gif_dir);
            } else {
                for (int i = 0; i < ngifs; i++) {
                    printf("=> %s%3d%s  %s\n", YELLOW, i + 1, RESET, ui_get_basename(gifs[i]));
                }
                free_gif_list(gifs, ngifs);
            }
            config_free(cfg);
            return 0;
        }

        const char *gif_to_apply = args.cli_gif_path;
        char **gifs = NULL;
        int ngifs = 0;

        // -l <name>: find gif by name (with or without .gif) in the active directory
        if (args.cli_list) {
            ngifs = scan_gifs(active_gif_dir, &gifs);
            int found = 0;
            for (int i = 0; i < ngifs; i++) {
                const char *base = ui_get_basename(gifs[i]);
                size_t namelen = strlen(args.cli_gif_path);
                size_t baselen = strlen(base);
                if (strcmp(base, args.cli_gif_path) == 0 ||
                    (baselen == namelen + 4 &&
                     strncmp(base, args.cli_gif_path, namelen) == 0 &&
                     strcmp(base + namelen, ".gif") == 0)) {
                    gif_to_apply = gifs[i];
                    found = 1;
                    break;
                }
            }
            if (!found) {
                fprintf(stderr, "Error: GIF '%s' not found in %s\n",
                        args.cli_gif_path, active_gif_dir);
                free_gif_list(gifs, ngifs);
                config_free(cfg);
                return 1;
            }
        }

        if (ensure_dir(config_get_current_dir(cfg)) != 0) {
            fprintf(stderr, "Warning: Could not create current directory\n");
        }
        int r = gif_apply(gif_to_apply, config_get_current_dir(cfg), typemode);
        if (r == 0 && args.verbose) printf("Applied GIF: %s\n", gif_to_apply);
        if (gifs) free_gif_list(gifs, ngifs);
        config_free(cfg);
        return r == 0 ? 0 : 1;
    }

    // ensure required directories exist
    if (ensure_dir(config_get_current_dir(cfg)) != 0) {
        fprintf(stderr, "Warning: Could not create current directory\n");
    }

    if (ensure_dir(active_gif_dir) != 0) {
        fprintf(stderr, "Error: Could not create GIF directory\n");
        config_free(cfg);
        return 1;
    }

    // create thumbnail directories if using Kitty
    if (config_is_kitty(cfg)) {
        ensure_dir(config_get_thumb_cache_dir(cfg));
        ensure_dir(active_thumb_dir);
    }

    // generate thumbnails if Kitty and ImageMagick are available
    if (config_is_kitty(cfg) && config_has_magick(cfg)) {
        thumbnails_generate(cfg, active_gif_dir, active_thumb_dir, args.regenerate);
    } else if (config_is_kitty(cfg) && !config_has_magick(cfg)) {
        printf("Note: ImageMagick not found, thumbnails will not be generated\n");
    }

    // scan for GIFs
    char **gifs = NULL;
    int ngifs = scan_gifs(active_gif_dir, &gifs);

    if (ngifs == 0) {
        fprintf(stderr, "No GIFs found in %s\n", active_gif_dir);
        printf("Please add GIF files to this directory.\n");
        config_free(cfg);
        return 1;
    }

    if (args.verbose) {
        printf("Found %d GIF(s)\n", ngifs);
    }

    // scan for thumbnails (if Kitty)
    char **thumbs = NULL;
    int nthumbs = 0;

    // only scan for thumbnails if in Kitty terminal
    if (config_is_kitty(cfg)) {
        nthumbs = scan_gifs(active_thumb_dir, &thumbs);
        if (nthumbs > 0 && args.verbose) {
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
        int selected = ui_loop(cfg, gifs, thumbs, ngifs, nthumbs, "session");

        // restore terminal
        terminal_restore();

        if (selected >= 0) {
            gif_apply(gifs[selected], config_get_current_dir(cfg), "session.gif");
            if (args.verbose) {
                printf("Applied GIF: %s\n", ui_get_basename(gifs[selected]));
            }
            result = 0;
        } else {
            if (args.verbose) {
                printf("Selection cancelled\n");
            }
            result = 0;
        }
    }
    else if (args.media_mode) {
        // initialize terminal for interactive mode
        terminal_init();
        terminal_setup_signals();

        // run interactive media selector
        int selected = ui_loop(cfg, gifs, thumbs, ngifs, nthumbs, "media");

        // restore terminal
        terminal_restore();

        if (selected >= 0) {
            gif_apply(gifs[selected], config_get_current_dir(cfg), "media.gif");
            if (args.verbose) {
                printf("Applied GIF: %s\n", ui_get_basename(gifs[selected]));
            }
            result = 0;
        } else {
            if (args.verbose) {
                printf("Selection cancelled\n");
            }
            result = 0;
        }
    }
    else if (args.cli_mode) {
        // handled above before TUI setup
        result = 0;
    }
    
    // cleanup
    free_gif_list(gifs, ngifs);
    if (thumbs) {
        free_gif_list(thumbs, nthumbs);
    }
    config_free(cfg);
    
    return result;
}

