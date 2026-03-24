// Windows implementation of ITerminal using conio + Win32 Console API

#ifdef _WIN32

#include "terminal_win32.h"
#include <conio.h>

using std::string;
using std::unique_ptr;
using std::make_unique;

// ============================================================
// Helper: convert TermColor to Windows console color attribute
// ============================================================
WORD termColorToWin(int fg, int bg, bool bright)
{
    static const WORD colorMap[8] =
    {
        0,                                                  // TC_BLACK
        FOREGROUND_RED,                                     // TC_RED
        FOREGROUND_GREEN,                                   // TC_GREEN
        FOREGROUND_RED | FOREGROUND_GREEN,                  // TC_YELLOW
        FOREGROUND_BLUE,                                    // TC_BLUE
        FOREGROUND_RED | FOREGROUND_BLUE,                   // TC_MAGENTA
        FOREGROUND_GREEN | FOREGROUND_BLUE,                 // TC_CYAN
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE // TC_WHITE
    };

    WORD attr = colorMap[fg & 7];
    WORD bgBits = colorMap[bg & 7];
    attr |= (bgBits << 4);
    if (bright)
    {
        attr |= FOREGROUND_INTENSITY;
    }
    return attr;
}

// ============================================================
// Win32Terminal implementation
// ============================================================

Win32Terminal::Win32Terminal()
    : m_hConsole(INVALID_HANDLE_VALUE)
    , m_hInput(INVALID_HANDLE_VALUE)
    , m_cols(80)
    , m_rows(25)
    , m_currentAttr(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
{
}

Win32Terminal::~Win32Terminal()
{
}

// --- Lifecycle ---

void Win32Terminal::init()
{
    m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    m_hInput = GetStdHandle(STD_INPUT_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(m_hConsole, &csbi))
    {
        m_cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        m_rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }

    DWORD mode;
    GetConsoleMode(m_hInput, &mode);
    mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    mode |= ENABLE_PROCESSED_INPUT;
    SetConsoleMode(m_hInput, mode);

    setCursorVisible(false);
    SetConsoleOutputCP(437);
}

void Win32Terminal::shutdown()
{
    setCursorVisible(true);
    resetAttr();
}

// --- Screen info ---

int Win32Terminal::getCols() const
{
    return m_cols;
}

int Win32Terminal::getRows() const
{
    return m_rows;
}

bool Win32Terminal::hasColors() const
{
    return true;
}

// --- Screen management ---

void Win32Terminal::clear()
{
    COORD topLeft = {0, 0};
    DWORD written;
    DWORD size = m_cols * m_rows;
    FillConsoleOutputCharacterA(m_hConsole, ' ', size, topLeft, &written);
    FillConsoleOutputAttribute(m_hConsole, m_currentAttr, size, topLeft, &written);
    moveTo(0, 0);
}

void Win32Terminal::refresh()
{
    // Windows console is immediate-mode; no explicit refresh needed
}

void Win32Terminal::clearToEol()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(m_hConsole, &csbi);
    DWORD written;
    int remaining = m_cols - csbi.dwCursorPosition.X;
    FillConsoleOutputCharacterA(m_hConsole, ' ', remaining,
                                csbi.dwCursorPosition, &written);
    FillConsoleOutputAttribute(m_hConsole, m_currentAttr, remaining,
                               csbi.dwCursorPosition, &written);
}

// --- Cursor ---

void Win32Terminal::moveTo(int row, int col)
{
    COORD pos = {static_cast<SHORT>(col), static_cast<SHORT>(row)};
    SetConsoleCursorPosition(m_hConsole, pos);
}

void Win32Terminal::setCursorVisible(bool visible)
{
    CONSOLE_CURSOR_INFO cci;
    GetConsoleCursorInfo(m_hConsole, &cci);
    cci.bVisible = visible ? TRUE : FALSE;
    SetConsoleCursorInfo(m_hConsole, &cci);
}

// --- Color/attribute ---

void Win32Terminal::setAttr(const TermAttr& attr)
{
    m_currentAttr = termColorToWin(attr.fg, attr.bg, attr.bright);
    SetConsoleTextAttribute(m_hConsole, m_currentAttr);
}

void Win32Terminal::resetAttr()
{
    m_currentAttr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    SetConsoleTextAttribute(m_hConsole, m_currentAttr);
}

// --- Text output ---

void Win32Terminal::printStr(int row, int col, const string& text)
{
    moveTo(row, col);
    DWORD written;
    WriteConsoleA(m_hConsole, text.c_str(),
                  static_cast<DWORD>(text.size()), &written, NULL);
}

void Win32Terminal::putCh(int row, int col, int ch)
{
    moveTo(row, col);
    char c = static_cast<char>(ch);
    DWORD written;
    WriteConsoleA(m_hConsole, &c, 1, &written, NULL);
}

void Win32Terminal::fillRegion(int row, int startCol, int endCol, char ch)
{
    COORD pos = {static_cast<SHORT>(startCol), static_cast<SHORT>(row)};
    DWORD written;
    int len = endCol - startCol;
    if (len <= 0)
    {
        return;
    }
    FillConsoleOutputCharacterA(m_hConsole, ch, len, pos, &written);
    FillConsoleOutputAttribute(m_hConsole, m_currentAttr, len, pos, &written);
}

// --- CP437 character output ---

void Win32Terminal::putCP437(int row, int col, int cp437char)
{
    putCh(row, col, cp437char);
}

// --- Input ---

int Win32Terminal::getKey()
{
    int ch = _getch();
    if (ch == 0 || ch == 0xE0)
    {
        ch = _getch();
        return mapExtendedKey(ch);
    }
    return mapKey(ch);
}

// --- Timing ---

void Win32Terminal::napMillis(int ms)
{
    Sleep(ms);
}

// --- Private helpers ---

int Win32Terminal::mapKey(int ch) const
{
    switch (ch)
    {
        case '\r':  return TK_ENTER;
        case '\n':  return TK_ENTER;
        case 27:    return TK_ESCAPE;
        case 8:     return TK_BACKSPACE;
        case 9:     return TK_TAB;
        case 127:   return TK_BACKSPACE;
        default:    return ch;
    }
}

int Win32Terminal::mapExtendedKey(int ch) const
{
    switch (ch)
    {
        case 72:  return TK_UP;
        case 80:  return TK_DOWN;
        case 75:  return TK_LEFT;
        case 77:  return TK_RIGHT;
        case 71:  return TK_HOME;
        case 79:  return TK_END;
        case 73:  return TK_PGUP;
        case 81:  return TK_PGDN;
        case 82:  return TK_INSERT;
        case 83:  return TK_DELETE;
        case 59:  return TK_F1;
        case 60:  return TK_F2;
        case 61:  return TK_F3;
        case 62:  return TK_F4;
        case 63:  return TK_F5;
        case 64:  return TK_F6;
        case 65:  return TK_F7;
        case 66:  return TK_F8;
        case 67:  return TK_F9;
        case 68:  return TK_F10;
        case 133: return TK_F11;
        case 134: return TK_F12;
        default:  return TK_NONE;
    }
}

// ============================================================
// Factory implementation for Windows
// ============================================================
ITerminal* g_term = nullptr;

unique_ptr<ITerminal> createTerminal()
{
    return make_unique<Win32Terminal>();
}

#endif // _WIN32
