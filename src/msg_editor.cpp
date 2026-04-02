// SlyMail message editor.
// Some web pages/links that could be informative:
// https://zed.dev/blog/building-a-text-editor-in-times-of-ai
// https://www.finseth.com/craft/craft.pdf
// https://web.mit.edu/nelhage/Public/the-craft-of-text-editing.pdf

#include "msg_editor.h"
#include "ui_common.h"
#include "colors.h"
#include "settings_dialog.h"
#include "bbs_colors.h"
#include "text_utils.h"
#include "file_dir_utils.h"
#include "file_browser.h"
#include <cctype>
#include <fstream>
#include <filesystem>

using std::string;
using std::vector;

MessageEditor::MessageEditor()
    : cursorRow(0), cursorCol(0), scrollRow(0)
    , insertMode(true), currentStyle(EditorStyle::Ice)
    , editTop(0), editBottom(0), editLeft(0), editRight(79)
    , editWidth(80), editHeight(0)
    , quoteWindowOpen(false), quoteSelected(0), quoteScroll(0)
    , quoteWinTop(0), quoteWinHeight(0)
    , lastSearchText(""), searchStartLine(0)
    , lastPickFg(7), lastPickBg(0), lastPickBright(false), lastPickSection(0)
{
}

TermAttr MessageEditor::randomBorderColor(const TermAttr& c1, const TermAttr& c2)
{
    return (rand() % 2 == 0) ? c1 : c2;
}

// Draw a horizontal line using a cached color pattern instead of random
static void drawCachedColorHLine(int row, int startCol, int len,
                                  const TermAttr& c1, const TermAttr& c2,
                                  const vector<int>& pattern)
{
    for (int i = 0; i < len; ++i)
    {
        int idx = startCol + i;
        int choice = (idx < static_cast<int>(pattern.size())) ? pattern[idx] : 0;
        g_term->setAttr(choice ? c2 : c1);
        g_term->putCP437(row, startCol + i, CP437_BOX_DRAWINGS_HORIZONTAL_SINGLE);
    }
}

// Pick a border color from a cached pattern
static TermAttr cachedBorderColor(int idx, const vector<int>& pattern,
                                   const TermAttr& c1, const TermAttr& c2)
{
    int choice = (idx >= 0 && idx < static_cast<int>(pattern.size())) ? pattern[idx] : 0;
    return choice ? c2 : c1;
}

void MessageEditor::init(const Settings& settings, const string& baseDir)
{
    namespace fs = std::filesystem;

    insertMode = settings.insertMode;
    currentStyle = settings.editorStyle;
    if (currentStyle == EditorStyle::Random)
    {
        currentStyle = (rand() % 2 == 0) ? EditorStyle::Ice : EditorStyle::Dct;
    }

    // Load attribute code toggle flags from settings
    attrFlags.synchronet = settings.attrSynchronet;
    attrFlags.wwiv = settings.attrWWIV;
    attrFlags.celerity = settings.attrCelerity;
    attrFlags.renegade = settings.attrRenegade;
    attrFlags.pcboard = settings.attrPCBoard;

    // Load themes from the .slymail data directory, falling back to baseDir
    string dataDir = getSlyMailDataDir();
    string configDir = dataDir + PATH_SEP_STR + "config_files";
    // Fall back to baseDir if the data directory config_files doesn't exist
    {
        namespace fs = std::filesystem;
        if (!fs::is_directory(configDir) || fs::is_empty(configDir))
        {
            string fallback = baseDir.empty() ? "config_files" : baseDir + PATH_SEP_STR + "config_files";
            if (fs::is_directory(fallback))
                configDir = fallback;
        }
    }

    // Load the theme files if they exist; otherwise, initialize themes to defaults
    // Ice
    const string iceFilename = settings.iceThemeFile.empty() ? "EditorIceColors_BlueIce.ini" : settings.iceThemeFile;
    const string iceThemeFilename = configDir + PATH_SEP_STR + iceFilename;
    if (fs::exists(iceThemeFilename))
        iceTheme = loadIceTheme(iceThemeFilename);
    // DCT
    const string DCTFilename = settings.dctThemeFile.empty() ? "EditorDCTColors_Default.ini" : settings.dctThemeFile;
    const string DCTThemeFilename = configDir + PATH_SEP_STR + DCTFilename;
    if (fs::exists(DCTThemeFilename))
        dctTheme = loadDctTheme(DCTThemeFilename);

    generateBorderColors();
}

// Helper to generate a vector of random 0/1 values for border color selection
static vector<int> generateRandomPattern(int len)
{
    vector<int> pattern(len);
    for (int i = 0; i < len; ++i)
    {
        pattern[i] = rand() % 2;
    }
    return pattern;
}

void MessageEditor::generateBorderColors()
{
    int cols = g_term->getCols();
    int rows = g_term->getRows();

    // ICE borders
    iceTopBorderColors = generateRandomPattern(cols);
    iceSepBorderColors = generateRandomPattern(cols);
    iceVertLeftColors = generateRandomPattern(rows);
    iceVertRightColors = generateRandomPattern(rows);
    iceBottomBorderColors = generateRandomPattern(cols);

    // DCT borders
    dctTopBorderColors = generateRandomPattern(cols);
    dctEditTopBorderColors = generateRandomPattern(cols);
    dctEditBottomBorderColors = generateRandomPattern(cols);
    dctVertLeftColors = generateRandomPattern(rows);
    dctVertRightColors = generateRandomPattern(rows);
}

void MessageEditor::calculateLayout()
{
    if (currentStyle == EditorStyle::Ice)
    {
        editTop = 4;    // After ICE header (rows 0-3)
    }
    else
    {
        editTop = 5;    // After DCT header (rows 0-4)
    }
    editBottom = g_term->getRows() - 2; // -2 for status bars
    editHeight = editBottom - editTop;
    editLeft = 0;
    editWidth = g_term->getCols();
    editRight = editWidth - 1;
}

void MessageEditor::prepareQuotes(const QwkMessage& msg, const Settings& settings)
{
    quoteLines.clear();

    // Build quote prefix based on settings
    string prefix;
    if (settings.quoteWithInitials)
    {
        // Extract author's initials from their name
        string initials;
        if (!msg.from.empty())
        {
            // Check if name has multiple words
            auto spacePos = msg.from.rfind(' ');
            if (spacePos != string::npos && spacePos + 1 < msg.from.size())
            {
                // Multiple words: first letter of first word + first letter of last word
                initials += static_cast<char>(toupper(static_cast<unsigned char>(msg.from[0])));
                initials += static_cast<char>(toupper(static_cast<unsigned char>(msg.from[spacePos + 1])));
            }
            else
            {
                // Single name: use first 2 characters (or 1 if name is 1 char)
                initials += static_cast<char>(toupper(static_cast<unsigned char>(msg.from[0])));
                if (msg.from.size() >= 2)
                {
                    initials += static_cast<char>(toupper(static_cast<unsigned char>(msg.from[1])));
                }
            }
        }
        if (settings.indentQuoteInitials)
        {
            prefix = " " + initials + "> ";
        }
        else
        {
            prefix = initials + "> ";
        }
    }
    else
    {
        prefix = settings.quotePrefix;
    }

    // Build raw quote lines with prefix
    vector<string> rawQuoteLines;
    std::istringstream stream(msg.body);
    string line;
    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        // Skip kludge lines
        if (!line.empty() && line[0] == '@')
        {
            continue;
        }
        if (line.find("SEEN-BY:") == 0)
        {
            continue;
        }
        if (line.find("PATH:") == 0)
        {
            continue;
        }
        // Trim leading spaces from quote lines if setting enabled
        if (settings.trimQuoteSpaces)
        {
            size_t start = line.find_first_not_of(" \t");
            if (start != string::npos)
            {
                line = line.substr(start);
            }
            else
            {
                line.clear();
            }
        }
        rawQuoteLines.push_back(prefix + line);
    }

    // Re-wrap quote lines.
    // If wrapQuoteLines is enabled, wrap to the terminal width - 1
    // so the text fits on screen. Otherwise, wrap to the standard 79 columns
    // (the traditional BBS line width convention).
    int quoteMaxWidth;
    if (settings.wrapQuoteLines)
    {
        quoteMaxWidth = g_term->getCols() - 1;
        if (quoteMaxWidth <= 0) quoteMaxWidth = 79;
    }
    else
    {
        quoteMaxWidth = 79;
    }
    quoteLines = wrapQuoteLines(rawQuoteLines, quoteMaxWidth);
}

// ---- ICE-style header drawing ----
// Matches SlyEdit1/SlyEdit2 screenshots exactly:
//   +------------------------------------------------------------------------------+
//   |TO: VHSFuture        FROM: Nightfox       05:17p|
//   |SUBJECT: Hello! New user          TL: 240  INS|
//   +--# 1--------------General----------------Mode  1--+

void MessageEditor::drawIceHeader()
{
    int cols = g_term->getCols();
    TermAttr bc1 = iceTheme.borderColor1;
    TermAttr bc2 = iceTheme.borderColor2;
    TermAttr labelAt = iceTheme.topLabelColor;
    TermAttr colonAt = iceTheme.topLabelColonColor;
    TermAttr toAt    = iceTheme.topToColor;
    TermAttr fromAt  = iceTheme.topFromColor;
    TermAttr subjAt  = iceTheme.topSubjectColor;
    TermAttr timeAt  = iceTheme.topTimeColor;
    TermAttr modeAt  = iceTheme.editMode;

    // Row 0: Top border with cached alternating colors
    g_term->setAttr(cachedBorderColor(0, iceTopBorderColors, bc1, bc2));
    g_term->putCP437(0, 0, CP437_BOX_DRAWINGS_UPPER_LEFT_SINGLE);
    drawCachedColorHLine(0, 1, cols - 2, bc1, bc2, iceTopBorderColors);
    g_term->setAttr(cachedBorderColor(cols - 1, iceTopBorderColors, bc1, bc2));
    g_term->putCP437(0, cols - 1, CP437_BOX_DRAWINGS_UPPER_RIGHT_SINGLE);

    // Row 1: TO: and FROM: fields
    g_term->setAttr(cachedBorderColor(1, iceVertLeftColors, bc1, bc2));
    g_term->putCP437(1, 0, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
    g_term->setAttr(cachedBorderColor(1, iceVertRightColors, bc1, bc2));
    g_term->putCP437(1, cols - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);

    printAt(1, 1, "TO", labelAt);
    printAt(1, 3, ":", colonAt);
    printAt(1, 5, padStr(truncateStr(toField, 27), 27), toAt);

    int fromX = 35;
    printAt(1, fromX, "FROM", labelAt);
    printAt(1, fromX + 4, ":", colonAt);
    printAt(1, fromX + 6, padStr(truncateStr(fromField, cols - fromX - 18), cols - fromX - 18), fromAt);

    // Time
    time_t now = std::time(nullptr);
    struct tm* tm = localtime(&now);
    char timeBuf[16];
    int hr = tm->tm_hour % 12;
    if (hr == 0)
    {
        hr = 12;
    }
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d%c",
             hr, tm->tm_min, tm->tm_hour >= 12 ? 'p' : 'a');
    printAt(1, cols - 7, timeBuf, timeAt);

    // Row 2: SUBJECT: and TL/INS
    g_term->setAttr(cachedBorderColor(2, iceVertLeftColors, bc1, bc2));
    g_term->putCP437(2, 0, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
    g_term->setAttr(cachedBorderColor(2, iceVertRightColors, bc1, bc2));
    g_term->putCP437(2, cols - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);

    printAt(2, 1, "SUBJECT", labelAt);
    printAt(2, 8, ":", colonAt);
    int subjW = cols - 24;
    printAt(2, 10, padStr(truncateStr(subjectField, subjW), subjW), subjAt);

    // TL (time left - just show a placeholder) and INS/OVR
    printAt(2, cols - 14, "TL", labelAt);
    printAt(2, cols - 12, ":", colonAt);
    printAt(2, cols - 10, "---", iceTheme.topTimeLeftColor);
    string modeStr = insertMode ? "INS" : "OVR";
    printAt(2, cols - 5, modeStr, modeAt);

    // Row 3: Separator with embedded info
    g_term->setAttr(cachedBorderColor(0, iceSepBorderColors, bc1, bc2));
    g_term->putCP437(3, 0, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
    drawCachedColorHLine(3, 1, cols - 2, bc1, bc2, iceSepBorderColors);
    g_term->setAttr(cachedBorderColor(cols - 1, iceSepBorderColors, bc1, bc2));
    g_term->putCP437(3, cols - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);

    // Embed area name centered
    int areaX = (cols - static_cast<int>(areaName.size())) / 2;
    printAt(3, areaX, " " + areaName + " ", labelAt);
}

// ---- DCT-style header drawing ----
// Matches SlyEdit5/SlyEdit6 screenshots exactly:
//   +-----------------------------------------------------------------------------+
//   |From  | Nightfox........................ |  Area  | General.................. |
//   |To    | VHSFuture....................... |  Time  | 05:30p |  Left | 240  |
//   |Subj  | Hello! New user................. |
//   +-----------------------------------------------------------------------------+

void MessageEditor::drawDctHeader()
{
    int cols = g_term->getCols();
    TermAttr tbc1 = dctTheme.topBorderColor1;
    TermAttr tbc2 = dctTheme.topBorderColor2;
    TermAttr ebc1 = dctTheme.editAreaBorderColor1;
    TermAttr ebc2 = dctTheme.editAreaBorderColor2;
    TermAttr labelAt  = dctTheme.topLabelColor;
    TermAttr bracketAt = dctTheme.topInfoBracketColor;
    TermAttr fromAt   = dctTheme.topFromColor;
    TermAttr fromFill = dctTheme.topFromFillColor;
    TermAttr toAt     = dctTheme.topToColor;
    TermAttr toFill   = dctTheme.topToFillColor;
    TermAttr subjAt   = dctTheme.topSubjColor;
    TermAttr subjFill = dctTheme.topSubjFillColor;
    TermAttr areaAt   = dctTheme.topAreaColor;
    TermAttr areaFill = dctTheme.topAreaFillColor;
    TermAttr timeAt   = dctTheme.topTimeColor;

    // Row 0: Top border with cached alternating colors
    g_term->setAttr(cachedBorderColor(0, dctTopBorderColors, tbc1, tbc2));
    g_term->putCP437(0, 0, CP437_BOX_DRAWINGS_UPPER_LEFT_SINGLE);
    drawCachedColorHLine(0, 1, cols - 2, tbc1, tbc2, dctTopBorderColors);
    g_term->setAttr(cachedBorderColor(cols - 1, dctTopBorderColors, tbc1, tbc2));
    g_term->putCP437(0, cols - 1, CP437_BOX_DRAWINGS_UPPER_RIGHT_SINGLE);

    // Helper: draw a DCT field with bullet fill (each field has its own fill color)
    auto drawDctField = [&](int row, int x, int width, const string& val,
                            TermAttr valAttr, TermAttr fieldFill)
    {
        printAt(row, x, " ", valAttr);
        string v = truncateStr(val, width - 2);
        printAt(row, x + 1, v, valAttr);
        int remaining = width - 2 - static_cast<int>(v.size());
        if (remaining > 0)
        {
            fillBullets(row, x + 1 + static_cast<int>(v.size()), remaining, fieldFill);
        }
        printAt(row, x + width - 1, " ", valAttr);
    };

    // Field layout calculation
    int fieldSepWidth = 1; // VLine separator
    int labelW = 6;
    int rightFieldStart = cols / 2 + 2;
    int leftFieldW = rightFieldStart - labelW - fieldSepWidth - 2;

    // Row 1: From | value... | Area | value...
    g_term->setAttr(cachedBorderColor(1, dctVertLeftColors, tbc1, tbc2));
    g_term->putCP437(1, 0, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
    g_term->setAttr(cachedBorderColor(1, dctVertRightColors, tbc1, tbc2));
    g_term->putCP437(1, cols - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);

    printAt(1, 1, "From ", labelAt);
    g_term->setAttr(bracketAt);
    g_term->putCP437(1, labelW, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
    drawDctField(1, labelW + 1, leftFieldW, fromField, fromAt, fromFill);
    g_term->setAttr(bracketAt);
    g_term->putCP437(1, rightFieldStart - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);

    printAt(1, rightFieldStart, " Area  ", labelAt);
    g_term->setAttr(bracketAt);
    g_term->putCP437(1, rightFieldStart + 7, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
    drawDctField(1, rightFieldStart + 8, cols - rightFieldStart - 9, areaName, areaAt, areaFill);

    // Row 2: To | value... | Time | value | Left | value
    g_term->setAttr(cachedBorderColor(2, dctVertLeftColors, tbc1, tbc2));
    g_term->putCP437(2, 0, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
    g_term->setAttr(cachedBorderColor(2, dctVertRightColors, tbc1, tbc2));
    g_term->putCP437(2, cols - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);

    printAt(2, 1, "To   ", labelAt);
    g_term->setAttr(bracketAt);
    g_term->putCP437(2, labelW, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
    drawDctField(2, labelW + 1, leftFieldW, toField, toAt, toFill);
    g_term->setAttr(bracketAt);
    g_term->putCP437(2, rightFieldStart - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);

    // Time
    time_t now = std::time(nullptr);
    struct tm* tm = localtime(&now);
    char timeBuf[16];
    int hr = tm->tm_hour % 12;
    if (hr == 0)
    {
        hr = 12;
    }
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d%c",
             hr, tm->tm_min, tm->tm_hour >= 12 ? 'p' : 'a');

    printAt(2, rightFieldStart, " Time ", labelAt);
    g_term->setAttr(bracketAt);
    g_term->putCP437(2, rightFieldStart + 6, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
    printAt(2, rightFieldStart + 7, " ", timeAt);
    printAt(2, rightFieldStart + 8, timeBuf, timeAt);

    int leftX = rightFieldStart + 16;
    g_term->setAttr(bracketAt);
    g_term->putCP437(2, leftX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
    printAt(2, leftX + 1, " Left ", labelAt);
    g_term->setAttr(bracketAt);
    g_term->putCP437(2, leftX + 7, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
    printAt(2, leftX + 8, " --- ", dctTheme.topTimeLeftColor);

    // Row 3: Subj | value...
    g_term->setAttr(cachedBorderColor(3, dctVertLeftColors, tbc1, tbc2));
    g_term->putCP437(3, 0, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
    g_term->setAttr(cachedBorderColor(3, dctVertRightColors, tbc1, tbc2));
    g_term->putCP437(3, cols - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);

    printAt(3, 1, "Subj ", labelAt);
    g_term->setAttr(bracketAt);
    g_term->putCP437(3, labelW, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
    drawDctField(3, labelW + 1, cols - labelW - 3, subjectField, subjAt, subjFill);

    // Row 4: Edit area top border (green) with cached alternating colors
    g_term->setAttr(cachedBorderColor(0, dctEditTopBorderColors, ebc1, ebc2));
    g_term->putCP437(4, 0, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
    drawCachedColorHLine(4, 1, cols - 2, ebc1, ebc2, dctEditTopBorderColors);
    g_term->setAttr(cachedBorderColor(cols - 1, dctEditTopBorderColors, ebc1, ebc2));
    g_term->putCP437(4, cols - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);

    // INS/OVR in border
    TermAttr modeBracket = dctTheme.editModeBrackets;
    TermAttr modeText    = dctTheme.editMode;
    int mx = cols - 7;
    printAt(4, mx, "[", modeBracket);
    printAt(4, mx + 1, insertMode ? "INS" : "OVR", modeText);
    printAt(4, mx + 4, "]", modeBracket);
}

// ---- ICE bottom status bar ----
// Matches SlyEdit2 screenshot:
//   |Registered To: Eric Oulashin| --- |CTRL  (A)bort (Q)Quote (Z)Save|
//   SlyEdit v2.01   Copyright 2026 Eric Oulashin   Press ESCape For Help
void MessageEditor::drawIceStatusBar()
{
    int cols = g_term->getCols();
    int rows = g_term->getRows();
    TermAttr bc1 = iceTheme.borderColor1;
    TermAttr bc2 = iceTheme.borderColor2;
    TermAttr regAt   = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr ctrlAt  = iceTheme.keyInfoLabelColor;
    TermAttr keyAt   = IceColors::keyLetter();
    TermAttr helpAt  = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr copyAt  = tAttr(TC_CYAN, TC_BLACK, false);

    int y = rows - 2;

    // Bottom border with side connectors (cached colors)
    g_term->setAttr(cachedBorderColor(0, iceBottomBorderColors, bc1, bc2));
    g_term->putCP437(y, 0, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
    drawCachedColorHLine(y, 1, cols - 2, bc1, bc2, iceBottomBorderColors);
    g_term->setAttr(cachedBorderColor(cols - 1, iceBottomBorderColors, bc1, bc2));
    g_term->putCP437(y, cols - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);

    // Embed "Registered To: name" on left
    string regStr = "Registered To: " + fromField;
    int regLen = static_cast<int>(regStr.size());
    printAt(y, 1, regStr, regAt);

    // Embed CTRL key hints on right
    string ctrlStr = "CTRL";
    int rightStart = cols - 42;
    if (rightStart < regLen + 5)
    {
        rightStart = regLen + 5;
    }

    g_term->setAttr(cachedBorderColor(rightStart - 1, iceBottomBorderColors, bc1, bc2));
    g_term->putCP437(y, rightStart - 1, CP437_BOX_DRAWINGS_HORIZONTAL_SINGLE);
    printAt(y, rightStart, " ", ctrlAt);
    printAt(y, rightStart + 1, ctrlStr, ctrlAt);

    // (A)bort (Q)Quote (Z)Save
    int kx = rightStart + 6;
    auto drawKey = [&](char letter, const string& desc)
    {
        printAt(y, kx, "(", ctrlAt);
        ++kx;
        printAt(y, kx, string(1, letter), keyAt);
        ++kx;
        printAt(y, kx, ")" + desc + " ", ctrlAt);
        kx += static_cast<int>(desc.size()) + 2;
    };
    drawKey('A', "bort");
    drawKey('Q', "Quote");
    drawKey('Z', "Save");

    // Row LINES-1: copyright/help line
    y = rows - 1;
    fillRow(y, tAttr(TC_BLACK, TC_BLACK, false));
    string copyLine = "SlyMail  " + string(fromField.empty() ? "" : fromField)
        + "   Press ESCape For Help";
    int cx = (cols - static_cast<int>(copyLine.size())) / 2;
    if (cx < 0)
    {
        cx = 0;
    }

    printAt(y, cx, "SlyMail", copyAt);
    cx += 9;
    printAt(y, cx, "  " + fromField + "   ", copyAt);
    cx += static_cast<int>(fromField.size()) + 5;
    printAt(y, cx, "Press ", helpAt);
    printAt(y, cx + 6, "ESC", tAttr(TC_GREEN, TC_BLACK, true));
    printAt(y, cx + 9, "ape For Help", helpAt);
}

// ---- DCT bottom status bar ----
// Matches SlyEdit6 screenshot:
//   [CTRL Z] Save  [CTRL A] Abort  [CTRL Q] Quote  [ESC] Menu        INS
void MessageEditor::drawDctStatusBar()
{
    int cols = g_term->getCols();
    int rows = g_term->getRows();
    TermAttr ebc1 = dctTheme.editAreaBorderColor1;
    TermAttr ebc2 = dctTheme.editAreaBorderColor2;
    TermAttr bracketAt = dctTheme.bottomHelpBrackets;
    TermAttr keyAt     = dctTheme.bottomHelpKeys;
    TermAttr descAt    = dctTheme.bottomHelpKeyDesc;
    TermAttr modeAt    = dctTheme.editMode;

    // Bottom border of edit area (cached colors)
    int y = rows - 2;
    g_term->setAttr(cachedBorderColor(0, dctEditBottomBorderColors, ebc1, ebc2));
    g_term->putCP437(y, 0, CP437_BOX_DRAWINGS_LOWER_LEFT_SINGLE);
    drawCachedColorHLine(y, 1, cols - 2, ebc1, ebc2, dctEditBottomBorderColors);
    g_term->setAttr(cachedBorderColor(cols - 1, dctEditBottomBorderColors, ebc1, ebc2));
    g_term->putCP437(y, cols - 1, CP437_BOX_DRAWINGS_LOWER_RIGHT_SINGLE);

    // Status line (centered, matching DCT style)
    y = rows - 1;
    fillRow(y, dctTheme.bottomHelpFill);

    // Calculate total width to center the help text
    // Format: [CTRL Z] Save  [CTRL A] Abort  [CTRL Q] Quote  [ESC] Menu
    int totalWidth = 0;
    struct DctHelpItem { string key; string desc; };
    vector<DctHelpItem> helpItems = {
        {"CTRL Z", "Save"}, {"CTRL A", "Abort"},
        {"CTRL Q", "Quote"}, {"ESC", "Menu"}
    };
    for (auto& item : helpItems)
    {
        // [key] desc  = brackets(2) + key + space + desc + spacing(2)
        totalWidth += 2 + static_cast<int>(item.key.size()) + 1
                    + static_cast<int>(item.desc.size()) + 2;
    }
    totalWidth -= 2;  // No trailing spaces after last item

    int x = (cols - totalWidth) / 2;
    if (x < 1)
    {
        x = 1;
    }

    auto drawDctKey = [&](const string& key, const string& desc)
    {
        printAt(y, x, "[", bracketAt);
        ++x;
        printAt(y, x, key, keyAt);
        x += static_cast<int>(key.size());
        printAt(y, x, "]", bracketAt);
        ++x;
        printAt(y, x, " " + desc + "  ", descAt);
        x += static_cast<int>(desc.size()) + 3;
    };

    drawDctKey("CTRL Z", "Save");
    drawDctKey("CTRL A", "Abort");
    drawDctKey("CTRL Q", "Quote");
    drawDctKey("ESC", "Menu");

    // INS/OVR at far right
    string modeStr = insertMode ? "INS" : "OVR";
    printAt(y, cols - static_cast<int>(modeStr.size()) - 1, modeStr, modeAt);
}

// Draw the edit area content
void MessageEditor::drawEditArea()
{
    TermAttr borderAttr;
    if (currentStyle == EditorStyle::Ice)
    {
        borderAttr = IceColors::borderNorm();
    }
    else
    {
        borderAttr = DctColors::editBorderN();
    }
    TermAttr textAttr  = IceColors::editText();
    TermAttr quoteAttr = IceColors::quoteText();

    int effectiveHeight = editHeight;
    if (quoteWindowOpen)
    {
        quoteWinHeight = editHeight * 42 / 100;
        if (quoteWinHeight < 5)
        {
            quoteWinHeight = 5;
        }
        effectiveHeight = editHeight - quoteWinHeight;
    }

    // Compute the running color state from line 0 up to scrollRow so that
    // colors set on previous lines carry over correctly to visible lines.
    TermAttr runningAttr = textAttr;
    for (int li = 0; li < scrollRow && li < static_cast<int>(lines.size()); ++li)
    {
        if (lines[li].text.find('\x1b') != string::npos)
        {
            // Parse the line to advance the color state (don't render)
            parseBBSColors(lines[li].text, runningAttr, attrFlags);
        }
    }

    for (int i = 0; i < effectiveHeight; ++i)
    {
        int lineIdx = scrollRow + i;
        int y = editTop + i;

        // Clear the edit area row
        g_term->setAttr(textAttr);
        g_term->fillRegion(y, editLeft, editWidth, ' ');

        if (lineIdx < static_cast<int>(lines.size()))
        {
            const auto& line = lines[lineIdx];
            bool isQuote = (!line.text.empty() &&
                (line.text[0] == '>' ||
                 (line.text.size() > 2 && line.text.find("> ") < 5)));

            // Use the running color attribute (inherited from previous lines)
            // unless this is a quote line (which has its own fixed color)
            TermAttr lineStartAttr = isQuote ? quoteAttr : runningAttr;

            // Check if line contains ANSI codes
            bool hasAnsi = (line.text.find('\x1b') != string::npos);

            if (hasAnsi || runningAttr != textAttr)
            {
                // Parse BBS color codes and render segment by segment
                // Start with the inherited color state
                TermAttr attr = lineStartAttr;
                auto segments = parseBBSColors(line.text, attr, attrFlags);
                int x = editLeft;
                for (const auto& seg : segments)
                {
                    if (x >= editRight + 1) break;
                    string segText = seg.text;
                    if (x + static_cast<int>(segText.size()) > editRight + 1)
                    {
                        segText = segText.substr(0, editRight + 1 - x);
                    }
                    printAt(y, x, segText, seg.attr);
                    x += static_cast<int>(segText.size());
                }
                // Update running attr to the state after this line
                runningAttr = attr;
            }
            else
            {
                string displayText = line.text;
                if (static_cast<int>(displayText.size()) > editWidth)
                {
                    displayText = displayText.substr(0, editWidth);
                }
                printAt(y, editLeft, displayText, lineStartAttr);
                // No ANSI codes, running attr stays the same
            }
        }
        else
        {
            // Past the end of the document — reset running color
            runningAttr = textAttr;
        }
    }
}

// Draw the quote window (SlyEdit style with labeled border)
// ICE uses double-horizontal lines (with mixed corners)
// DCT uses single-line borders with different colors
void MessageEditor::drawQuoteWindow()
{
    if (!quoteWindowOpen || quoteLines.empty())
    {
        return;
    }

    TermAttr textAttr, selAttr, helpAttr;
    if (currentStyle == EditorStyle::Ice)
    {
        textAttr   = iceTheme.quoteWinText;
        selAttr    = iceTheme.quoteLineHighlightColor;
        helpAttr   = iceTheme.quoteWinBorderTextColor;
    }
    else
    {
        textAttr   = dctTheme.quoteWinText;
        selAttr    = dctTheme.quoteLineHighlightColor;
        helpAttr   = dctTheme.quoteWinBorderTextColor;
    }

    quoteWinTop = editTop + editHeight - quoteWinHeight;
    int contentHeight = quoteWinHeight - 2;

    // Top border with "Quote Window" label
    if (currentStyle == EditorStyle::Ice)
    {
        // Ice mode
        g_term->setAttr(randomBorderColor(iceTheme.borderColor1, iceTheme.borderColor2));
        g_term->putCP437(quoteWinTop, 0, CP437_BOX_DRAWINGS_UPPER_LEFT_SINGLE);
        g_term->putCP437(quoteWinTop, 1, CP437_LEFT_HALF_BLOCK);
        printAt(quoteWinTop, 2, "Quote Window", iceTheme.quoteWinBorderTextColor);
        g_term->setAttr(randomBorderColor(iceTheme.borderColor1, iceTheme.borderColor2));
        g_term->putCP437(quoteWinTop, 14, CP437_RIGHT_HALF_BLOCK);
        const int lastCol = editWidth-1;
        for (int col = 15; col < lastCol; ++col)
        {
            g_term->setAttr(randomBorderColor(iceTheme.borderColor1, iceTheme.borderColor2));
            g_term->putCP437(quoteWinTop, col, CP437_BOX_DRAWINGS_DOUBLE_HORIZONTAL);
        }
        g_term->setAttr(randomBorderColor(iceTheme.borderColor1, iceTheme.borderColor2));
        g_term->putCP437(quoteWinTop, editWidth - 1, CP437_BOX_DRAWINGS_DOWN_SINGLE_AND_LEFT_DOUBLE);
        //g_term->putCP437(quoteWinTop, editWidth - 1, CP437_BOX_DRAWINGS_VERTICAL_SINGLE_AND_HORIZONTAL_DOUBLE);
    }
    else
    {
        // DCT mode
        g_term->setAttr(dctTheme.quoteWinBorderColor);
        g_term->putCP437(quoteWinTop, 0, CP437_BOX_DRAWINGS_UPPER_LEFT_SINGLE);
        g_term->drawHLine(quoteWinTop, 1, editWidth - 2);
        g_term->putCP437(quoteWinTop, editWidth - 1, CP437_BOX_DRAWINGS_UPPER_RIGHT_SINGLE);
        printAt(quoteWinTop, 2, " Quote Window ", dctTheme.quoteWinBorderTextColor);
    }

    // Quote lines
    for (int i = 0; i < contentHeight; ++i)
    {
        int lineIdx = quoteScroll + i;
        int y = quoteWinTop + 1 + i;

        // Clear interior
        g_term->setAttr(textAttr);
        g_term->fillRegion(y, 0, editWidth, ' ');

        if (lineIdx < static_cast<int>(quoteLines.size()))
        {
            bool isSel = (lineIdx == quoteSelected);
            if (isSel)
            {
                fillRow(y, selAttr, 0, editWidth);
                printAt(y, 0, truncateStr(quoteLines[lineIdx], editWidth - 1), selAttr);
            }
            else
            {
                printAt(y, 0, truncateStr(quoteLines[lineIdx], editWidth - 1), textAttr);
            }
        }
    }

    // Scrollbar on the right edge if content overflows
    int totalQuoteLines = static_cast<int>(quoteLines.size());
    if (totalQuoteLines > contentHeight)
    {
        drawScrollbar(quoteWinTop + 1, contentHeight, quoteSelected,
                     totalQuoteLines,
                     tAttr(TC_BLACK, TC_BLACK, true),
                     tAttr(TC_WHITE, TC_BLACK, true));
    }

    // Bottom border with instructions
    const int bottomY = quoteWinTop + quoteWinHeight - 1;
    if (currentStyle == EditorStyle::Ice)
    {
        // Ice mode
        int col = 0;
        g_term->setAttr(randomBorderColor(iceTheme.borderColor1, iceTheme.borderColor2));
        g_term->putCP437(bottomY, col++, CP437_BOX_DRAWINGS_LOWER_LEFT_SINGLE);
        // 1st section
        string helpStr = "^Q/ESC=End";
        g_term->setAttr(randomBorderColor(iceTheme.borderColor1, iceTheme.borderColor2));
        g_term->putCP437(bottomY, col++, CP437_LEFT_HALF_BLOCK);
        g_term->setAttr(iceTheme.quoteWinBorderTextColor);
        printAt(bottomY, col, helpStr, iceTheme.quoteWinBorderTextColor);
        col += (int)helpStr.length();
        // 2nd section
        helpStr = "CR=Accept";
        g_term->setAttr(randomBorderColor(iceTheme.borderColor1, iceTheme.borderColor2));
        g_term->putCP437(bottomY, col++, CP437_RIGHT_HALF_BLOCK);
        g_term->setAttr(randomBorderColor(iceTheme.borderColor1, iceTheme.borderColor2));
        g_term->putCP437(bottomY, col++, CP437_LEFT_HALF_BLOCK);
        g_term->setAttr(iceTheme.quoteWinBorderTextColor);
        printAt(bottomY, col, helpStr, iceTheme.quoteWinBorderTextColor);
        col += (int)helpStr.length();
        // 3rd section
        helpStr = "Up/Down/PgUp/PgDn/Home/End=Scroll";
        g_term->setAttr(randomBorderColor(iceTheme.borderColor1, iceTheme.borderColor2));
        g_term->putCP437(bottomY, col++, CP437_RIGHT_HALF_BLOCK);
        g_term->setAttr(randomBorderColor(iceTheme.borderColor1, iceTheme.borderColor2));
        g_term->putCP437(bottomY, col++, CP437_LEFT_HALF_BLOCK);
        g_term->setAttr(iceTheme.quoteWinBorderTextColor);
        printAt(bottomY, col, helpStr, iceTheme.quoteWinBorderTextColor);
        col += (int)helpStr.length();
        // Rest of the border
        const int lastCol = editWidth-1;
        for (; col < lastCol; ++col)
        {
            g_term->setAttr(randomBorderColor(iceTheme.borderColor1, iceTheme.borderColor2));
            g_term->putCP437(quoteWinTop, col, CP437_BOX_DRAWINGS_DOUBLE_HORIZONTAL);
        }
        g_term->setAttr(randomBorderColor(iceTheme.borderColor1, iceTheme.borderColor2));
        g_term->putCP437(bottomY, col, CP437_BOX_DRAWINGS_UP_SINGLE_AND_LEFT_DOUBLE);
    }
    else
    {
        // DCT mode
        g_term->setAttr(dctTheme.quoteWinBorderColor);
        g_term->putCP437(bottomY, 0, CP437_BOX_DRAWINGS_LOWER_LEFT_SINGLE);
        const string helpStr = " ^Q/ESC=End | CR=Accept | Up/Down/PgUp/PgDn/Home/End=Scroll ";
        g_term->drawHLine(bottomY, 1, editWidth - 2);
        g_term->putCP437(bottomY, editWidth - 1, CP437_BOX_DRAWINGS_LOWER_RIGHT_SINGLE);
        int helpX = (editWidth - static_cast<int>(helpStr.length())) / 2;
        if (helpX < 1)
            helpX = 1;
        printAt(bottomY, helpX, helpStr, helpAttr);
    }
}

// ICE-style yes/no prompt on the bottom row
bool MessageEditor::promptYesNoIce(const string& question)
{
    int rows = g_term->getRows();
    int y = rows - 1;

    TermAttr selBdr  = iceTheme.selectedOptionBorderColor;
    TermAttr selTxt  = iceTheme.selectedOptionTextColor;
    TermAttr unsBdr  = iceTheme.unselectedOptionBorderColor;
    TermAttr unsTxt  = iceTheme.unselectedOptionTextColor;

    bool selectedYes = true;

    for (;;)
    {
        fillRow(y, tAttr(TC_BLACK, TC_BLACK, false));

        // Draw question
        printAt(y, 1, question + "? ", iceTheme.topLabelColor);

        int optX = 1 + static_cast<int>(question.size()) + 2;

        // [Yes]
        TermAttr yesBdr = selectedYes ? selBdr : unsBdr;
        TermAttr yesTxt = selectedYes ? selTxt : unsTxt;
        g_term->setAttr(yesBdr);
        g_term->putCP437(y, optX, CP437_LEFT_HALF_BLOCK);
        printAt(y, optX + 1, "Yes", yesTxt);
        g_term->setAttr(yesBdr);
        g_term->putCP437(y, optX + 4, CP437_RIGHT_HALF_BLOCK);

        optX += 6;

        // [No]
        TermAttr noBdr = selectedYes ? unsBdr : selBdr;
        TermAttr noTxt = selectedYes ? unsTxt : selTxt;
        g_term->setAttr(noBdr);
        g_term->putCP437(y, optX, CP437_LEFT_HALF_BLOCK);
        printAt(y, optX + 1, "No", noTxt);
        g_term->setAttr(noBdr);
        g_term->putCP437(y, optX + 3, CP437_RIGHT_HALF_BLOCK);

        g_term->refresh();

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_LEFT:
            case TK_RIGHT:
                selectedYes = !selectedYes;
                break;
            case 'y':
            case 'Y':
                return true;
            case 'n':
            case 'N':
                return false;
            case TK_ENTER:
                return selectedYes;
            case TK_ESCAPE:
                return false;
            default:
                break;
        }
    }
}

// DCT-style yes/no dialog centered in the edit area
bool MessageEditor::promptYesNoDct(const string& question, const string& title)
{
    int cols = g_term->getCols();
    int rows = g_term->getRows();

    int dlgW = static_cast<int>(question.size()) + 8;
    if (dlgW < 30)
    {
        dlgW = 30;
    }
    if (dlgW > cols - 4)
    {
        dlgW = cols - 4;
    }
    int dlgH = 7;
    int dlgY = (rows - dlgH) / 2;
    int dlgX = (cols - dlgW) / 2;

    TermAttr borderAttr = dctTheme.textBoxBorder;
    TermAttr titleAttr  = dctTheme.textBoxBorderText;
    TermAttr textAttr   = dctTheme.textBoxInnerText;
    TermAttr brkAttr    = dctTheme.yesNoBoxBrackets;
    TermAttr ynAttr     = dctTheme.yesNoBoxYesNoText;

    bool selectedYes = true;

    for (;;)
    {
        // Clear dialog area
        for (int r = 0; r < dlgH; ++r)
        {
            fillRow(dlgY + r, borderAttr, dlgX, dlgX + dlgW);
        }

        // Draw box
        drawBox(dlgY, dlgX, dlgH, dlgW, borderAttr);

        // Title
        if (!title.empty())
        {
            string titleStr = " " + title + " ";
            int titleX = dlgX + (dlgW - static_cast<int>(titleStr.size())) / 2;
            printAt(dlgY, titleX, titleStr, titleAttr);
        }

        // Question text centered
        int qx = dlgX + (dlgW - static_cast<int>(question.size())) / 2;
        printAt(dlgY + 2, qx, question + "?", textAttr);

        // Yes/No buttons centered
        int btnW = 13; // "[Yes]  [No]"
        int bx = dlgX + (dlgW - btnW) / 2;

        // [Yes]
        if (selectedYes)
        {
            printAt(dlgY + 4, bx, "[", brkAttr);
            printAt(dlgY + 4, bx + 1, "Yes", ynAttr);
            printAt(dlgY + 4, bx + 4, "]", brkAttr);
        }
        else
        {
            printAt(dlgY + 4, bx, " Yes ", textAttr);
        }

        bx += 7;

        // [No]
        if (!selectedYes)
        {
            printAt(dlgY + 4, bx, "[", brkAttr);
            printAt(dlgY + 4, bx + 1, "No", ynAttr);
            printAt(dlgY + 4, bx + 3, "]", brkAttr);
        }
        else
        {
            printAt(dlgY + 4, bx, " No  ", textAttr);
        }

        g_term->refresh();

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_LEFT:
            case TK_RIGHT:
            case TK_UP:
            case TK_DOWN:
                selectedYes = !selectedYes;
                break;
            case 'y':
            case 'Y':
                return true;
            case 'n':
            case 'N':
                return false;
            case TK_ENTER:
                return selectedYes;
            case TK_ESCAPE:
                return false;
            default:
                break;
        }
    }
}

bool MessageEditor::confirmQuestion(const string& question, const string& title)
{
    if (currentStyle == EditorStyle::Ice)
    {
        return promptYesNoIce(question);
    }
    else
    {
        return promptYesNoDct(question, title);
    }
}

// ESC menu (SlyEdit style command menu)
int MessageEditor::showEscMenu()
{
    int cols = g_term->getCols();
    int rows = g_term->getRows();
    int menuW = 34;
    int menuH = 15;
    int menuY = (rows - menuH) / 2;
    int menuX = (cols - menuW) / 2;

    TermAttr borderAttr = tAttr(TC_CYAN, TC_BLACK, true);
    TermAttr keyAttr    = tAttr(TC_YELLOW, TC_BLACK, true);
    TermAttr descAttr   = tAttr(TC_WHITE, TC_BLACK, false);

    // Shadow
    g_term->setAttr(tAttr(TC_BLACK, TC_BLACK, true));
    for (int r = 1; r < menuH + 1; ++r)
    {
        for (int c = 2; c < menuW + 2; ++c)
        {
            if (menuY + r < rows && menuX + c < cols)
            {
                g_term->putCh(menuY + r, menuX + c, ' ');
            }
        }
    }

    for (int r = 0; r < menuH; ++r)
    {
        fillRow(menuY + r, tAttr(TC_WHITE, TC_BLACK, false), menuX, menuX + menuW);
    }

    drawBox(menuY, menuX, menuH, menuW, borderAttr, "Editor Menu", borderAttr);

    int y = menuY + 2;
    auto menuItem = [&](const string& key, const string& desc)
    {
        printAt(y, menuX + 3, key, keyAttr);
        printAt(y, menuX + 3 + static_cast<int>(key.size()) + 1,
                ": " + desc, descAttr);
        ++y;
    };

    menuItem("S", "Save message");
    menuItem("A", "Abort (cancel)");
    menuItem("I", "Toggle Insert/Overwrite");
    menuItem("D", "Delete current line");
    menuItem("Q", "Open quote window");
    menuItem("W", "Word/text search");
    menuItem("G", "Insert graphic character");
    menuItem("L", "Command key help");
    menuItem("C", "Change subject");
    ++y;
    printAt(y, menuX + 3, "ESC to close menu",
            tAttr(TC_GREEN, TC_BLACK, false));

    g_term->refresh();
    return g_term->getKey();
}

// Handle quote window
bool MessageEditor::handleQuoteWindow()
{
    if (quoteLines.empty())
    {
        messageDialog("Quote", "No quote lines available.");
        return false;
    }
    quoteWindowOpen = true;
    quoteSelected = 0;
    quoteScroll = 0;

    // Ensure quoteWinHeight is set before the loop so the first iteration's
    // key handler uses the correct contentHeight for scroll boundary checks.
    // (drawEditArea() sets this, but only runs at the end of the first draw
    // decision — the key handler runs after, with the stale value otherwise.)
    quoteWinHeight = editHeight * 42 / 100;
    if (quoteWinHeight < 5)
    {
        quoteWinHeight = 5;
    }

    bool qNeedFullRedraw = true;
    int prevQSel    = -1;
    int prevQScroll = -1;

    while (quoteWindowOpen)
    {
        int qWinTop       = editTop + editHeight - quoteWinHeight;
        int contentHeight = quoteWinHeight - 2;

        // Use loaded theme colors for partial row drawing
        TermAttr qTextAttr, qSelAttr;
        if (currentStyle == EditorStyle::Ice)
        {
            qTextAttr   = iceTheme.quoteWinText;
            qSelAttr    = iceTheme.quoteLineHighlightColor;
        }
        else
        {
            qTextAttr   = dctTheme.quoteWinText;
            qSelAttr    = dctTheme.quoteLineHighlightColor;
        }

        // Lambda: redraw a single quote-window row
        auto drawQRow = [&](int lineIdx)
        {
            if (lineIdx < quoteScroll || lineIdx >= quoteScroll + contentHeight) return;
            int i = lineIdx - quoteScroll;
            int y = qWinTop + 1 + i;
            bool isSel = (lineIdx == quoteSelected);

            g_term->setAttr(qTextAttr);
            g_term->fillRegion(y, 0, editWidth, ' ');

            if (lineIdx < static_cast<int>(quoteLines.size()))
            {
                if (isSel)
                {
                    fillRow(y, qSelAttr, 0, editWidth);
                    printAt(y, 0, truncateStr(quoteLines[lineIdx], editWidth - 1), qSelAttr);
                }
                else
                {
                    printAt(y, 0, truncateStr(quoteLines[lineIdx], editWidth - 1), qTextAttr);
                }
            }
        };

        // Lambda: redraw the quote-window scrollbar
        auto drawQSB = [&]()
        {
            int totalQL = static_cast<int>(quoteLines.size());
            if (totalQL > contentHeight)
            {
                drawScrollbar(qWinTop + 1, contentHeight, quoteSelected, totalQL,
                             tAttr(TC_BLACK, TC_BLACK, true),
                             tAttr(TC_WHITE, TC_BLACK, true));
            }
        };

        // --- Draw decision ---
        bool qScrollChanged = (quoteScroll != prevQScroll);

        if (qNeedFullRedraw)
        {
            // Full redraw: edit area + complete quote window (including borders)
            drawEditArea();
            drawQuoteWindow();
            qNeedFullRedraw = false;
        }
        else if (qScrollChanged)
        {
            // Scroll changed: redraw all quote line rows and scrollbar,
            // but not the borders
            for (int qi = 0; qi < contentHeight; ++qi)
            {
                drawQRow(quoteScroll + qi);
            }
            drawQSB();
        }
        else if (quoteSelected != prevQSel)
        {
            // Partial update: deselect old row, select new row
            drawQRow(prevQSel);
            drawQRow(quoteSelected);
            drawQSB();
        }

        g_term->refresh();
        prevQSel    = quoteSelected;
        prevQScroll = quoteScroll;

        int ch = g_term->getKey();

        switch (ch)
        {
            case TK_UP:
                if (quoteSelected > 0)
                {
                    --quoteSelected;
                    if (quoteSelected < quoteScroll)
                    {
                        quoteScroll = quoteSelected;
                    }
                }
                break;
            case TK_DOWN:
                if (quoteSelected < static_cast<int>(quoteLines.size()) - 1)
                {
                    ++quoteSelected;
                    if (quoteSelected >= quoteScroll + contentHeight)
                    {
                        quoteScroll = quoteSelected - contentHeight + 1;
                    }
                }
                break;
            case TK_PGUP:
                quoteSelected -= contentHeight;
                if (quoteSelected < 0)
                {
                    quoteSelected = 0;
                }
                quoteScroll = quoteSelected;
                break;
            case TK_PGDN:
                quoteSelected += contentHeight;
                if (quoteSelected >= static_cast<int>(quoteLines.size()))
                {
                    quoteSelected = static_cast<int>(quoteLines.size()) - 1;
                }
                if (quoteSelected >= quoteScroll + contentHeight)
                {
                    quoteScroll = quoteSelected - contentHeight + 1;
                }
                break;
            case TK_HOME:
                quoteSelected = 0;
                quoteScroll = 0;
                break;
            case TK_END:
                quoteSelected = static_cast<int>(quoteLines.size()) - 1;
                if (quoteSelected >= quoteScroll + contentHeight)
                {
                    quoteScroll = quoteSelected - contentHeight + 1;
                }
                break;
            case TK_ENTER:
            {
                EditorLine ql;
                ql.text = quoteLines[quoteSelected];
                ql.isQuoteLine = true;
                lines.insert(lines.begin() + cursorRow, ql);
                ++cursorRow;
                // Auto-advance lightbar down one line (if not at bottom)
                if (quoteSelected < static_cast<int>(quoteLines.size()) - 1)
                {
                    ++quoteSelected;
                    if (quoteSelected >= quoteScroll + contentHeight)
                    {
                        quoteScroll = quoteSelected - contentHeight + 1;
                    }
                }
                // Edit area changed: need full redraw next iteration
                qNeedFullRedraw = true;
                break;
            }
            case TK_CTRL_Q:
            case TK_ESCAPE:
                quoteWindowOpen = false;
                break;
            case TK_RESIZE:
                calculateLayout();
                generateBorderColors();
                g_term->clear(); // Clear to erase old borders at previous positions
                qNeedFullRedraw = true;
                break;
            default:
                break;
        }
    }
    return true;
}

// Show help screen (SlyEdit3 style)
void MessageEditor::showHelpScreen()
{
    int cols = g_term->getCols();
    g_term->clear();
    int y = 1;
    TermAttr headAttr  = tAttr(TC_RED, TC_BLACK, false);
    TermAttr keyAttr   = tAttr(TC_CYAN, TC_BLACK, true);
    TermAttr descAttr  = tAttr(TC_CYAN, TC_BLACK, false);

    // Title with program info
    drawProgramInfoLine(y);
    ++y;
    ++y;

    // Navigation help
    string navStr = "Up, Dn, PgUp, PgDn, HOME, END, ESC/Q=Close";
    int navX = (cols - static_cast<int>(navStr.size()) - 2) / 2;
    drawBox(y, navX - 1, 3, static_cast<int>(navStr.size()) + 4,
            tAttr(TC_RED, TC_BLACK, false));
    printAt(y + 1, navX + 1, navStr, descAttr);
    y += 4;

    // Two column help
    TermAttr sectionAttr = headAttr;
    drawHLine(y, 0, cols / 2, sectionAttr);
    printAt(y, 1, " Help keys ", sectionAttr);
    drawHLine(y, cols / 2, cols / 2, sectionAttr);
    printAt(y, cols / 2 + 1, " Slash commands (on blank line) ", sectionAttr);
    ++y;
    ++y;

    auto helpLine = [&](int col, const string& key, const string& desc)
    {
        int startX = (col == 0) ? 1 : cols / 2 + 1;
        printAt(y, startX, padStr(key, 20), keyAttr);
        printAt(y, startX + 20, ": " + desc, descAttr);
    };

    // Left column
    helpLine(0, "Ctrl-G", "Input graphic character");
    helpLine(1, "/A", "Abort");
    ++y;
    helpLine(0, "Ctrl-L", "Command key list (this)");
    helpLine(1, "/S", "Save");
    ++y;
    helpLine(0, "Ctrl-T", "List text replacements");
    helpLine(1, "/Q", "Quote message");
    ++y;
    helpLine(0, "", "");
    helpLine(1, "/U", "Your user settings");
    ++y;
    helpLine(0, "", "");
    helpLine(1, "/?", "Show help");
    ++y;
    ++y;

    // Command/edit keys section
    drawHLine(y, 0, cols, sectionAttr);
    printAt(y, 1, " Command/edit keys ", sectionAttr);
    ++y;
    ++y;

    auto cmdLine = [&](const string& key1, const string& desc1,
                       const string& key2, const string& desc2)
    {
        printAt(y, 1, padStr(key1, 22), keyAttr);
        printAt(y, 23, ": " + padStr(desc1, 22), descAttr);
        printAt(y, cols / 2 + 1, padStr(key2, 12), keyAttr);
        printAt(y, cols / 2 + 13, ": " + desc2, descAttr);
        ++y;
    };

    cmdLine("Ctrl-A", "Abort message", "PageUp", "Page up");
    cmdLine("Ctrl-Z", "Save message", "PageDown", "Page down");
    cmdLine("Ctrl-Q", "Quote message", "Ctrl-W", "Word/text search");
    cmdLine("Insert/Ctrl-I", "Toggle insert/overwrite", "Ctrl-D", "Delete line");
    cmdLine("Ctrl-S", "Change subject", "ESC", "Command menu");
    cmdLine("Ctrl-U", "Your user settings", "Ctrl-K", "Change text color");

    y += 2;
    printCentered(y, "Hit a key", tAttr(TC_GREEN, TC_BLACK, false));
    g_term->refresh();
    g_term->getKey();
}

// Word/text search with wrap-around, case-insensitive, and highlight
void MessageEditor::findText()
{
    int cols = g_term->getCols();
    int rows = g_term->getRows();
    int y = rows - 1;

    // Prompt for search text
    fillRow(y, tAttr(TC_BLACK, TC_BLACK, false));
    printAt(y, 0, "Text: ", tAttr(TC_CYAN, TC_BLACK, false));

    string searchText = getStringInput(y, 6, cols - 8, lastSearchText,
        tAttr(TC_CYAN, TC_BLACK, true));

    if (searchText.empty())
    {
        return;
    }

    // If different search text, reset start position
    if (searchText != lastSearchText)
    {
        searchStartLine = 0;
        lastSearchText = searchText;
    }

    // Case-insensitive search
    string upperSearch = searchText;
    for (auto& c : upperSearch) c = static_cast<char>(toupper(static_cast<unsigned char>(c)));

    // Search from searchStartLine
    int totalLines = static_cast<int>(lines.size());
    for (int i = 0; i < totalLines; ++i)
    {
        int lineIdx = (searchStartLine + i) % totalLines;
        string upperLine = lines[lineIdx].text;
        for (auto& c : upperLine) c = static_cast<char>(toupper(static_cast<unsigned char>(c)));

        size_t pos = upperLine.find(upperSearch);
        if (pos != string::npos)
        {
            // Found! Move cursor there
            cursorRow = lineIdx;
            cursorCol = static_cast<int>(pos);
            searchStartLine = lineIdx + 1;

            // Adjust scroll
            int effectiveHeight = editHeight;
            if (quoteWindowOpen) effectiveHeight -= quoteWinHeight;
            if (cursorRow < scrollRow) scrollRow = cursorRow;
            if (cursorRow >= scrollRow + effectiveHeight)
                scrollRow = cursorRow - effectiveHeight + 1;

            // Briefly highlight the found text
            int displayRow = editTop + (cursorRow - scrollRow);
            if (displayRow >= editTop && displayRow < editTop + effectiveHeight)
            {
                string lineText = lines[cursorRow].text;
                int highlightStart = static_cast<int>(pos);
                int highlightEnd = highlightStart + static_cast<int>(searchText.size());

                // Normal part before
                printAt(displayRow, editLeft,
                        lineText.substr(0, highlightStart),
                        tAttr(TC_WHITE, TC_BLACK, false));
                // Highlighted part
                printAt(displayRow, editLeft + highlightStart,
                        lineText.substr(highlightStart, searchText.size()),
                        tAttr(TC_BLACK, TC_BLUE, true));
                // Normal part after
                if (highlightEnd < static_cast<int>(lineText.size()))
                {
                    printAt(displayRow, editLeft + highlightEnd,
                            lineText.substr(highlightEnd),
                            tAttr(TC_WHITE, TC_BLACK, false));
                }
                g_term->refresh();
                g_term->napMillis(1000);
            }
            return;
        }
    }

    // Not found
    fillRow(y, tAttr(TC_BLACK, TC_BLACK, false));
    printAt(y, 0, "The text wasn't found!", tAttr(TC_YELLOW, TC_BLACK, true));
    g_term->refresh();
    g_term->napMillis(1500);
    searchStartLine = 0;
}

// Insert a graphic (CP437 128-255) character by code number
void MessageEditor::insertGraphicChar()
{
    int rows = g_term->getRows();
    int y = rows - 1;

    for (;;)
    {
        fillRow(y, tAttr(TC_BLACK, TC_BLACK, false));
        printAt(y, 0, "Enter Graphics Code (", tAttr(TC_WHITE, TC_BLACK, false));
        printAt(y, 21, "?", tAttr(TC_WHITE, TC_BLACK, true));
        printAt(y, 22, " for a list, ", tAttr(TC_WHITE, TC_BLACK, false));
        printAt(y, 35, "Ctrl-C", tAttr(TC_WHITE, TC_BLACK, true));
        printAt(y, 41, " to cancel): ", tAttr(TC_WHITE, TC_BLACK, false));

        string codeStr = getStringInput(y, 54, 3, "",
            tAttr(TC_WHITE, TC_BLUE, true));

        if (codeStr.empty())
        {
            return;  // Cancelled
        }

        if (codeStr == "?")
        {
            // Show character list
            showGraphicCharList();
            continue;  // Loop back to prompt
        }

        // Parse the code
        try
        {
            int code = std::stoi(codeStr);
            if (code >= 128 && code <= 255)
            {
                // Insert the character
                char ch = static_cast<char>(code);
                if (insertMode)
                {
                    lines[cursorRow].text.insert(cursorCol, 1, ch);
                }
                else
                {
                    if (cursorCol < static_cast<int>(lines[cursorRow].text.size()))
                    {
                        lines[cursorRow].text[cursorCol] = ch;
                    }
                    else
                    {
                        lines[cursorRow].text += ch;
                    }
                }
                ++cursorCol;
                return;
            }
        }
        catch (...) {}
    }
}

// Display CP437 characters 128-255 in a reference grid
void MessageEditor::showGraphicCharList()
{
    g_term->clear();
    int y = 0;
    int x = 0;
    int cols = g_term->getCols();
    TermAttr charAttr = tAttr(TC_WHITE, TC_BLUE, false);
    TermAttr numAttr  = tAttr(TC_WHITE, TC_BLUE, true);

    g_term->setAttr(tAttr(TC_WHITE, TC_BLUE, false));
    // Clear screen with blue background
    for (int r = 0; r < g_term->getRows(); ++r)
    {
        g_term->fillRegion(r, 0, cols, ' ');
    }

    y = 0;
    x = 0;
    for (int code = 128; code <= 255; ++code)
    {
        g_term->setAttr(charAttr);
        g_term->putCP437(y, x, code);
        g_term->setAttr(numAttr);
        g_term->printStr(y, x + 1, ":" + std::to_string(code));
        x += 6;

        // New line at specific codes to create organized columns
        if (code == 137 || code == 147 || code == 157 || code == 167 ||
            code == 177 || code == 187 || code == 197 || code == 207 ||
            code == 217 || code == 227 || code == 237 || code == 247)
        {
            ++y;
            x = 0;
        }
    }

    y += 2;
    printAt(y, (cols - 26) / 2, "PRESS ANY KEY TO CONTINUE",
            tAttr(TC_WHITE, TC_BLUE, true));
    g_term->refresh();
    g_term->getKey();
}

// Show a command key help window (Ctrl-L)
void MessageEditor::showCommandKeyHelp()
{
    int rows = g_term->getRows();

    g_term->clear();

    // Use program info at top
    drawProgramInfoLine(0);

    int y = 2;
    TermAttr headAttr = tAttr(TC_GREEN, TC_BLACK, true);
    TermAttr keyAttr  = tAttr(TC_CYAN, TC_BLACK, true);
    TermAttr sepAttr  = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr descAttr = tAttr(TC_CYAN, TC_BLACK, false);

    // Two column layout helper
    auto helpRow = [&](const string& k1, const string& d1,
                       const string& k2, const string& d2)
    {
        if (y >= rows - 2) return;
        printAt(y, 1, padStr(k1, 14), keyAttr);
        printAt(y, 15, ": ", sepAttr);
        printAt(y, 17, padStr(d1, 24), descAttr);
        if (!k2.empty())
        {
            g_term->setAttr(sepAttr);
            g_term->putCP437(y, 42, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
            printAt(y, 44, padStr(k2, 12), keyAttr);
            printAt(y, 56, ": ", sepAttr);
            printAt(y, 58, d2, descAttr);
        }
        ++y;
    };

    // Section: Help keys + Slash commands
    printAt(y, 1, "Help keys", headAttr);
    printAt(y, 44, "Slash commands (on blank line)", headAttr);
    ++y;
    g_term->setAttr(sepAttr);
    g_term->drawHLine(y, 1, 9);
    g_term->putCP437(y, 42, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
    g_term->drawHLine(y, 44, 30);
    ++y;

    helpRow("Ctrl-G", "Input graphic character", "/A", "Abort");
    helpRow("Ctrl-L", "Command key list", "/S", "Save");
    helpRow("", "", "/Q", "Quote message");
    helpRow("", "", "/U", "Your user settings");
    helpRow("", "", "/?", "Show help");
    ++y;

    // Section: Command/edit keys
    printAt(y, 1, "Command/edit keys", headAttr);
    ++y;
    g_term->setAttr(sepAttr);
    g_term->drawHLine(y, 1, 17);
    ++y;

    helpRow("Ctrl-A", "Abort message", "PageUp", "Page up");
    helpRow("Ctrl-Z", "Save message", "PageDown", "Page down");
    helpRow("Ctrl-Q", "Quote message", "Ctrl-W", "Word/text search");
    helpRow("Insert", "Toggle insert/overwrite", "Ctrl-D", "Delete line");
    helpRow("Ctrl-S", "Change subject", "ESC", "Command menu");
    helpRow("Ctrl-U", "Your user settings", "Ctrl-G", "Graphic character");
    helpRow("Ctrl-L", "Command key help", "F1", "Help screen");

    ++y;
    printAt(y, 1, "Press any key to continue...", tAttr(TC_GREEN, TC_BLACK, false));
    g_term->refresh();
    g_term->getKey();
}

// Main editor loop
// Color picker dialog (Ctrl-K)
// Shows a grid of 16 foreground colors x 8 background colors.
// The selected color is inserted as an ANSI escape code at the cursor position.
void MessageEditor::pickColor()
{
    int cols = g_term->getCols();
    int rows = g_term->getRows();

    // Color names for display
    static const char* fgNames[] = {
        "Black", "Red", "Green", "Yellow", "Blue", "Magenta", "Cyan", "White",
        "BrBlack", "BrRed", "BrGreen", "BrYellow", "BrBlue", "BrMagenta", "BrCyan", "BrWhite"
    };
    static const char* bgNames[] = {
        "Black", "Red", "Green", "Yellow", "Blue", "Magenta", "Cyan", "White"
    };
    // Restore last-used color selection
    int selFg = lastPickFg;
    int selBg = lastPickBg;
    bool selBright = lastPickBright;
    int selSpecial = 0; // 0 = Normal, 1 = High

    // Dialog dimensions
    int dlgW = 50;
    int dlgH = 20;
    int dlgX = (cols - dlgW) / 2;
    int dlgY = (rows - dlgH) / 2;

    TermAttr borderAttr = tAttr(TC_CYAN, TC_BLACK, true);
    TermAttr labelAttr = tAttr(TC_WHITE, TC_BLACK, true);
    TermAttr helpAttr = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr cursorAttr = tAttr(TC_WHITE, TC_BLUE, true);
    TermAttr specialAttr = tAttr(TC_YELLOW, TC_BLACK, true);

    int section = lastPickSection; // 0 = foreground, 1 = background, 2 = special attributes

    // Helper to save the current picker state for next time
    auto savePickState = [&]()
    {
        lastPickFg = selFg;
        lastPickBg = selBg;
        lastPickBright = selBright;
        lastPickSection = section;
    };

    while (true)
    {
        // Clear dialog area
        for (int r = 0; r < dlgH; ++r)
        {
            fillRow(dlgY + r, tAttr(TC_BLACK, TC_BLACK, false), dlgX, dlgX + dlgW);
        }
        drawBox(dlgY, dlgX, dlgH, dlgW, borderAttr, "Select Text Color", borderAttr);

        // Foreground colors (2 rows of 8)
        printAt(dlgY + 1, dlgX + 2, "Foreground:", labelAttr);
        for (int i = 0; i < 8; ++i)
        {
            // Normal intensity
            int x = dlgX + 2 + i * 5;
            TermAttr colorAttr = tAttr(i, TC_BLACK, false);
            bool isSel = (section == 0 && selFg == i && !selBright);
            if (isSel)
            {
                printAt(dlgY + 2, x, " " + string(1, "krgybmcw"[i]) + " ", cursorAttr);
            }
            else
            {
                printAt(dlgY + 2, x, " " + string(1, "krgybmcw"[i]) + " ", colorAttr);
            }

            // High intensity
            TermAttr brightAttr = tAttr(i, TC_BLACK, true);
            isSel = (section == 0 && selFg == i && selBright);
            if (isSel)
            {
                printAt(dlgY + 3, x, " " + string(1, "KRGYBMCW"[i]) + " ", cursorAttr);
            }
            else
            {
                printAt(dlgY + 3, x, " " + string(1, "KRGYBMCW"[i]) + " ", brightAttr);
            }
        }

        // Background colors (1 row of 8)
        printAt(dlgY + 5, dlgX + 2, "Background:", labelAttr);
        for (int i = 0; i < 8; ++i)
        {
            int x = dlgX + 2 + i * 5;
            TermAttr colorAttr = tAttr(TC_WHITE, i, true);
            bool isSel = (section == 1 && selBg == i);
            if (isSel)
            {
                printAt(dlgY + 6, x, " " + std::to_string(i) + " ", cursorAttr);
            }
            else
            {
                printAt(dlgY + 6, x, " " + std::to_string(i) + " ", colorAttr);
            }
        }

        // Special attributes row
        printAt(dlgY + 8, dlgX + 2, "Attributes:", labelAttr);
        {
            // Normal (reset)
            bool isSel = (section == 2 && selSpecial == 0);
            printAt(dlgY + 9, dlgX + 2, " N ", isSel ? cursorAttr : specialAttr);
            printAt(dlgY + 9, dlgX + 5, "=Normal(reset)", isSel ? cursorAttr : helpAttr);

            // High (bright/bold)
            isSel = (section == 2 && selSpecial == 1);
            printAt(dlgY + 9, dlgX + 22, " H ", isSel ? cursorAttr : specialAttr);
            printAt(dlgY + 9, dlgX + 25, "=High(bright)", isSel ? cursorAttr : helpAttr);
        }

        // Preview
        printAt(dlgY + 11, dlgX + 2, "Preview:", labelAttr);
        TermAttr previewAttr = tAttr(selFg, selBg, selBright);
        printAt(dlgY + 12, dlgX + 2, " Sample Text ", previewAttr);

        // Current selection info
        string fgStr = fgNames[selFg + (selBright ? 8 : 0)];
        string bgStr = bgNames[selBg];
        printAt(dlgY + 14, dlgX + 2, "FG: " + fgStr + "  BG: " + bgStr, labelAttr);

        // Help
        printAt(dlgY + 15, dlgX + 2, "Arrows=Select, Tab=FG/BG/Attr section", helpAttr);
        printAt(dlgY + 16, dlgX + 2, "Enter=Apply, N=Normal, H=High, ESC=Cancel", helpAttr);

        // Section indicator
        string secStr;
        switch (section)
        {
            case 0: secStr = "[Foreground]"; break;
            case 1: secStr = "[Background]"; break;
            case 2: secStr = "[Attributes]"; break;
        }
        printAt(dlgY + dlgH - 1, dlgX + (dlgW - static_cast<int>(secStr.size())) / 2,
                secStr, borderAttr);

        g_term->refresh();

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_LEFT:
                if (section == 0)
                {
                    if (selFg > 0) --selFg;
                }
                else if (section == 1)
                {
                    if (selBg > 0) --selBg;
                }
                else if (section == 2)
                {
                    selSpecial = (selSpecial == 0) ? 1 : 0;
                }
                break;
            case TK_RIGHT:
                if (section == 0)
                {
                    if (selFg < 7) ++selFg;
                }
                else if (section == 1)
                {
                    if (selBg < 7) ++selBg;
                }
                else if (section == 2)
                {
                    selSpecial = (selSpecial == 0) ? 1 : 0;
                }
                break;
            case TK_UP:
                if (section == 0)
                {
                    selBright = !selBright;
                }
                break;
            case TK_DOWN:
                if (section == 0)
                {
                    selBright = !selBright;
                }
                break;
            case TK_TAB:
                section = (section + 1) % 3;
                break;
            case 'n': case 'N':
            {
                // Insert normal/reset ANSI code: ESC[0m
                // Resets all attributes (foreground, background, bold) to defaults
                string ansiReset = "\x1b[0m";
                lines[cursorRow].text.insert(cursorCol, ansiReset);
                cursorCol += static_cast<int>(ansiReset.size());
                savePickState();
                return;
            }
            case 'h': case 'H':
            {
                // Insert high/bright/bold ANSI code: ESC[1m
                // Turns on bright/bold intensity for the current color
                string ansiBright = "\x1b[1m";
                lines[cursorRow].text.insert(cursorCol, ansiBright);
                cursorCol += static_cast<int>(ansiBright.size());
                savePickState();
                return;
            }
            case TK_ENTER:
            {
                if (section == 2)
                {
                    // Apply the selected special attribute
                    if (selSpecial == 0)
                    {
                        // Normal (reset)
                        string ansiReset = "\x1b[0m";
                        lines[cursorRow].text.insert(cursorCol, ansiReset);
                        cursorCol += static_cast<int>(ansiReset.size());
                    }
                    else
                    {
                        // High (bright/bold)
                        string ansiBright = "\x1b[1m";
                        lines[cursorRow].text.insert(cursorCol, ansiBright);
                        cursorCol += static_cast<int>(ansiBright.size());
                    }
                    savePickState();
                    return;
                }
                // Insert ANSI color code at cursor position
                TermAttr selectedColor = tAttr(selFg, selBg, selBright);
                string ansiCode = termAttrToAnsi(selectedColor);
                lines[cursorRow].text.insert(cursorCol, ansiCode);
                cursorCol += static_cast<int>(ansiCode.size());
                savePickState();
                return;
            }
            case TK_ESCAPE:
                return; // Cancel without inserting
            default:
                break;
        }
    }
}

EditorResult MessageEditor::run(Settings& settings, const string& baseDir)
{
    init(settings, baseDir);
    calculateLayout();

    if (lines.empty())
    {
        // Start with a normal/reset attribute code so the message begins
        // with a known color state. This ensures that if the user inserts
        // a color code later, it only affects text after it and doesn't
        // affect existing text that the new text is being inserted in front of.
        lines.push_back(EditorLine{"\x1b[0m"});
        cursorCol = static_cast<int>(lines[0].text.size());
    }

    g_term->setCursorVisible(true);

    bool needFullRedraw = true;
    bool needEditRedraw = true;  // Whether the edit area content needs redrawing
    int prevScrollRow = -1;

    while (true)
    {
        g_term->setCursorVisible(true);

        if (needFullRedraw)
        {
            g_term->clear();
            if (currentStyle == EditorStyle::Ice)
            {
                drawIceHeader();
                drawEditArea();
                if (!quoteWindowOpen)
                    drawIceStatusBar();
            }
            else
            {
                drawDctHeader();
                drawEditArea();
                if (!quoteWindowOpen)
                    drawDctStatusBar();
            }
            if (quoteWindowOpen)
            {
                drawQuoteWindow();
            }
            needFullRedraw = false;
            needEditRedraw = false;
            prevScrollRow = scrollRow;
        }
        else if (needEditRedraw || scrollRow != prevScrollRow)
        {
            // Content changed or scrolled — redraw the edit area and status bar
            drawEditArea();
            if (!quoteWindowOpen)
            {
                if (currentStyle == EditorStyle::Ice)
                    drawIceStatusBar();
                else
                    drawDctStatusBar();
            }
            if (quoteWindowOpen)
            {
                drawQuoteWindow();
            }
            needEditRedraw = false;
            prevScrollRow = scrollRow;
        }
        // else: cursor-only movement — just reposition the cursor, no redraw

        // Position cursor — use display column that skips ANSI sequences
        int displayRow = cursorRow - scrollRow;
        int displayCol = byteColToDisplayCol(lines[cursorRow].text, cursorCol);
        if (displayRow >= 0 && displayRow < editHeight)
        {
            g_term->moveTo(editTop + displayRow, editLeft + displayCol);
        }

        g_term->refresh();

        int ch = g_term->getKey();

        // Assume content changed; cursor-only keys will clear this flag
        needEditRedraw = true;

        if (cursorRow < 0)
        {
            cursorRow = 0;
        }
        if (cursorRow >= static_cast<int>(lines.size()))
        {
            cursorRow = static_cast<int>(lines.size()) - 1;
        }
        if (cursorCol < 0)
        {
            cursorCol = 0;
        }

        switch (ch)
        {
            case TK_CTRL_Z:
                g_term->setCursorVisible(false);
                return EditorResult::Saved;

            case TK_CTRL_A:
                if (confirmQuestion("Abort message", "Abort"))
                {
                    g_term->setCursorVisible(false);
                    return EditorResult::Aborted;
                }
                break;

            case TK_CTRL_Q:
                handleQuoteWindow();
                needFullRedraw = true;
                break;

            case TK_F1:
                showHelpScreen();
                needFullRedraw = true;
                break;

            case TK_ESCAPE:
            {
                int menuChoice = showEscMenu();
                switch (menuChoice)
                {
                    case 's':
                    case 'S':
                        g_term->setCursorVisible(false);
                        return EditorResult::Saved;
                    case 'a':
                    case 'A':
                        if (confirmQuestion("Abort message", "Abort"))
                        {
                            g_term->setCursorVisible(false);
                            return EditorResult::Aborted;
                        }
                        break;
                    case 'i':
                    case 'I':
                        insertMode = !insertMode;
                        break;
                    case 'd':
                    case 'D':
                        if (lines.size() > 1)
                        {
                            lines.erase(lines.begin() + cursorRow);
                            if (cursorRow >= static_cast<int>(lines.size()))
                            {
                                cursorRow = static_cast<int>(lines.size()) - 1;
                            }
                            cursorCol = 0;
                        }
                        else
                        {
                            lines[0].text.clear();
                            cursorCol = 0;
                        }
                        break;
                    case 'q':
                    case 'Q':
                        handleQuoteWindow();
                        break;
                    case 'w':
                    case 'W':
                        findText();
                        break;
                    case 'g':
                    case 'G':
                        insertGraphicChar();
                        break;
                    case 'l':
                    case 'L':
                        showCommandKeyHelp();
                        break;
                    case 'c':
                    case 'C':
                    {
                        int subCols = g_term->getCols();
                        int subY, subX, subMaxLen;
                        if (currentStyle == EditorStyle::Ice)
                        {
                            subY = 2;
                            subX = 10;
                            subMaxLen = subCols - 24;
                        }
                        else
                        {
                            subY = 3;
                            subX = 8;
                            subMaxLen = subCols - 6 - 3 - 2;
                        }
                        if (subMaxLen < 10) subMaxLen = 10;
                        if (subMaxLen > 72) subMaxLen = 72;
                        string newSubj = getStringInput(subY, subX, subMaxLen,
                            subjectField, tAttr(TC_WHITE, TC_BLACK, true));
                        if (!newSubj.empty())
                        {
                            subjectField = newSubj;
                        }
                        break;
                    }
                    default:
                        break;
                }
                needFullRedraw = true;
                break;
            }

            case TK_UP:
                if (cursorRow > 0)
                {
                    --cursorRow;
                    if (cursorCol > static_cast<int>(lines[cursorRow].text.size()))
                    {
                        cursorCol = static_cast<int>(lines[cursorRow].text.size());
                    }
                    int seqUp = ansiSeqLen(lines[cursorRow].text, cursorCol);
                    if (seqUp > 0) cursorCol += seqUp;
                }
                needEditRedraw = false; // Cursor-only movement
                break;

            case TK_DOWN:
                if (cursorRow < static_cast<int>(lines.size()) - 1)
                {
                    ++cursorRow;
                    if (cursorCol > static_cast<int>(lines[cursorRow].text.size()))
                    {
                        cursorCol = static_cast<int>(lines[cursorRow].text.size());
                    }
                    int seqDn = ansiSeqLen(lines[cursorRow].text, cursorCol);
                    if (seqDn > 0) cursorCol += seqDn;
                }
                needEditRedraw = false;
                break;

            case TK_LEFT:
                if (cursorCol > 0)
                {
                    cursorCol = skipBackward(lines[cursorRow].text, cursorCol);
                }
                else if (cursorRow > 0)
                {
                    --cursorRow;
                    cursorCol = static_cast<int>(lines[cursorRow].text.size());
                }
                needEditRedraw = false;
                break;

            case TK_RIGHT:
                if (cursorCol < static_cast<int>(lines[cursorRow].text.size()))
                {
                    cursorCol = skipForward(lines[cursorRow].text, cursorCol);
                }
                else if (cursorRow < static_cast<int>(lines.size()) - 1)
                {
                    ++cursorRow;
                    cursorCol = 0;
                }
                needEditRedraw = false;
                break;

            case TK_HOME:
                cursorCol = 0;
                needEditRedraw = false;
                break;

            case TK_END:
                cursorCol = static_cast<int>(lines[cursorRow].text.size());
                needEditRedraw = false;
                break;

            case TK_PGUP:
                cursorRow -= editHeight;
                if (cursorRow < 0)
                {
                    cursorRow = 0;
                }
                if (cursorCol > static_cast<int>(lines[cursorRow].text.size()))
                {
                    cursorCol = static_cast<int>(lines[cursorRow].text.size());
                }
                { int sq = ansiSeqLen(lines[cursorRow].text, cursorCol); if (sq > 0) cursorCol += sq; }
                needEditRedraw = false; // Scroll change is detected separately
                break;

            case TK_PGDN:
                cursorRow += editHeight;
                if (cursorRow >= static_cast<int>(lines.size()))
                {
                    cursorRow = static_cast<int>(lines.size()) - 1;
                }
                if (cursorCol > static_cast<int>(lines[cursorRow].text.size()))
                {
                    cursorCol = static_cast<int>(lines[cursorRow].text.size());
                }
                { int sq = ansiSeqLen(lines[cursorRow].text, cursorCol); if (sq > 0) cursorCol += sq; }
                needEditRedraw = false;
                break;

            case TK_INSERT:
                insertMode = !insertMode;
                // Status bar needs to update the INS/OVR indicator
                break;

            case TK_BACKSPACE:
            case TK_BACKSPACE_8:
                if (cursorCol > 0)
                {
                    cursorCol = eraseBackward(lines[cursorRow].text, cursorCol);
                    // After deleting, pull words up from the next line if room
                    pullUpWords(lines, cursorRow, editWidth - 1);
                }
                else if (cursorRow > 0)
                {
                    cursorCol = static_cast<int>(lines[cursorRow - 1].text.size());
                    // Add a space between joined lines if needed
                    if (!lines[cursorRow - 1].text.empty() && !lines[cursorRow].text.empty()
                        && lines[cursorRow - 1].text.back() != ' '
                        && lines[cursorRow].text[0] != ' ')
                    {
                        lines[cursorRow - 1].text += ' ';
                    }
                    lines[cursorRow - 1].text += lines[cursorRow].text;
                    lines.erase(lines.begin() + cursorRow);
                    --cursorRow;
                }
                break;

            case TK_DELETE:
                if (cursorCol < static_cast<int>(lines[cursorRow].text.size()))
                {
                    eraseForward(lines[cursorRow].text, cursorCol);
                    // After deleting, pull words up from the next line if room
                    pullUpWords(lines, cursorRow, editWidth - 1);
                }
                else if (cursorRow < static_cast<int>(lines.size()) - 1)
                {
                    // Add a space between joined lines if needed
                    if (!lines[cursorRow].text.empty() && !lines[cursorRow + 1].text.empty()
                        && lines[cursorRow].text.back() != ' '
                        && lines[cursorRow + 1].text[0] != ' ')
                    {
                        lines[cursorRow].text += ' ';
                    }
                    lines[cursorRow].text += lines[cursorRow + 1].text;
                    lines.erase(lines.begin() + cursorRow + 1);
                }
                break;

            case TK_CTRL_D:
                if (cursorRow < static_cast<int>(lines.size()) - 1)
                {
                    // There's a line below: remove the current line,
                    // shifting everything below up into its place
                    lines.erase(lines.begin() + cursorRow);
                }
                else
                {
                    // Already on the last line: just clear it
                    lines[cursorRow].text.clear();
                }
                cursorCol = 0;
                break;

            case TK_CTRL_G:
                insertGraphicChar();
                needFullRedraw = true;
                break;

            case TK_CTRL_O:
            {
                // Import a text file at the cursor position
                g_term->setCursorVisible(false);
                string startDir = getSlyMailDataDir();
                string filePath = showFileBrowser(startDir, "", "*");
                if (!filePath.empty())
                {
                    std::ifstream ifs(filePath);
                    if (ifs.is_open())
                    {
                        string fileLine;
                        bool first = true;
                        while (std::getline(ifs, fileLine))
                        {
                            if (!fileLine.empty() && fileLine.back() == '\r')
                                fileLine.pop_back();
                            if (first)
                            {
                                // Insert into current line at cursor position
                                lines[cursorRow].text.insert(cursorCol, fileLine);
                                cursorCol += static_cast<int>(fileLine.size());
                                first = false;
                            }
                            else
                            {
                                // Split current line and insert new line
                                string remainder = lines[cursorRow].text.substr(cursorCol);
                                lines[cursorRow].text = lines[cursorRow].text.substr(0, cursorCol);
                                EditorLine newLine;
                                newLine.text = fileLine + remainder;
                                newLine.hardBreak = true;
                                ++cursorRow;
                                lines.insert(lines.begin() + cursorRow, newLine);
                                cursorCol = static_cast<int>(fileLine.size());
                            }
                        }
                    }
                    else
                    {
                        messageDialog("Error", "Could not open file.");
                    }
                }
                g_term->setCursorVisible(true);
                needFullRedraw = true;
                break;
            }

            case TK_CTRL_K:
                pickColor();
                needFullRedraw = true;
                break;

            case TK_CTRL_L:
                showCommandKeyHelp();
                needFullRedraw = true;
                break;

            case TK_CTRL_S:
            {
                // Change subject - position cursor at subject field and allow editing
                int cols = g_term->getCols();
                int subY, subX, subjectMaxLen;
                if (currentStyle == EditorStyle::Ice)
                {
                    // Ice: "SUBJECT:" at col 1-8, ":" at col 8, value starts at col 10
                    subY = 2;
                    subX = 10;
                    subjectMaxLen = cols - 24; // Match the width used in drawIceHeader
                }
                else
                {
                    // DCT: "Subj " at col 1-5, "|" at col 6, drawDctField starts at 7,
                    // then " " + text at col 8
                    subY = 3;
                    subX = 8;
                    subjectMaxLen = cols - 6 - 3 - 2; // Match drawDctField width
                }
                if (subjectMaxLen < 10) subjectMaxLen = 10;
                if (subjectMaxLen > 72) subjectMaxLen = 72;
                string newSubj = getStringInput(subY, subX, subjectMaxLen,
                    subjectField, tAttr(TC_WHITE, TC_BLACK, true));
                if (!newSubj.empty())
                {
                    subjectField = newSubj;
                }
                needFullRedraw = true;
                break;
            }

            case TK_CTRL_W:
                findText();
                needFullRedraw = true;
                break;

            case TK_CTRL_U:
            {
                // Open user settings dialog (matching SlyEdit's Ctrl-U)
                EditorStyle oldStyle = settings.editorStyle;
                showEditorSettings(settings, baseDir);
                // If editor style changed, apply it
                if (settings.editorStyle != oldStyle)
                {
                    if (settings.editorStyle == EditorStyle::Random)
                    {
                        currentStyle = (rand() % 2 == 0)
                            ? EditorStyle::Ice : EditorStyle::Dct;
                    }
                    else
                    {
                        currentStyle = settings.editorStyle;
                    }
                    calculateLayout();
                    generateBorderColors();
                }
                needFullRedraw = true;
                break;
            }

            case TK_ENTER:
            {
                // Check for slash commands (only if line contains just the command)
                // Strip ANSI escape sequences first so embedded reset codes
                // (e.g. the initial ESC[0m on line 1) don't prevent detection.
                string trimmedLine;
                {
                    const string& raw = lines[cursorRow].text;
                    for (size_t ci = 0; ci < raw.size(); ++ci)
                    {
                        if (static_cast<uint8_t>(raw[ci]) == 0x1B && ci + 1 < raw.size() && raw[ci + 1] == '[')
                        {
                            // Skip ESC[ ... <final byte>
                            ci += 2;
                            while (ci < raw.size() && raw[ci] >= 0x20 && raw[ci] <= 0x3F) ++ci;
                            // ci now points at the final byte (0x40-0x7E) — the loop ++ci skips it
                            continue;
                        }
                        trimmedLine += raw[ci];
                    }
                }
                while (!trimmedLine.empty() && trimmedLine.front() == ' ') trimmedLine.erase(0, 1);
                while (!trimmedLine.empty() && trimmedLine.back() == ' ') trimmedLine.pop_back();

                string upperLine = trimmedLine;
                for (auto& c : upperLine) c = static_cast<char>(toupper(static_cast<unsigned char>(c)));

                if (upperLine == "/A")
                {
                    lines[cursorRow].text.clear();
                    cursorCol = 0;
                    if (confirmQuestion("Abort message", "Abort"))
                    {
                        g_term->setCursorVisible(false);
                        return EditorResult::Aborted;
                    }
                }
                else if (upperLine == "/S")
                {
                    lines[cursorRow].text.clear();
                    cursorCol = 0;
                    g_term->setCursorVisible(false);
                    return EditorResult::Saved;
                }
                else if (upperLine == "/Q")
                {
                    lines[cursorRow].text.clear();
                    cursorCol = 0;
                    handleQuoteWindow();
                    needFullRedraw = true;
                }
                else if (upperLine == "/U")
                {
                    lines[cursorRow].text.clear();
                    cursorCol = 0;
                    EditorStyle oldStyle = settings.editorStyle;
                    showEditorSettings(settings, baseDir);
                    if (settings.editorStyle != oldStyle)
                    {
                        if (settings.editorStyle == EditorStyle::Random)
                            currentStyle = (rand() % 2 == 0) ? EditorStyle::Ice : EditorStyle::Dct;
                        else
                            currentStyle = settings.editorStyle;
                        calculateLayout();
                        generateBorderColors();
                    }
                    needFullRedraw = true;
                }
                else if (upperLine == "/?")
                {
                    lines[cursorRow].text.clear();
                    cursorCol = 0;
                    showCommandKeyHelp();
                    needFullRedraw = true;
                }
                else
                {
                    // Normal enter - split line
                    // If the cursor is inside an ANSI sequence, adjust the
                    // split point to AFTER the sequence so it stays intact
                    int splitPos = cursorCol;
                    int seqAt = ansiSeqLen(lines[cursorRow].text, splitPos);
                    if (seqAt > 0)
                    {
                        splitPos += seqAt; // move past the sequence
                    }
                    // Also check if we're in the MIDDLE of a sequence
                    // (between ESC[ and the final byte) by scanning backward
                    if (splitPos > 0 && splitPos < static_cast<int>(lines[cursorRow].text.size()))
                    {
                        // Scan backward to see if there's an unclosed ESC[
                        for (int sc = splitPos - 1; sc >= 0; --sc)
                        {
                            if (static_cast<uint8_t>(lines[cursorRow].text[sc]) == 0x1B)
                            {
                                int sl = ansiSeqLen(lines[cursorRow].text, sc);
                                if (sl > 0 && sc + sl > splitPos)
                                {
                                    // We're inside this sequence — move split past it
                                    splitPos = sc + sl;
                                }
                                break;
                            }
                            char c = lines[cursorRow].text[sc];
                            if (!((c >= '0' && c <= '9') || c == ';' || c == '['))
                            {
                                break; // Not inside an ANSI sequence
                            }
                        }
                    }

                    string remainder = lines[cursorRow].text.substr(splitPos);
                    lines[cursorRow].text = lines[cursorRow].text.substr(0, splitPos);
                    lines[cursorRow].hardBreak = true;
                    EditorLine newLine;
                    newLine.text = remainder;
                    lines.insert(lines.begin() + cursorRow + 1, newLine);
                    ++cursorRow;
                    cursorCol = 0;
                }
                break;
            }

            case TK_RESIZE:
                calculateLayout();
                generateBorderColors();
                needFullRedraw = true;
                break;

            default:
                if (ch >= 32 && ch < 256)
                {
                    if (insertMode)
                    {
                        lines[cursorRow].text.insert(cursorCol, 1, static_cast<char>(ch));
                        ++cursorCol;
                    }
                    else
                    {
                        // Overwrite mode: skip over ANSI sequences at cursor position
                        // so we don't corrupt them
                        int writePos = cursorCol;
                        int seqLen = ansiSeqLen(lines[cursorRow].text, writePos);
                        if (seqLen > 0)
                        {
                            // Cursor is at start of ANSI sequence — insert before it
                            // rather than overwriting the escape byte
                            lines[cursorRow].text.insert(writePos, 1, static_cast<char>(ch));
                        }
                        else if (writePos < static_cast<int>(lines[cursorRow].text.size()))
                        {
                            lines[cursorRow].text[writePos] = static_cast<char>(ch);
                        }
                        else
                        {
                            lines[cursorRow].text += static_cast<char>(ch);
                        }
                        ++cursorCol;
                    }

                    // After advancing the cursor, skip over any ANSI sequence
                    // we may have landed on
                    {
                        int seqAtCursor = ansiSeqLen(lines[cursorRow].text, cursorCol);
                        if (seqAtCursor > 0)
                        {
                            cursorCol += seqAtCursor;
                        }
                    }

                    // Word wrap — use display width (excluding ANSI sequences)
                    int lineDisplayWidth = byteColToDisplayCol(
                        lines[cursorRow].text,
                        static_cast<int>(lines[cursorRow].text.size()));
                    int maxLineWidth = editWidth - 1;
                    if (lineDisplayWidth > maxLineWidth)
                    {
                        // Find wrap position: walk from byte end backwards
                        // looking for a space at or before the max line width
                        int wrapPos = static_cast<int>(lines[cursorRow].text.size());
                        for (int w = wrapPos; w > 0; --w)
                        {
                            if (byteColToDisplayCol(lines[cursorRow].text, w) <= maxLineWidth
                                && lines[cursorRow].text[w - 1] == ' ')
                            {
                                wrapPos = w;
                                break;
                            }
                        }
                        // If no space found, wrap at the byte position closest to max line width
                        if (wrapPos == static_cast<int>(lines[cursorRow].text.size()))
                        {
                            for (int w = 0; w < static_cast<int>(lines[cursorRow].text.size()); ++w)
                            {
                                if (byteColToDisplayCol(lines[cursorRow].text, w) >= maxLineWidth)
                                {
                                    wrapPos = w;
                                    break;
                                }
                            }
                        }
                        // Ensure the wrap position doesn't fall inside an ANSI sequence
                        {
                            const string& lineText = lines[cursorRow].text;
                            for (int sc = wrapPos - 1; sc >= 0; --sc)
                            {
                                if (static_cast<uint8_t>(lineText[sc]) == 0x1B)
                                {
                                    int sl = ansiSeqLen(lineText, sc);
                                    if (sl > 0 && sc + sl > wrapPos)
                                    {
                                        // wrapPos is inside this ANSI sequence;
                                        // move it to before the sequence
                                        wrapPos = sc;
                                    }
                                    break;
                                }
                                char c = lineText[sc];
                                if (!((c >= '0' && c <= '9') || c == ';' || c == '['))
                                {
                                    break;
                                }
                            }
                        }

                        string overflow = lines[cursorRow].text.substr(wrapPos);
                        lines[cursorRow].text = lines[cursorRow].text.substr(0, wrapPos);

                        // Strip trailing spaces from current line
                        while (!lines[cursorRow].text.empty() && lines[cursorRow].text.back() == ' ')
                        {
                            lines[cursorRow].text.pop_back();
                        }

                        // Strip leading spaces from overflow
                        int strippedLeading = 0;
                        while (!overflow.empty() && overflow[0] == ' ')
                        {
                            overflow = overflow.substr(1);
                            ++strippedLeading;
                        }

                        // Handle overflow: for quote lines, always create a new line
                        // (never merge with the next line). For normal text, prepend
                        // to the next line if it exists.
                        int nextLineIdx = cursorRow + 1;
                        bool curIsQuote = lines[cursorRow].isQuoteLine;

                        if (curIsQuote && !overflow.empty())
                        {
                            // Quote line: push overflow onto a new line, also marked as quote
                            EditorLine newLine;
                            newLine.text = overflow;
                            newLine.isQuoteLine = true;
                            if (nextLineIdx < static_cast<int>(lines.size()))
                            {
                                lines.insert(lines.begin() + nextLineIdx, newLine);
                            }
                            else
                            {
                                lines.push_back(newLine);
                            }
                        }
                        else if (nextLineIdx < static_cast<int>(lines.size()) && !overflow.empty())
                        {
                            string& nextText = lines[nextLineIdx].text;
                            if (!nextText.empty())
                            {
                                // Only add a space if overflow doesn't end with one
                                // and nextText doesn't start with one
                                bool needSpace = true;
                                if (!overflow.empty() && overflow.back() == ' ') needSpace = false;
                                if (!nextText.empty() && nextText[0] == ' ') needSpace = false;
                                nextText = overflow + (needSpace ? " " : "") + nextText;
                            }
                            else
                            {
                                nextText = overflow;
                            }
                        }
                        else if (!overflow.empty())
                        {
                            EditorLine newLine;
                            newLine.text = overflow;
                            if (nextLineIdx < static_cast<int>(lines.size()))
                            {
                                lines.insert(lines.begin() + nextLineIdx, newLine);
                            }
                            else
                            {
                                lines.push_back(newLine);
                            }
                        }

                        // Adjust cursor position if it was past the wrap point
                        if (cursorCol > static_cast<int>(lines[cursorRow].text.size()))
                        {
                            int offset = cursorCol - static_cast<int>(lines[cursorRow].text.size());
                            offset -= strippedLeading;
                            if (offset < 0) offset = 0;
                            ++cursorRow;
                            cursorCol = offset;
                            int sq = ansiSeqLen(lines[cursorRow].text, cursorCol);
                            if (sq > 0) cursorCol += sq;
                            if (cursorCol > static_cast<int>(lines[cursorRow].text.size()))
                            {
                                cursorCol = static_cast<int>(lines[cursorRow].text.size());
                            }
                        }

                        // Cascade: re-wrap subsequent lines if they became too long
                        for (int cascadeRow = cursorRow + 1;
                             cascadeRow < static_cast<int>(lines.size());
                             ++cascadeRow)
                        {
                            int cDispW = byteColToDisplayCol(
                                lines[cascadeRow].text,
                                static_cast<int>(lines[cascadeRow].text.size()));
                            if (cDispW <= maxLineWidth) break; // Line fits, stop cascading

                            // Find wrap point
                            int nwp = static_cast<int>(lines[cascadeRow].text.size());
                            for (int w = nwp; w > 0; --w)
                            {
                                if (byteColToDisplayCol(lines[cascadeRow].text, w) <= maxLineWidth
                                    && lines[cascadeRow].text[w - 1] == ' ')
                                {
                                    nwp = w;
                                    break;
                                }
                            }
                            if (nwp >= static_cast<int>(lines[cascadeRow].text.size())) break;

                            string cOverflow = lines[cascadeRow].text.substr(nwp);
                            lines[cascadeRow].text = lines[cascadeRow].text.substr(0, nwp);
                            // Strip trailing/leading spaces
                            while (!lines[cascadeRow].text.empty() && lines[cascadeRow].text.back() == ' ')
                            {
                                lines[cascadeRow].text.pop_back();
                            }
                            while (!cOverflow.empty() && cOverflow[0] == ' ')
                            {
                                cOverflow = cOverflow.substr(1);
                            }
                            if (cOverflow.empty()) break;

                            int afterIdx = cascadeRow + 1;
                            if (afterIdx < static_cast<int>(lines.size()))
                            {
                                string& afterText = lines[afterIdx].text;
                                if (!afterText.empty())
                                {
                                    bool needSp = true;
                                    if (!cOverflow.empty() && cOverflow.back() == ' ') needSp = false;
                                    if (!afterText.empty() && afterText[0] == ' ') needSp = false;
                                    afterText = cOverflow + (needSp ? " " : "") + afterText;
                                }
                                else
                                {
                                    afterText = cOverflow;
                                }
                            }
                            else
                            {
                                EditorLine nl;
                                nl.text = cOverflow;
                                lines.push_back(nl);
                            }
                        }
                    }
                }
                break;
        }

        // Scroll to keep cursor visible
        int effectiveHeight = editHeight;
        if (quoteWindowOpen)
        {
            effectiveHeight -= quoteWinHeight;
        }
        if (cursorRow < scrollRow)
        {
            scrollRow = cursorRow;
        }
        if (cursorRow >= scrollRow + effectiveHeight)
        {
            scrollRow = cursorRow - effectiveHeight + 1;
        }

        // Final cursor bounds check
        if (cursorRow >= 0 && cursorRow < static_cast<int>(lines.size()))
        {
            if (cursorCol > static_cast<int>(lines[cursorRow].text.size()))
            {
                cursorCol = static_cast<int>(lines[cursorRow].text.size());
            }
            // If the clamp landed us on an ANSI sequence, skip past it
            // (unless we're at position 0, which is always valid)
            if (cursorCol > 0)
            {
                int sq = ansiSeqLen(lines[cursorRow].text, cursorCol);
                if (sq > 0) cursorCol += sq;
                // Re-clamp after skip
                if (cursorCol > static_cast<int>(lines[cursorRow].text.size()))
                {
                    cursorCol = static_cast<int>(lines[cursorRow].text.size());
                }
            }
        }
    }
}

// Helper: strip all ANSI escape sequences from a string, returning only visible text.
static string stripAnsiCodes(const string& s)
{
    string result;
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (static_cast<uint8_t>(s[i]) == 0x1B && i + 1 < s.size() && s[i + 1] == '[')
        {
            i += 2;
            while (i < s.size() && s[i] >= 0x20 && s[i] <= 0x3F) ++i;
            // skip final byte
            continue;
        }
        result += s[i];
    }
    return result;
}

string MessageEditor::getBody() const
{
    // Build the message body by joining soft-wrapped non-quote lines into
    // single long paragraph lines.  This matches how SlyEdit for Synchronet
    // saves messages: each paragraph of user-typed text becomes one long line,
    // allowing the recipient's reader to word-wrap it to their own terminal
    // width.  Quote lines are preserved exactly as-is.
    //
    // The joining logic: accumulate non-quote, non-empty lines that don't
    // have hardBreak set.  When a line has hardBreak (user pressed Enter),
    // or when a quote line or empty line is reached, the accumulated paragraph
    // is emitted and a new one starts.
    vector<string> outputLines;

    size_t i = 0;
    while (i < lines.size())
    {
        // Quote lines: output as-is
        if (lines[i].isQuoteLine)
        {
            outputLines.push_back(lines[i].text);
            ++i;
            continue;
        }

        // Treat lines with only ANSI codes (no visible text) as empty
        string visibleText = stripAnsiCodes(lines[i].text);
        bool effectivelyEmpty = visibleText.empty() ||
            visibleText.find_first_not_of(" \t") == string::npos;

        if (effectivelyEmpty)
        {
            outputLines.push_back("");
            ++i;
            continue;
        }

        // Non-quote, non-empty line: accumulate paragraph text.
        // Join consecutive lines that don't have hardBreak set, stopping
        // when we hit a hardBreak, empty line, or quote line.
        string paragraph;
        while (i < lines.size() && !lines[i].isQuoteLine)
        {
            // Check if this line is effectively empty (only ANSI codes / whitespace)
            string vis = stripAnsiCodes(lines[i].text);
            if (vis.empty() || vis.find_first_not_of(" \t") == string::npos)
                break;

            // Strip ANSI codes from the line text for the saved body
            string cleanText = stripAnsiCodes(lines[i].text);

            if (!paragraph.empty())
            {
                // Add a space to join words at the wrap boundary
                if (paragraph.back() != ' ' &&
                    !cleanText.empty() && cleanText[0] != ' ')
                {
                    paragraph += ' ';
                }
            }
            paragraph += cleanText;
            bool wasHardBreak = lines[i].hardBreak;
            ++i;
            if (wasHardBreak) break; // Hard break ends the paragraph
        }

        if (!paragraph.empty())
        {
            outputLines.push_back(paragraph);
        }
    }

    // Join output lines with newlines
    string body;
    for (size_t j = 0; j < outputLines.size(); ++j)
    {
        body += outputLines[j];
        if (j + 1 < outputLines.size())
            body += "\n";
    }
    return body;
}

// ---- Tagline support ----

// Load taglines from file
vector<string> loadTaglines(const string& baseDir)
{
    vector<string> taglines;
    string path = baseDir + PATH_SEP_STR + "tagline_files" + PATH_SEP_STR + "taglines.txt";
    std::ifstream f(path);
    if (!f.is_open())
    {
        return taglines;
    }
    string line;
    while (std::getline(f, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        if (!line.empty() && line[0] != '#' && line[0] != ';')
        {
            taglines.push_back(line);
        }
    }
    return taglines;
}

// Show tagline selection dialog; returns selected tagline or empty string
string selectTagline(const string& baseDir)
{
    vector<string> taglines = loadTaglines(baseDir);
    if (taglines.empty())
    {
        messageDialog("Taglines", "No taglines found in tagline_files/taglines.txt");
        return "";
    }

    int dlgW = 70;
    if (dlgW > g_term->getCols() - 4)
    {
        dlgW = g_term->getCols() - 4;
    }
    int visibleItems = g_term->getRows() - 8;
    if (visibleItems < 5)
    {
        visibleItems = 5;
    }
    int dlgH = visibleItems + 4;
    int dlgY = (g_term->getRows() - dlgH) / 2;
    int dlgX = (g_term->getCols() - dlgW) / 2;

    TermAttr borderAttr = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr titleAttr  = tAttr(TC_BLUE, TC_BLACK, true);
    TermAttr itemAttr   = tAttr(TC_CYAN, TC_BLACK, false);
    TermAttr selAttr    = tAttr(TC_BLUE, TC_WHITE, false);

    int cursor = 0;
    int scrollOffset = 0;

    for (;;)
    {
        if (cursor < scrollOffset)
        {
            scrollOffset = cursor;
        }
        if (cursor >= scrollOffset + visibleItems)
        {
            scrollOffset = cursor - visibleItems + 1;
        }

        g_term->setAttr(borderAttr);
        g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

        // Title
        string title = " Select Tagline (R=Random) ";
        int titleX = dlgX + 3;
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, titleX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        g_term->setAttr(titleAttr);
        g_term->printStr(dlgY, titleX + 1, title);
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, titleX + static_cast<int>(title.size()) + 1,
                         CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

        // Items
        for (int i = 0; i < visibleItems; ++i)
        {
            int idx = i + scrollOffset;
            int y = dlgY + 1 + i;
            if (idx >= static_cast<int>(taglines.size()))
            {
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
                continue;
            }
            bool isSel = (idx == cursor);
            if (isSel)
            {
                fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
            }
            else
            {
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
            }
            printAt(y, dlgX + 2, truncateStr(taglines[idx], dlgW - 4),
                    isSel ? selAttr : itemAttr);
        }

        // Bottom help
        int helpY = dlgY + dlgH - 2;
        g_term->setAttr(borderAttr);
        g_term->putCP437(helpY, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
        g_term->drawHLine(helpY, dlgX + 1, dlgW - 2);
        g_term->putCP437(helpY, dlgX + dlgW - 1,
                         CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        string helpText = "Up, Dn, Enter=Select, R=Random, ESC=None";
        int helpX = dlgX + 2;
        g_term->setAttr(borderAttr);
        g_term->putCP437(helpY, helpX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        g_term->setAttr(titleAttr);
        g_term->printStr(helpY, helpX + 1, helpText);
        g_term->setAttr(borderAttr);
        g_term->putCP437(helpY, helpX + static_cast<int>(helpText.size()) + 1,
                         CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

        g_term->refresh();

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:
                if (cursor > 0) --cursor;
                break;
            case TK_DOWN:
                if (cursor < static_cast<int>(taglines.size()) - 1) ++cursor;
                break;
            case TK_PGUP:
                cursor -= visibleItems;
                if (cursor < 0) cursor = 0;
                break;
            case TK_PGDN:
                cursor += visibleItems;
                if (cursor >= static_cast<int>(taglines.size()))
                    cursor = static_cast<int>(taglines.size()) - 1;
                break;
            case TK_HOME:
                cursor = 0;
                break;
            case TK_END:
                cursor = static_cast<int>(taglines.size()) - 1;
                break;
            case TK_ENTER:
                return taglines[cursor];
            case 'r':
            case 'R':
                return taglines[rand() % taglines.size()];
            case TK_ESCAPE:
                return "";
            default:
                break;
        }
    }
}

// ---- Spell-check support ----

// Load dictionary words into a set
vector<string> loadDictionary(const string& path)
{
    vector<string> words;
    std::ifstream f(path);
    if (!f.is_open())
    {
        return words;
    }
    string word;
    while (std::getline(f, word))
    {
        if (!word.empty() && word.back() == '\r')
        {
            word.pop_back();
        }
        if (!word.empty())
        {
            // Convert to lowercase for case-insensitive matching
            for (auto& c : word)
            {
                c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
            }
            words.push_back(word);
        }
    }
    // Sort for binary search
    std::sort(words.begin(), words.end());
    return words;
}

// Check if a word is in the dictionary (case-insensitive)
bool isWordInDict(const vector<string>& dict, const string& word)
{
    string lower = word;
    for (auto& c : lower)
    {
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }
    return std::binary_search(dict.begin(), dict.end(), lower);
}

// Extract words from text for spell-checking
vector<string> extractWords(const string& text)
{
    vector<string> words;
    string current;
    for (char c : text)
    {
        if (isalpha(static_cast<unsigned char>(c)) || c == '\'')
        {
            current += c;
        }
        else
        {
            if (!current.empty())
            {
                words.push_back(current);
                current.clear();
            }
        }
    }
    if (!current.empty())
    {
        words.push_back(current);
    }
    return words;
}

// Perform spell-check on the message body and show correction dialog
// Returns true if any corrections were made
bool performSpellCheck(vector<EditorLine>& lines,
                       const string& baseDir,
                       const string& dictList)
{
    // Load all configured dictionaries
    vector<string> allDictWords;
    std::istringstream dictStream(dictList);
    string dictFile;
    while (std::getline(dictStream, dictFile, ','))
    {
        if (dictFile.empty())
        {
            continue;
        }
        string path = baseDir + PATH_SEP_STR + "dictionary_files" + PATH_SEP_STR + dictFile;
        auto words = loadDictionary(path);
        allDictWords.insert(allDictWords.end(), words.begin(), words.end());
    }

    if (allDictWords.empty())
    {
        messageDialog("Spell Check", "No dictionaries loaded. Configure in settings.");
        return false;
    }
    std::sort(allDictWords.begin(), allDictWords.end());
    allDictWords.erase(std::unique(allDictWords.begin(), allDictWords.end()),
                       allDictWords.end());

    // Find misspelled words
    struct Misspelling
    {
        int lineNum;
        int wordStart;
        int wordLen;
        string word;
    };
    vector<Misspelling> misspellings;

    for (int lineNum = 0; lineNum < static_cast<int>(lines.size()); ++lineNum)
    {
        const string& text = lines[lineNum].text;
        int i = 0;
        while (i < static_cast<int>(text.size()))
        {
            if (isalpha(static_cast<unsigned char>(text[i])))
            {
                int start = i;
                string word;
                while (i < static_cast<int>(text.size()) && (isalpha(static_cast<unsigned char>(text[i])) || text[i] == '\''))
                {
                    word += text[i];
                    ++i;
                }
                // Skip very short words and words with numbers
                if (word.size() >= 2 && !isWordInDict(allDictWords, word))
                {
                    misspellings.push_back({lineNum, start, static_cast<int>(word.size()), word});
                }
            }
            else
            {
                ++i;
            }
        }
    }

    if (misspellings.empty())
    {
        messageDialog("Spell Check", "No misspelled words found!");
        return false;
    }

    // Show corrections dialog
    bool anyChanged = false;
    for (size_t mi = 0; mi < misspellings.size(); ++mi)
    {
        auto& ms = misspellings[mi];
        // Verify the word still exists at this position (previous edits may have shifted things)
        if (ms.lineNum >= static_cast<int>(lines.size()))
        {
            continue;
        }

        string remaining = std::to_string(misspellings.size() - mi) + " word(s) remaining";

        int dlgW = 50;
        int dlgH = 7;
        int dlgY = (g_term->getRows() - dlgH) / 2;
        int dlgX = (g_term->getCols() - dlgW) / 2;

        TermAttr borderAttr = tAttr(TC_GREEN, TC_BLACK, false);
        TermAttr titleAttr  = tAttr(TC_BLUE, TC_BLACK, true);
        TermAttr wordAttr   = tAttr(TC_RED, TC_BLACK, true);
        TermAttr helpAttr   = tAttr(TC_CYAN, TC_BLACK, false);

        for (int r = 0; r < dlgH; ++r)
        {
            fillRow(dlgY + r, tAttr(TC_BLACK, TC_BLACK, false), dlgX, dlgX + dlgW);
        }
        g_term->setAttr(borderAttr);
        g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

        string title = " Spell Check ";
        int titleX = dlgX + 3;
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, titleX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        g_term->setAttr(titleAttr);
        g_term->printStr(dlgY, titleX + 1, title);
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, titleX + static_cast<int>(title.size()) + 1,
                         CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

        printAt(dlgY + 1, dlgX + 2, remaining, helpAttr);
        printAt(dlgY + 2, dlgX + 2, "Word: ", helpAttr);
        printAt(dlgY + 2, dlgX + 8, ms.word, wordAttr);
        printAt(dlgY + 3, dlgX + 2, "R)eplace, S)kip, A)dd, Q)uit",
                tAttr(TC_GREEN, TC_BLACK, false));
        printAt(dlgY + 4, dlgX + 2, "Replace with: ", helpAttr);

        g_term->refresh();

        int ch = g_term->getKey();
        if (ch == 'q' || ch == 'Q' || ch == TK_ESCAPE)
        {
            break;
        }
        else if (ch == 's' || ch == 'S')
        {
            continue;
        }
        else if (ch == 'a' || ch == 'A')
        {
            // Add to dictionary (skip for now - would need user dictionary)
            continue;
        }
        else if (ch == 'r' || ch == 'R')
        {
            string replacement = getStringInput(
                dlgY + 4, dlgX + 16, dlgW - 20, ms.word,
                tAttr(TC_WHITE, TC_BLACK, true));
            if (!replacement.empty() && replacement != ms.word)
            {
                // Replace the word in the line
                string& lineText = lines[ms.lineNum].text;
                if (ms.wordStart + ms.wordLen <= static_cast<int>(lineText.size()))
                {
                    lineText.replace(ms.wordStart, ms.wordLen, replacement);
                    anyChanged = true;
                }
            }
        }
    }

    return anyChanged;
}

// ---- Post-save processing (tagline + spell-check) ----

void postSaveProcessing(vector<EditorLine>& lines,
                        Settings& settings,
                        const string& baseDir)
{
    // Spell-check prompt
    if (settings.promptSpellCheck && !settings.spellDictionaries.empty())
    {
        if (confirmDialog("Run spell checker on your message?"))
        {
            performSpellCheck(lines, baseDir, settings.spellDictionaries);
        }
    }

    // Tagline selection - prompt user first
    if (settings.taglines)
    {
        if (confirmDialog("Add a tagline to the message?"))
        {
            string tagline = selectTagline(baseDir);
            if (!tagline.empty())
            {
                // Add tagline with "..." prefix
                lines.push_back(EditorLine{""});
                lines.push_back(EditorLine{"..." + tagline});
            }
        }
    }
}

// ---- Convenience functions ----

EditorResult editReply(const QwkMessage& origMsg,
                       const string& userName,
                       const string& confName,
                       QwkReply& reply,
                       Settings& settings,
                       const string& baseDir)
{
    MessageEditor editor;
    editor.toField = origMsg.from;
    editor.fromField = userName;
    editor.subjectField = origMsg.subject;
    if (editor.subjectField.substr(0, 4) != "Re: " &&
        editor.subjectField.substr(0, 3) != "Re:")
    {
        if (editor.subjectField.size() > 21)
        {
            editor.subjectField = editor.subjectField.substr(0, 21);
        }
        editor.subjectField = "Re: " + editor.subjectField;
    }
    editor.areaName = confName;
    editor.prepareQuotes(origMsg, settings);

    EditorResult result = editor.run(settings, baseDir);
    if (result == EditorResult::Saved)
    {
        postSaveProcessing(editor.lines, settings, baseDir);
        string body = editor.getBody();
        // Check if the message body is empty (whitespace-only counts as empty)
        bool isEmpty = true;
        for (char c : body)
        {
            if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
            {
                isEmpty = false;
                break;
            }
        }
        if (isEmpty)
        {
            messageDialog("Message Aborted", "Message was empty. Message not posted.");
            return EditorResult::Aborted;
        }
        reply.conference = origMsg.conference;
        reply.to = editor.toField;
        reply.from = editor.fromField;
        reply.subject = editor.subjectField;
        reply.body = body;
        reply.replyToNum = origMsg.number;
        // Set editor identifier string
        string styleStr = (editor.currentStyle == EditorStyle::Ice) ? "Ice style" : "DCT style";
        reply.editor = string(PROGRAM_NAME) + " " + PROGRAM_VERSION
                     + " (" + PROGRAM_DATE + ") (" + styleStr + ")";
    }
    return result;
}

EditorResult editNewMessage(const string& userName,
                            const string& confName,
                            int confNumber,
                            QwkReply& reply,
                            Settings& settings,
                            const string& baseDir)
{
    int cols = g_term->getCols();
    int rows = g_term->getRows();

    MessageEditor editor;
    editor.toField = "All";
    editor.fromField = userName;
    editor.subjectField = "";
    editor.areaName = confName;

    // Prompt for To and Subject with styled dialog
    g_term->clear();
    TermAttr borderAttr = tAttr(TC_CYAN, TC_BLACK, true);
    int boxW = 50;
    int boxH = 9;
    int boxY = (rows - boxH) / 2;
    int boxX = (cols - boxW) / 2;

    drawBox(boxY, boxX, boxH, boxW, borderAttr, "New Message", borderAttr);

    printAt(boxY + 2, boxX + 3, "Conference: " + confName,
        tAttr(TC_GREEN, TC_BLACK, false));

    printAt(boxY + 4, boxX + 3, "     To: ", tAttr(TC_CYAN, TC_BLACK, true));
    string to = getStringInput(boxY + 4, boxX + 12, 25, "All",
        tAttr(TC_WHITE, TC_BLACK, true));
    if (to.empty())
    {
        return EditorResult::Aborted;
    }
    editor.toField = to;

    printAt(boxY + 5, boxX + 3, "Subject: ", tAttr(TC_CYAN, TC_BLACK, true));
    string subj = getStringInput(boxY + 5, boxX + 12, 25, "",
        tAttr(TC_WHITE, TC_BLACK, true));
    if (subj.empty())
    {
        return EditorResult::Aborted;
    }
    editor.subjectField = subj;

    EditorResult result = editor.run(settings, baseDir);
    if (result == EditorResult::Saved)
    {
        postSaveProcessing(editor.lines, settings, baseDir);
        string body = editor.getBody();
        // Check if the message body is empty (whitespace-only counts as empty)
        bool isEmpty = true;
        for (char c : body)
        {
            if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
            {
                isEmpty = false;
                break;
            }
        }
        if (isEmpty)
        {
            messageDialog("Message Aborted", "Message was empty. Message not posted.");
            return EditorResult::Aborted;
        }
        reply.conference = confNumber;
        reply.to = editor.toField;
        reply.from = editor.fromField;
        reply.subject = editor.subjectField;
        reply.body = body;
        reply.replyToNum = 0;
        // Set editor identifier string
        string styleStr = (editor.currentStyle == EditorStyle::Ice) ? "Ice style" : "DCT style";
        reply.editor = string(PROGRAM_NAME) + " " + PROGRAM_VERSION
                     + " (" + PROGRAM_DATE + ") (" + styleStr + ")";
    }
    return result;
}
