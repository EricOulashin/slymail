#ifndef SLYMAIL_TERMINAL_NCURSES_H
#define SLYMAIL_TERMINAL_NCURSES_H

#ifndef SLYMAIL_WINDOWS

#include "terminal.h"
#include <ncurses.h>

// ============================================================
// NCursesTerminal - ncurses-based terminal implementation
// for Linux, macOS, BSD, and other Unix-like operating systems
// ============================================================
class NCursesTerminal : public ITerminal
{
public:
    NCursesTerminal();
    ~NCursesTerminal() override;

    // --- Lifecycle ---
    void init() override;
    void shutdown() override;

    // --- Screen info ---
    int getCols() const override;
    int getRows() const override;
    bool hasColors() const override;

    // --- Screen management ---
    void clear() override;
    void refresh() override;
    void clearToEol() override;

    // --- Cursor ---
    void moveTo(int row, int col) override;
    void setCursorVisible(bool visible) override;

    // --- Color/attribute ---
    void setAttr(const TermAttr& attr) override;
    void resetAttr() override;

    // --- Text output ---
    void printStr(int row, int col, const std::string& text) override;
    void putCh(int row, int col, int ch) override;
    void fillRegion(int row, int startCol, int endCol, char ch = ' ') override;

    // --- CP437 character output ---
    void putCP437(int row, int col, int cp437char) override;

    // --- Box-drawing (overrides for ncurses efficiency) ---
    void drawHLine(int row, int col, int len) override;
    void drawVLine(int row, int col, int len) override;

    // --- Input ---
    int getKey() override;

    // --- Timing ---
    void napMillis(int ms) override;

private:
    attr_t m_currentAttr;

    // Map CP437 character codes to ncurses ACS character types
    chtype mapCP437toACS(int cp437) const;

    // Map ncurses key codes to platform-independent TermKey values
    int mapKey(int ch) const;
};

#endif // !SLYMAIL_WINDOWS

#endif // SLYMAIL_TERMINAL_NCURSES_H
