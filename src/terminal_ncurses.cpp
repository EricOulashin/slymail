// NCurses implementation of ITerminal for Linux/Mac/BSD

#ifndef SLYMAIL_WINDOWS

#include "terminal_ncurses.h"
#include <unistd.h>

using std::string;
using std::unique_ptr;
using std::make_unique;

// ============================================================
// NCursesTerminal implementation
// ============================================================

NCursesTerminal::NCursesTerminal()
    : m_currentAttr(A_NORMAL)
{
}

NCursesTerminal::~NCursesTerminal()
{
}

// --- Lifecycle ---

void NCursesTerminal::init()
{
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

void NCursesTerminal::putCP437(int row, int col, int cp437char)
{
    chtype acs = mapCP437toACS(cp437char);
    mvaddch(row, col, acs);
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
    return mapKey(ch);
}

// --- Timing ---

void NCursesTerminal::napMillis(int ms)
{
    napms(ms);
}

// --- Private helpers ---

chtype NCursesTerminal::mapCP437toACS(int cp437) const
{
    switch (cp437)
    {
        // Single-line box drawing
        case CP437_BOX_DRAWINGS_HORIZONTAL_SINGLE:  return ACS_HLINE;
        case CP437_BOX_DRAWINGS_LIGHT_VERTICAL:     return ACS_VLINE;
        case CP437_BOX_DRAWINGS_UPPER_LEFT_SINGLE:  return ACS_ULCORNER;
        case CP437_BOX_DRAWINGS_UPPER_RIGHT_SINGLE: return ACS_URCORNER;
        case CP437_BOX_DRAWINGS_LOWER_LEFT_SINGLE:  return ACS_LLCORNER;
        case CP437_BOX_DRAWINGS_LOWER_RIGHT_SINGLE: return ACS_LRCORNER;
        case CP437_BOX_DRAWINGS_LIGHT_LEFT_T:       return ACS_LTEE;
        case CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT: return ACS_RTEE;
        case CP437_BOX_DRAWINGS_UP_AND_HORIZONTAL:  return ACS_BTEE;
        case CP437_BOX_DRAWINGS_DOWN_AND_HORIZONTAL: return ACS_TTEE;
        case CP437_BOX_DRAWINGS_VERTICAL_AND_HORIZONTAL: return ACS_PLUS;

        // Double-line box drawing (ncurses has no double-line ACS,
        // so fall back to single-line equivalents)
        case CP437_BOX_DRAWINGS_DOUBLE_HORIZONTAL:  return ACS_HLINE;
        case CP437_BOX_DRAWINGS_DOUBLE_VERTICAL:    return ACS_VLINE;
        case CP437_BOX_DRAWINGS_DOUBLE_DOWN_AND_RIGHT: return ACS_ULCORNER;
        case CP437_BOX_DRAWINGS_DOUBLE_DOWN_AND_LEFT:  return ACS_URCORNER;
        case CP437_BOX_DRAWINGS_DOUBLE_UP_AND_RIGHT:   return ACS_LLCORNER;
        case CP437_BOX_DRAWINGS_DOUBLE_UP_AND_LEFT:    return ACS_LRCORNER;
        case CP437_BOX_DRAWINGS_DOUBLE_VERTICAL_AND_RIGHT: return ACS_LTEE;
        case CP437_BOX_DRAWINGS_DOUBLE_VERTICAL_AND_LEFT:  return ACS_RTEE;
        case CP437_BOX_DRAWINGS_DOUBLE_UP_AND_HORIZONTAL:  return ACS_BTEE;
        case CP437_BOX_DRAWINGS_DOUBLE_DOWN_AND_HORIZONTAL: return ACS_TTEE;
        case CP437_BOX_DRAWINGS_DOUBLE_VERTICAL_AND_HORIZONTAL: return ACS_PLUS;

        // Mixed single/double (approximate with single-line)
        case CP437_BOX_DRAWINGS_VERTICAL_SINGLE_AND_LEFT_DOUBLE: return ACS_RTEE;
        case CP437_BOX_DRAWINGS_VERTICAL_DOUBLE_AND_LEFT_SINGLE: return ACS_RTEE;
        case CP437_BOX_DRAWINGS_DOWN_DOUBLE_AND_LEFT_SINGLE: return ACS_URCORNER;
        case CP437_BOX_DRAWINGS_DOWN_SINGLE_AND_LEFT_DOUBLE: return ACS_URCORNER;
        case CP437_BOX_DRAWINGS_UP_DOUBLE_AND_LEFT_SINGLE: return ACS_LRCORNER;
        case CP437_BOX_DRAWINGS_UP_SINGLE_AND_LEFT_DOUBLE: return ACS_LRCORNER;
        case CP437_BOX_DRAWINGS_VERTICAL_SINGLE_AND_RIGHT_DOUBLE: return ACS_LTEE;
        case CP437_BOX_DRAWINGS_VERTICAL_DOUBLE_AND_RIGHT_SINGLE: return ACS_LTEE;
        case CP437_BOX_DRAWINGS_UP_SINGLE_AND_HORIZONTAL_DOUBLE: return ACS_BTEE;
        case CP437_BOX_DRAWINGS_UP_DOUBLE_AND_HORIZONTAL_SINGLE: return ACS_BTEE;
        case CP437_BOX_DRAWINGS_DOWN_SINGLE_AND_HORIZONTAL_DOUBLE: return ACS_TTEE;
        case CP437_BOX_DRAWINGS_DOWN_DOUBLE_AND_HORIZONTAL_SINGLE: return ACS_TTEE;
        case CP437_BOX_DRAWINGS_UP_DOUBLE_AND_RIGHT_SINGLE: return ACS_LLCORNER;
        case CP437_BOX_DRAWINGS_UP_SINGLE_AND_RIGHT_DOUBLE: return ACS_LLCORNER;
        case CP437_BOX_DRAWINGS_DOWN_SINGLE_AND_RIGHT_DOUBLE: return ACS_ULCORNER;
        case CP437_BOX_DRAWINGS_DOWN_DOUBLE_AND_RIGHT_SINGLE: return ACS_ULCORNER;
        case CP437_BOX_DRAWINGS_VERTICAL_DOUBLE_AND_HORIZONTAL_SINGLE: return ACS_PLUS;
        case CP437_BOX_DRAWINGS_VERTICAL_SINGLE_AND_HORIZONTAL_DOUBLE: return ACS_PLUS;

        // Shade/block characters
        case CP437_LIGHT_SHADE:     return ACS_BOARD;
        case CP437_MEDIUM_SHADE:    return ACS_CKBOARD;
        case CP437_DARK_SHADE:      return ACS_CKBOARD;
        case CP437_FULL_BLOCK:      return ACS_BLOCK;
        case CP437_UPPER_HALF_BLOCK: return ACS_BLOCK;
        case CP437_LOWER_HALF_BLOCK: return ACS_BLOCK;

        // Bullets and dots
        case CP437_BULLET:          return ACS_BULLET;
        case CP437_BULLET_OPERATOR: return ACS_BULLET;
        case CP437_MIDDLE_DOT:      return ACS_BULLET;
        case CP437_BLACK_SQUARE:    return ACS_BLOCK;

        // Arrows
        case CP437_UPWARDS_ARROW:   return ACS_UARROW;
        case CP437_DOWNWARDS_ARROW: return ACS_DARROW;
        case CP437_RIGHTWARDS_ARROW: return ACS_RARROW;
        case CP437_LEFTWARDS_ARROW: return ACS_LARROW;

        // Check mark
        case CP437_CHECK_MARK:      return ACS_CKBOARD;

        // Degree and plus/minus
        case CP437_DEGREE_SIGN:     return ACS_DEGREE;
        case CP437_PLUS_MINUS_SIGN: return ACS_PLMINUS;

        // Diamond
        case CP437_BLACK_DIAMOND_SUIT: return ACS_DIAMOND;

        // Default: output as regular character
        default: return static_cast<chtype>(cp437 & 0xFF);
    }
}

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
