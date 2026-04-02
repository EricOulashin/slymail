// SlyMail Configuration Program
// Standalone utility to configure SlyMail settings outside of the main application.
// Uses the same terminal abstraction (ITerminal) for cross-platform ncurses/conio support.
// Settings are saved to slymail.ini in the SlyMail data directory (~/.slymail).

#include "terminal.h"
#include "file_dir_utils.h"
#include "colors.h"
#include "ui_common.h"
#include "settings.h"
#include "settings_dialog.h"
#include "file_browser.h"
#include "program_info.h"

#include <filesystem>
#include <ctime>

using std::string;
using std::vector;

namespace fs = std::filesystem;


// ============================================================
// Configuration menu categories
// ============================================================
enum class ConfigCategory
{
    Editor,
    Reader,
    Themes,
    General,
    COUNT
};

static const char* categoryNames[] =
{
    "Editor Settings",
    "Reader Settings",
    "Theme Settings",
    "General Settings"
};

// ============================================================
// Draw the main config screen header
// ============================================================
static void drawHeader()
{
    int cols = g_term->getCols();

    // Top bar
    g_term->setAttr(tAttr(TC_WHITE, TC_BLUE, true));
    g_term->fillRegion(0, 0, cols, ' ');
    string title = string(CONFIG_PROGRAM_NAME) + " v" + string(PROGRAM_VERSION);
    g_term->printStr(0, (cols - static_cast<int>(title.size())) / 2, title);

    // Separator
    g_term->setAttr(tAttr(TC_CYAN, TC_BLACK, false));
    g_term->drawHLine(1, 0, cols);
}

// ============================================================
// Draw the bottom help bar
// ============================================================
static void drawFooter(const string& helpText)
{
    int cols = g_term->getCols();
    int rows = g_term->getRows();

    g_term->setAttr(tAttr(TC_WHITE, TC_BLUE, true));
    g_term->fillRegion(rows - 1, 0, cols, ' ');
    g_term->printStr(rows - 1, (cols - static_cast<int>(helpText.size())) / 2, helpText);
}

// ============================================================
// Editor settings screen
// ============================================================
static bool editEditorSettings(Settings& settings, const string& baseDir)
{
    // Use the same editor settings dialog from settings_dialog.h
    return showEditorSettings(settings, baseDir);
}

// ============================================================
// Reader settings screen
// ============================================================
static bool editReaderSettings(Settings& settings)
{
    struct ReaderItem
    {
        string label;
        bool*  value;
    };
    vector<ReaderItem> items =
    {
        {"Show kludge/control lines (@MSGID, @REPLY, etc.)", &settings.showKludgeLines},
        {"Show tear lines and origin lines",                  &settings.showTearLine},
        {"Show scrollbar in message reader",                  &settings.useScrollbar},
        {"Only show areas with new mail",                     &settings.onlyShowAreasWithNewMail},
        {"Strip ANSI codes from messages",                    &settings.stripAnsi},
        {"Show messages in reverse order (newest first)",      &settings.reverseOrder},
    };

    // Total rows = bool items + 1 extra row for "Attribute code toggles..."
    int boolItemCount = static_cast<int>(items.size());
    int itemCount = boolItemCount + 1;  // +1 for sub-menu
    int attrToggleIdx = boolItemCount;  // Last item is the sub-menu
    int selected = 0;
    bool changed = false;

    int dlgW = 73;
    if (dlgW > g_term->getCols() - 4)
    {
        dlgW = g_term->getCols() - 4;
    }
    int dlgH = itemCount + 5;
    int dlgY = (g_term->getRows() - dlgH) / 2;
    int dlgX = (g_term->getCols() - dlgW) / 2;

    TermAttr borderAttr = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr titleAttr  = tAttr(TC_BLUE, TC_BLACK, true);
    TermAttr itemAttr   = tAttr(TC_CYAN, TC_BLACK, false);
    TermAttr selAttr    = tAttr(TC_BLUE, TC_WHITE, false);
    TermAttr checkAttr  = tAttr(TC_GREEN, TC_BLACK, true);
    int checkCol = dlgX + 54;

    for (;;)
    {
        // Clear and draw
        for (int r = 0; r < g_term->getRows(); ++r)
        {
            fillRow(r, tAttr(TC_BLACK, TC_BLACK, false), 0, g_term->getCols());
        }
        drawHeader();

        g_term->setAttr(borderAttr);
        g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

        // Title
        string titleText = " Reader Settings ";
        int titleX = dlgX + 3;
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, titleX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        g_term->setAttr(titleAttr);
        g_term->printStr(dlgY, titleX + 1, titleText);
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, titleX + static_cast<int>(titleText.size()) + 1,
                         CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

        // "Enabled" label
        g_term->setAttr(titleAttr);
        g_term->printStr(dlgY, dlgX + dlgW - 14, " Enabled ");

        // Items
        for (int i = 0; i < itemCount; ++i)
        {
            int y = dlgY + 1 + i;
            bool isSel = (i == selected);

            if (isSel)
            {
                fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
            }
            else
            {
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
            }

            TermAttr lbl = isSel ? selAttr : itemAttr;

            if (i < boolItemCount)
            {
                printAt(y, dlgX + 2, truncateStr(items[i].label, 50), lbl);

                // Checkbox
                TermAttr chk = isSel ? selAttr : checkAttr;
                if (*(items[i].value))
                {
                    printAt(y, checkCol, "[", chk);
                    g_term->setAttr(chk);
                    g_term->putCP437(y, checkCol + 1, CP437_CHECK_MARK);
                    printAt(y, checkCol + 2, "]", chk);
                }
                else
                {
                    printAt(y, checkCol, "[ ]", chk);
                }
            }
            else
            {
                // Sub-menu item: "Attribute code toggles..."
                printAt(y, dlgX + 2, "Attribute code toggles...", lbl);
            }
        }

        // Scrollbar if content overflows
        if (itemCount > (dlgH - 5))
        {
            drawScrollbar(dlgY + 1, itemCount, selected, itemCount,
                         tAttr(TC_BLACK, TC_BLACK, true),
                         tAttr(TC_WHITE, TC_BLACK, true));
        }

        // Bottom help
        int helpY = dlgY + dlgH - 2;
        g_term->setAttr(borderAttr);
        g_term->putCP437(helpY, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
        g_term->drawHLine(helpY, dlgX + 1, dlgW - 2);
        g_term->putCP437(helpY, dlgX + dlgW - 1,
                         CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        string helpText = "Up, Dn, Enter/Space=Toggle, ESC=Done";
        int helpX = dlgX + (dlgW - static_cast<int>(helpText.size())) / 2;
        g_term->setAttr(titleAttr);
        g_term->printStr(helpY, helpX, helpText);

        drawFooter("Arrow keys to navigate, Enter/Space to toggle, ESC to go back");
        g_term->refresh();

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:
                if (selected > 0) --selected;
                break;
            case TK_DOWN:
                if (selected < itemCount - 1) ++selected;
                break;
            case TK_HOME:
                selected = 0;
                break;
            case TK_END:
                selected = itemCount - 1;
                break;
            case TK_ENTER:
            case ' ':
                if (selected == attrToggleIdx)
                {
                    // Open attribute code toggles sub-dialog
                    if (showAttrCodeToggles(settings))
                    {
                        changed = true;
                    }
                }
                else
                {
                    *(items[selected].value) = !*(items[selected].value);
                    changed = true;
                }
                break;
            case TK_ESCAPE:
            case 'q':
            case 'Q':
                return changed;
            default:
                break;
        }
    }
}

// ============================================================
// Theme settings screen
// ============================================================
static bool editThemeSettings(Settings& settings, const string& baseDir)
{
    struct ThemeItem
    {
        string label;
        string* value;
        string prefix;   // filename prefix to filter
    };
    vector<ThemeItem> items =
    {
        {"Ice mode theme file:  " + (settings.iceThemeFile.empty() ? "(default)" : settings.iceThemeFile),
         &settings.iceThemeFile, "EditorIceColors_"},
        {"DCT mode theme file:  " + (settings.dctThemeFile.empty() ? "(default)" : settings.dctThemeFile),
         &settings.dctThemeFile, "EditorDCTColors_"},
    };

    int itemCount = static_cast<int>(items.size());
    int selected = 0;
    bool changed = false;

    for (;;)
    {
        // Rebuild labels with current values
        items[0].label = "Ice mode theme:  " + (settings.iceThemeFile.empty() ? "(default: BlueIce)" : settings.iceThemeFile);
        items[1].label = "DCT mode theme:  " + (settings.dctThemeFile.empty() ? "(default: Default)" : settings.dctThemeFile);

        int dlgW = 73;
        if (dlgW > g_term->getCols() - 4)
        {
            dlgW = g_term->getCols() - 4;
        }
        int dlgH = itemCount + 5;
        int dlgY = (g_term->getRows() - dlgH) / 2;
        int dlgX = (g_term->getCols() - dlgW) / 2;

        TermAttr borderAttr = tAttr(TC_GREEN, TC_BLACK, false);
        TermAttr titleAttr  = tAttr(TC_BLUE, TC_BLACK, true);
        TermAttr itemAttr   = tAttr(TC_CYAN, TC_BLACK, false);
        TermAttr selAttr    = tAttr(TC_BLUE, TC_WHITE, false);

        // Clear
        for (int r = 0; r < g_term->getRows(); ++r)
        {
            fillRow(r, tAttr(TC_BLACK, TC_BLACK, false), 0, g_term->getCols());
        }
        drawHeader();

        g_term->setAttr(borderAttr);
        g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

        string titleText = " Theme Settings ";
        int titleX = dlgX + 3;
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, titleX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        g_term->setAttr(titleAttr);
        g_term->printStr(dlgY, titleX + 1, titleText);
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, titleX + static_cast<int>(titleText.size()) + 1,
                         CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

        for (int i = 0; i < itemCount; ++i)
        {
            int y = dlgY + 1 + i;
            bool isSel = (i == selected);

            if (isSel)
            {
                fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
            }
            else
            {
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
            }

            printAt(y, dlgX + 2, truncateStr(items[i].label, dlgW - 4),
                    isSel ? selAttr : itemAttr);
        }

        // Fill empty rows between items and help
        for (int r = dlgY + 1 + itemCount; r < dlgY + dlgH - 2; ++r)
        {
            fillRow(r, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
            g_term->setAttr(borderAttr);
            g_term->putCP437(r, dlgX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
            g_term->putCP437(r, dlgX + dlgW - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
        }

        int helpY = dlgY + dlgH - 2;
        g_term->setAttr(borderAttr);
        g_term->putCP437(helpY, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
        g_term->drawHLine(helpY, dlgX + 1, dlgW - 2);
        g_term->putCP437(helpY, dlgX + dlgW - 1,
                         CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        string helpText = "Enter=Select theme file, ESC=Done";
        int helpX = dlgX + (dlgW - static_cast<int>(helpText.size())) / 2;
        g_term->setAttr(titleAttr);
        g_term->printStr(helpY, helpX, helpText);

        drawFooter("Enter to browse theme files, ESC to go back");
        g_term->refresh();

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:
                if (selected > 0) --selected;
                break;
            case TK_DOWN:
                if (selected < itemCount - 1) ++selected;
                break;
            case TK_ENTER:
            case ' ':
            {
                // Show theme file selector
                string configDir = baseDir + PATH_SEP_STR + "config_files";
                vector<string> themeFiles;
                try
                {
                    for (auto& entry : fs::directory_iterator(configDir))
                    {
                        if (entry.is_regular_file())
                        {
                            string name = entry.path().filename().string();
                            if (name.find(items[selected].prefix) == 0 &&
                                name.size() > 4 &&
                                name.substr(name.size() - 4) == ".ini")
                            {
                                themeFiles.push_back(name);
                            }
                        }
                    }
                }
                catch (...) {}
                std::sort(themeFiles.begin(), themeFiles.end());

                if (themeFiles.empty())
                {
                    messageDialog("Themes", "No theme files found in config/");
                    break;
                }

                // Show selection dialog
                int cursor = 0;
                int sDlgW = 60;
                if (sDlgW > g_term->getCols() - 4) sDlgW = g_term->getCols() - 4;
                int visible = static_cast<int>(themeFiles.size());
                if (visible > g_term->getRows() - 8) visible = g_term->getRows() - 8;
                int sDlgH = visible + 4;
                int sDlgY = (g_term->getRows() - sDlgH) / 2;
                int sDlgX = (g_term->getCols() - sDlgW) / 2;
                int scrollOff = 0;
                bool themeChanged = false;

                // Pre-select current theme
                for (int i = 0; i < static_cast<int>(themeFiles.size()); ++i)
                {
                    if (themeFiles[i] == *(items[selected].value))
                    {
                        cursor = i;
                        break;
                    }
                }

                for (;;)
                {
                    if (cursor < scrollOff) scrollOff = cursor;
                    if (cursor >= scrollOff + visible) scrollOff = cursor - visible + 1;

                    for (int r = 0; r < g_term->getRows(); ++r)
                    {
                        fillRow(r, tAttr(TC_BLACK, TC_BLACK, false), 0, g_term->getCols());
                    }
                    drawHeader();

                    g_term->setAttr(borderAttr);
                    g_term->drawBox(sDlgY, sDlgX, sDlgH, sDlgW);

                    string sTitle = " Select Theme File ";
                    int sTitleX = sDlgX + 3;
                    g_term->setAttr(borderAttr);
                    g_term->putCP437(sDlgY, sTitleX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
                    g_term->setAttr(titleAttr);
                    g_term->printStr(sDlgY, sTitleX + 1, sTitle);
                    g_term->setAttr(borderAttr);
                    g_term->putCP437(sDlgY, sTitleX + static_cast<int>(sTitle.size()) + 1,
                                     CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

                    for (int i = 0; i < visible && i + scrollOff < static_cast<int>(themeFiles.size()); ++i)
                    {
                        int idx = i + scrollOff;
                        int y = sDlgY + 1 + i;
                        bool isSel = (idx == cursor);

                        if (isSel)
                        {
                            fillRow(y, selAttr, sDlgX + 1, sDlgX + sDlgW - 1);
                        }
                        else
                        {
                            fillRow(y, tAttr(TC_BLACK, TC_BLACK, false),
                                    sDlgX + 1, sDlgX + sDlgW - 1);
                        }

                        // Show display name (strip prefix and .ini)
                        string displayName = themeFiles[idx];
                        if (displayName.size() > 4)
                        {
                            displayName = displayName.substr(0, displayName.size() - 4);
                        }
                        size_t prefixLen = items[selected].prefix.size();
                        if (displayName.size() > prefixLen)
                        {
                            displayName = displayName.substr(prefixLen);
                        }
                        printAt(y, sDlgX + 2, truncateStr(displayName, sDlgW - 4),
                                isSel ? selAttr : itemAttr);
                    }

                    // Scrollbar if content overflows
                    int totalThemeFiles = static_cast<int>(themeFiles.size());
                    if (totalThemeFiles > visible)
                    {
                        drawScrollbar(sDlgY + 1, visible, cursor, totalThemeFiles,
                                     tAttr(TC_BLACK, TC_BLACK, true),
                                     tAttr(TC_WHITE, TC_BLACK, true));
                    }

                    int sHelpY = sDlgY + sDlgH - 2;
                    g_term->setAttr(borderAttr);
                    g_term->putCP437(sHelpY, sDlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
                    g_term->drawHLine(sHelpY, sDlgX + 1, sDlgW - 2);
                    g_term->putCP437(sHelpY, sDlgX + sDlgW - 1,
                                     CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
                    string sHelp = "Enter=Select, ESC=Cancel";
                    int sHelpX = sDlgX + (sDlgW - static_cast<int>(sHelp.size())) / 2;
                    g_term->setAttr(titleAttr);
                    g_term->printStr(sHelpY, sHelpX, sHelp);

                    drawFooter("Select a theme file");
                    g_term->refresh();

                    int sCh = g_term->getKey();
                    switch (sCh)
                    {
                        case TK_UP:
                            if (cursor > 0) --cursor;
                            break;
                        case TK_DOWN:
                            if (cursor < static_cast<int>(themeFiles.size()) - 1) ++cursor;
                            break;
                        case TK_HOME:
                            cursor = 0;
                            break;
                        case TK_END:
                            cursor = static_cast<int>(themeFiles.size()) - 1;
                            break;
                        case TK_PGUP:
                            cursor -= visible;
                            if (cursor < 0) cursor = 0;
                            break;
                        case TK_PGDN:
                            cursor += visible;
                            if (cursor >= static_cast<int>(themeFiles.size()))
                                cursor = static_cast<int>(themeFiles.size()) - 1;
                            break;
                        case TK_ENTER:
                            *(items[selected].value) = themeFiles[cursor];
                            themeChanged = true;
                            goto themeSelDone;
                        case TK_ESCAPE:
                            goto themeSelDone;
                        default:
                            break;
                    }
                }
                themeSelDone:
                if (themeChanged)
                {
                    changed = true;
                }
                break;
            }
            case TK_ESCAPE:
            case 'q':
            case 'Q':
                return changed;
            default:
                break;
        }
    }
}

// ============================================================
// General settings screen
// ============================================================
static bool editGeneralSettings(Settings& settings)
{
    struct GenItem
    {
        string label;
        string* value;
        int maxLen;
    };
    vector<GenItem> items =
    {
        {"Your name (for replies)", &settings.userName, 25},
        {"Reply packet directory",  &settings.replyDir, 60},
    };

    int strItemCount = static_cast<int>(items.size());
    // Non-string items
    int itemCount = strItemCount + 5;
    int extEditorsListIdx = strItemCount;
    int useExtEditorIdx = strItemCount + 1;
    int selectEditorIdx = strItemCount + 2;
    int lightbarIdx = strItemCount + 3;
    int splashIdx = strItemCount + 4;
    int selected = 0;
    bool changed = false;

    int dlgW = 73;
    if (dlgW > g_term->getCols() - 4)
    {
        dlgW = g_term->getCols() - 4;
    }
    int dlgH = itemCount + 5;
    int dlgY = (g_term->getRows() - dlgH) / 2;
    int dlgX = (g_term->getCols() - dlgW) / 2;

    TermAttr borderAttr = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr titleAttr  = tAttr(TC_BLUE, TC_BLACK, true);
    TermAttr itemAttr   = tAttr(TC_CYAN, TC_BLACK, false);
    TermAttr selAttr    = tAttr(TC_BLUE, TC_WHITE, false);
    TermAttr valueAttr  = tAttr(TC_WHITE, TC_BLACK, true);

    for (;;)
    {
        for (int r = 0; r < g_term->getRows(); ++r)
        {
            fillRow(r, tAttr(TC_BLACK, TC_BLACK, false), 0, g_term->getCols());
        }
        drawHeader();

        g_term->setAttr(borderAttr);
        g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

        string titleText = " General Settings ";
        int titleX = dlgX + 3;
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, titleX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        g_term->setAttr(titleAttr);
        g_term->printStr(dlgY, titleX + 1, titleText);
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, titleX + static_cast<int>(titleText.size()) + 1,
                         CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

        for (int i = 0; i < itemCount; ++i)
        {
            int y = dlgY + 1 + i;
            bool isSel = (i == selected);

            if (isSel)
            {
                fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
            }
            else
            {
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
            }

            if (i < strItemCount)
            {
                string display = items[i].label + ": " +
                    (items[i].value->empty() ? "(not set)" : *(items[i].value));
                printAt(y, dlgX + 2, truncateStr(display, dlgW - 4),
                        isSel ? selAttr : itemAttr);
            }
            else if (i == extEditorsListIdx)
            {
                TermAttr lbl = isSel ? selAttr : itemAttr;
                printAt(y, dlgX + 2, "External Editors...", lbl);
            }
            else if (i == selectEditorIdx)
            {
                TermAttr lbl = isSel ? selAttr : itemAttr;
                string edName = settings.selectedEditor.empty() ? "(none)" : settings.selectedEditor;
                printAt(y, dlgX + 2, "External Editor: " + edName, lbl);
            }
            else
            {
                // Toggle items
                bool toggleVal = false;
                string toggleLabel;
                if (i == splashIdx)
                {
                    toggleLabel = "Show splash screen on startup";
                    toggleVal = settings.showSplashScreen;
                }
                else if (i == useExtEditorIdx)
                {
                    toggleLabel = "Use external editor";
                    toggleVal = settings.useExternalEditor;
                }
                else if (i == lightbarIdx)
                {
                    toggleLabel = "Use lightbar navigation in message list";
                    toggleVal = settings.lightbarMode;
                }
                TermAttr lbl = isSel ? selAttr : itemAttr;
                printAt(y, dlgX + 2, toggleLabel, lbl);
                TermAttr chk = isSel ? selAttr : tAttr(TC_GREEN, TC_BLACK, true);
                int checkCol = dlgX + dlgW - 8;
                if (toggleVal)
                {
                    printAt(y, checkCol, "[", chk);
                    g_term->setAttr(chk);
                    g_term->putCP437(y, checkCol + 1, CP437_CHECK_MARK);
                    printAt(y, checkCol + 2, "]", chk);
                }
                else
                {
                    printAt(y, checkCol, "[ ]", chk);
                }
            }
        }

        // Fill empty rows
        for (int r = dlgY + 1 + itemCount; r < dlgY + dlgH - 2; ++r)
        {
            fillRow(r, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
            g_term->setAttr(borderAttr);
            g_term->putCP437(r, dlgX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
            g_term->putCP437(r, dlgX + dlgW - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
        }

        int helpY = dlgY + dlgH - 2;
        g_term->setAttr(borderAttr);
        g_term->putCP437(helpY, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
        g_term->drawHLine(helpY, dlgX + 1, dlgW - 2);
        g_term->putCP437(helpY, dlgX + dlgW - 1,
                         CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        string helpText = "Enter=Edit value, ESC=Done";
        int helpX = dlgX + (dlgW - static_cast<int>(helpText.size())) / 2;
        g_term->setAttr(titleAttr);
        g_term->printStr(helpY, helpX, helpText);

        drawFooter("Enter to edit, ESC to go back");
        g_term->refresh();

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:
                if (selected > 0) --selected;
                break;
            case TK_DOWN:
                if (selected < itemCount - 1) ++selected;
                break;
            case TK_ENTER:
            case ' ':
            {
                if (selected == splashIdx)
                {
                    settings.showSplashScreen = !settings.showSplashScreen;
                    changed = true;
                }
                else if (selected == useExtEditorIdx)
                {
                    settings.useExternalEditor = !settings.useExternalEditor;
                    changed = true;
                }
                else if (selected == lightbarIdx)
                {
                    settings.lightbarMode = !settings.lightbarMode;
                    changed = true;
                }
                else if (selected == extEditorsListIdx)
                {
                    if (showExternalEditorsList(settings))
                        changed = true;
                }
                else if (selected == selectEditorIdx)
                {
                    // Cycle through configured editors (or none)
                    if (!settings.externalEditors.empty())
                    {
                        int curIdx = -1;
                        for (int j = 0; j < static_cast<int>(settings.externalEditors.size()); ++j)
                        {
                            if (settings.externalEditors[j].name == settings.selectedEditor)
                            { curIdx = j; break; }
                        }
                        if (curIdx >= 0)
                        {
                            if (curIdx + 1 < static_cast<int>(settings.externalEditors.size()))
                                settings.selectedEditor = settings.externalEditors[curIdx + 1].name;
                            else
                                settings.selectedEditor.clear();
                        }
                        else
                        {
                            settings.selectedEditor = settings.externalEditors[0].name;
                        }
                    }
                    else
                    {
                        settings.selectedEditor.clear();
                    }
                    changed = true;
                }
                else if (selected == 1) // Reply packet directory — use directory chooser
                {
                    string startDir = settings.replyDir;
                    if (startDir.empty())
                    {
                        startDir = getSlyMailDataDir() + PATH_SEP_STR + "REP";
                    }
                    string val = showDirChooser(startDir, "Select Reply Packet Directory");
                    if (!val.empty())
                    {
                        settings.replyDir = val;
                        changed = true;
                    }
                }
                else
                {
                    int y = dlgY + 1 + selected;
                    string val = getStringInput(y, dlgX + 2, items[selected].maxLen,
                        *(items[selected].value), valueAttr);
                    if (!val.empty())
                    {
                        *(items[selected].value) = val;
                        changed = true;
                    }
                }
                break;
            }
            case TK_ESCAPE:
            case 'q':
            case 'Q':
                return changed;
            default:
                break;
        }
    }
}

// ============================================================
// Main configuration menu
// ============================================================
static void showMainMenu(Settings& settings, const string& baseDir)
{
    int selected = 0;
    int numCategories = static_cast<int>(ConfigCategory::COUNT);

    for (;;)
    {
        g_term->clear();
        drawHeader();

        int cols = g_term->getCols();
        int rows = g_term->getRows();

        int dlgW = 50;
        if (dlgW > cols - 4)
        {
            dlgW = cols - 4;
        }
        int dlgH = numCategories + 4;  // top border + items + help separator + bottom border
        int dlgY = (rows - dlgH) / 2;
        int dlgX = (cols - dlgW) / 2;

        TermAttr borderAttr = tAttr(TC_GREEN, TC_BLACK, false);
        TermAttr titleAttr  = tAttr(TC_BLUE, TC_BLACK, true);
        TermAttr itemAttr   = tAttr(TC_CYAN, TC_BLACK, false);
        TermAttr selAttr    = tAttr(TC_BLUE, TC_WHITE, false);
        g_term->setAttr(borderAttr);
        g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

        // Title
        string titleText = " SlyMail Configuration ";
        int titleX = dlgX + 3;
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, titleX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        g_term->setAttr(titleAttr);
        g_term->printStr(dlgY, titleX + 1, titleText);
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, titleX + static_cast<int>(titleText.size()) + 1,
                         CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

        // Menu items
        for (int i = 0; i < numCategories; ++i)
        {
            int y = dlgY + 1 + i;
            bool isSel = (i == selected);

            if (isSel)
            {
                fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
            }
            else
            {
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
            }

            printAt(y, dlgX + 2, categoryNames[i], isSel ? selAttr : itemAttr);
        }

        // Bottom help
        int helpY = dlgY + dlgH - 2;
        g_term->setAttr(borderAttr);
        g_term->putCP437(helpY, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
        g_term->drawHLine(helpY, dlgX + 1, dlgW - 2);
        g_term->putCP437(helpY, dlgX + dlgW - 1,
                         CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        string helpText = "Enter=Open, S=Save, Q/ESC=Quit";
        int helpX = dlgX + (dlgW - static_cast<int>(helpText.size())) / 2;
        g_term->setAttr(titleAttr);
        g_term->printStr(helpY, helpX, helpText);

        // Settings file path info
        printAt(rows - 3, 2, "Settings file: " + Settings::getSettingsPath(),
                tAttr(TC_BLACK, TC_BLACK, true));

        drawFooter("Enter=Open category, S=Save settings, Q/ESC=Quit");
        g_term->refresh();

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:
                if (selected > 0) --selected;
                break;
            case TK_DOWN:
                if (selected < numCategories - 1) ++selected;
                break;
            case TK_HOME:
                selected = 0;
                break;
            case TK_END:
                selected = numCategories - 1;
                break;
            case TK_ENTER:
            case ' ':
            {
                ConfigCategory cat = static_cast<ConfigCategory>(selected);
                bool catChanged = false;
                switch (cat)
                {
                    case ConfigCategory::Editor:
                        catChanged = editEditorSettings(settings, baseDir);
                        break;
                    case ConfigCategory::Reader:
                        catChanged = editReaderSettings(settings);
                        break;
                    case ConfigCategory::Themes:
                        catChanged = editThemeSettings(settings, baseDir);
                        break;
                    case ConfigCategory::General:
                        catChanged = editGeneralSettings(settings);
                        break;
                    default:
                        break;
                }
                if (catChanged)
                {
                    settings.save();
                    messageDialog("Settings", "Settings saved to " + Settings::getSettingsPath());
                }
                break;
            }
            case 's':
            case 'S':
            {
                settings.save();
                messageDialog("Settings", "Settings saved to " + Settings::getSettingsPath());
                break;
            }
            case 'q':
            case 'Q':
            case TK_ESCAPE:
                return;
            default:
                break;
        }
    }
}

// ============================================================
// Main entry point
// ============================================================
int main(int argc, char* argv[])
{
    // Determine base directory (where the executable lives)
    // Theme files, dictionaries, and taglines are relative to this
    std::string baseDir = ".";
    if (argc > 0 && argv[0] != nullptr)
    {
        fs::path exePath = fs::path(argv[0]).parent_path();
        if (!exePath.empty())
        {
            baseDir = exePath.string();
        }
    }

    // Settings are stored in the SlyMail data directory (~/.slymail)
    settingsDir() = getSlyMailDataDir();

    // Initialize terminal
    auto terminal = createTerminal();
    g_term = terminal.get();
    g_term->init();

    // Load settings
    Settings settings;
    bool settingsExist = settings.load();

    // First-run: if no settings file exists, prompt for user name
    if (!settingsExist)
    {
        g_term->clear();
        int cols = g_term->getCols();
        int rows = g_term->getRows();

        int boxW = 60;
        if (boxW > cols - 4) boxW = cols - 4;
        int boxH = 8;
        int boxY = (rows - boxH) / 2;
        int boxX = (cols - boxW) / 2;

        TermAttr borderAttr = tAttr(TC_GREEN, TC_BLACK, false);
        TermAttr titleAttr  = tAttr(TC_BLUE, TC_BLACK, true);
        TermAttr textAttr   = tAttr(TC_CYAN, TC_BLACK, false);
        TermAttr inputAttr  = tAttr(TC_WHITE, TC_BLACK, true);

        drawBox(boxY, boxX, boxH, boxW, borderAttr, " Welcome ", titleAttr);

        printAt(boxY + 2, boxX + 3, "Welcome to SlyMail!", titleAttr);
        printAt(boxY + 3, boxX + 3, "Please enter the name you intend", textAttr);
        printAt(boxY + 4, boxX + 3, "to use when writing messages:", textAttr);

        g_term->refresh();

        string name = getStringInput(boxY + 5, boxX + 3, boxW - 6, "", inputAttr);
        if (!name.empty())
        {
            settings.userName = name;
            settings.save();
        }
    }

    // Run the configuration menu
    showMainMenu(settings, baseDir);

    // Shutdown terminal
    g_term->shutdown();

    return 0;
}
