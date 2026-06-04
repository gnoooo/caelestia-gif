#!/bin/bash

# Automatic installation script for caelestia-gif

set -e

# CONFIG
URL="https://github.com/gnoooo/caelestia-gif.git"
PKG_NAME="caelestia-gif"
INSTALL_DIR="/usr/bin"

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root (e.g., using sudo)"
    echo "Otherwise, the script can't copy to $INSTALL_DIR"
    exit 1
fi

# Clone repo
TMP_DIR=$(mktemp -d)        
git clone "$URL" "$TMP_DIR/$PKG_NAME"
cd "$TMP_DIR/$PKG_NAME" || exit 1

# Build
make

# Install binary and docs
install -Dm755 "./bin/$PKG_NAME" "$INSTALL_DIR/$PKG_NAME"
install -Dm644 "./README.md" "/usr/share/doc/$PKG_NAME/README.md"
install -Dm644 "./LICENSE" "/usr/share/doc/$PKG_NAME/LICENSE"

# Print success message
echo "$PKG_NAME installed to $INSTALL_DIR/$PKG_NAME"
echo "Documentation installed to /usr/share/doc/$PKG_NAME/"
echo "License installed to /usr/share/doc/$PKG_NAME/LICENSE"

# Init
echo "Running caelestia-gif --init..."
caelestia-gif --init
