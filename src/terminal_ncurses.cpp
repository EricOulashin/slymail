// NCurses implementation of ITerminal for Linux/Mac/BSD

#ifndef SLYMAIL_WINDOWS

#include "terminal_ncurses.h"
#include <unistd.h>
#include <clocale>

using std::string;
using std::unique_ptr;
using std::make_unique;

// ============================================================
// NCursesTerminal implementation
// ============================================================

NCursesTerminal::NCursesTerminal()
    : m_currentAttr(A_NORMAL)
    , m_utf8Locale(false)
{
}

NCursesTerminal::~NCursesTerminal()
{
}

// --- Lifecycle ---

void NCursesTerminal::init()
{
    // Enable UTF-8 support: set the locale from the environment
    // so ncurses handles multi-byte characters correctly.
    setlocale(LC_ALL, "");

    // Detect whether the locale uses UTF-8 encoding.
    // This determines how putCP437 outputs characters: UTF-8 strings
    // for Unicode terminals, or raw bytes for CP437/Latin-1 terminals.
    const char* ctype = setlocale(LC_CTYPE, nullptr);
    m_utf8Locale = ctype && (strstr(ctype, "UTF-8") || strstr(ctype, "utf-8")
                          || strstr(ctype, "UTF8")  || strstr(ctype, "utf8"));

    initscr();
    raw();      // Use raw() instead of cbreak() to also disable
                // XON/XOFF flow control so Ctrl-Q/Ctrl-S pass through
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    mouseinterval(0);

    if (::has_colors())
    {
        start_color();
        use_default_colors();
        // Initialize all 64 foreground/background color pairs
        for (int fg = 0; fg < 8; ++fg)
        {
            for (int bg = 0; bg < 8; ++bg)
            {
                init_pair(static_cast<short>(fg * 8 + bg + 1),
                          static_cast<short>(fg),
                          static_cast<short>(bg));
            }
        }
    }
}

void NCursesTerminal::shutdown()
{
    endwin();
}

// --- Screen info ---

int NCursesTerminal::getCols() const
{
    return COLS;
}

int NCursesTerminal::getRows() const
{
    return LINES;
}

bool NCursesTerminal::hasColors() const
{
    return ::has_colors();
}

// --- Screen management ---

void NCursesTerminal::clear()
{
    ::clear();
}

void NCursesTerminal::refresh()
{
    ::refresh();
}

void NCursesTerminal::clearToEol()
{
    ::clrtoeol();
}

// --- Cursor ---

void NCursesTerminal::moveTo(int row, int col)
{
    ::move(row, col);
}

void NCursesTerminal::setCursorVisible(bool visible)
{
    curs_set(visible ? 1 : 0);
}

// --- Color/attribute ---

void NCursesTerminal::setAttr(const TermAttr& attr)
{
    attrset(A_NORMAL);

    int pairId = attr.fg * 8 + attr.bg + 1;
    attr_t a = COLOR_PAIR(pairId);
    if (attr.bright)
    {
        a |= A_BOLD;
    }
    attron(a);
    m_currentAttr = a;
}

void NCursesTerminal::resetAttr()
{
    attrset(A_NORMAL);
    m_currentAttr = A_NORMAL;
}

// --- Text output ---

void NCursesTerminal::printStr(int row, int col, const string& text)
{
    mvprintw(row, col, "%s", text.c_str());
}

void NCursesTerminal::putCh(int row, int col, int ch)
{
    mvaddch(row, col, ch);
}

void NCursesTerminal::fillRegion(int row, int startCol, int endCol, char ch)
{
    ::move(row, startCol);
    for (int c = startCol; c < endCol; ++c)
    {
        addch(ch);
    }
}

// --- CP437 character output ---

// Complete CP437 → Unicode code point lookup table (indices 0x00..0xFF).
// Used by putCP437 to output the correct Unicode character for each CP437 byte.
// This is the same mapping used by the Windows terminal (s_cp437 in terminal_win32.cpp).
static const wchar_t s_cp437ToUnicode[256] = {
    // 0x00-0x0F
    0x0020, 0x263A, 0x263B, 0x2665, 0x2666, 0x2663, 0x2660, 0x2022,
    0x25D8, 0x25CB, 0x25D9, 0x2642, 0x2640, 0x266A, 0x266B, 0x263C,
    // 0x10-0x1F
    0x25BA, 0x25C4, 0x2195, 0x203C, 0x00B6, 0x00A7, 0x25AC, 0x21A8,
    0x2191, 0x2193, 0x2192, 0x2190, 0x221F, 0x2194, 0x25B2, 0x25BC,
    // 0x20-0x7E: standard ASCII
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x2302,
    // 0x80-0x8F
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,
    0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
    // 0x90-0x9F
    0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,
    0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x20A7, 0x0192,
    // 0xA0-0xAF
    0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA,
    0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
    // 0xB0-0xBF (shade blocks, box singles/doubles)
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
    0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
    // 0xC0-0xCF
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
    // 0xD0-0xDF (more box, block elements)
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
    0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
    // 0xE0-0xEF (Greek / math)
    0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x00B5, 0x03C4,
    0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,
    // 0xF0-0xFF
    0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248,
    0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0,
};

void NCursesTerminal::putCP437(int row, int col, int cp437char)
{
    unsigned idx = static_cast<unsigned>(cp437char) & 0xFF;
    if (m_utf8Locale)
    {
        // UTF-8 terminal: output the correct Unicode character via the
        // ncursesw wide-character API (mvadd_wch).  This properly handles
        // multi-byte UTF-8 output as a single display column.
        cchar_t cc;
        wchar_t wstr[2] = { s_cp437ToUnicode[idx], L'\0' };
        setcchar(&cc, wstr, m_currentAttr, PAIR_NUMBER(m_currentAttr), nullptr);
        mvadd_wch(row, col, &cc);
    }
    else
    {
        // CP437 / non-UTF-8 terminal: output the raw byte directly.
        // The terminal's native character set handles the rendering.
        mvaddch(row, col, static_cast<chtype>(idx));
    }
}

void NCursesTerminal::drawHLine(int row, int col, int len)
{
    mvhline(row, col, ACS_HLINE, len);
}

void NCursesTerminal::drawVLine(int row, int col, int len)
{
    mvvline(row, col, ACS_VLINE, len);
}

// --- Input ---

int NCursesTerminal::getKey()
{
    int ch = getch();
    if (ch == KEY_RESIZE)
    {
        // Terminal was resized — update ncurses internal state
        // and return TK_RESIZE so the caller can redraw
        return TK_RESIZE;
    }
    return mapKey(ch);
}

// --- Timing ---

void NCursesTerminal::napMillis(int ms)
{
    napms(ms);
}

// --- Private helpers ---

int NCursesTerminal::mapKey(int ch) const
{
    switch (ch)
    {
        case KEY_UP:        return TK_UP;
        case KEY_DOWN:      return TK_DOWN;
        case KEY_LEFT:      return TK_LEFT;
        case KEY_RIGHT:     return TK_RIGHT;
        case KEY_HOME:      return TK_HOME;
        case KEY_END:       return TK_END;
        case KEY_PPAGE:     return TK_PGUP;
        case KEY_NPAGE:     return TK_PGDN;
        case KEY_IC:        return TK_INSERT;
        case KEY_DC:        return TK_DELETE;
        case KEY_BACKSPACE: return TK_BACKSPACE;
        case KEY_ENTER:     return TK_ENTER;
        case KEY_F(1):      return TK_F1;
        case KEY_F(2):      return TK_F2;
        case KEY_F(3):      return TK_F3;
        case KEY_F(4):      return TK_F4;
        case KEY_F(5):      return TK_F5;
        case KEY_F(6):      return TK_F6;
        case KEY_F(7):      return TK_F7;
        case KEY_F(8):      return TK_F8;
        case KEY_F(9):      return TK_F9;
        case KEY_F(10):     return TK_F10;
        case KEY_F(11):     return TK_F11;
        case KEY_F(12):     return TK_F12;
        case '\n':          return TK_ENTER;
        case '\r':          return TK_ENTER;
        case 127:           return TK_BACKSPACE;
        default:            return ch;
    }
}

// ============================================================
// Factory implementation for non-Windows platforms
// ============================================================
ITerminal* g_term = nullptr;

unique_ptr<ITerminal> createTerminal()
{
    return make_unique<NCursesTerminal>();
}

#endif // !SLYMAIL_WINDOWS
