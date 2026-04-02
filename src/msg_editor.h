#ifndef SLYMAIL_MSG_EDITOR_H
#define SLYMAIL_MSG_EDITOR_H

#include "terminal.h"
#include "theme.h"
#include "qwk.h"
#include "settings.h"
#include "bbs_colors.h"
#include "text_input.h"
#include <chrono>

enum class EditorResult
{
    Saved,
    Aborted,
};

// EditorLine is an alias for TextLine from text_input.h
using EditorLine = TextLine;

// The message editor (SlyEdit-style with ICE and DCT modes)
class MessageEditor
{
public:
    std::string toField;
    std::string fromField;
    std::string subjectField;
    std::string areaName;

    std::vector<EditorLine> lines;
    std::vector<std::string> quoteLines;

    int cursorRow;
    int cursorCol;
    int scrollRow;

    bool insertMode;
    EditorStyle currentStyle;

    int editTop;
    int editBottom;
    int editLeft;
    int editRight;
    int editWidth;
    int editHeight;

    bool quoteWindowOpen;
    int quoteSelected;
    int quoteScroll;
    int quoteWinTop;
    int quoteWinHeight;

    std::string lastSearchText;
    int searchStartLine;

    IceTheme iceTheme;
    DctTheme dctTheme;
    AttrCodeFlags attrFlags;  // Attribute code toggle flags (from settings)

    // Last-selected color in the Ctrl-K color picker (persists across calls)
    int lastPickFg;
    int lastPickBg;
    bool lastPickBright;
    int lastPickSection;  // 0=foreground, 1=background, 2=attributes

    // Pre-generated border color patterns (generated once per mode change)
    // Each vector stores per-character color choice (0 or 1) for a border line
    std::vector<int> iceTopBorderColors;       // Top border row
    std::vector<int> iceSepBorderColors;       // Separator row (row 3/4)
    std::vector<int> iceVertLeftColors;        // Left vertical chars per row
    std::vector<int> iceVertRightColors;       // Right vertical chars per row
    std::vector<int> iceBottomBorderColors;    // Status bar border
    std::vector<int> dctTopBorderColors;       // DCT top border
    std::vector<int> dctEditTopBorderColors;   // DCT edit area top border
    std::vector<int> dctEditBottomBorderColors; // DCT edit area bottom border
    std::vector<int> dctVertLeftColors;        // DCT left vertical chars
    std::vector<int> dctVertRightColors;       // DCT right vertical chars

    MessageEditor();

    // Generate random border color patterns (called once on init and on style change)
    void generateBorderColors();

    TermAttr randomBorderColor(const TermAttr& c1, const TermAttr& c2);
    void init(const Settings& settings, const std::string& baseDir = "");
    void calculateLayout();
    void prepareQuotes(const QwkMessage& msg, const Settings& settings);
    void drawIceHeader();
    void drawDctHeader();
    void drawIceStatusBar();
    void drawDctStatusBar();
    void drawEditArea();
    void drawQuoteWindow();
    bool promptYesNoIce(const std::string& question);
    bool promptYesNoDct(const std::string& question, const std::string& title = "");
    bool confirmQuestion(const std::string& question, const std::string& title = "");
    int showEscMenu();
    bool handleQuoteWindow();
    void showHelpScreen();
    void findText();
    void insertGraphicChar();
    void showGraphicCharList();
    void showCommandKeyHelp();
    void pickColor();
    EditorResult run(Settings& settings, const std::string& baseDir = "");
    std::string getBody() const;
};

// ---- Tagline support ----
std::vector<std::string> loadTaglines(const std::string& baseDir);
std::string selectTagline(const std::string& baseDir);

// ---- Spell-check support ----
std::vector<std::string> loadDictionary(const std::string& path);
bool isWordInDict(const std::vector<std::string>& dict, const std::string& word);
std::vector<std::string> extractWords(const std::string& text);
bool performSpellCheck(std::vector<EditorLine>& lines, const std::string& baseDir, const std::string& dictList);

// ---- Post-save processing ----
void postSaveProcessing(std::vector<EditorLine>& lines, Settings& settings, const std::string& baseDir);

// ---- Convenience functions ----
EditorResult editReply(const QwkMessage& origMsg, const std::string& userName, const std::string& confName, QwkReply& reply, Settings& settings, const std::string& baseDir = "");
EditorResult editNewMessage(const std::string& userName, const std::string& confName, int confNumber, QwkReply& reply, Settings& settings, const std::string& baseDir = "");

#endif // SLYMAIL_MSG_EDITOR_H
