#ifndef SLYMAIL_TERMINAL_H
#define SLYMAIL_TERMINAL_H

// SlyMail Terminal Abstraction Layer
//
// Abstract base class ITerminal defines the interface for all terminal
// operations (screen output, input, color, box-drawing).
//
// Derived classes:
//   NCursesTerminal - Linux/Mac/BSD using ncurses
//   Win32Terminal   - Windows using conio + Win32 Console API
//
// Factory function createTerminal() returns the appropriate implementation.

#include "cp437defs.h"

#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <algorithm>
#include <functional>
#include <map>
#include <fstream>
#include <sstream>
#include <memory>
#include <filesystem>
#include <climits>

// Platform-specific headers for getpid(), getcwd(), etc.
#ifdef _WIN32
    #include <direct.h>
    #include <windows.h>
    #include <process.h>
    #define getcwd _getcwd
    #define getpid _getpid
#else
    #include <unistd.h>
    #include <sys/types.h>
#endif

namespace fs = std::filesystem;

// ============================================================
// Platform detection
// ============================================================
#ifdef _WIN32
    #define SLYMAIL_WINDOWS 1
    #define PATH_SEP '\\'
    #define PATH_SEP_STR "\\"
#else
    #define SLYMAIL_WINDOWS 0
    #define PATH_SEP '/'
    #define PATH_SEP_STR "/"
#endif

// ============================================================
// Platform-independent key constants
// ============================================================
enum TermKey
{
    TK_NONE        = 0,

    // Control keys (ASCII)
    TK_CTRL_A      = 1,
    TK_CTRL_B      = 2,
    TK_CTRL_C      = 3,
    TK_CTRL_D      = 4,
    TK_CTRL_E      = 5,
    TK_CTRL_F      = 6,
    TK_CTRL_G      = 7,
    TK_BACKSPACE_8 = 8,    // Ctrl-H / Backspace on some terminals
    TK_TAB         = 9,
    TK_LINEFEED    = 10,   // Ctrl-J / Enter on some terminals
    TK_CTRL_K      = 11,
    TK_CTRL_L      = 12,
    TK_RETURN      = 13,   // Ctrl-M / Enter
    TK_CTRL_N      = 14,
    TK_CTRL_O      = 15,
    TK_CTRL_P      = 16,
    TK_CTRL_Q      = 17,
    TK_CTRL_R      = 18,
    TK_CTRL_S      = 19,
    TK_CTRL_T      = 20,
    TK_CTRL_U      = 21,
    TK_CTRL_V      = 22,
    TK_CTRL_W      = 23,
    TK_CTRL_X      = 24,
    TK_CTRL_Y      = 25,
    TK_CTRL_Z      = 26,
    TK_ESCAPE      = 27,
    TK_BACKSPACE   = 127,  // ASCII DEL

    // Extended keys (above 255 to avoid ASCII conflicts)
    TK_UP          = 300,
    TK_DOWN        = 301,
    TK_LEFT        = 302,
    TK_RIGHT       = 303,
    TK_HOME        = 304,
    TK_END         = 305,
    TK_PGUP        = 306,
    TK_PGDN        = 307,
    TK_INSERT      = 308,
    TK_DELETE       = 309,
    TK_ENTER       = 310,  // Enter key (mapped from platform-specific)
    TK_F1          = 311,
    TK_F2          = 312,
    TK_F3          = 313,
    TK_F4          = 314,
    TK_F5          = 315,
    TK_F6          = 316,
    TK_F7          = 317,
    TK_F8          = 318,
    TK_F9          = 319,
    TK_F10         = 320,
    TK_F11         = 321,
    TK_F12         = 322,
};

// ============================================================
// Platform-independent color indices
// ============================================================
enum TermColor
{
    TC_BLACK   = 0,
    TC_RED     = 1,
    TC_GREEN   = 2,
    TC_YELLOW  = 3,
    TC_BLUE    = 4,
    TC_MAGENTA = 5,
    TC_CYAN    = 6,
    TC_WHITE   = 7,
};

// ============================================================
// TermAttr - color/attribute specification
// ============================================================
struct TermAttr
{
    int  fg;
    int  bg;
    bool bright;

    TermAttr()
        : fg(TC_WHITE), bg(TC_BLACK), bright(false)
    {
    }

    TermAttr(int f, int b, bool br = false)
        : fg(f), bg(b), bright(br)
    {
    }

    bool operator==(const TermAttr& o) const
    {
        return fg == o.fg && bg == o.bg && bright == o.bright;
    }

    bool operator!=(const TermAttr& o) const
    {
        return !(*this == o);
    }
};

// Convenience function to create a TermAttr
inline TermAttr tAttr(int fg, int bg = TC_BLACK, bool bright = false)
{
    return TermAttr(fg, bg, bright);
}

// ============================================================
// ITerminal - Abstract terminal interface
// ============================================================
class ITerminal
{
public:
    virtual ~ITerminal() = default;

    // --- Lifecycle ---
    virtual void init() = 0;
    virtual void shutdown() = 0;

    // --- Screen info ---
    virtual int  getCols() const = 0;
    virtual int  getRows() const = 0;
    virtual bool hasColors() const = 0;

    // --- Screen management ---
    virtual void clear() = 0;
    virtual void refresh() = 0;
    virtual void clearToEol() = 0;

    // --- Cursor ---
    virtual void moveTo(int row, int col) = 0;
    virtual void setCursorVisible(bool visible) = 0;

    // --- Color/attribute ---
    virtual void setAttr(const TermAttr& attr) = 0;
    virtual void resetAttr() = 0;

    // --- Text output ---
    // Print string at position with current attribute
    virtual void printStr(int row, int col, const std::string& text) = 0;
    // Print single character at position with current attribute
    virtual void putCh(int row, int col, int ch) = 0;
    // Fill a row region with a character using current attribute
    virtual void fillRegion(int row, int startCol, int endCol, char ch = ' ') = 0;

    // --- CP437 character output ---
    // Outputs a CP437 character at the given position.
    // On ncurses, maps CP437 codes to ACS equivalents.
    // On Windows, outputs the CP437 byte directly.
    virtual void putCP437(int row, int col, int cp437char) = 0;

    // --- Box-drawing convenience methods (non-virtual, use putCP437) ---

    virtual void drawHLine(int row, int col, int len)
    {
        for (int i = 0; i < len; ++i)
        {
            putCP437(row, col + i, CP437_BOX_DRAWINGS_HORIZONTAL_SINGLE);
        }
    }

    virtual void drawVLine(int row, int col, int len)
    {
        for (int i = 0; i < len; ++i)
        {
            putCP437(row + i, col, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
        }
    }

    virtual void drawBox(int row, int col, int height, int width)
    {
        putCP437(row, col, CP437_BOX_DRAWINGS_UPPER_LEFT_SINGLE);
        putCP437(row, col + width - 1, CP437_BOX_DRAWINGS_UPPER_RIGHT_SINGLE);
        putCP437(row + height - 1, col, CP437_BOX_DRAWINGS_LOWER_LEFT_SINGLE);
        putCP437(row + height - 1, col + width - 1, CP437_BOX_DRAWINGS_LOWER_RIGHT_SINGLE);
        drawHLine(row, col + 1, width - 2);
        drawHLine(row + height - 1, col + 1, width - 2);
        drawVLine(row + 1, col, height - 2);
        drawVLine(row + 1, col + width - 1, height - 2);
    }

    // --- Input ---
    virtual int getKey() = 0;

    // --- Timing ---
    virtual void napMillis(int ms) = 0;
};

// ============================================================
// Global terminal instance
// ============================================================
extern ITerminal* g_term;

// ============================================================
// Factory function
// ============================================================
std::unique_ptr<ITerminal> createTerminal();

// ============================================================
// Platform utility functions
// ============================================================

// Extract a QWK file (ZIP archive) to a destination directory
inline std::string extractQwkPacket(const std::string& qwkPath, const std::string& destDir)
{
    fs::create_directories(destDir);
#if SLYMAIL_WINDOWS
    std::string cmd = "powershell -Command \"Expand-Archive -Path '"
        + qwkPath + "' -DestinationPath '" + destDir + "' -Force\" 2>NUL";
#else
    std::string cmd = "unzip -o -qq \"" + qwkPath + "\" -d \"" + destDir + "\" 2>/dev/null";
#endif
    int ret = system(cmd.c_str());
    if (ret != 0)
    {
        return "";
    }
    return destDir;
}

// Create a ZIP archive (for REP packets)
inline bool createZipArchive(const std::string& zipPath, const std::string& sourceDir)
{
    // Convert zipPath to absolute so it works after cd to sourceDir
    std::string absZipPath = zipPath;
    try
    {
        absZipPath = fs::absolute(zipPath).string();
    }
    catch (...)
    {
    }

    // Remove existing file first to avoid appending to old archive
    try
    {
        fs::remove(absZipPath);
    }
    catch (...)
    {
    }

#if SLYMAIL_WINDOWS
    std::string cmd = "powershell -Command \"Compress-Archive -Path '"
        + sourceDir + PATH_SEP_STR + "*' -DestinationPath '"
        + absZipPath + "' -Force\" 2>NUL";
#else
    std::string cmd = "cd \"" + sourceDir + "\" && zip -j -q \"" + absZipPath + "\" * 2>/dev/null";
#endif
    return system(cmd.c_str()) == 0;
}

// Get user's config directory for settings
inline std::string getConfigDir()
{
#if SLYMAIL_WINDOWS
    const char* appdata = getenv("APPDATA");
    if (appdata)
    {
        std::string dir = std::string(appdata) + "\\SlyMail";
        fs::create_directories(dir);
        return dir;
    }
    return ".";
#else
    const char* home = getenv("HOME");
    if (home)
    {
        std::string dir = std::string(home) + "/.config/slymail";
        fs::create_directories(dir);
        return dir;
    }
    return ".";
#endif
}

// Get a temporary directory for QWK extraction
inline std::string getTempDir()
{
#if SLYMAIL_WINDOWS
    char buf[260];
    GetTempPathA(260, buf);
    std::string dir = std::string(buf) + "slymail_tmp";
#else
    std::string dir = "/tmp/slymail_tmp_" + std::to_string(getpid());
#endif
    fs::create_directories(dir);
    return dir;
}

// Clean up a temporary directory
inline void cleanupTempDir(const std::string& dir)
{
    if (!dir.empty())
    {
        fs::remove_all(dir);
    }
}

#endif // SLYMAIL_TERMINAL_H
