#!/bin/bash

set -e  # stop on error

PKG_NAME="caelestia-gif"
INSTALL_DIR="/usr/bin"
DOC_DIR="/usr/share/doc/$PKG_NAME"
LICENSE_DIR="/usr/share/doc/$PKG_NAME/LICENSE"

# Check root
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root (e.g. with sudo)"
    exit 1
fi

# Remove binary
if [ -f "$INSTALL_DIR/$PKG_NAME" ]; then
    rm -f "$INSTALL_DIR/$PKG_NAME"
    echo "Removed binary: $INSTALL_DIR/$PKG_NAME"
else
    echo "No binary found at $INSTALL_DIR/$PKG_NAME"
fi

# Remove docs
if [ -d "$DOC_DIR" ]; then
    rm -rf "$DOC_DIR"
    echo "Removed documentation: $DOC_DIR"
else
    echo "No documentation found at $DOC_DIR"
fi

# Remove license
if [ -f "$LICENSE_DIR" ]; then
    rm -f "$LICENSE_DIR"
    echo "Removed license: $LICENSE_DIR"
else
    echo "No license found at $LICENSE_DIR"
fi

echo "$PKG_NAME successfully uninstalled."
