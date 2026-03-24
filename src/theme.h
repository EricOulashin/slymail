#ifndef SLYMAIL_THEME_H
#define SLYMAIL_THEME_H

// SlyMail Theme Loading
//
// Parses Synchronet-style color attribute strings from INI theme files
// and populates Ice-style and DCT-style theme structs.

#include "terminal.h"

#include <string>
#include <vector>
#include <map>

// ============================================================
// Synchronet attribute string parser
// ============================================================
// Parses strings like "nbh", "n4hc", "nhw" into TermAttr.
//
// Format:
//   n = normal (reset to defaults), always appears first if present
//   Foreground letters: k=black r=red g=green y=yellow b=blue m=magenta c=cyan w=white
//   h = high/bright
//   Background digits: 0=black 1=red 2=green 3=yellow/brown 4=blue 5=magenta 6=cyan 7=lightgray
//
TermAttr parseSyncAttr(const std::string& attrStr);

// ============================================================
// INI file reader (simple key=value, ignoring comments/sections)
// ============================================================
std::map<std::string, std::string> readIniFile(const std::string& path);

// Helper: look up a key in the map and parse it, or return a default TermAttr
TermAttr getAttrOrDefault(const std::map<std::string, std::string>& kvMap,
                          const std::string& key,
                          const TermAttr& defaultAttr);

// ============================================================
// Ice Theme (matches EditorIceColors_BlueIce.ini)
// ============================================================
struct IceTheme
{
    TermAttr borderColor1;
    TermAttr borderColor2;
    TermAttr quoteLineColor;
    TermAttr keyInfoLabelColor;

    // Top informational area
    TermAttr topInfoBkgColor;
    TermAttr topLabelColor;
    TermAttr topLabelColonColor;
    TermAttr topToColor;
    TermAttr topFromColor;
    TermAttr topSubjectColor;
    TermAttr topTimeColor;
    TermAttr topTimeLeftColor;
    TermAttr editMode;

    // Quote window
    TermAttr quoteWinText;
    TermAttr quoteLineHighlightColor;
    TermAttr quoteWinBorderTextColor;

    // Multi-choice options
    TermAttr selectedOptionBorderColor;
    TermAttr selectedOptionTextColor;
    TermAttr unselectedOptionBorderColor;
    TermAttr unselectedOptionTextColor;

    // List boxes
    TermAttr listBoxBorder;
    TermAttr listBoxBorderText;
    TermAttr listBoxItemText;
    TermAttr listBoxItemHighlight;
};

// ============================================================
// DCT Theme (matches EditorDCTColors_Default.ini)
// ============================================================
struct DctTheme
{
    // Border colors
    TermAttr topBorderColor1;
    TermAttr topBorderColor2;
    TermAttr editAreaBorderColor1;
    TermAttr editAreaBorderColor2;
    TermAttr editModeBrackets;
    TermAttr editMode;

    // Top informational area
    TermAttr topLabelColor;
    TermAttr topLabelColonColor;
    TermAttr topFromColor;
    TermAttr topFromFillColor;
    TermAttr topToColor;
    TermAttr topToFillColor;
    TermAttr topSubjColor;
    TermAttr topSubjFillColor;
    TermAttr topAreaColor;
    TermAttr topAreaFillColor;
    TermAttr topTimeColor;
    TermAttr topTimeFillColor;
    TermAttr topTimeLeftColor;
    TermAttr topTimeLeftFillColor;
    TermAttr topInfoBracketColor;

    // Quote window
    TermAttr quoteWinText;
    TermAttr quoteLineHighlightColor;
    TermAttr quoteWinBorderTextColor;
    TermAttr quoteWinBorderColor;

    // Bottom help row
    TermAttr bottomHelpBrackets;
    TermAttr bottomHelpKeys;
    TermAttr bottomHelpFill;
    TermAttr bottomHelpKeyDesc;

    // Text boxes
    TermAttr textBoxBorder;
    TermAttr textBoxBorderText;
    TermAttr textBoxInnerText;
    TermAttr yesNoBoxBrackets;
    TermAttr yesNoBoxYesNoText;

    // List boxes
    TermAttr listBoxBorder;
    TermAttr listBoxBorderText;
    TermAttr listBoxItemText;
    TermAttr listBoxItemHighlight;
};

// ============================================================
// Load Ice Theme
// ============================================================
IceTheme loadIceTheme(const std::string& path);

// ============================================================
// Load DCT Theme
// ============================================================
DctTheme loadDctTheme(const std::string& path);

// ============================================================
// Random two-color border drawing
// ============================================================

void drawRandomTwoColorLine(int row, int startCol, const std::vector<int>& chars,
                            const TermAttr& color1, const TermAttr& color2);

void drawRandomTwoColorHLine(int row, int col, int len,
                             const TermAttr& color1, const TermAttr& color2);

#endif // SLYMAIL_THEME_H
