# Caelestia GIF Manager (`caelestia-gif`)

![Caelestia GIF Manager logo](images/wide_logo.png)

A terminal user interface (TUI) written in C for managing GIFs (sessionGif and mediaGif) in the Caelestia shell environment.

## Table of Contents
[TOC]

---

## Introduction
Caelestia GIF Manager is a lightweight TUI tool designed to easily browse, preview and set GIF for your Caelestia shell session menu and media player. It supports thumbnail generation (via ImageMagick) and display previews directly in the Kitty terminal emulator. Use the `session` subcommand to select a session GIF quickly.

## Installation
### Commands
There are many different ways to install Caelestia GIF Manager:
1. From AUR (on Arch Linux)
```bash
yay -S caelestia-gif
caelestia-gif --init
# or
paru -S caelestia-gif
caelestia-gif --init
```

2. With the built-in Bash install script: (NOT IMPLEMENTED YET)
   - Using a single command (`curl`+`sh`): 
      ```bash
       curl -sSL https://gitlab.com/gnoooo/caelestia-gif/-/raw/master/install.sh?ref_type=heads | sh
      ```

3. From source (manual compilation):
```bash
git clone https://gitlab.com/gnoooo/caelestia-gif.git
cd caelestia-gif
make
sudo make install
caelestia-gif --init
```

### What is `install.sh`?
The repository will soon contain an `install.sh` file. This file is a simple Bash script that automates the installation of Caelestia GIF Manager. It will:
- Clone the repository from GitLab
- Compile the source code using `make`
- Install the compiled binary to `/usr/local/bin/` using install command
- Run `caelestia-gif --init` to set up the necessary configuration and directories


## Usage
To run Caelestia GIF Manager, simply execute the following command in your terminal:
```bash
caelestia-gif [-h] [-v] [--init] [subcommand] [flags] 
```


Where `[subcommand]` can be:
| Subcommand | Description                                 |
|------------|---------------------------------------------|
| session    | Opens the TUI to manage session GIFs.      |
| media      | Opens the TUI to manage media GIFs. (NOT IMPLEMENTED YET) |
| cli        | Command-line interface to set GIF directly without TUI. (NOT IMPLEMENTED YET) |



You can add some flags:
- `-h`, `--help`: Show help information for the command or subcommand.
- `-r`, `--regenerate`: Regenerate thumbnails for all GIFs in the directory.
- `-k`, `--no-kitty`: Force disable Kitty Graphics Protocol for GIF preview.
- `--verbose`: Enable verbose output for debugging purposes.

### Example
```bash
caelestia-gif session -r
```
In the TUI:
- Arrow keys: navigate
- Enter: select a GIF
- `o`: open the selected GIF file (alternative to Kitty preview)
- `q` or Ctrl+C: exit the TUI

## Configuration
### Post installation setup
To use this tool, you will need to have a specific configuration (see [directory structure section](#default-directory-structure)). Caelestia GIF Manager includes a setup helper, who can can be used with the command `caelestia-gif --init`. This command automates the configuration of Caelestia GIF Manager so the script runs perfectly. 

It :
- Creates the default Caelestia GIF directory (`~/Pictures/CaelestiaGifs/`) if it doesn't exist.
- Creates subdirectories for session and media GIFs under the main directory.
- Backs up existing configuration of Caelestia shell (`~/.config/caelestia/shell.json` renamed to `shell.bak.json`) if it exists.
- Creates a default configuration file for Caelestia shell if missing.
- Adds or updates the `sessionGif` entry (later `mediaGif` too) to point to your current GIF, by default:
```bash
    ~/Pictures/CaelestiaGifs/.current/session.gif"
```

### Environment Variables
You can customize the directory where Caelestia GIF Manager looks for GIFs by setting the following environment variables:
- `CAELESTIA_GIF_DIR`: Sets the base directory for storing GIFs.

Right now (1.0.2), the environment variable support is not fully implemented, so it's recommended to use the default directory structure or modify the source code directly. I will fix that in future releases.

## Dependencies
Caelestia GIF Manager relies on the following dependencies (which are, if you're using an AUR helper, automatically installed):
- Depends: 
    - `imagemagick` (for thumbnail generation)
    - `kitty` (for GIF preview using Graphics Protocol, optional but recommended)
    - `ncurses` (for some TUI functionality)
    - `cjson` (for JSON manipulation, used in `--init`)
    - `bash`
- Make depends:
    - `gcc`
    - `make`
    - `base-devel`

The command to install all (on Arch Linux):
```bash
sudo pacman -S imagemagick kitty ncurses cjson bash gcc make base-devel --needed
```

## Default directory structure
By default, Caelestia GIF Manager uses the following directory structure to store and manage GIFs:

```
~/
└── Pictures/
    ├── CaelestiaGifs/
    │   ├── sessionGif/
    │   │   └── [your_gifs]
    │   │
    │   ├── mediaGif/
    │   │   └── [your_gifs]
    │   │
    │   └── .current/
    │       ├── session.gif
    │       └── media.gif
    │
    └── .cache/
        └── caelestia_gifs_thumb/
            ├── sessionGif/
            └── mediaGif/
```

If you wish to change the default directory, don't use the env variable, it's not well implemented (yet?). Instead, you'll have to modify the source code and recompile it. (i'm sorry, i'll fix that later...)

If you're using Kitty (which is recommended), the Graphics Protocol will be used to display GIF preview. For this functionality, we have to generate thumbnails for each GIF using ImageMagick, because if not, the GIFs would be too large or too small to be displayed in the terminal. These images are stored in `$HOME/.cache/caelestia_gifs_thumb/sessionGif/` and `$HOME/.cache/caelestia_gifs_thumb/mediaGif/`.

These images are not generated each time, only when a new GIF is added to the directory.

## Uninstallation
To uninstall Caelestia GIF Manager, you can use your AUR helper:
```bash
yay -R caelestia-gif
# or
paru -R caelestia-gif
```

There is also a script to uninstall:
```bash
curl -sSL https://gitlab.com/gnoooo/caelestia-gif/-/raw/master/uninstall.sh?ref_type=heads | sh
```

Or you can manually remove the installed files:
```bash
sudo rm /usr/bin/caelestia-gif
sudo rm /usr/share/doc/caelestia-gif/README.md
sudo rm -r /usr/share/licences/caelestia-gif/LICENSE
```

---

## Licence
Licensed under the GPL-3.0-or-later License. See the [LICENSE](https://gitlab.com/gnoooo/caelestia-gif/-/blob/master/LICENSE) file for more details.

---

## Incoming
- Media GIF management (`caelestia-gif media` subcommand)
- Command-line interface (`caelestia-gif cli` subcommand)
- Better environment variable support for custom directories
- Improved installation script
- Memory optimizations
- Segmentation of code into multiple files for better maintainability (because right now it's a mess)
- Change current.gif by a symlink to avoid copying files
- Flag to force script to not use Kitty graphics protocol 
- Flag for verbose

