# SlyMail - QWK Offline Mail Reader
# Makefile for Linux/Mac/BSD (ncurses) and Windows (conio + Win32 Console API)

# Program names
TARGET = slymail
CONFIG_TARGET = config

# Source files
SRCDIR = src

# Header files (for dependency tracking)
HEADERS = $(SRCDIR)/terminal.h $(SRCDIR)/terminal_ncurses.h $(SRCDIR)/terminal_win32.h \
          $(SRCDIR)/cp437defs.h $(SRCDIR)/colors.h $(SRCDIR)/program_info.h \
          $(SRCDIR)/qwk.h $(SRCDIR)/settings.h $(SRCDIR)/ui_common.h \
          $(SRCDIR)/file_browser.h $(SRCDIR)/msg_list.h $(SRCDIR)/msg_reader.h \
          $(SRCDIR)/msg_editor.h $(SRCDIR)/settings_dialog.h $(SRCDIR)/theme.h \
          $(SRCDIR)/bbs_colors.h $(SRCDIR)/utf8_util.h $(SRCDIR)/voting.h \
          $(SRCDIR)/remote_systems.h $(SRCDIR)/search.h $(SRCDIR)/text_input.h \
          $(SRCDIR)/text_utils.h $(SRCDIR)/ansi_render.h $(SRCDIR)/file_dir_utils.h

# Object files
OBJDIR = obj

# SlyMail objects
SLYMAIL_OBJECTS = $(OBJDIR)/main.o $(OBJDIR)/qwk.o $(OBJDIR)/settings.o $(OBJDIR)/msg_reader.o \
                  $(OBJDIR)/msg_editor.o $(OBJDIR)/settings_dialog.o $(OBJDIR)/terminal.o \
                  $(OBJDIR)/terminal_common.o $(OBJDIR)/file_dir_utils.o \
                  $(OBJDIR)/ui_common.o $(OBJDIR)/theme.o $(OBJDIR)/file_browser.o $(OBJDIR)/msg_list.o \
                  $(OBJDIR)/bbs_colors.o $(OBJDIR)/utf8_util.o $(OBJDIR)/voting.o \
                  $(OBJDIR)/remote_systems.o $(OBJDIR)/search.o $(OBJDIR)/text_input.o \
                  $(OBJDIR)/text_utils.o $(OBJDIR)/ansi_render.o

# Config program objects
CONFIG_OBJECTS = $(OBJDIR)/config.o $(OBJDIR)/settings.o $(OBJDIR)/settings_dialog.o \
                 $(OBJDIR)/terminal.o $(OBJDIR)/terminal_common.o $(OBJDIR)/file_dir_utils.o \
                 $(OBJDIR)/ui_common.o $(OBJDIR)/theme.o \
                 $(OBJDIR)/remote_systems.o $(OBJDIR)/file_browser.o

# Compiler
CXX = g++

# Detect OS
UNAME_S := $(shell uname -s 2>/dev/null || echo Windows)

# Common flags
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I$(SRCDIR)

# OS-specific: terminal implementation and link flags
ifeq ($(UNAME_S),Linux)
    TERM_SRC = $(SRCDIR)/terminal_ncurses.cpp
    CXXFLAGS += -D_XOPEN_SOURCE_EXTENDED
    LDFLAGS = -lncursesw -ltinfo
endif
ifeq ($(UNAME_S),Darwin)
    TERM_SRC = $(SRCDIR)/terminal_ncurses.cpp
    CXXFLAGS += -D_XOPEN_SOURCE_EXTENDED
    LDFLAGS = -lncurses
endif
ifeq ($(UNAME_S),FreeBSD)
    TERM_SRC = $(SRCDIR)/terminal_ncurses.cpp
    CXXFLAGS += -D_XOPEN_SOURCE_EXTENDED
    LDFLAGS = -lncursesw
endif
ifeq ($(UNAME_S),NetBSD)
    TERM_SRC = $(SRCDIR)/terminal_ncurses.cpp
    CXXFLAGS += -D_XOPEN_SOURCE_EXTENDED
    LDFLAGS = -lncursesw
endif
ifeq ($(UNAME_S),OpenBSD)
    TERM_SRC = $(SRCDIR)/terminal_ncurses.cpp
    CXXFLAGS += -D_XOPEN_SOURCE_EXTENDED
    LDFLAGS = -lncursesw
endif

# Windows (MinGW/MSYS2)
ifeq ($(OS),Windows_NT)
    TARGET = slymail.exe
    CONFIG_TARGET = config.exe
    TERM_SRC = $(SRCDIR)/terminal_win32.cpp
    LDFLAGS =
endif

# Debug build
ifdef DEBUG
    CXXFLAGS += -g -DDEBUG -O0
endif

# Default target: build both programs
all: $(OBJDIR) $(TARGET) $(CONFIG_TARGET)

# Create object directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Link SlyMail
$(TARGET): $(SLYMAIL_OBJECTS)
	$(CXX) $(SLYMAIL_OBJECTS) -o $(TARGET) $(LDFLAGS)

# Link Config program
$(CONFIG_TARGET): $(CONFIG_OBJECTS)
	$(CXX) $(CONFIG_OBJECTS) -o $(CONFIG_TARGET) $(LDFLAGS)

# Compile main.cpp (SlyMail)
$(OBJDIR)/main.o: $(SRCDIR)/main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile config.cpp (Config program)
$(OBJDIR)/config.o: $(SRCDIR)/config.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile settings.cpp
$(OBJDIR)/settings.o: $(SRCDIR)/settings.cpp $(SRCDIR)/settings.h $(SRCDIR)/terminal.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile qwk.cpp
$(OBJDIR)/qwk.o: $(SRCDIR)/qwk.cpp $(SRCDIR)/qwk.h $(SRCDIR)/terminal.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile msg_reader.cpp
$(OBJDIR)/msg_reader.o: $(SRCDIR)/msg_reader.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile msg_editor.cpp
$(OBJDIR)/msg_editor.o: $(SRCDIR)/msg_editor.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile settings_dialog.cpp
$(OBJDIR)/settings_dialog.o: $(SRCDIR)/settings_dialog.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile terminal implementation (platform-specific)
$(OBJDIR)/terminal.o: $(TERM_SRC) $(SRCDIR)/terminal.h $(SRCDIR)/terminal_ncurses.h $(SRCDIR)/terminal_win32.h $(SRCDIR)/cp437defs.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile terminal.cpp (platform-independent terminal base class methods)
$(OBJDIR)/terminal_common.o: $(SRCDIR)/terminal.cpp $(SRCDIR)/terminal.h $(SRCDIR)/file_dir_utils.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile file_dir_utils.cpp
$(OBJDIR)/file_dir_utils.o: $(SRCDIR)/file_dir_utils.cpp $(SRCDIR)/file_dir_utils.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile ui_common.cpp
$(OBJDIR)/ui_common.o: $(SRCDIR)/ui_common.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile theme.cpp
$(OBJDIR)/theme.o: $(SRCDIR)/theme.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile file_browser.cpp
$(OBJDIR)/file_browser.o: $(SRCDIR)/file_browser.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile msg_list.cpp
$(OBJDIR)/msg_list.o: $(SRCDIR)/msg_list.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile bbs_colors.cpp
$(OBJDIR)/bbs_colors.o: $(SRCDIR)/bbs_colors.cpp $(SRCDIR)/bbs_colors.h $(SRCDIR)/terminal.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile utf8_util.cpp
$(OBJDIR)/utf8_util.o: $(SRCDIR)/utf8_util.cpp $(SRCDIR)/utf8_util.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile voting.cpp
$(OBJDIR)/voting.o: $(SRCDIR)/voting.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile remote_systems.cpp
$(OBJDIR)/remote_systems.o: $(SRCDIR)/remote_systems.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile search.cpp
$(OBJDIR)/search.o: $(SRCDIR)/search.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile text_input.cpp
$(OBJDIR)/text_input.o: $(SRCDIR)/text_input.cpp $(SRCDIR)/text_input.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile text_utils.cpp
$(OBJDIR)/text_utils.o: $(SRCDIR)/text_utils.cpp $(SRCDIR)/text_utils.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile ansi_render.cpp
$(OBJDIR)/ansi_render.o: $(SRCDIR)/ansi_render.cpp $(SRCDIR)/ansi_render.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Test executables
TEST_TEXT_INPUT = test_text_input
TEST_WRAP_QUOTE = test_wrap_quote_lines
TEST_TI_OBJECTS = $(OBJDIR)/test_text_input.o $(OBJDIR)/text_input.o $(OBJDIR)/text_utils.o
TEST_WQ_OBJECTS = $(OBJDIR)/test_wrap_quote_lines.o $(OBJDIR)/text_utils.o

# Build all tests
test: $(OBJDIR) $(TEST_TEXT_INPUT) $(TEST_WRAP_QUOTE)

$(TEST_TEXT_INPUT): $(TEST_TI_OBJECTS)
	$(CXX) $(TEST_TI_OBJECTS) -o $(TEST_TEXT_INPUT)

$(TEST_WRAP_QUOTE): $(TEST_WQ_OBJECTS)
	$(CXX) $(TEST_WQ_OBJECTS) -o $(TEST_WRAP_QUOTE)

$(OBJDIR)/test_text_input.o: tests/test_text_input.cpp $(SRCDIR)/text_input.h $(SRCDIR)/text_utils.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/test_wrap_quote_lines.o: tests/test_wrap_quote_lines.cpp $(SRCDIR)/text_utils.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Linux release archive
nixReleaseArc: $(OBJDIR) $(TARGET) $(CONFIG_TARGET)
	bash ./makeNixReleaseArchive.sh $(RELEASE_ARC_FLAGS)

# Run all tests
runtest: test
	./$(TEST_TEXT_INPUT)
	./$(TEST_WRAP_QUOTE)

# Clean
clean:
	rm -rf $(OBJDIR) $(TARGET) $(CONFIG_TARGET) $(TEST_TEXT_INPUT) $(TEST_WRAP_QUOTE)

# Install (Linux/Mac/BSD)
install: $(TARGET) $(CONFIG_TARGET)
	install -m 755 $(TARGET) /usr/local/bin/
	install -m 755 $(CONFIG_TARGET) /usr/local/bin/slymail-config

# Uninstall
uninstall:
	rm -f /usr/local/bin/$(TARGET)
	rm -f /usr/local/bin/slymail-config

# Debug build
debug:
	$(MAKE) DEBUG=1

.PHONY: all clean install uninstall debug test runtest
