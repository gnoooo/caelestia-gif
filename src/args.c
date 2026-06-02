#include "args.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef VERSION
#define VERSION "unknown"
#endif

/**
 * @brief Print version information
 */
void args_print_version(void) {
    printf("caelestia-gif version %s\n", VERSION);
}

/**
 * @brief Print main help message
 */
void args_print_help_main(void) {
    printf("usage: caelestia-gif [-h] [-v] [--init] COMMAND ...\n");
    printf("\n");
    printf("Main control script for Caelestia GIFs\n");
    printf("\n");
    printf("options:\n");
    printf("  -h, --help     show this help message and exit\n");
    printf("  -v, --version  print the current version\n");
    printf("  --init         run post-installation setup\n");
    printf("\n");
    printf("subcommands:\n");
    printf("  COMMAND        the subcommand to run\n");
    printf("    session      run in session mode (to change sessionGif)\n");
    printf("    media        run in media mode (to change mediaGif)\n");
    printf("    cli          run in CLI mode (NOT IMPLEMENTED YET)\n");
}


/**
 * @brief Print session mode help message
 */
void args_print_help_session(void) {
    printf("usage: caelestia-gif session [-h] [-r]\n");
    printf("\n");
    printf("Interactive session GIF selector\n");
    printf("\n");
    printf("options:\n");
    printf("  -h, --help        show this help message and exit\n");
    printf("  -r, --regenerate  regenerate all thumbnails\n");
    printf("  -k, --no-kitty    disable kitty terminal graphics support\n");
    printf("  --verbose         enable verbose output\n");
}

/**
 * @brief Print media mode help message
 */
void args_print_help_media(void) {
    printf("usage: caelestia-gif media [-h] [-r]\n");
    printf("\n");
    printf("Interactive media GIF selector (NOT IMPLEMENTED YET)\n");
    printf("\n");
    printf("options:\n");
    printf("  -h, --help        show this help message and exit\n");
    printf("  -r, --regenerate  regenerate all thumbnails\n");
    printf("  -k, --no-kitty    disable kitty terminal graphics support\n");
    printf("  --verbose         enable verbose output\n");
}

/**
 * @brief Initialize Args structure with default values
 */
static Args args_init_defaults(void) {
    Args args = {0};
    args.show_help = 0;
    args.show_version = 0;
    args.init_mode = 0;
    args.regenerate = 0;
    args.verbose = 0;
    args.session_mode = 0;
    args.media_mode = 0;
    args.cli_mode = 0;
    return args;
}

/**
 * @brief Parse subcommand-specific options
 *
 * @param args Pointer to Args structure to populate
 * @param argc Argument count
 * @param argv Argument vector
 * @param start_idx Index to start parsing from
 * @param subcommand Name of the subcommand being parsed
 */
static int args_parse_subcommand(
        Args *args, 
        int argc, 
        char *argv[], 
        int start_idx, 
        const char *subcommand
    ) {
    // parse subcommand-specific options
    for (int i = start_idx; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            // if help is requested, set show_help and return
            args->show_help = 1;
            return 0;
        } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--regenerate") == 0) {
            // if regenerate is requested, set regenerate flag
            args->regenerate = 1;
        } else if (strcmp(argv[i], "-k") == 0 || strcmp(argv[i], "--no-kitty") == 0) {
            // if no-kitty is requested, set no_kitty flag
            args->no_kitty = 1;
        } else if (strcmp(argv[i], "--verbose") == 0) {
            // if verbose is requested, set verbose flag
            args->verbose = 1;
        } else {
            // else unknown argument
            fprintf(stderr, "Error: Unknown argument for %s mode: %s\n", subcommand, argv[i]);
            return -1;
        }
    }
    return 0;
}

/**
 * @brief Parse command-line arguments
 *
 * @param argc Argument count
 * @param argv Argument vector
 */
Args args_parse(int argc, char *argv[]) {
    Args args = args_init_defaults();
    
    // no arguments, show help
    if (argc == 1) {
        args.show_help = 1;
        return args;
    }
    
    // parse arguments
    for (int i = 1; i < argc; i++) {
        // global options
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            // if help is requested, set show_help and return
            args.show_help = 1;
            return args;
        } 
        else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            // if version is requested, set show_version and return
            args.show_version = 1;
            return args;
        } 
        else if (strcmp(argv[i], "--init") == 0) {
            // if init is requested, set init_mode and return
            args.init_mode = 1;
            return args;
        }
        
        // subcommands
        else if (strcmp(argv[i], "session") == 0) {
            // if session mode is requested, set session_mode
            args.session_mode = 1;
            
            // parse session-specific options
            if (args_parse_subcommand(&args, argc, argv, i + 1, "session") != 0) {
                // if error in parsing, show help for session mode
                args.show_help = 1;
                args.session_mode = 0;
            }
            return args;
        }
        else if (strcmp(argv[i], "media") == 0) {
            args.media_mode = 1;
            if (args_parse_subcommand(&args, argc, argv, i + 1, "media") != 0) {
                args.show_help = 1;
                args.media_mode = 0;
            }
            return args;
        }
        else if (strcmp(argv[i], "cli") == 0) {
            args.cli_mode = 1;
            if (args_parse_subcommand(&args, argc, argv, i + 1, "cli") != 0) {
                args.show_help = 1;
                args.cli_mode = 0;
            }
            return args;
        }
        else {
            // else unknown command
            fprintf(stderr, "Error: Unknown command or option: %s\n", argv[i]);
            args.show_help = 1;
            return args;
        }
    }
    
    return args;
}
