// Windows implementation of ITerminal using conio + Win32 Console API

#ifdef _WIN32

#include "terminal_win32.h"
#include <conio.h>
#include <cstring>

using std::string;
using std::unique_ptr;
using std::make_unique;

// ============================================================
// CP437 byte → Unicode code point mapping (indices 0x00..0xFF)
// ============================================================
static const wchar_t s_cp437[256] = {
    // 0x00-0x07
    0x0020, 0x263A, 0x263B, 0x2665, 0x2666, 0x2663, 0x2660, 0x2022,
    // 0x08-0x0F
    0x25D8, 0x25CB, 0x25D9, 0x2642, 0x2640, 0x266A, 0x266B, 0x263C,
    // 0x10-0x17
    0x25BA, 0x25C4, 0x2195, 0x203C, 0x00B6, 0x00A7, 0x25AC, 0x21A8,
    // 0x18-0x1F
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
    // 0xB0-0xBF  (shade blocks, box singles/doubles)
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
    0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
    // 0xC0-0xCF
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
    // 0xD0-0xDF  (more box, block elements)
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
    0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
    // 0xE0-0xEF  (Greek / math)
    0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x00B5, 0x03C4,
    0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,
    // 0xF0-0xFF
    0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248,
    0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0,
};

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
    , m_pendFg(7)        // TC_WHITE
    , m_pendBg(0)        // TC_BLACK
    , m_pendBright(false)
    , m_logRow(0)
    , m_logCol(0)
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

    // Enable UTF-8 output on Windows (CP 65001)
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    // Enable virtual terminal processing for ANSI sequences
    DWORD outMode;
    if (GetConsoleMode(m_hConsole, &outMode))
    {
        outMode |= 0x0004; // ENABLE_VIRTUAL_TERMINAL_PROCESSING
        SetConsoleMode(m_hConsole, outMode);
    }

    // Allocate double-buffers now that we know m_cols / m_rows
    CellBuf def = { L' ', 7, 0, false };
    m_back.assign(static_cast<size_t>(m_rows * m_cols), def);
    m_front.assign(static_cast<size_t>(m_rows * m_cols), def);
    m_wbuf.resize(static_cast<size_t>(m_cols));
}

void Win32Terminal::shutdown()
{
    setCursorVisible(true);
    // Restore default console colors
    DWORD written;
    WriteConsoleA(m_hConsole, "\033[0m", 4, &written, NULL);
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
    // Immediately clear the physical console
    DWORD written;
    WriteConsoleA(m_hConsole, "\033[0m\033[2J\033[H", 12, &written, NULL);

    // Reset pending attribute and logical cursor
    m_pendFg = 7;
    m_pendBg = 0;
    m_pendBright = false;
    m_logRow = 0;
    m_logCol = 0;

    // Reset both buffers to the cleared state so refresh() only writes
    // cells that are actually changed after clear().
    CellBuf def = { L' ', 7, 0, false };
    m_back.assign(static_cast<size_t>(m_rows * m_cols), def);
    m_front.assign(static_cast<size_t>(m_rows * m_cols), def);
}

void Win32Terminal::refresh()
{
    // Track the physical cursor position during this refresh pass.
    // Start with an invalid sentinel so the first dirty cell always
    // positions the cursor explicitly.
    int physRow = -1;
    int physCol = -1;

    // Track the last ANSI attribute that was emitted.
    // -1 means "not yet set" so the first dirty cell always emits.
    int emitFg     = -1;
    int emitBg     = -1;
    bool emitBright = false;

    for (int r = 0; r < m_rows; ++r)
    {
        int base = r * m_cols;
        int c = 0;
        while (c < m_cols)
        {
            // Skip clean cells
            if (m_back[base + c] == m_front[base + c])
            {
                ++c;
                continue;
            }

            // Found a dirty cell — build a run of same-attribute cells.
            int runStart = c;
            const CellBuf& ref = m_back[base + c];

            // Extend the run as long as cells share the same attribute.
            // We include clean cells within the run to avoid extra cursor
            // moves; we then trim the trailing clean cells back off.
            int runEnd = c;
            for (int nc = c + 1; nc < m_cols; ++nc)
            {
                const CellBuf& ncCell = m_back[base + nc];
                if (ncCell.fg != ref.fg || ncCell.bg != ref.bg || ncCell.bright != ref.bright)
                    break;
                runEnd = nc;
            }

            // Trim trailing clean cells from the run to avoid unnecessary writes
            while (runEnd > runStart && m_back[base + runEnd] == m_front[base + runEnd])
                --runEnd;

            int runLen = runEnd - runStart + 1;

            // Position the physical cursor if it isn't already at runStart
            if (r != physRow || runStart != physCol)
            {
                COORD pos = { static_cast<SHORT>(runStart), static_cast<SHORT>(r) };
                SetConsoleCursorPosition(m_hConsole, pos);
                physRow = r;
                physCol = runStart;
            }

            // Emit an ANSI color sequence only when the attribute changes
            if (ref.fg != emitFg || ref.bg != emitBg || ref.bright != emitBright)
            {
                int fgCode = ref.bright ? (90 + ref.fg) : (30 + ref.fg);
                int bgCode = 40 + ref.bg;
                char seq[20];
                snprintf(seq, sizeof(seq), "\033[0;%d;%dm", fgCode, bgCode);
                DWORD written;
                WriteConsoleA(m_hConsole, seq,
                              static_cast<DWORD>(strlen(seq)), &written, NULL);
                emitFg     = ref.fg;
                emitBg     = ref.bg;
                emitBright = ref.bright;
            }

            // Build and emit the wide-char run in one WriteConsoleW call
            for (int i = 0; i < runLen; ++i)
                m_wbuf[i] = m_back[base + runStart + i].ch;

            DWORD written;
            WriteConsoleW(m_hConsole, m_wbuf.data(),
                          static_cast<DWORD>(runLen), &written, NULL);

            // Sync the front buffer for these cells
            for (int i = 0; i < runLen; ++i)
                m_front[base + runStart + i] = m_back[base + runStart + i];

            physCol = runStart + runLen;
            c = runEnd + 1;
        }
    }

    // Restore the physical cursor to the logical position so that
    // input / cursor-visible operations land at the right place.
    if (physRow != m_logRow || physCol != m_logCol)
    {
        COORD pos = { static_cast<SHORT>(m_logCol), static_cast<SHORT>(m_logRow) };
        SetConsoleCursorPosition(m_hConsole, pos);
    }
}

void Win32Terminal::clearToEol()
{
    // Fill the back buffer from the current logical column to end of row
    // with spaces in the current pending attribute.
    if (m_logRow < 0 || m_logRow >= m_rows) return;
    int base = m_logRow * m_cols;
    for (int c = m_logCol; c < m_cols; ++c)
    {
        m_back[base + c] = { L' ', m_pendFg, m_pendBg, m_pendBright };
    }
}

// --- Cursor ---

void Win32Terminal::moveTo(int row, int col)
{
    // Only update the logical cursor; the physical cursor is moved in refresh().
    m_logRow = row;
    m_logCol = col;
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
    // Store as the pending attribute; no console output until refresh().
    m_pendFg     = attr.fg;
    m_pendBg     = attr.bg;
    m_pendBright = attr.bright;
}

void Win32Terminal::resetAttr()
{
    m_pendFg     = 7; // TC_WHITE
    m_pendBg     = 0; // TC_BLACK
    m_pendBright = false;
}

// --- Text output ---

void Win32Terminal::printStr(int row, int col, const string& text)
{
    if (text.empty() || row < 0 || row >= m_rows) return;
    m_logRow = row;
    m_logCol = col;

    // Convert the UTF-8 string to UTF-16 wide chars
    int wlen = MultiByteToWideChar(CP_UTF8, 0,
                                   text.c_str(), static_cast<int>(text.size()),
                                   nullptr, 0);
    if (wlen <= 0) return;

    std::vector<wchar_t> wide(static_cast<size_t>(wlen));
    MultiByteToWideChar(CP_UTF8, 0,
                        text.c_str(), static_cast<int>(text.size()),
                        wide.data(), wlen);

    for (int i = 0; i < wlen && m_logCol >= 0 && m_logCol < m_cols; ++i, ++m_logCol)
    {
        m_back[m_logRow * m_cols + m_logCol] = { wide[i], m_pendFg, m_pendBg, m_pendBright };
    }
}

void Win32Terminal::putCh(int row, int col, int ch)
{
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) return;
    m_logRow = row;
    m_logCol = col + 1;
    m_back[row * m_cols + col] = { static_cast<wchar_t>(ch),
                                   m_pendFg, m_pendBg, m_pendBright };
}

void Win32Terminal::fillRegion(int row, int startCol, int endCol, char ch)
{
    int len = endCol - startCol;
    if (len <= 0 || row < 0 || row >= m_rows) return;
    wchar_t wc = static_cast<wchar_t>(static_cast<unsigned char>(ch));
    int base = row * m_cols;
    for (int c = startCol; c < endCol && c < m_cols; ++c)
    {
        m_back[base + c] = { wc, m_pendFg, m_pendBg, m_pendBright };
    }
}

// --- CP437 character output ---

void Win32Terminal::putCP437(int row, int col, int cp437char)
{
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) return;
    m_logRow = row;
    m_logCol = col + 1;
    wchar_t wc = s_cp437[static_cast<unsigned char>(cp437char)];
    m_back[row * m_cols + col] = { wc, m_pendFg, m_pendBg, m_pendBright };
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
