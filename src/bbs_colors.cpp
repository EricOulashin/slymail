#include "bbs_colors.h"

using std::string;
using std::vector;

// ============================================================
// Internal helpers
// ============================================================

// Synchronet Ctrl-A code background color digit to TermColor
static int syncBgDigitToColor(char ch)
{
    switch (ch)
    {
        case '0': return TC_BLACK;
        case '1': return TC_RED;
        case '2': return TC_GREEN;
        case '3': return TC_YELLOW;
        case '4': return TC_BLUE;
        case '5': return TC_MAGENTA;
        case '6': return TC_CYAN;
        case '7': return TC_WHITE;
        default:  return -1;
    }
}

// Synchronet Ctrl-A code foreground letter to TermColor
static int syncFgLetterToColor(char ch)
{
    switch (ch)
    {
        case 'k': case 'K': return TC_BLACK;
        case 'r': case 'R': return TC_RED;
        case 'g': case 'G': return TC_GREEN;
        case 'y': case 'Y': return TC_YELLOW;
        case 'b': case 'B': return TC_BLUE;
        case 'm': case 'M': return TC_MAGENTA;
        case 'c': case 'C': return TC_CYAN;
        case 'w': case 'W': return TC_WHITE;
        default:  return -1;
    }
}

// Apply a Synchronet Ctrl-A attribute character to a TermAttr
static bool applySyncAttr(char ch, TermAttr& attr)
{
    // Normal/reset
    if (ch == 'n' || ch == 'N' || ch == '-' || ch == '_')
    {
        attr = tAttr(TC_WHITE, TC_BLACK, false);
        return true;
    }
    // High intensity
    if (ch == 'h' || ch == 'H')
    {
        attr.bright = true;
        return true;
    }
    // Blink (we don't support blink, but consume the code)
    if (ch == 'i' || ch == 'I' || ch == 'f' || ch == 'F')
    {
        return true;
    }
    // Foreground color
    int fg = syncFgLetterToColor(ch);
    if (fg >= 0)
    {
        attr.fg = fg;
        return true;
    }
    // Background color
    int bg = syncBgDigitToColor(ch);
    if (bg >= 0)
    {
        attr.bg = bg;
        return true;
    }
    return false;
}

// ============================================================
// ANSI SGR parameter handling
// ============================================================

void applyAnsiSgrParam(int param, TermAttr& attr)
{
    switch (param)
    {
        case 0: // Reset all attributes
            attr = tAttr(TC_WHITE, TC_BLACK, false);
            break;
        case 1: // Bold / Bright
            attr.bright = true;
            break;
        case 2:  // Dim / Normal intensity (turn off bold)
        case 22: // Normal intensity (also turns off bold)
            attr.bright = false;
            break;
        case 4:  // Underline — no terminal support, consume silently
        case 24: // Underline off
            break;
        case 5:  // Blink
        case 6:  // Rapid blink
        case 25: // Blink off
            break;
        case 7: // Reverse video — swap fg and bg
        {
            int tmp = attr.fg;
            attr.fg = attr.bg;
            attr.bg = tmp;
            break;
        }
        case 27: // Reverse video off — swap back (best effort)
        {
            int tmp = attr.fg;
            attr.fg = attr.bg;
            attr.bg = tmp;
            break;
        }
        case 8:  // Conceal / Hidden — set fg to bg color
            attr.fg = attr.bg;
            break;
        case 28: // Reveal (opposite of conceal) — reset fg to white
            attr.fg = TC_WHITE;
            break;

        // Standard foreground colors (30-37)
        case 30: case 31: case 32: case 33:
        case 34: case 35: case 36: case 37:
            attr.fg = param - 30;
            break;

        case 39: // Default foreground color
            attr.fg = TC_WHITE;
            attr.bright = false;
            break;

        // Standard background colors (40-47)
        case 40: case 41: case 42: case 43:
        case 44: case 45: case 46: case 47:
            attr.bg = param - 40;
            break;

        case 49: // Default background color
            attr.bg = TC_BLACK;
            break;

        // Bright/Aixterm foreground colors (90-97)
        case 90: case 91: case 92: case 93:
        case 94: case 95: case 96: case 97:
            attr.fg = param - 90;
            attr.bright = true;
            break;

        // Bright/Aixterm background colors (100-107)
        // Map to normal bg colors (terminal can't do bright backgrounds)
        case 100: case 101: case 102: case 103:
        case 104: case 105: case 106: case 107:
            attr.bg = param - 100;
            break;

        // 38 and 48 are handled specially in applyAnsiSgrParams (extended color)
        default:
            break;
    }
}

// Apply a full vector of SGR parameters, handling extended color sequences
// (38;5;n for 256-color, 38;2;r;g;b for truecolor, and 48;5;n, 48;2;r;g;b)
static void applyAnsiSgrParams(const vector<int>& params, TermAttr& attr)
{
    for (size_t i = 0; i < params.size(); ++i)
    {
        int p = params[i];

        // Extended foreground color: 38;5;n (256-color) or 38;2;r;g;b (truecolor)
        if (p == 38 && i + 1 < params.size())
        {
            if (params[i + 1] == 5 && i + 2 < params.size())
            {
                // 256-color mode: map to nearest 8-color
                int colorIdx = params[i + 2];
                if (colorIdx >= 0 && colorIdx <= 7)
                {
                    attr.fg = colorIdx;
                    attr.bright = false;
                }
                else if (colorIdx >= 8 && colorIdx <= 15)
                {
                    attr.fg = colorIdx - 8;
                    attr.bright = true;
                }
                else if (colorIdx >= 16 && colorIdx <= 231)
                {
                    // 216-color cube: map to nearest 8-color
                    int ci = colorIdx - 16;
                    int r = ci / 36, g = (ci / 6) % 6, b = ci % 6;
                    // Simple mapping: find dominant channel
                    if (r > g && r > b) attr.fg = TC_RED;
                    else if (g > r && g > b) attr.fg = TC_GREEN;
                    else if (b > r && b > g) attr.fg = TC_BLUE;
                    else if (r > 0 && g > 0 && b == 0) attr.fg = TC_YELLOW;
                    else if (r > 0 && b > 0 && g == 0) attr.fg = TC_MAGENTA;
                    else if (g > 0 && b > 0 && r == 0) attr.fg = TC_CYAN;
                    else if (r > 3 || g > 3 || b > 3) { attr.fg = TC_WHITE; attr.bright = true; }
                    else if (r > 0 || g > 0 || b > 0) attr.fg = TC_WHITE;
                    else attr.fg = TC_BLACK;
                }
                else
                {
                    // Grayscale ramp (232-255): map to black, dark gray, white
                    int gray = colorIdx - 232; // 0-23
                    if (gray < 8) { attr.fg = TC_BLACK; attr.bright = true; }
                    else if (gray < 16) attr.fg = TC_WHITE;
                    else { attr.fg = TC_WHITE; attr.bright = true; }
                }
                i += 2;
                continue;
            }
            else if (params[i + 1] == 2 && i + 4 < params.size())
            {
                // Truecolor: map r;g;b to nearest 8-color
                int r = params[i + 2], g = params[i + 3], b = params[i + 4];
                if (r > g && r > b) attr.fg = TC_RED;
                else if (g > r && g > b) attr.fg = TC_GREEN;
                else if (b > r && b > g) attr.fg = TC_BLUE;
                else if (r > 128 && g > 128 && b < 64) attr.fg = TC_YELLOW;
                else if (r > 128 && b > 128 && g < 64) attr.fg = TC_MAGENTA;
                else if (g > 128 && b > 128 && r < 64) attr.fg = TC_CYAN;
                else if (r > 192 || g > 192 || b > 192) { attr.fg = TC_WHITE; attr.bright = true; }
                else if (r > 64 || g > 64 || b > 64) attr.fg = TC_WHITE;
                else attr.fg = TC_BLACK;
                i += 4;
                continue;
            }
        }

        // Extended background color: 48;5;n or 48;2;r;g;b
        if (p == 48 && i + 1 < params.size())
        {
            if (params[i + 1] == 5 && i + 2 < params.size())
            {
                int colorIdx = params[i + 2];
                if (colorIdx >= 0 && colorIdx <= 7)
                {
                    attr.bg = colorIdx;
                }
                else if (colorIdx >= 8 && colorIdx <= 15)
                {
                    attr.bg = colorIdx - 8;
                }
                else if (colorIdx >= 16 && colorIdx <= 231)
                {
                    int ci = colorIdx - 16;
                    int r = ci / 36, g = (ci / 6) % 6, b = ci % 6;
                    if (r > g && r > b) attr.bg = TC_RED;
                    else if (g > r && g > b) attr.bg = TC_GREEN;
                    else if (b > r && b > g) attr.bg = TC_BLUE;
                    else if (r > 0 && g > 0) attr.bg = TC_YELLOW;
                    else if (r > 0 && b > 0) attr.bg = TC_MAGENTA;
                    else if (g > 0 && b > 0) attr.bg = TC_CYAN;
                    else if (r > 0 || g > 0 || b > 0) attr.bg = TC_WHITE;
                    else attr.bg = TC_BLACK;
                }
                else
                {
                    int gray = colorIdx - 232;
                    if (gray < 12) attr.bg = TC_BLACK;
                    else attr.bg = TC_WHITE;
                }
                i += 2;
                continue;
            }
            else if (params[i + 1] == 2 && i + 4 < params.size())
            {
                int r = params[i + 2], g = params[i + 3], b = params[i + 4];
                if (r > g && r > b) attr.bg = TC_RED;
                else if (g > r && g > b) attr.bg = TC_GREEN;
                else if (b > r && b > g) attr.bg = TC_BLUE;
                else if (r > 128 && g > 128) attr.bg = TC_YELLOW;
                else if (r > 128 && b > 128) attr.bg = TC_MAGENTA;
                else if (g > 128 && b > 128) attr.bg = TC_CYAN;
                else if (r > 64 || g > 64 || b > 64) attr.bg = TC_WHITE;
                else attr.bg = TC_BLACK;
                i += 4;
                continue;
            }
        }

        // Standard single-parameter SGR code
        applyAnsiSgrParam(p, attr);
    }
}

// Parse ANSI escape sequence starting at ESC[ and return number of chars consumed
// (including the ESC and [). Updates attr with any SGR parameters found.
// Returns 0 if not a valid/recognized sequence.
static size_t parseAnsiSequence(const string& line, size_t pos, TermAttr& attr)
{
    // pos points to ESC (0x1B)
    if (pos + 1 >= line.size() || line[pos + 1] != '[')
    {
        return 0;
    }

    size_t i = pos + 2; // after ESC[
    // Collect parameters
    vector<int> params;
    int currentParam = 0;
    bool hasParam = false;

    while (i < line.size())
    {
        char ch = line[i];
        if (ch >= '0' && ch <= '9')
        {
            currentParam = currentParam * 10 + (ch - '0');
            hasParam = true;
            ++i;
        }
        else if (ch == ';')
        {
            params.push_back(hasParam ? currentParam : 0);
            currentParam = 0;
            hasParam = false;
            ++i;
        }
        else if (ch >= 0x40 && ch <= 0x7E) // Final byte
        {
            if (hasParam)
            {
                params.push_back(currentParam);
            }
            ++i; // consume final byte

            // Process SGR sequences (final byte 'm')
            if (ch == 'm')
            {
                if (params.empty())
                {
                    // ESC[m is the same as ESC[0m
                    applyAnsiSgrParam(0, attr);
                }
                else
                {
                    // Use the extended handler that supports 256-color and truecolor
                    applyAnsiSgrParams(params, attr);
                }
            }
            // For non-SGR sequences (cursor movement, screen clearing, etc.),
            // we consume them but don't change the color attribute.
            return i - pos;
        }
        else
        {
            // Invalid sequence character
            return 0;
        }
    }
    return 0; // Unterminated sequence
}

// ============================================================
// PCBoard @X## code handling
// ============================================================

static int hexDigitValue(char ch)
{
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    return -1;
}

// PCBoard @X## format: first hex digit = background, second = foreground
// Bit 3 of foreground = high intensity
static bool parsePCBoardCode(char bgHex, char fgHex, TermAttr& attr)
{
    int bgVal = hexDigitValue(bgHex);
    int fgVal = hexDigitValue(fgHex);
    if (bgVal < 0 || fgVal < 0) return false;

    // Foreground: low 3 bits = color, bit 3 = bright
    attr.fg = fgVal & 0x07;
    attr.bright = (fgVal & 0x08) != 0;
    // Background: low 3 bits = color (bit 3 = blink, ignored)
    attr.bg = bgVal & 0x07;
    return true;
}

// ============================================================
// WWIV heart code handling
// ============================================================

static bool applyWWIVCode(char digit, TermAttr& attr)
{
    switch (digit)
    {
        case '0': attr = tAttr(TC_WHITE, TC_BLACK, false); break;        // Normal
        case '1': attr = tAttr(TC_CYAN, TC_BLACK, true); break;          // Bright cyan
        case '2': attr = tAttr(TC_YELLOW, TC_BLACK, true); break;        // Bright yellow
        case '3': attr = tAttr(TC_MAGENTA, TC_BLACK, false); break;      // Magenta
        case '4': attr = tAttr(TC_WHITE, TC_BLUE, true); break;          // Bright white on blue
        case '5': attr = tAttr(TC_GREEN, TC_BLACK, false); break;        // Green
        case '6': attr = tAttr(TC_RED, TC_BLACK, true); break;           // Bright red (blink ignored)
        case '7': attr = tAttr(TC_BLUE, TC_BLACK, true); break;          // Bright blue
        case '8': attr = tAttr(TC_BLUE, TC_BLACK, false); break;         // Blue
        case '9': attr = tAttr(TC_CYAN, TC_BLACK, false); break;         // Cyan
        default: return false;
    }
    return true;
}

// ============================================================
// Celerity pipe code handling
// ============================================================

static bool applyCelerityCode(char ch, TermAttr& attr)
{
    // Lowercase = normal intensity foreground on black
    switch (ch)
    {
        case 'k': attr = tAttr(TC_BLACK, TC_BLACK, false); return true;
        case 'b': attr = tAttr(TC_BLUE, TC_BLACK, false); return true;
        case 'g': attr = tAttr(TC_GREEN, TC_BLACK, false); return true;
        case 'c': attr = tAttr(TC_CYAN, TC_BLACK, false); return true;
        case 'r': attr = tAttr(TC_RED, TC_BLACK, false); return true;
        case 'm': attr = tAttr(TC_MAGENTA, TC_BLACK, false); return true;
        case 'y': attr = tAttr(TC_YELLOW, TC_BLACK, false); return true;
        case 'w': attr = tAttr(TC_WHITE, TC_BLACK, false); return true;
        // Dark/bright
        case 'd': attr = tAttr(TC_BLACK, TC_BLACK, true); return true;
        // Uppercase = high intensity foreground on black
        case 'B': attr = tAttr(TC_BLUE, TC_BLACK, true); return true;
        case 'G': attr = tAttr(TC_GREEN, TC_BLACK, true); return true;
        case 'C': attr = tAttr(TC_CYAN, TC_BLACK, true); return true;
        case 'R': attr = tAttr(TC_RED, TC_BLACK, true); return true;
        case 'M': attr = tAttr(TC_MAGENTA, TC_BLACK, true); return true;
        case 'Y': attr = tAttr(TC_YELLOW, TC_BLACK, true); return true;
        case 'W': attr = tAttr(TC_WHITE, TC_BLACK, true); return true;
        case 'S': return true; // Swap (not fully implemented - would need tracking)
        default: return false;
    }
}

// ============================================================
// Renegade pipe code handling
// ============================================================

static bool applyRenegadeCode(int code, TermAttr& attr)
{
    // 00-07: Normal foreground colors
    if (code >= 0 && code <= 7)
    {
        attr.fg = code;
        attr.bright = false;
        return true;
    }
    // 08-15: High intensity foreground colors
    if (code >= 8 && code <= 15)
    {
        attr.fg = code - 8;
        attr.bright = true;
        return true;
    }
    // 16-23: Background colors
    if (code >= 16 && code <= 23)
    {
        // Renegade bg order: black, blue, green, cyan, red, magenta, yellow, white
        static const int bgMap[] = { TC_BLACK, TC_BLUE, TC_GREEN, TC_CYAN,
                                     TC_RED, TC_MAGENTA, TC_YELLOW, TC_WHITE };
        attr.bg = bgMap[code - 16];
        return true;
    }
    // 24-31: Blinking (we set the color but ignore blink)
    if (code >= 24 && code <= 31)
    {
        static const int bgMap[] = { TC_BLACK, TC_BLUE, TC_GREEN, TC_CYAN,
                                     TC_RED, TC_MAGENTA, TC_YELLOW, TC_WHITE };
        attr.fg = TC_WHITE;
        attr.bright = false;
        attr.bg = bgMap[code - 24];
        return true;
    }
    return false;
}

// ============================================================
// Main parsing function
// ============================================================

vector<ColoredSegment> parseBBSColors(const string& line,
                                       TermAttr& currentAttr,
                                       const AttrCodeFlags& flags)
{
    vector<ColoredSegment> segments;
    string currentText;

    auto flushText = [&]()
    {
        if (!currentText.empty())
        {
            segments.push_back({currentText, currentAttr});
            currentText.clear();
        }
    };

    size_t i = 0;
    while (i < line.size())
    {
        uint8_t ch = static_cast<uint8_t>(line[i]);

        // --- ANSI escape sequence (always enabled) ---
        if (ch == 0x1B && i + 1 < line.size() && line[i + 1] == '[')
        {
            flushText();
            size_t consumed = parseAnsiSequence(line, i, currentAttr);
            if (consumed > 0)
            {
                i += consumed;
                continue;
            }
            // Not a valid sequence, output ESC as-is
        }

        // --- Synchronet Ctrl-A codes ---
        if (flags.synchronet && ch == 0x01 && i + 1 < line.size())
        {
            char next = line[i + 1];
            if (applySyncAttr(next, currentAttr))
            {
                flushText();
                i += 2;
                continue;
            }
            // Not a recognized Ctrl-A code, output as-is
        }

        // --- WWIV heart codes (Ctrl-C + digit) ---
        if (flags.wwiv && ch == 0x03 && i + 1 < line.size())
        {
            char next = line[i + 1];
            if (next >= '0' && next <= '9')
            {
                flushText();
                applyWWIVCode(next, currentAttr);
                i += 2;
                continue;
            }
        }

        // --- PCBoard/Wildcat @X## codes ---
        if (flags.pcboard && ch == '@' && i + 3 < line.size() &&
            (line[i + 1] == 'X' || line[i + 1] == 'x'))
        {
            char h1 = line[i + 2];
            char h2 = line[i + 3];
            if (hexDigitValue(h1) >= 0 && hexDigitValue(h2) >= 0)
            {
                flushText();
                parsePCBoardCode(h1, h2, currentAttr);
                i += 4;
                continue;
            }
        }

        // --- Pipe codes (Renegade and/or Celerity) ---
        // Renegade is checked first since its pattern (|NN) is more specific.
        if (ch == '|' && i + 1 < line.size())
        {
            if (flags.renegade && i + 2 < line.size())
            {
                char d1 = line[i + 1];
                char d2 = line[i + 2];
                if (d1 >= '0' && d1 <= '3' && d2 >= '0' && d2 <= '9')
                {
                    int code = (d1 - '0') * 10 + (d2 - '0');
                    if (code <= 31)
                    {
                        flushText();
                        applyRenegadeCode(code, currentAttr);
                        i += 3;
                        continue;
                    }
                }
            }
            if (flags.celerity)
            {
                char next = line[i + 1];
                if (applyCelerityCode(next, currentAttr))
                {
                    flushText();
                    i += 2;
                    continue;
                }
            }
        }

        // --- Regular character ---
        currentText += static_cast<char>(ch);
        ++i;
    }

    flushText();
    return segments;
}

// ============================================================
// Strip all BBS color codes
// ============================================================

string stripBBSColors(const string& line, const AttrCodeFlags& flags)
{
    TermAttr dummy = tAttr(TC_WHITE, TC_BLACK, false);
    auto segments = parseBBSColors(line, dummy, flags);
    string result;
    for (const auto& seg : segments)
    {
        result += seg.text;
    }
    return result;
}

// ============================================================
// Convert TermAttr to ANSI SGR sequence
// ============================================================

string termAttrToAnsi(const TermAttr& attr)
{
    // Always reset first, then set attributes
    string result = "\x1b[0";
    if (attr.bright)
    {
        result += ";1";
    }
    result += ";" + std::to_string(30 + attr.fg);
    if (attr.bg != TC_BLACK)
    {
        result += ";" + std::to_string(40 + attr.bg);
    }
    result += "m";
    return result;
}
