#ifndef SLYMAIL_TERMINAL_WIN32_H
#define SLYMAIL_TERMINAL_WIN32_H

#ifdef _WIN32

#include "terminal.h"
#include <windows.h>
#include <vector>

// ============================================================
// CellBuf - one screen cell in the double-buffer
// ============================================================
struct CellBuf
{
    wchar_t ch;
    int     fg;
    int     bg;
    bool    bright;

    bool operator==(const CellBuf& o) const
    {
        return ch == o.ch && fg == o.fg && bg == o.bg && bright == o.bright;
    }
    bool operator!=(const CellBuf& o) const { return !(*this == o); }
};

// ============================================================
// Win32Terminal - Windows console implementation
// using conio + Win32 Console API with double-buffering
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

    // Double buffers: back = desired state, front = last displayed state
    std::vector<CellBuf>  m_back;
    std::vector<CellBuf>  m_front;

    // Reusable wide-char write buffer (sized to m_cols)
    std::vector<wchar_t>  m_wbuf;

    // Pending attribute applied by the next buffer write
    int  m_pendFg;
    int  m_pendBg;
    bool m_pendBright;

    // Logical cursor (updated by moveTo/printStr/putCh/putCP437)
    int m_logRow;
    int m_logCol;

    // Map standard keys
    int mapKey(int ch) const;

    // Map extended keys (0x00 or 0xE0 prefix)
    int mapExtendedKey(int ch) const;
};

// Helper: convert TermColor to Windows console color attribute
WORD termColorToWin(int fg, int bg, bool bright);

#endif // _WIN32

#endif // SLYMAIL_TERMINAL_WIN32_H
