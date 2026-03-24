#include "theme.h"

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cctype>

using std::string;
using std::vector;
using std::map;

// ============================================================
// Synchronet attribute string parser
// ============================================================

TermAttr parseSyncAttr(const string& attrStr)
{
    int fg = TC_WHITE;
    int bg = TC_BLACK;
    bool bright = false;

    for (size_t i = 0; i < attrStr.size(); ++i)
    {
        char ch = attrStr[i];
        switch (ch)
        {
            case 'n': case 'N':
                fg = TC_WHITE;
                bg = TC_BLACK;
                bright = false;
                break;
            case 'k': case 'K':
                fg = TC_BLACK;
                break;
            case 'r': case 'R':
                fg = TC_RED;
                break;
            case 'g': case 'G':
                fg = TC_GREEN;
                break;
            case 'y': case 'Y':
                fg = TC_YELLOW;
                break;
            case 'b': case 'B':
                fg = TC_BLUE;
                break;
            case 'm': case 'M':
                fg = TC_MAGENTA;
                break;
            case 'c': case 'C':
                fg = TC_CYAN;
                break;
            case 'w': case 'W':
                fg = TC_WHITE;
                break;
            case 'h': case 'H':
                bright = true;
                break;
            case '0':
                bg = TC_BLACK;
                break;
            case '1':
                bg = TC_RED;
                break;
            case '2':
                bg = TC_GREEN;
                break;
            case '3':
                bg = TC_YELLOW;
                break;
            case '4':
                bg = TC_BLUE;
                break;
            case '5':
                bg = TC_MAGENTA;
                break;
            case '6':
                bg = TC_CYAN;
                break;
            case '7':
                bg = TC_WHITE;
                break;
            default:
                break;
        }
    }

    return TermAttr(fg, bg, bright);
}

// ============================================================
// INI file reader
// ============================================================

map<string, string> readIniFile(const string& path)
{
    map<string, string> kvMap;
    std::ifstream inFile(path);
    if (!inFile.is_open())
    {
        return kvMap;
    }

    string line;
    while (std::getline(inFile, line))
    {
        // Trim leading whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == string::npos)
        {
            continue;
        }
        line = line.substr(start);

        // Skip comments and section headers
        if (line.empty() || line[0] == ';' || line[0] == '#' || line[0] == '[')
        {
            continue;
        }

        // Find '=' separator
        size_t eqPos = line.find('=');
        if (eqPos == string::npos)
        {
            continue;
        }

        string key = line.substr(0, eqPos);
        string val = line.substr(eqPos + 1);

        // Trim trailing whitespace from key
        size_t keyEnd = key.find_last_not_of(" \t");
        if (keyEnd != string::npos)
        {
            key = key.substr(0, keyEnd + 1);
        }

        // Trim whitespace from value
        size_t valStart = val.find_first_not_of(" \t");
        if (valStart != string::npos)
        {
            val = val.substr(valStart);
        }
        else
        {
            val = "";
        }
        size_t valEnd = val.find_last_not_of(" \t\r\n");
        if (valEnd != string::npos)
        {
            val = val.substr(0, valEnd + 1);
        }

        kvMap[key] = val;
    }

    return kvMap;
}

// Helper: look up a key in the map and parse it, or return a default TermAttr
TermAttr getAttrOrDefault(const map<string, string>& kvMap,
                          const string& key,
                          const TermAttr& defaultAttr)
{
    auto it = kvMap.find(key);
    if (it != kvMap.end() && !it->second.empty())
    {
        return parseSyncAttr(it->second);
    }
    return defaultAttr;
}

// ============================================================
// Load Ice Theme
// ============================================================

IceTheme loadIceTheme(const string& path)
{
    IceTheme theme;
    auto kv = readIniFile(path);

    // Defaults match EditorIceColors_BlueIce.ini
    theme.borderColor1              = getAttrOrDefault(kv, "BorderColor1",              TermAttr(TC_BLUE,   TC_BLACK, false));
    theme.borderColor2              = getAttrOrDefault(kv, "BorderColor2",              TermAttr(TC_BLUE,   TC_BLACK, true));
    theme.quoteLineColor            = getAttrOrDefault(kv, "QuoteLineColor",            TermAttr(TC_CYAN,   TC_BLACK, false));
    theme.keyInfoLabelColor         = getAttrOrDefault(kv, "KeyInfoLabelColor",         TermAttr(TC_CYAN,   TC_BLACK, true));

    theme.topInfoBkgColor           = getAttrOrDefault(kv, "TopInfoBkgColor",           TermAttr(TC_WHITE,  TC_BLUE,  false));
    theme.topLabelColor             = getAttrOrDefault(kv, "TopLabelColor",             TermAttr(TC_CYAN,   TC_BLACK, true));
    theme.topLabelColonColor        = getAttrOrDefault(kv, "TopLabelColonColor",        TermAttr(TC_BLUE,   TC_BLACK, true));
    theme.topToColor                = getAttrOrDefault(kv, "TopToColor",                TermAttr(TC_WHITE,  TC_BLACK, true));
    theme.topFromColor              = getAttrOrDefault(kv, "TopFromColor",              TermAttr(TC_WHITE,  TC_BLACK, true));
    theme.topSubjectColor           = getAttrOrDefault(kv, "TopSubjectColor",           TermAttr(TC_WHITE,  TC_BLACK, true));
    theme.topTimeColor              = getAttrOrDefault(kv, "TopTimeColor",              TermAttr(TC_GREEN,  TC_BLACK, true));
    theme.topTimeLeftColor          = getAttrOrDefault(kv, "TopTimeLeftColor",          TermAttr(TC_GREEN,  TC_BLACK, true));
    theme.editMode                  = getAttrOrDefault(kv, "EditMode",                  TermAttr(TC_CYAN,   TC_BLACK, true));

    theme.quoteWinText              = getAttrOrDefault(kv, "QuoteWinText",              TermAttr(TC_WHITE,  TC_BLACK, true));
    theme.quoteLineHighlightColor   = getAttrOrDefault(kv, "QuoteLineHighlightColor",   TermAttr(TC_CYAN,   TC_BLUE,  true));
    theme.quoteWinBorderTextColor   = getAttrOrDefault(kv, "QuoteWinBorderTextColor",   TermAttr(TC_CYAN,   TC_BLACK, true));

    theme.selectedOptionBorderColor   = getAttrOrDefault(kv, "SelectedOptionBorderColor",   TermAttr(TC_BLUE,  TC_BLUE,  true));
    theme.selectedOptionTextColor     = getAttrOrDefault(kv, "SelectedOptionTextColor",     TermAttr(TC_CYAN,  TC_BLUE,  true));
    theme.unselectedOptionBorderColor = getAttrOrDefault(kv, "UnselectedOptionBorderColor", TermAttr(TC_BLUE,  TC_BLACK, false));
    theme.unselectedOptionTextColor   = getAttrOrDefault(kv, "UnselectedOptionTextColor",   TermAttr(TC_WHITE, TC_BLACK, false));

    theme.listBoxBorder             = getAttrOrDefault(kv, "listBoxBorder",             TermAttr(TC_BLUE,   TC_BLACK, false));
    theme.listBoxBorderText         = getAttrOrDefault(kv, "listBoxBorderText",         TermAttr(TC_BLUE,   TC_BLACK, true));
    theme.listBoxItemText           = getAttrOrDefault(kv, "listBoxItemText",           TermAttr(TC_CYAN,   TC_BLACK, false));
    theme.listBoxItemHighlight      = getAttrOrDefault(kv, "listBoxItemHighlight",      TermAttr(TC_BLUE,   TC_WHITE, false));

    return theme;
}

// ============================================================
// Load DCT Theme
// ============================================================

DctTheme loadDctTheme(const string& path)
{
    DctTheme theme;
    auto kv = readIniFile(path);

    // Defaults match EditorDCTColors_Default.ini
    theme.topBorderColor1           = getAttrOrDefault(kv, "TopBorderColor1",           TermAttr(TC_RED,     TC_BLACK, false));
    theme.topBorderColor2           = getAttrOrDefault(kv, "TopBorderColor2",           TermAttr(TC_RED,     TC_BLACK, true));
    theme.editAreaBorderColor1      = getAttrOrDefault(kv, "EditAreaBorderColor1",      TermAttr(TC_GREEN,   TC_BLACK, false));
    theme.editAreaBorderColor2      = getAttrOrDefault(kv, "EditAreaBorderColor2",      TermAttr(TC_GREEN,   TC_BLACK, true));
    theme.editModeBrackets          = getAttrOrDefault(kv, "EditModeBrackets",          TermAttr(TC_BLACK,   TC_BLACK, true));
    theme.editMode                  = getAttrOrDefault(kv, "EditMode",                  TermAttr(TC_WHITE,   TC_BLACK, false));

    theme.topLabelColor             = getAttrOrDefault(kv, "TopLabelColor",             TermAttr(TC_BLUE,    TC_BLACK, true));
    theme.topLabelColonColor        = getAttrOrDefault(kv, "TopLabelColonColor",        TermAttr(TC_BLUE,    TC_BLACK, false));
    theme.topFromColor              = getAttrOrDefault(kv, "TopFromColor",              TermAttr(TC_CYAN,    TC_BLACK, true));
    theme.topFromFillColor          = getAttrOrDefault(kv, "TopFromFillColor",          TermAttr(TC_CYAN,    TC_BLACK, false));
    theme.topToColor                = getAttrOrDefault(kv, "TopToColor",                TermAttr(TC_CYAN,    TC_BLACK, true));
    theme.topToFillColor            = getAttrOrDefault(kv, "TopToFillColor",            TermAttr(TC_CYAN,    TC_BLACK, false));
    theme.topSubjColor              = getAttrOrDefault(kv, "TopSubjColor",              TermAttr(TC_WHITE,   TC_BLACK, true));
    theme.topSubjFillColor          = getAttrOrDefault(kv, "TopSubjFillColor",          TermAttr(TC_WHITE,   TC_BLACK, false));
    theme.topAreaColor              = getAttrOrDefault(kv, "TopAreaColor",              TermAttr(TC_GREEN,   TC_BLACK, true));
    theme.topAreaFillColor          = getAttrOrDefault(kv, "TopAreaFillColor",          TermAttr(TC_GREEN,   TC_BLACK, false));
    theme.topTimeColor              = getAttrOrDefault(kv, "TopTimeColor",              TermAttr(TC_YELLOW,  TC_BLACK, true));
    theme.topTimeFillColor          = getAttrOrDefault(kv, "TopTimeFillColor",          TermAttr(TC_RED,     TC_BLACK, false));
    theme.topTimeLeftColor          = getAttrOrDefault(kv, "TopTimeLeftColor",          TermAttr(TC_YELLOW,  TC_BLACK, true));
    theme.topTimeLeftFillColor      = getAttrOrDefault(kv, "TopTimeLeftFillColor",      TermAttr(TC_RED,     TC_BLACK, false));
    theme.topInfoBracketColor       = getAttrOrDefault(kv, "TopInfoBracketColor",       TermAttr(TC_MAGENTA, TC_BLACK, false));

    theme.quoteWinText              = getAttrOrDefault(kv, "QuoteWinText",              TermAttr(TC_BLUE,    TC_WHITE, false));
    theme.quoteLineHighlightColor   = getAttrOrDefault(kv, "QuoteLineHighlightColor",   TermAttr(TC_WHITE,   TC_BLACK, false));
    theme.quoteWinBorderTextColor   = getAttrOrDefault(kv, "QuoteWinBorderTextColor",   TermAttr(TC_RED,     TC_WHITE, false));
    theme.quoteWinBorderColor       = getAttrOrDefault(kv, "QuoteWinBorderColor",       TermAttr(TC_BLACK,   TC_WHITE, false));

    theme.bottomHelpBrackets        = getAttrOrDefault(kv, "BottomHelpBrackets",        TermAttr(TC_BLACK,   TC_BLACK, true));
    theme.bottomHelpKeys            = getAttrOrDefault(kv, "BottomHelpKeys",            TermAttr(TC_RED,     TC_BLACK, true));
    theme.bottomHelpFill            = getAttrOrDefault(kv, "BottomHelpFill",            TermAttr(TC_RED,     TC_BLACK, false));
    theme.bottomHelpKeyDesc         = getAttrOrDefault(kv, "BottomHelpKeyDesc",         TermAttr(TC_CYAN,    TC_BLACK, false));

    theme.textBoxBorder             = getAttrOrDefault(kv, "TextBoxBorder",             TermAttr(TC_BLACK,   TC_WHITE, false));
    theme.textBoxBorderText         = getAttrOrDefault(kv, "TextBoxBorderText",         TermAttr(TC_RED,     TC_WHITE, false));
    theme.textBoxInnerText          = getAttrOrDefault(kv, "TextBoxInnerText",          TermAttr(TC_BLUE,    TC_WHITE, false));
    theme.yesNoBoxBrackets          = getAttrOrDefault(kv, "YesNoBoxBrackets",          TermAttr(TC_BLACK,   TC_WHITE, false));
    theme.yesNoBoxYesNoText         = getAttrOrDefault(kv, "YesNoBoxYesNoText",         TermAttr(TC_WHITE,   TC_WHITE, true));

    theme.listBoxBorder             = getAttrOrDefault(kv, "listBoxBorder",             TermAttr(TC_GREEN,   TC_BLACK, false));
    theme.listBoxBorderText         = getAttrOrDefault(kv, "listBoxBorderText",         TermAttr(TC_BLUE,    TC_BLACK, true));
    theme.listBoxItemText           = getAttrOrDefault(kv, "listBoxItemText",           TermAttr(TC_CYAN,    TC_BLACK, false));
    theme.listBoxItemHighlight      = getAttrOrDefault(kv, "listBoxItemHighlight",      TermAttr(TC_WHITE,   TC_BLUE,  true));

    return theme;
}

// ============================================================
// Random two-color border drawing
// ============================================================

void drawRandomTwoColorLine(int row, int startCol, const vector<int>& chars,
                            const TermAttr& color1, const TermAttr& color2)
{
    for (size_t i = 0; i < chars.size(); ++i)
    {
        const TermAttr& chosen = (std::rand() % 2 == 0) ? color1 : color2;
        g_term->setAttr(chosen);
        g_term->putCP437(row, startCol + static_cast<int>(i), chars[i]);
    }
}

void drawRandomTwoColorHLine(int row, int col, int len,
                             const TermAttr& color1, const TermAttr& color2)
{
    for (int i = 0; i < len; ++i)
    {
        const TermAttr& chosen = (std::rand() % 2 == 0) ? color1 : color2;
        g_term->setAttr(chosen);
        g_term->putCP437(row, col + i, CP437_BOX_DRAWINGS_HORIZONTAL_SINGLE);
    }
}
