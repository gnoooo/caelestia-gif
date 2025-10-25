# Makefile for caelestia-gif manager
# version
VERSION = 1.0.2

# compiler
CC := gcc

# Flags
CFLAGS := -Wall -Wextra -O2 -std=c11 -D_GNU_SOURCE -Iinclude -DVERSION=\"$(VERSION)\"
LDFLAGS := -lcjson

# Directories
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
INCLUDE_DIR := include

# Output binary
TARGET := $(BIN_DIR)/caelestia-gif

# Source files
SOURCES := $(wildcard $(SRC_DIR)/*.c)

# Object files
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

# Header files
HEADERS := $(wildcard $(INCLUDE_DIR)/*.h)


# Installation directory
DESTDIR ?= /usr

# Default target
all: $(TARGET)

GREEN := \033[0;32m
YELLOW := \033[0;33m
BLUE := \033[0;34m
RED := \033[0;31m
RESET := \033[0m

# Build the binary
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	@echo -e "$(BLUE)Linking $@..."
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo -e "Build complete: $@$(RESET)"
	@echo ""

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(OBJ_DIR)
	@echo -e "$(YELLOW)Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@
	@echo -e "Compiled: $@$(RESET)"
	@echo ""

# Create obj directory if it doesn't exist
$(OBJ_DIR):
	@echo -e "Creating $(OBJ_DIR) directory..."
	mkdir -p $@
	@echo ""

$(BIN_DIR):
	@echo -e "Creating $(BIN_DIR) directory..."
	mkdir -p $@
	@echo ""


# Clean up build files
clean:
	@echo -e "$(GREEN)Cleaning up..."
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo -e "Clean complete.$(RESET)"
	@echo ""

# Install the binary
install: $(TARGET)
	# mkdir -p /usr/bin
	@echo "Installing $(TARGET) to /usr/bin/..."
	install -Dm755 $(TARGET) $(DESTDIR)/bin/caelestia-gif
	@echo "Installation complete: $(DESTDIR)/bin/caelestia-gif"
	@echo ""
	@echo "You can now run 'caelestia-gif' from anywhere."
	@if [ "$(DESTDIR)" != "/usr" ]; then \
		echo ""; \
		echo "Note: Since you're installing to $(DESTDIR), make sure $(DESTDIR)/bin is in your PATH."; \
		echo "Add this to your shell config (.bashrc, .zshrc, etc.):"; \
		echo "    export PATH=\"$(DESTDIR)/bin:\$$PATH\""; \
	fi


uninstall:
	@echo "Removing $(DESTDIR)/bin/caelestia-gif..."
	rm -f $(DESTDIR)/bin/caelestia-gif
	@echo "Uninstall complete."

rebuild: clean all

info:
	@echo "Project: caelestia-gif"
	@echo "Version: $(VERSION)"
	@echo "Compiler: $(CC)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo ""
	@echo "Directories:"
	@echo "  Source: $(SRC_DIR)"
	@echo "  Objects: $(OBJ_DIR)"
	@echo "  Binary: $(BIN_DIR)"
	@echo "  Headers: $(INCLUDE_DIR)"
	@echo ""
	@echo "Sources:"
	@$(foreach src,$(SOURCES),echo "  $(src)";)
	@echo ""
	@echo "Objects:"
	@$(foreach obj,$(OBJECTS),echo "  $(obj)";)
	@echo ""
	@echo "Target: $(TARGET)"


debug: CFLAGS += -g -DDEBUG
debug: clean all
	@echo "Debug build complete with symbols"

run: $(TARGET)
	@$(TARGET) session

help-run: $(TARGET)
	@$(TARGET) --help

# Phony targets
.PHONY: all clean install uninstall rebuild info debug run help-run
