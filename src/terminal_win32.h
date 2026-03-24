#ifndef SLYMAIL_TERMINAL_WIN32_H
#define SLYMAIL_TERMINAL_WIN32_H

#ifdef _WIN32

#include "terminal.h"
#include <windows.h>

// ============================================================
// Win32Terminal - Windows console implementation
// using conio + Win32 Console API
// ============================================================
class Win32Terminal : public ITerminal
{
public:
    Win32Terminal();
    ~Win32Terminal() override;

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

    // --- Input ---
    int getKey() override;

    // --- Timing ---
    void napMillis(int ms) override;

private:
    HANDLE m_hConsole;
    HANDLE m_hInput;
    int    m_cols;
    int    m_rows;
    WORD   m_currentAttr;

    // Map standard keys
    int mapKey(int ch) const;

    // Map extended keys (0x00 or 0xE0 prefix)
    int mapExtendedKey(int ch) const;
};

// Helper: convert TermColor to Windows console color attribute
WORD termColorToWin(int fg, int bg, bool bright);

#endif // _WIN32

#endif // SLYMAIL_TERMINAL_WIN32_H
