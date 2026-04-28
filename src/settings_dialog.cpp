#include "settings_dialog.h"
#include "ui_common.h"
#include "file_dir_utils.h"
#include "file_browser.h"
#include "i18n.h"
#include <filesystem>

using std::string;
using std::vector;

namespace fs = std::filesystem;

// ============================================================
// UI Mode selection dialog
// Shows a small dialog with Ice / DCT / Random options.
// Returns the selected EditorStyle, or the current one on ESC.
// ============================================================
EditorStyle showUIModeDialog(EditorStyle currentStyle)
{
    // Map styles to display labels
    const char* labels[] = {"Ice", "DCT", "Random"};
    const int numChoices = 3;

    // Determine initial cursor from current style
    int cursor = 0;
    switch (currentStyle)
    {
        case EditorStyle::Ice:    cursor = 0; break;
        case EditorStyle::Dct:    cursor = 1; break;
        case EditorStyle::Random: cursor = 2; break;
    }

    int dlgW = 24;
    int dlgH = numChoices + 4; // top border + title-sep + items + bottom border
    int dlgY = (g_term->getRows() - dlgH) / 2;
    int dlgX = (g_term->getCols() - dlgW) / 2;

    TermAttr borderAttr = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr titleAttr  = tAttr(TC_BLUE, TC_BLACK, true);
    TermAttr itemAttr   = tAttr(TC_CYAN, TC_BLACK, false);
    TermAttr selAttr    = tAttr(TC_BLUE, TC_WHITE, false);

    bool needFullRedraw = true;
    int prevCursor = -1;

    // Lambda: draw a single item row
    auto drawItem = [&](int i)
    {
        int y = dlgY + 1 + i;
        bool isSel = (i == cursor);
        if (isSel)
            fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
        else
            fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
        printAt(y, dlgX + 2, labels[i], isSel ? selAttr : itemAttr);
    };

    for (;;)
    {
        if (needFullRedraw)
        {
            clearDialogArea(dlgY, dlgX, dlgH, dlgW);
            // Draw box
            g_term->setAttr(borderAttr);
            g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

            // Title in top border
            string title = _(" UI Mode ");
            int titleX = dlgX + (dlgW - static_cast<int>(title.size()) - 2) / 2;
            g_term->setAttr(borderAttr);
            g_term->putCP437(dlgY, titleX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
            g_term->setAttr(titleAttr);
            g_term->printStr(dlgY, titleX + 1, title);
            g_term->setAttr(borderAttr);
            g_term->putCP437(dlgY, titleX + static_cast<int>(title.size()) + 1,
                             CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

            // Draw all items
            for (int i = 0; i < numChoices; ++i)
            {
                drawItem(i);
            }

            // Fill any empty rows between items and help separator
            for (int r = dlgY + 1 + numChoices; r < dlgY + dlgH - 2; ++r)
            {
                fillRow(r, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
                g_term->setAttr(borderAttr);
                g_term->putCP437(r, dlgX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
                g_term->putCP437(r, dlgX + dlgW - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
            }

            // Bottom help line
            int helpY = dlgY + dlgH - 2;
            g_term->setAttr(borderAttr);
            g_term->putCP437(helpY, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
            g_term->drawHLine(helpY, dlgX + 1, dlgW - 2);
            g_term->putCP437(helpY, dlgX + dlgW - 1,
                             CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
            string helpText = _("Enter=OK, ESC=Cancel");
            int helpX = dlgX + (dlgW - static_cast<int>(helpText.size())) / 2;
            g_term->setAttr(titleAttr);
            g_term->printStr(helpY, helpX, helpText);

            needFullRedraw = false;
        }
        else if (cursor != prevCursor)
        {
            drawItem(prevCursor);
            drawItem(cursor);
        }

        g_term->refresh();
        prevCursor = cursor;

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:
                if (cursor > 0)
                {
                    --cursor;
                }
                break;
            case TK_DOWN:
                if (cursor < numChoices - 1)
                {
                    ++cursor;
                }
                break;
            case TK_HOME:
                cursor = 0;
                break;
            case TK_END:
                cursor = numChoices - 1;
                break;
            case TK_ENTER:
            {
                clearDialogArea(dlgY, dlgX, dlgH, dlgW);
                switch (cursor)
                {
                    case 0: return EditorStyle::Ice;
                    case 1: return EditorStyle::Dct;
                    case 2: return EditorStyle::Random;
                }
                return currentStyle;
            }
            case TK_ESCAPE:
                clearDialogArea(dlgY, dlgX, dlgH, dlgW);
                return currentStyle;
            case TK_RESIZE:
                needFullRedraw = true;
                break;
            default:
                break;
        }
    }
}

// Find available dictionary files in the dictionaries directory
// Checks ~/.slymail/dictionary_files first, then baseDir/dictionary_files
vector<string> findDictionaries(const string& baseDir)
{
    vector<string> dicts;
    string dictDir = getSlyMailDataDir() + PATH_SEP_STR + "dictionary_files";
    if (!fs::is_directory(dictDir))
        dictDir = baseDir + PATH_SEP_STR + "dictionary_files";
    try
    {
        for (auto& entry : fs::directory_iterator(dictDir))
        {
            if (entry.is_regular_file())
            {
                string name = entry.path().filename().string();
                if (name.size() > 4 && name.substr(name.size() - 4) == ".txt")
                {
                    dicts.push_back(name);
                }
            }
        }
    }
    catch (const fs::filesystem_error&)
    {
        // Dictionary directory not found or not accessible — return empty list
    }
    catch (...)
    {
    }
    std::sort(dicts.begin(), dicts.end());
    return dicts;
}

// Find available theme files in the config directory
vector<string> findThemeFiles(const string& baseDir,
                              const string& prefix)
{
    vector<string> files;
    string configDir = getSlyMailDataDir() + PATH_SEP_STR + "config_files";
    if (!fs::is_directory(configDir))
        configDir = baseDir + PATH_SEP_STR + "config_files";
    try
    {
        for (auto& entry : fs::directory_iterator(configDir))
        {
            if (entry.is_regular_file())
            {
                string name = entry.path().filename().string();
                if (name.size() > prefix.size() + 4
                    && name.substr(0, prefix.size()) == prefix
                    && name.substr(name.size() - 4) == ".ini")
                {
                    files.push_back(name);
                }
            }
        }
    }
    catch (const fs::filesystem_error&)
    {
        // Theme directory not found or not accessible — return empty list
    }
    catch (...)
    {
    }
    std::sort(files.begin(), files.end());
    return files;
}

// Show theme file selection dialog
// Returns selected filename, or empty string on cancel
string showThemeSelector(const string& baseDir, EditorStyle currentStyle)
{
    vector<string> themes;

    if (currentStyle == EditorStyle::Random)
    {
        // Show all theme files
        auto iceThemes = findThemeFiles(baseDir, "EditorIceColors_");
        auto dctThemes = findThemeFiles(baseDir, "EditorDCTColors_");
        themes.insert(themes.end(), iceThemes.begin(), iceThemes.end());
        themes.insert(themes.end(), dctThemes.begin(), dctThemes.end());
        std::sort(themes.begin(), themes.end());
    }
    else if (currentStyle == EditorStyle::Ice)
    {
        themes = findThemeFiles(baseDir, "EditorIceColors_");
    }
    else
    {
        themes = findThemeFiles(baseDir, "EditorDCTColors_");
    }

    if (themes.empty())
    {
        messageDialog(_("Themes"), _("No theme files found in config/"));
        return "";
    }

    int cursor = 0;
    int dlgW = 60;
    if (dlgW > g_term->getCols() - 4)
    {
        dlgW = g_term->getCols() - 4;
    }
    int dlgH = static_cast<int>(themes.size()) + 5;
    if (dlgH > g_term->getRows() - 4)
    {
        dlgH = g_term->getRows() - 4;
    }
    int dlgY = (g_term->getRows() - dlgH) / 2;
    int dlgX = (g_term->getCols() - dlgW) / 2;

    TermAttr borderAttr = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr titleAttr  = tAttr(TC_BLUE, TC_BLACK, true);
    TermAttr itemAttr   = tAttr(TC_CYAN, TC_BLACK, false);
    TermAttr selAttr    = tAttr(TC_BLUE, TC_WHITE, false);

    int visibleItems = dlgH - 5;
    int scrollOffset = 0;

    bool needFullRedraw  = true;
    int prevCursor       = -1;
    int prevScrollOffset = -1;

    // Helper: display name for a theme entry
    auto themeDisplayName = [&](int idx) -> string
    {
        string dn = themes[idx];
        if (dn.size() > 4)
            dn = dn.substr(0, dn.size() - 4);
        if (dn.size() >= 16 && dn.substr(0, 16) == "EditorIceColors_")
            dn = "Ice: " + dn.substr(16);
        else if (dn.size() >= 16 && dn.substr(0, 16) == "EditorDCTColors_")
            dn = "DCT: " + dn.substr(16);
        return dn;
    };

    // Lambda: draw a single theme row
    auto drawThemeRow = [&](int idx)
    {
        if (idx < scrollOffset || idx >= scrollOffset + visibleItems) return;
        if (idx < 0 || idx >= static_cast<int>(themes.size())) return;
        int y = dlgY + 3 + (idx - scrollOffset);
        bool isSel = (idx == cursor);
        if (isSel)
            fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
        else
            fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
        printAt(y, dlgX + 2, truncateStr(themeDisplayName(idx), dlgW - 4),
                isSel ? selAttr : itemAttr);
    };

    // Lambda: draw scrollbar
    auto drawThemeSB = [&]()
    {
        int totalThemes = static_cast<int>(themes.size());
        if (totalThemes > visibleItems)
        {
            drawScrollbar(dlgY + 3, visibleItems, cursor, totalThemes,
                         tAttr(TC_BLACK, TC_BLACK, true),
                         tAttr(TC_WHITE, TC_BLACK, true));
        }
    };

    for (;;)
    {
        // Clamp scroll
        if (cursor < scrollOffset)
            scrollOffset = cursor;
        if (cursor >= scrollOffset + visibleItems)
            scrollOffset = cursor - visibleItems + 1;

        bool scrollChanged = (scrollOffset != prevScrollOffset);

        if (needFullRedraw || scrollChanged)
        {
            clearDialogArea(dlgY, dlgX, dlgH, dlgW);
            // Draw box
            g_term->setAttr(borderAttr);
            g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

            // Title in top border
            string title = _(" Select Theme ");
            int titleX = dlgX + (dlgW - static_cast<int>(title.size()) - 2) / 2;
            g_term->setAttr(borderAttr);
            g_term->putCP437(dlgY, titleX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
            g_term->setAttr(titleAttr);
            g_term->printStr(dlgY, titleX + 1, title);
            g_term->setAttr(borderAttr);
            g_term->putCP437(dlgY, titleX + static_cast<int>(title.size()) + 1,
                             CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

            // Header
            int headerY = dlgY + 1;
            g_term->setAttr(titleAttr);
            g_term->printStr(headerY, dlgX + 2, _("Theme File"));

            // Separator
            g_term->setAttr(borderAttr);
            g_term->putCP437(headerY + 1, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
            g_term->drawHLine(headerY + 1, dlgX + 1, dlgW - 2);
            g_term->putCP437(headerY + 1, dlgX + dlgW - 1,
                             CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);

            // All visible items
            for (int i = 0; i < visibleItems && i + scrollOffset < static_cast<int>(themes.size()); ++i)
            {
                drawThemeRow(scrollOffset + i);
            }

            drawThemeSB();

            // Bottom help
            int helpY = dlgY + dlgH - 2;
            g_term->setAttr(borderAttr);
            g_term->putCP437(helpY, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
            g_term->drawHLine(helpY, dlgX + 1, dlgW - 2);
            g_term->putCP437(helpY, dlgX + dlgW - 1,
                             CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
            string helpText = _("Up, Dn, Enter=Select, ESC/Q=Cancel");
            int helpX = dlgX + (dlgW - static_cast<int>(helpText.size())) / 2;
            g_term->setAttr(titleAttr);
            g_term->printStr(helpY, helpX, helpText);

            needFullRedraw = false;
        }
        else if (cursor != prevCursor)
        {
            drawThemeRow(prevCursor);
            drawThemeRow(cursor);
            drawThemeSB();
        }

        g_term->refresh();
        prevCursor       = cursor;
        prevScrollOffset = scrollOffset;

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:
                if (cursor > 0)
                {
                    --cursor;
                }
                break;
            case TK_DOWN:
                if (cursor < static_cast<int>(themes.size()) - 1)
                {
                    ++cursor;
                }
                break;
            case TK_HOME:
                cursor = 0;
                break;
            case TK_END:
                cursor = static_cast<int>(themes.size()) - 1;
                break;
            case TK_ENTER:
                clearDialogArea(dlgY, dlgX, dlgH, dlgW);
                return themes[cursor];
            case TK_ESCAPE:
            case 'q':
            case 'Q':
                clearDialogArea(dlgY, dlgX, dlgH, dlgW);
                return "";
            case TK_RESIZE:
                needFullRedraw = true;
                break;
            default:
                break;
        }
    }
}

// Show dictionary selection sub-dialog
// Returns comma-separated list of selected dictionaries
string showDictionarySelector(const string& baseDir,
                              const string& currentSelection)
{
    vector<string> dicts = findDictionaries(baseDir);
    if (dicts.empty())
    {
        messageDialog(_("Dictionaries"), _("No dictionary files found in dictionary_files/"));
        return currentSelection;
    }

    // Parse current selection into a set
    vector<bool> selected(dicts.size(), false);
    for (size_t i = 0; i < dicts.size(); ++i)
    {
        if (currentSelection.find(dicts[i]) != string::npos)
        {
            selected[i] = true;
        }
    }

    int cursor = 0;
    int dlgW = 60;
    if (dlgW > g_term->getCols() - 4)
    {
        dlgW = g_term->getCols() - 4;
    }
    int dlgH = static_cast<int>(dicts.size()) + 5;
    if (dlgH > g_term->getRows() - 4)
    {
        dlgH = g_term->getRows() - 4;
    }
    int dlgY = (g_term->getRows() - dlgH) / 2;
    int dlgX = (g_term->getCols() - dlgW) / 2;

    // Colors matching SlyEdit's ChoiceScrollbox
    TermAttr borderAttr = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr titleAttr  = tAttr(TC_BLUE, TC_BLACK, true);
    TermAttr itemAttr   = tAttr(TC_CYAN, TC_BLACK, false);
    TermAttr selAttr    = tAttr(TC_BLUE, TC_WHITE, false);
    TermAttr checkAttr  = tAttr(TC_GREEN, TC_BLACK, true);
    TermAttr selCheckAt = tAttr(TC_WHITE, TC_BLUE, true);

    int visibleItems = dlgH - 5;
    int scrollOffset = 0;
    int checkCol     = dlgX + dlgW - 8;

    bool needFullRedraw  = true;
    int prevCursor       = -1;
    int prevScrollOffset = -1;

    // Helper: display name for a dictionary entry
    auto dictDisplayName = [&](int idx) -> string
    {
        string dn = dicts[idx];
        if (dn.size() > 4) dn = dn.substr(0, dn.size() - 4);
        if (dn.size() >= 11 && dn.substr(0, 11) == "dictionary_")
            dn = dn.substr(11);
        return dn;
    };

    // Lambda: draw a single dictionary row
    auto drawDictRow = [&](int idx)
    {
        if (idx < scrollOffset || idx >= scrollOffset + visibleItems) return;
        if (idx < 0 || idx >= static_cast<int>(dicts.size())) return;
        int y = dlgY + 3 + (idx - scrollOffset);
        bool isSel = (idx == cursor);
        if (isSel)
            fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
        else
            fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
        TermAttr lbl = isSel ? selAttr : itemAttr;
        TermAttr chk = isSel ? selCheckAt : checkAttr;
        printAt(y, dlgX + 2, truncateStr(dictDisplayName(idx), dlgW - 16), lbl);
        if (selected[idx])
        {
            g_term->setAttr(chk);
            g_term->printStr(y, checkCol, "[");
            g_term->putCP437(y, checkCol + 1, CP437_CHECK_MARK);
            g_term->printStr(y, checkCol + 2, "]");
        }
        else
        {
            printAt(y, checkCol, "[ ]", chk);
        }
    };

    // Lambda: draw scrollbar
    auto drawDictSB = [&]()
    {
        int totalDicts = static_cast<int>(dicts.size());
        if (totalDicts > visibleItems)
        {
            drawScrollbar(dlgY + 3, visibleItems, cursor, totalDicts,
                         tAttr(TC_BLACK, TC_BLACK, true),
                         tAttr(TC_WHITE, TC_BLACK, true));
        }
    };

    for (;;)
    {
        // Clamp scroll
        if (cursor < scrollOffset)
            scrollOffset = cursor;
        if (cursor >= scrollOffset + visibleItems)
            scrollOffset = cursor - visibleItems + 1;

        bool scrollChanged = (scrollOffset != prevScrollOffset);

        if (needFullRedraw || scrollChanged)
        {
            clearDialogArea(dlgY, dlgX, dlgH, dlgW);
            // Draw box
            g_term->setAttr(borderAttr);
            g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

            // Title in top border
            string title = _(" Select Dictionaries ");
            int titleX = dlgX + (dlgW - static_cast<int>(title.size()) - 2) / 2;
            g_term->setAttr(borderAttr);
            g_term->putCP437(dlgY, titleX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
            g_term->setAttr(titleAttr);
            g_term->printStr(dlgY, titleX + 1, title);
            g_term->setAttr(borderAttr);
            g_term->putCP437(dlgY, titleX + static_cast<int>(title.size()) + 1,
                             CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

            // Header
            int headerY = dlgY + 1;
            g_term->setAttr(titleAttr);
            g_term->printStr(headerY, dlgX + 2, _("Dictionary"));
            g_term->printStr(headerY, dlgX + dlgW - 12, _("Selected"));

            // Separator
            g_term->setAttr(borderAttr);
            g_term->putCP437(headerY + 1, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
            g_term->drawHLine(headerY + 1, dlgX + 1, dlgW - 2);
            g_term->putCP437(headerY + 1, dlgX + dlgW - 1,
                             CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);

            // All visible items
            for (int i = 0; i < visibleItems && i + scrollOffset < static_cast<int>(dicts.size()); ++i)
            {
                drawDictRow(scrollOffset + i);
            }

            drawDictSB();

            // Bottom help
            int helpY = dlgY + dlgH - 2;
            g_term->setAttr(borderAttr);
            g_term->putCP437(helpY, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
            g_term->drawHLine(helpY, dlgX + 1, dlgW - 2);
            g_term->putCP437(helpY, dlgX + dlgW - 1,
                             CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
            string helpText = _("Up, Dn, Enter/Space=Toggle, ESC/Q=Done");
            int helpX = dlgX + (dlgW - static_cast<int>(helpText.size())) / 2;
            g_term->setAttr(titleAttr);
            g_term->printStr(helpY, helpX, helpText);

            needFullRedraw = false;
        }
        else if (cursor != prevCursor)
        {
            drawDictRow(prevCursor);
            drawDictRow(cursor);
            drawDictSB();
        }

        g_term->refresh();
        prevCursor       = cursor;
        prevScrollOffset = scrollOffset;

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:
                if (cursor > 0)
                {
                    --cursor;
                }
                break;
            case TK_DOWN:
                if (cursor < static_cast<int>(dicts.size()) - 1)
                {
                    ++cursor;
                }
                break;
            case TK_HOME:
                cursor = 0;
                break;
            case TK_END:
                cursor = static_cast<int>(dicts.size()) - 1;
                break;
            case TK_ENTER:
            case ' ':
                selected[cursor] = !selected[cursor];
                // Redraw just the toggled row (checkbox state changed)
                drawDictRow(cursor);
                g_term->refresh();
                break;
            case TK_ESCAPE:
            case 'q':
            case 'Q':
            {
                // Build comma-separated result
                string result;
                for (size_t i = 0; i < dicts.size(); ++i)
                {
                    if (selected[i])
                    {
                        if (!result.empty())
                        {
                            result += ",";
                        }
                        result += dicts[i];
                    }
                }
                clearDialogArea(dlgY, dlgX, dlgH, dlgW);
                return result;
            }
            case TK_RESIZE:
                needFullRedraw = true;
                break;
            default:
                break;
        }
    }
}

// ============================================================
// showLanguageSelector - language picker dialog
// Returns the chosen language code, or currentCode on ESC.
// ============================================================
string showLanguageSelector(const string& currentCode)
{
    vector<LangEntry> langs = i18n_supported_languages();
    int totalItems = static_cast<int>(langs.size());

    // Find initial cursor position
    int cursor = 0;
    for (int i = 0; i < totalItems; ++i)
    {
        if (langs[i].code == currentCode)
        {
            cursor = i;
            break;
        }
    }

    int dlgW = 50;
    if (dlgW > g_term->getCols() - 4)
        dlgW = g_term->getCols() - 4;
    int visibleItems = totalItems;
    if (visibleItems > g_term->getRows() - 8)
        visibleItems = g_term->getRows() - 8;
    int dlgH = visibleItems + 4;
    int dlgY = (g_term->getRows() - dlgH) / 2;
    int dlgX = (g_term->getCols() - dlgW) / 2;

    TermAttr borderAttr = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr titleAttr  = tAttr(TC_BLUE, TC_BLACK, true);
    TermAttr itemAttr   = tAttr(TC_CYAN, TC_BLACK, false);
    TermAttr selAttr    = tAttr(TC_BLUE, TC_WHITE, false);

    int scrollOffset  = 0;
    bool needFullRedraw = true;
    int prevCursor    = -1;
    int prevScrollOff = -1;

    auto drawRow = [&](int idx)
    {
        if (idx < scrollOffset || idx >= scrollOffset + visibleItems) return;
        if (idx < 0 || idx >= totalItems) return;
        int y = dlgY + 1 + (idx - scrollOffset);
        bool isSel = (idx == cursor);
        if (isSel)
            fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
        else
            fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
        string display = langs[idx].displayName;
        printAt(y, dlgX + 2, truncateStr(display, dlgW - 4), isSel ? selAttr : itemAttr);
    };

    for (;;)
    {
        if (cursor < scrollOffset)
            scrollOffset = cursor;
        if (cursor >= scrollOffset + visibleItems)
            scrollOffset = cursor - visibleItems + 1;

        bool scrollChanged = (scrollOffset != prevScrollOff);

        if (needFullRedraw || scrollChanged)
        {
            clearDialogArea(dlgY, dlgX, dlgH, dlgW);
            g_term->setAttr(borderAttr);
            g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

            string title = _(" Select Language ");
            int titleX = dlgX + (dlgW - static_cast<int>(title.size()) - 2) / 2;
            g_term->setAttr(borderAttr);
            g_term->putCP437(dlgY, titleX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
            g_term->setAttr(titleAttr);
            g_term->printStr(dlgY, titleX + 1, title);
            g_term->setAttr(borderAttr);
            g_term->putCP437(dlgY, titleX + static_cast<int>(title.size()) + 1,
                             CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

            for (int i = 0; i < visibleItems && i + scrollOffset < totalItems; ++i)
                drawRow(scrollOffset + i);

            if (totalItems > visibleItems)
            {
                drawScrollbar(dlgY + 1, visibleItems, cursor, totalItems,
                             tAttr(TC_BLACK, TC_BLACK, true),
                             tAttr(TC_WHITE, TC_BLACK, true));
            }

            int helpY = dlgY + dlgH - 2;
            g_term->setAttr(borderAttr);
            g_term->putCP437(helpY, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
            g_term->drawHLine(helpY, dlgX + 1, dlgW - 2);
            g_term->putCP437(helpY, dlgX + dlgW - 1,
                             CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
            string helpText = _("Up, Dn, Enter=Select, ESC=Cancel");
            int helpX = dlgX + (dlgW - static_cast<int>(helpText.size())) / 2;
            g_term->setAttr(titleAttr);
            g_term->printStr(helpY, helpX, helpText);

            needFullRedraw = false;
        }
        else if (cursor != prevCursor)
        {
            drawRow(prevCursor);
            drawRow(cursor);
        }

        g_term->refresh();
        prevCursor    = cursor;
        prevScrollOff = scrollOffset;

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:
                if (cursor > 0) --cursor;
                break;
            case TK_DOWN:
                if (cursor < totalItems - 1) ++cursor;
                break;
            case TK_HOME:
                cursor = 0;
                break;
            case TK_END:
                cursor = totalItems - 1;
                break;
            case TK_PGUP:
                cursor -= visibleItems;
                if (cursor < 0) cursor = 0;
                break;
            case TK_PGDN:
                cursor += visibleItems;
                if (cursor >= totalItems) cursor = totalItems - 1;
                break;
            case TK_ENTER:
            case ' ':
                clearDialogArea(dlgY, dlgX, dlgH, dlgW);
                return langs[cursor].code;
            case TK_ESCAPE:
            case 'q':
            case 'Q':
                clearDialogArea(dlgY, dlgX, dlgH, dlgW);
                return currentCode;
            case TK_RESIZE:
                needFullRedraw = true;
                break;
            default:
                break;
        }
    }
}

// ============================================================
// showEditorSettings - SlyEdit-style user settings dialog
// Called from the editor when Ctrl-U is pressed.
// Matches the exact layout from SlyEdit4_Ice_UserSettings.png
// ============================================================
bool showEditorSettings(Settings& settings, const string& baseDir)
{
    vector<EditorSettingItem> items =
    {
        {_("Choose UI mode"),                        SettingType::Choice,  ESET_UI_MODE},
        {_("Taglines"),                              SettingType::Toggle,  ESET_TAGLINES},
        {_("Spell-check dictionary/dictionaries"),   SettingType::SubMenu, ESET_SPELL_DICT},
        {_("Prompt for spell checker on save"),      SettingType::Toggle,  ESET_PROMPT_SPELL},
        {_("Wrap quote lines to terminal width"),    SettingType::Toggle,  ESET_WRAP_QUOTES},
        {_("Quote with author's initials"),          SettingType::Toggle,  ESET_QUOTE_INITIALS},
        {_("Indent quote lines containing initials"), SettingType::Toggle, ESET_INDENT_INITIALS},
        {_("Trim spaces from quote lines"),          SettingType::Toggle,  ESET_TRIM_QUOTE_SPACES},
        {_("Language"),                              SettingType::Choice,  ESET_LANGUAGE},
    };

    int itemCount = static_cast<int>(items.size());
    int selected = 0;
    bool changed = false;

    // Dialog dimensions matching SlyEdit's ChoiceScrollbox
    int dlgW = 73;
    if (dlgW > g_term->getCols() - 4)
    {
        dlgW = g_term->getCols() - 4;
    }
    int dlgH = itemCount + 5;  // header + separator + items + separator + help
    int dlgY = (g_term->getRows() - dlgH) / 2;
    int dlgX = (g_term->getCols() - dlgW) / 2;

    // Colors matching SlyEdit ChoiceScrollbox (from choice_scroll_box.js)
    TermAttr borderAttr  = tAttr(TC_GREEN, TC_BLACK, false);   // listBoxBorder
    TermAttr titleAttr   = tAttr(TC_BLUE, TC_BLACK, true);     // listBoxBorderText
    TermAttr itemAttr    = tAttr(TC_CYAN, TC_BLACK, false);    // listBoxItemText
    TermAttr selAttr     = tAttr(TC_BLUE, TC_WHITE, false);     // listBoxItemHighlight
    TermAttr checkAttr   = tAttr(TC_GREEN, TC_BLACK, true);

    // Checkmark column position (matching SlyEdit's format: "%-46s [ ]")
    int checkCol = dlgX + 48;

    bool needFullRedraw = true;
    int prevSelected2   = -1;  // separate from outer 'selected' to avoid name clash

    // Lambda: draw a single editor-settings row
    auto drawESetRow = [&](int i)
    {
        if (i < 0 || i >= itemCount) return;
        int y = dlgY + 1 + i;
        bool isSel = (i == selected);
        if (isSel)
            fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
        else
            fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
        TermAttr lbl = isSel ? selAttr : itemAttr;
        printAt(y, dlgX + 2, truncateStr(items[i].label, 46), lbl);
        if (items[i].type == SettingType::Toggle)
        {
            bool val = false;
            switch (items[i].id)
            {
                case ESET_TAGLINES:          val = settings.taglines; break;
                case ESET_PROMPT_SPELL:      val = settings.promptSpellCheck; break;
                case ESET_WRAP_QUOTES:       val = settings.wrapQuoteLines; break;
                case ESET_QUOTE_INITIALS:    val = settings.quoteWithInitials; break;
                case ESET_INDENT_INITIALS:   val = settings.indentQuoteInitials; break;
                case ESET_TRIM_QUOTE_SPACES: val = settings.trimQuoteSpaces; break;
                default: break;
            }
            TermAttr chk = isSel ? selAttr : checkAttr;
            if (val)
            {
                g_term->setAttr(chk);
                g_term->printStr(y, checkCol, "[");
                g_term->putCP437(y, checkCol + 1, CP437_CHECK_MARK);
                g_term->printStr(y, checkCol + 2, "]");
            }
            else
            {
                printAt(y, checkCol, "[ ]", chk);
            }
        }
    };

    for (;;)
    {
        if (needFullRedraw)
        {
        // Erase the exact dialog footprint so no background content bleeds
        // through, without wiping the full screen behind the dialog.
        clearDialogArea(dlgY, dlgX, dlgH, dlgW);

        // Draw outer box
        g_term->setAttr(borderAttr);
        g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

        // Title bar: +--|Setting|---------------------- Enabled - Page 1 of 1 --+
        // T-characters around "Setting" text in top border
        string settingTitle = _(" Setting ");
        int settingTitleX = dlgX + 3;
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, settingTitleX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        g_term->setAttr(titleAttr);
        g_term->printStr(dlgY, settingTitleX + 1, settingTitle);
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, settingTitleX + static_cast<int>(settingTitle.size()) + 1,
                         CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

        // "Enabled" and "Page 1 of 1" in top border
        string enabledText = _(" Enabled ");
        int enabledX = dlgX + dlgW - 24;
        g_term->setAttr(titleAttr);
        g_term->printStr(dlgY, enabledX, enabledText);

        string pageText = _(" Page 1 of 1 ");
        g_term->printStr(dlgY, dlgX + dlgW - static_cast<int>(pageText.size()) - 1, pageText);

        // Draw all items
        for (int i = 0; i < itemCount; ++i)
        {
            drawESetRow(i);
        }

        // Bottom separator with help text
        int helpY = dlgY + dlgH - 2;
        g_term->setAttr(borderAttr);
        g_term->putCP437(helpY, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
        g_term->drawHLine(helpY, dlgX + 1, dlgW - 2);
        g_term->putCP437(helpY, dlgX + dlgW - 1,
                         CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);

        // Help text with T-characters around it
        string helpText = _("Up, Dn, Enter=Select/toggle, ?=Help, ESC/Q/Ctrl-U=Close");
        int helpTextX = dlgX + 2;
        g_term->setAttr(borderAttr);
        g_term->putCP437(helpY, helpTextX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        g_term->setAttr(titleAttr);
        g_term->printStr(helpY, helpTextX + 1, helpText);
        g_term->setAttr(borderAttr);
        g_term->putCP437(helpY, helpTextX + static_cast<int>(helpText.size()) + 1,
                         CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

            needFullRedraw = false;
        }
        else if (selected != prevSelected2)
        {
            drawESetRow(prevSelected2);
            drawESetRow(selected);
        }

        g_term->refresh();
        prevSelected2 = selected;

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:
                if (selected > 0)
                {
                    --selected;
                }
                break;
            case TK_DOWN:
                if (selected < itemCount - 1)
                {
                    ++selected;
                }
                break;
            case TK_HOME:
                selected = 0;
                break;
            case TK_END:
                selected = itemCount - 1;
                break;
            case TK_PGUP:
                selected = 0;
                break;
            case TK_PGDN:
                selected = itemCount - 1;
                break;
            case TK_ENTER:
            case ' ':
            {
                switch (items[selected].id)
                {
                    case ESET_UI_MODE:
                    {
                        EditorStyle newStyle = showUIModeDialog(settings.editorStyle);
                        if (newStyle != settings.editorStyle)
                        {
                            settings.editorStyle = newStyle;
                            changed = true;
                        }
                        needFullRedraw = true;
                        break;
                    }
                    case ESET_TAGLINES:
                        settings.taglines = !settings.taglines;
                        changed = true;
                        drawESetRow(selected);
                        g_term->refresh();
                        break;
                    case ESET_SPELL_DICT:
                    {
                        string result = showDictionarySelector(
                            baseDir, settings.spellDictionaries);
                        if (result != settings.spellDictionaries)
                        {
                            settings.spellDictionaries = result;
                            changed = true;
                        }
                        needFullRedraw = true;
                        break;
                    }
                    case ESET_PROMPT_SPELL:
                        settings.promptSpellCheck = !settings.promptSpellCheck;
                        changed = true;
                        drawESetRow(selected);
                        g_term->refresh();
                        break;
                    case ESET_WRAP_QUOTES:
                        settings.wrapQuoteLines = !settings.wrapQuoteLines;
                        changed = true;
                        drawESetRow(selected);
                        g_term->refresh();
                        break;
                    case ESET_QUOTE_INITIALS:
                        settings.quoteWithInitials = !settings.quoteWithInitials;
                        changed = true;
                        drawESetRow(selected);
                        g_term->refresh();
                        break;
                    case ESET_INDENT_INITIALS:
                        settings.indentQuoteInitials = !settings.indentQuoteInitials;
                        changed = true;
                        drawESetRow(selected);
                        g_term->refresh();
                        break;
                    case ESET_TRIM_QUOTE_SPACES:
                        settings.trimQuoteSpaces = !settings.trimQuoteSpaces;
                        changed = true;
                        drawESetRow(selected);
                        g_term->refresh();
                        break;
                    case ESET_LANGUAGE:
                    {
                        string newCode = showLanguageSelector(settings.language);
                        if (newCode != settings.language)
                        {
                            settings.language = newCode;
                            changed = true;
                            messageDialog(_("Language"), _("Restart required for language change to take effect."));
                        }
                        needFullRedraw = true;
                        break;
                    }
                }
                break;
            }
            case TK_ESCAPE:
            case 'q':
            case 'Q':
            case TK_CTRL_U:
                if (changed)
                {
                    settings.save();
                }
                return changed;
            case TK_RESIZE:
                needFullRedraw = true;
                break;
            default:
                break;
        }
    }
}

// ============================================================
// showSettingsDialog - Reader settings dialog
// Shows only settings relevant to reading/listing messages.
// Editor-only settings (UI mode, insert mode, quote width, quote prefix,
// taglines, spell check, dictionaries, quote initials, trim quote spaces,
// user name) are NOT shown here - they belong in the editor's Ctrl-U dialog.
// ============================================================
bool showSettingsDialog(Settings& settings, const string& baseDir)
{
    (void)baseDir;  // Not needed for reader settings
    int selected = 0;
    bool changed = false;

    // Dialog layout (fixed for this terminal size)
    int dlgW = 73;
    if (dlgW > g_term->getCols() - 4) dlgW = g_term->getCols() - 4;

    int visibleItems = g_term->getRows() - 10;
    const int itemCount = 14;
    if (visibleItems > itemCount) visibleItems = itemCount;
    int dlgH = visibleItems + 5;
    int dlgY = (g_term->getRows() - dlgH) / 2;
    int dlgX = (g_term->getCols() - dlgW) / 2;
    int checkCol = dlgX + 48;

    static int scrollOffset = 0;

    TermAttr borderAttr = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr titleAttr  = tAttr(TC_BLUE, TC_BLACK, true);
    TermAttr itemAttr   = tAttr(TC_CYAN, TC_BLACK, false);
    TermAttr checkAttr  = tAttr(TC_GREEN, TC_BLACK, true);
    TermAttr selAttr    = tAttr(TC_BLUE, TC_WHITE, false);

    bool needFullRedraw  = true;
    int prevSelected     = -1;
    int prevScrollOffset = -1;

    while (true)
    {
        // Build items fresh each iteration (values reflect current settings)
        vector<SettingItem> items;
        items.push_back({_("Show kludge lines"),
                         settings.showKludgeLines ? "Y" : "N", true, false, SET_SHOW_KLUDGE});
        items.push_back({_("Show tear/origin lines"),
                         settings.showTearLine ? "Y" : "N", true, false, SET_SHOW_TEAR});
        items.push_back({_("Scrollbar in reader"),
                         settings.useScrollbar ? "Y" : "N", true, false, SET_USE_SCROLLBAR});
        items.push_back({_("Only show areas with new mail"),
                         settings.onlyShowAreasWithNewMail ? "Y" : "N", true, false, SET_ONLY_AREAS_WITH_MAIL});
        items.push_back({_("Strip ANSI codes from messages"),
                         settings.stripAnsi ? "Y" : "N", true, false, SET_STRIP_ANSI});
        items.push_back({_("Attribute code toggles..."),
                         "", false, true, SET_ATTR_CODE_TOGGLES});
        items.push_back({_("Search using regular expression"),
                         settings.useRegexSearch ? "Y" : "N", true, false, SET_REGEX_SEARCH});
        items.push_back({_("List messages in reversed"),
                         settings.reverseOrder ? "Y" : "N", true, false, SET_REVERSE_ORDER});
        items.push_back({_("Show splash screen on startup"),
                         settings.showSplashScreen ? "Y" : "N", true, false, SET_SPLASH_SCREEN});
        items.push_back({_("Reply packet directory"),
                         settings.replyDir.empty() ? _("(current dir)") : settings.replyDir,
                         false, false, SET_REPLY_DIR});
        items.push_back({_("Use external editor"),
                         settings.useExternalEditor ? "Y" : "N", true, false, SET_USE_EXTERNAL_EDITOR});
        items.push_back({_("External Editors..."),
                         "", false, true, SET_EXTERNAL_EDITORS_LIST});
        items.push_back({_("External Editor"),
                         settings.selectedEditor.empty() ? _("(none)") : settings.selectedEditor,
                         false, false, SET_SELECT_EXTERNAL_EDITOR});
        {
            // Determine language display name
            string langDisplay;
            if (settings.language.empty())
            {
                langDisplay = _("OS Default");
            }
            else
            {
                langDisplay = settings.language; // fallback
                for (const auto& le : i18n_supported_languages())
                {
                    if (le.code == settings.language)
                    {
                        langDisplay = le.displayName;
                        break;
                    }
                }
            }
            items.push_back({_("Language"),
                             langDisplay,
                             false, false, SET_LANGUAGE});
        }

        // Clamp scroll
        if (selected < scrollOffset) scrollOffset = selected;
        if (selected >= scrollOffset + visibleItems) scrollOffset = selected - visibleItems + 1;

        // Lambda: draw a single row
        auto drawSSDRow = [&](int idx)
        {
            if (idx < scrollOffset || idx >= scrollOffset + visibleItems) return;
            if (idx < 0 || idx >= itemCount) return;
            int y = dlgY + 1 + (idx - scrollOffset);
            bool isSel = (idx == selected);
            if (isSel)
                fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
            else
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
            TermAttr lbl = isSel ? selAttr : itemAttr;
            printAt(y, dlgX + 2, truncateStr(items[idx].label, 46), lbl);
            if (items[idx].isBool)
            {
                TermAttr chk = isSel ? selAttr : checkAttr;
                if (items[idx].value == "Y")
                {
                    g_term->setAttr(chk);
                    g_term->printStr(y, checkCol, "[");
                    g_term->putCP437(y, checkCol + 1, CP437_CHECK_MARK);
                    g_term->printStr(y, checkCol + 2, "]");
                }
                else
                {
                    printAt(y, checkCol, "[ ]", chk);
                }
            }
            else
            {
                // Non-bool: show current value (e.g. reply dir)
                printAt(y, dlgX + 48, truncateStr(items[idx].value, dlgW - 50), lbl);
            }
        };

        // Lambda: draw scrollbar
        auto drawSSDSB = [&]()
        {
            if (itemCount > visibleItems)
            {
                drawScrollbar(dlgY + 1, visibleItems, selected, itemCount,
                             tAttr(TC_BLACK, TC_BLACK, true),
                             tAttr(TC_WHITE, TC_BLACK, true));
            }
        };

        bool scrollChanged = (scrollOffset != prevScrollOffset);

        if (needFullRedraw || scrollChanged)
        {
            clearDialogArea(dlgY, dlgX, dlgH, dlgW);

            // Draw box
            g_term->setAttr(borderAttr);
            g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

            // Title with T-characters
            string title = _(" Reader Settings ");
            int titleX = dlgX + 3;
            g_term->setAttr(borderAttr);
            g_term->putCP437(dlgY, titleX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
            g_term->setAttr(titleAttr);
            g_term->printStr(dlgY, titleX + 1, title);
            g_term->setAttr(borderAttr);
            g_term->putCP437(dlgY, titleX + static_cast<int>(title.size()) + 1,
                             CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

            // "Enabled" and page info
            int enabledX = dlgX + dlgW - 24;
            g_term->setAttr(titleAttr);
            g_term->printStr(dlgY, enabledX, _(" Enabled "));

            int totalPages  = (itemCount + visibleItems - 1) / visibleItems;
            int currentPage = (scrollOffset / visibleItems) + 1;
            string pageText = " Page " + std::to_string(currentPage) + " of "
                                  + std::to_string(totalPages) + " ";
            g_term->printStr(dlgY, dlgX + dlgW - static_cast<int>(pageText.size()) - 1, pageText);

            // All visible items
            for (int i = 0; i < visibleItems && i + scrollOffset < itemCount; ++i)
            {
                drawSSDRow(scrollOffset + i);
            }

            drawSSDSB();

            // Bottom separator with help text
            int helpY = dlgY + dlgH - 2;
            g_term->setAttr(borderAttr);
            g_term->putCP437(helpY, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
            g_term->drawHLine(helpY, dlgX + 1, dlgW - 2);
            g_term->putCP437(helpY, dlgX + dlgW - 1,
                             CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);

            string helpText = _("Up, Dn, Enter=Select/toggle, S=Save, ESC/Q=Close");
            int helpTextX = dlgX + 2;
            g_term->setAttr(borderAttr);
            g_term->putCP437(helpY, helpTextX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
            g_term->setAttr(titleAttr);
            g_term->printStr(helpY, helpTextX + 1, helpText);
            g_term->setAttr(borderAttr);
            g_term->putCP437(helpY, helpTextX + static_cast<int>(helpText.size()) + 1,
                             CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

            needFullRedraw = false;
        }
        else if (selected != prevSelected)
        {
            drawSSDRow(prevSelected);
            drawSSDRow(selected);
            drawSSDSB();
        }

        g_term->refresh();
        prevSelected     = selected;
        prevScrollOffset = scrollOffset;

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
                scrollOffset = 0;
                break;
            case TK_END:
                selected = itemCount - 1;
                break;
            case TK_PGUP:
                selected -= visibleItems;
                if (selected < 0) selected = 0;
                break;
            case TK_PGDN:
                selected += visibleItems;
                if (selected >= itemCount) selected = itemCount - 1;
                break;
            case TK_ENTER:
            case ' ':
                switch (items[selected].id)
                {
                    case SET_SHOW_KLUDGE:
                        settings.showKludgeLines = !settings.showKludgeLines;
                        changed = true;
                        needFullRedraw = true;  // items rebuilt next iter with new value
                        break;
                    case SET_SHOW_TEAR:
                        settings.showTearLine = !settings.showTearLine;
                        changed = true;
                        needFullRedraw = true;
                        break;
                    case SET_USE_SCROLLBAR:
                        settings.useScrollbar = !settings.useScrollbar;
                        changed = true;
                        needFullRedraw = true;
                        break;
                    case SET_ONLY_AREAS_WITH_MAIL:
                        settings.onlyShowAreasWithNewMail = !settings.onlyShowAreasWithNewMail;
                        changed = true;
                        needFullRedraw = true;
                        break;
                    case SET_STRIP_ANSI:
                        settings.stripAnsi = !settings.stripAnsi;
                        changed = true;
                        needFullRedraw = true;
                        break;
                    case SET_ATTR_CODE_TOGGLES:
                        if (showAttrCodeToggles(settings))
                        {
                            changed = true;
                        }
                        needFullRedraw = true;
                        break;
                    case SET_REGEX_SEARCH:
                        settings.useRegexSearch = !settings.useRegexSearch;
                        changed = true;
                        needFullRedraw = true;
                        break;
                    case SET_REVERSE_ORDER:
                        settings.reverseOrder = !settings.reverseOrder;
                        changed = true;
                        needFullRedraw = true;
                        break;
                    case SET_SPLASH_SCREEN:
                        settings.showSplashScreen = !settings.showSplashScreen;
                        changed = true;
                        needFullRedraw = true;
                        break;
                    case SET_REPLY_DIR:
                    {
                        string startDir = settings.replyDir;
                        if (startDir.empty())
                        {
                            startDir = getSlyMailDataDir() + PATH_SEP_STR + "REP";
                        }
                        string val = showDirChooser(startDir, _("Select Reply Packet Directory"));
                        if (!val.empty())
                        {
                            settings.replyDir = val;
                            changed = true;
                        }
                        needFullRedraw = true;
                        break;
                    }
                    case SET_USE_EXTERNAL_EDITOR:
                        settings.useExternalEditor = !settings.useExternalEditor;
                        changed = true;
                        needFullRedraw = true;
                        break;
                    case SET_EXTERNAL_EDITORS_LIST:
                        if (showExternalEditorsList(settings))
                            changed = true;
                        needFullRedraw = true;
                        break;
                    case SET_SELECT_EXTERNAL_EDITOR:
                    {
                        // Cycle through configured editors (or none)
                        if (settings.externalEditors.empty())
                        {
                            settings.selectedEditor.clear();
                        }
                        else
                        {
                            // Find current index
                            int curIdx = -1;
                            for (int j = 0; j < static_cast<int>(settings.externalEditors.size()); ++j)
                            {
                                if (settings.externalEditors[j].name == settings.selectedEditor)
                                {
                                    curIdx = j;
                                    break;
                                }
                            }
                            // Cycle: current → next → ... → (none) → first → ...
                            if (curIdx < 0 || curIdx + 1 >= static_cast<int>(settings.externalEditors.size()))
                            {
                                // At end or not found → if was valid, go to none; if none, go to first
                                if (curIdx >= 0)
                                    settings.selectedEditor.clear();
                                else
                                    settings.selectedEditor = settings.externalEditors[0].name;
                            }
                            else
                            {
                                settings.selectedEditor = settings.externalEditors[curIdx + 1].name;
                            }
                        }
                        changed = true;
                        needFullRedraw = true;
                        break;
                    }
                    case SET_LANGUAGE:
                    {
                        string newCode = showLanguageSelector(settings.language);
                        if (newCode != settings.language)
                        {
                            settings.language = newCode;
                            changed = true;
                            messageDialog(_("Language"), _("Restart required for language change to take effect."));
                        }
                        needFullRedraw = true;
                        break;
                    }
                }
                break;
            case 's':
            case 'S':
                if (changed)
                {
                    settings.save();
                    messageDialog(_("Settings"), _("Settings saved successfully."));
                }
                return changed;
            case TK_ESCAPE:
            case 'q':
            case 'Q':
                if (changed)
                {
                    if (confirmDialog(_("Save changes before closing?")))
                    {
                        settings.save();
                    }
                }
                return changed;
            case TK_RESIZE:
                needFullRedraw = true;
                break;
            default:
                break;
        }
    }
}

// ============================================================
// Attribute code toggles sub-dialog
// Checkbox list for enabling/disabling interpretation of color
// attribute codes from various BBS software packages.
// ============================================================
bool showAttrCodeToggles(Settings& settings)
{
    struct AttrItem
    {
        string label;
        bool*  value;
    };
    vector<AttrItem> items =
    {
        {_("Synchronet Ctrl-A attribute codes"),   &settings.attrSynchronet},
        {_("WWIV attribute codes"),                &settings.attrWWIV},
        {_("Celerity attribute codes"),            &settings.attrCelerity},
        {_("Renegade attribute codes"),            &settings.attrRenegade},
        {_("PCBoard/Wildcat attribute codes"),     &settings.attrPCBoard},
    };

    int itemCount = static_cast<int>(items.size());
    int selected = 0;
    bool changed = false;

    int dlgW = 60;
    if (dlgW > g_term->getCols() - 4)
    {
        dlgW = g_term->getCols() - 4;
    }
    int dlgH = itemCount + 7;
    int dlgY = (g_term->getRows() - dlgH) / 2;
    int dlgX = (g_term->getCols() - dlgW) / 2;

    TermAttr borderAttr = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr titleAttr  = tAttr(TC_BLUE, TC_BLACK, true);
    TermAttr itemAttr   = tAttr(TC_CYAN, TC_BLACK, false);
    TermAttr selAttr    = tAttr(TC_BLUE, TC_WHITE, false);
    TermAttr checkAttr  = tAttr(TC_GREEN, TC_BLACK, true);
    TermAttr noteAttr   = tAttr(TC_YELLOW, TC_BLACK, false);
    int checkCol = dlgX + dlgW - 8;

    while (true)
    {
        clearDialogArea(dlgY, dlgX, dlgH, dlgW);

        // Box
        g_term->setAttr(borderAttr);
        g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

        // Title
        string titleText = _(" Attribute Code Toggles ");
        int titleX = dlgX + 3;
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, titleX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        g_term->setAttr(titleAttr);
        g_term->printStr(dlgY, titleX + 1, titleText);
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, titleX + static_cast<int>(titleText.size()) + 1,
                         CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

        // "Enabled" column header
        g_term->setAttr(titleAttr);
        g_term->printStr(dlgY, checkCol - 2, _(" Enabled "));

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
            printAt(y, dlgX + 2, truncateStr(items[i].label, checkCol - dlgX - 4), lbl);

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

        // Notes
        int noteY = dlgY + 1 + itemCount;
        printAt(noteY, dlgX + 2, _("ANSI escape codes are always enabled."), noteAttr);
        printAt(noteY + 1, dlgX + 2, _("These apply to the reader and editor."), noteAttr);

        // Bottom help
        int helpY = dlgY + dlgH - 1;
        string helpText = _(" Up/Dn, Enter/Space=Toggle, ESC=Done ");
        int helpX = dlgX + (dlgW - static_cast<int>(helpText.size())) / 2;
        g_term->setAttr(titleAttr);
        g_term->printStr(helpY, helpX, helpText);

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
                *(items[selected].value) = !*(items[selected].value);
                changed = true;
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
// Drop file type selector dialog
// ============================================================
DropFileType showDropFileTypeSelector(DropFileType current)
{
    struct DFItem { string label; DropFileType type; };
    vector<DFItem> items = {
        {"None",       DropFileType::None},
        {"MSGINF",     DropFileType::MSGINF},
        {"EDITOR.INF", DropFileType::EDITORINF},
        {"DOOR.SYS",   DropFileType::DOORSYS},
        {"DOOR32.SYS", DropFileType::DOOR32SYS},
    };
    int totalItems = static_cast<int>(items.size());

    int selected = 0;
    for (int i = 0; i < totalItems; ++i)
    {
        if (items[i].type == current) { selected = i; break; }
    }

    int dlgW = 30;
    int dlgH = totalItems + 4;
    int dlgY = (g_term->getRows() - dlgH) / 2;
    int dlgX = (g_term->getCols() - dlgW) / 2;

    TermAttr borderAttr = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr titleAttr  = tAttr(TC_BLUE, TC_BLACK, true);
    TermAttr itemAttr   = tAttr(TC_CYAN, TC_BLACK, false);
    TermAttr selAttr    = tAttr(TC_BLUE, TC_WHITE, false);

    for (;;)
    {
        clearDialogArea(dlgY, dlgX, dlgH, dlgW);
        g_term->setAttr(borderAttr);
        g_term->drawBox(dlgY, dlgX, dlgH, dlgW);
        string title = _(" Drop File Type ");
        int titleX = dlgX + (dlgW - static_cast<int>(title.size())) / 2;
        g_term->setAttr(titleAttr);
        g_term->printStr(dlgY, titleX, title);

        for (int i = 0; i < totalItems; ++i)
        {
            int y = dlgY + 1 + i;
            bool isSel = (i == selected);
            if (isSel)
                fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
            else
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
            printAt(y, dlgX + 3, items[i].label, isSel ? selAttr : itemAttr);
        }

        int helpY = dlgY + dlgH - 2;
        g_term->setAttr(borderAttr);
        g_term->putCP437(helpY, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
        g_term->drawHLine(helpY, dlgX + 1, dlgW - 2);
        g_term->putCP437(helpY, dlgX + dlgW - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        string helpText = _("Enter=Select, ESC=Cancel");
        g_term->setAttr(titleAttr);
        g_term->printStr(helpY, dlgX + (dlgW - static_cast<int>(helpText.size())) / 2, helpText);

        g_term->refresh();
        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:   if (selected > 0) --selected; break;
            case TK_DOWN: if (selected < totalItems - 1) ++selected; break;
            case TK_ENTER:
            case ' ':
                clearDialogArea(dlgY, dlgX, dlgH, dlgW);
                return items[selected].type;
            case TK_ESCAPE:
            case 'q': case 'Q':
                clearDialogArea(dlgY, dlgX, dlgH, dlgW);
                return current;
            default: break;
        }
    }
}

// ============================================================
// External editor configuration dialog
// ============================================================
bool showExternalEditorConfig(ExternalEditorConfig& editor)
{
    bool changed = false;

    enum { FIELD_NAME, FIELD_STARTUP_DIR, FIELD_CMD_LINE,
           FIELD_WORDWRAP, FIELD_AUTO_QUOTE, FIELD_DROP_FILE, FIELD_STRIP_KLUDGE,
           FIELD_COUNT };

    int dlgW = 73;
    if (dlgW > g_term->getCols() - 4) dlgW = g_term->getCols() - 4;
    int dlgH = FIELD_COUNT + 6;
    int dlgY = (g_term->getRows() - dlgH) / 2;
    int dlgX = (g_term->getCols() - dlgW) / 2;
    int labelW = 28;
    int valX = dlgX + labelW + 1;
    int valW = dlgW - labelW - 3;

    int selected = 0;

    TermAttr borderAttr = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr titleAttr  = tAttr(TC_BLUE, TC_BLACK, true);
    TermAttr itemAttr   = tAttr(TC_CYAN, TC_BLACK, false);
    TermAttr selAttr    = tAttr(TC_BLUE, TC_WHITE, false);
    TermAttr valueAttr  = tAttr(TC_WHITE, TC_BLACK, true);

    auto drawDialog = [&]() {
        clearDialogArea(dlgY, dlgX, dlgH, dlgW);
        g_term->setAttr(borderAttr);
        g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

        string title = _(" External Editor ");
        int tX = dlgX + 3;
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, tX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        g_term->setAttr(titleAttr);
        g_term->printStr(dlgY, tX + 1, title);
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, tX + static_cast<int>(title.size()) + 1,
                         CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

        // Build display values
        string wrapStr;
        if (editor.wordWrapQuotedText)
            wrapStr = string(_("Yes, for ")) + std::to_string(editor.wordWrapNumCols) + _(" columns");
        else
            wrapStr = _("No");

        struct FieldInfo { string label; string value; };
        vector<FieldInfo> fields = {
            {_("Name:"),                 editor.name},
            {_("Startup Directory:"),    editor.startupDir.empty() ? _("(not set)") : editor.startupDir},
            {_("Command Line:"),         editor.commandLine.empty() ? _("(not set)") : editor.commandLine},
            {_("Word-wrap Quoted Text:"), wrapStr},
            {_("Auto Quoted Text:"),     extQuoteModeStr(editor.autoQuoteMode)},
            {_("Editor Info Files:"),    dropFileTypeStr(editor.dropFileType)},
            {_("Strip FidoNet Kludges:"), editor.stripFidoKludges ? _("Yes") : _("No")},
        };

        for (int i = 0; i < FIELD_COUNT; ++i)
        {
            int y = dlgY + 1 + i;
            bool isSel = (i == selected);
            if (isSel)
                fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
            else
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);

            TermAttr lbl = isSel ? selAttr : itemAttr;
            printAt(y, dlgX + 2, fields[i].label, lbl);
            printAt(y, valX, truncateStr(fields[i].value, valW), isSel ? selAttr : valueAttr);
        }

        // Note about %f
        int noteY = dlgY + 1 + FIELD_COUNT;
        fillRow(noteY, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
        printAt(noteY, dlgX + 2, _("Note: %f = temp filename in command line"),
                tAttr(TC_YELLOW, TC_BLACK, false));

        // Help bar
        int helpY = dlgY + dlgH - 2;
        g_term->setAttr(borderAttr);
        g_term->putCP437(helpY, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
        g_term->drawHLine(helpY, dlgX + 1, dlgW - 2);
        g_term->putCP437(helpY, dlgX + dlgW - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        string helpText = _("Enter=Edit, Del=Clear, ESC=Done");
        g_term->setAttr(titleAttr);
        g_term->printStr(helpY, dlgX + (dlgW - static_cast<int>(helpText.size())) / 2, helpText);

        g_term->refresh();
    };

    for (;;)
    {
        drawDialog();
        int ch = g_term->getKey();

        switch (ch)
        {
            case TK_UP:   if (selected > 0) --selected; break;
            case TK_DOWN: if (selected < FIELD_COUNT - 1) ++selected; break;
            case TK_DELETE:
                if (selected == FIELD_STARTUP_DIR)
                {
                    editor.startupDir.clear();
                    changed = true;
                }
                else if (selected == FIELD_CMD_LINE)
                {
                    editor.commandLine.clear();
                    changed = true;
                }
                break;
            case TK_ENTER:
            case ' ':
            {
                switch (selected)
                {
                    case FIELD_NAME:
                    {
                        int y = dlgY + 1 + selected;
                        string val = getStringInput(y, valX, valW, editor.name, valueAttr);
                        if (!val.empty())
                        {
                            editor.name = val;
                            changed = true;
                        }
                        break;
                    }
                    case FIELD_STARTUP_DIR:
                    {
                        int y = dlgY + 1 + selected;
                        string val = getStringInput(y, valX, valW, editor.startupDir, valueAttr);
                        if (!val.empty())
                        {
                            editor.startupDir = val;
                            changed = true;
                        }
                        break;
                    }
                    case FIELD_CMD_LINE:
                    {
                        // Show file browser, then let user edit the result
                        string startDir = editor.startupDir;
                        if (startDir.empty())
                        {
#ifdef _WIN32
                            startDir = "C:\\";
#else
                            startDir = "/usr/bin";
#endif
                        }
                        string val = showFileBrowser(startDir, "", "*");
                        if (!val.empty())
                        {
                            // Extract directory and filename
                            auto sep = val.find_last_of("/\\");
                            if (sep != string::npos)
                            {
                                editor.startupDir = val.substr(0, sep);
                                editor.commandLine = val.substr(sep + 1);
                            }
                            else
                            {
                                editor.commandLine = val;
                            }
                            changed = true;
                        }
                        // Redraw the editor-config dialog so the file browser
                        // residue is wiped and the command-line edit happens
                        // on a clean screen.
                        drawDialog();
                        // Let user edit the command line string
                        {
                            int y = dlgY + 1 + selected;
                            string edited = getStringInput(y, valX, valW, editor.commandLine, valueAttr);
                            if (!edited.empty())
                            {
                                editor.commandLine = edited;
                                changed = true;
                            }
                        }
                        break;
                    }
                    case FIELD_WORDWRAP:
                    {
                        // Prompt Yes/No first
                        if (confirmDialog(_("Word-wrap quoted text?")))
                        {
                            editor.wordWrapQuotedText = true;
                            // Prompt for column count
                            int y = dlgY + 1 + selected;
                            fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
                            printAt(y, dlgX + 2, _("For how many columns? "), valueAttr);
                            string numStr = getNumericInput(y, dlgX + 24, 5, valueAttr);
                            int cols = 79;
                            if (!numStr.empty())
                            {
                                try { cols = std::stoi(numStr); } catch (...) {}
                            }
                            if (cols <= 0) cols = 79;
                            editor.wordWrapNumCols = cols;
                        }
                        else
                        {
                            editor.wordWrapQuotedText = false;
                        }
                        changed = true;
                        break;
                    }
                    case FIELD_AUTO_QUOTE:
                    {
                        // Cycle: Always -> Prompt -> Never -> Always
                        if (editor.autoQuoteMode == ExtQuoteMode::Always)
                            editor.autoQuoteMode = ExtQuoteMode::Prompt;
                        else if (editor.autoQuoteMode == ExtQuoteMode::Prompt)
                            editor.autoQuoteMode = ExtQuoteMode::Never;
                        else
                            editor.autoQuoteMode = ExtQuoteMode::Always;
                        changed = true;
                        break;
                    }
                    case FIELD_DROP_FILE:
                    {
                        DropFileType newType = showDropFileTypeSelector(editor.dropFileType);
                        if (newType != editor.dropFileType)
                        {
                            editor.dropFileType = newType;
                            changed = true;
                        }
                        break;
                    }
                    case FIELD_STRIP_KLUDGE:
                        editor.stripFidoKludges = !editor.stripFidoKludges;
                        changed = true;
                        break;
                }
                break;
            }
            case TK_ESCAPE:
            case 'q': case 'Q':
                clearDialogArea(dlgY, dlgX, dlgH, dlgW);
                return changed;
            default: break;
        }
    }
}

// ============================================================
// External editors list dialog
// ============================================================
bool showExternalEditorsList(Settings& settings)
{
    int selected = 0;
    bool changed = false;

    for (;;)
    {
        int totalItems = static_cast<int>(settings.externalEditors.size()) + 1; // +1 for "Add new"

        int dlgW = 65;
        if (dlgW > g_term->getCols() - 4) dlgW = g_term->getCols() - 4;
        int visibleItems = g_term->getRows() - 10;
        if (visibleItems > totalItems) visibleItems = totalItems;
        if (visibleItems < 3) visibleItems = 3;
        int dlgH = visibleItems + 4;
        int dlgY = (g_term->getRows() - dlgH) / 2;
        int dlgX = (g_term->getCols() - dlgW) / 2;

        int scrollOffset = 0;
        if (selected >= totalItems) selected = totalItems - 1;
        if (selected < 0) selected = 0;
        if (selected >= scrollOffset + visibleItems) scrollOffset = selected - visibleItems + 1;
        if (selected < scrollOffset) scrollOffset = selected;

        TermAttr borderAttr = tAttr(TC_GREEN, TC_BLACK, false);
        TermAttr titleAttr  = tAttr(TC_BLUE, TC_BLACK, true);
        TermAttr itemAttr   = tAttr(TC_CYAN, TC_BLACK, false);
        TermAttr selAttr    = tAttr(TC_BLUE, TC_WHITE, false);
        TermAttr activeAttr = tAttr(TC_GREEN, TC_BLACK, true);

        clearDialogArea(dlgY, dlgX, dlgH, dlgW);
        g_term->setAttr(borderAttr);
        g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

        string title = _(" External Editors ");
        int tX = dlgX + 3;
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, tX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        g_term->setAttr(titleAttr);
        g_term->printStr(dlgY, tX + 1, title);
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, tX + static_cast<int>(title.size()) + 1,
                         CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

        for (int i = 0; i < visibleItems; ++i)
        {
            int idx = scrollOffset + i;
            int y = dlgY + 1 + i;
            bool isSel = (idx == selected);

            if (isSel)
                fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
            else
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);

            if (idx < static_cast<int>(settings.externalEditors.size()))
            {
                const auto& ed = settings.externalEditors[idx];
                string display = ed.name;
                if (!ed.commandLine.empty())
                    display += " - " + ed.commandLine;
                bool isActive = (ed.name == settings.selectedEditor);
                printAt(y, dlgX + 2, truncateStr(display, dlgW - 10), isSel ? selAttr : itemAttr);
                if (isActive)
                    printAt(y, dlgX + dlgW - 5, "[*]", isSel ? selAttr : activeAttr);
            }
            else
            {
                printAt(y, dlgX + 2, _("(Add new editor...)"), isSel ? selAttr : itemAttr);
            }
        }

        // Scrollbar
        if (totalItems > visibleItems)
        {
            drawScrollbar(dlgY + 1, visibleItems, selected, totalItems,
                         tAttr(TC_BLACK, TC_BLACK, true),
                         tAttr(TC_WHITE, TC_BLACK, true));
        }

        // Help bar
        int helpY = dlgY + dlgH - 2;
        g_term->setAttr(borderAttr);
        g_term->putCP437(helpY, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
        g_term->drawHLine(helpY, dlgX + 1, dlgW - 2);
        g_term->putCP437(helpY, dlgX + dlgW - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        string helpText = _("Enter=Edit, S=Select, D=Delete, ESC=Done");
        g_term->setAttr(titleAttr);
        g_term->printStr(helpY, dlgX + (dlgW - static_cast<int>(helpText.size())) / 2, helpText);

        g_term->refresh();
        int ch = g_term->getKey();

        switch (ch)
        {
            case TK_UP:   if (selected > 0) --selected; break;
            case TK_DOWN: if (selected < totalItems - 1) ++selected; break;
            case TK_HOME: selected = 0; break;
            case TK_END:  selected = totalItems - 1; break;
            case TK_PGUP: selected -= visibleItems; if (selected < 0) selected = 0; break;
            case TK_PGDN: selected += visibleItems; if (selected >= totalItems) selected = totalItems - 1; break;
            case TK_ENTER:
            case ' ':
            {
                if (selected >= static_cast<int>(settings.externalEditors.size()))
                {
                    // Add new editor
                    ExternalEditorConfig newEd;
                    if (showExternalEditorConfig(newEd) && !newEd.name.empty())
                    {
                        settings.externalEditors.push_back(newEd);
                        changed = true;
                    }
                }
                else
                {
                    // Edit existing
                    if (showExternalEditorConfig(settings.externalEditors[selected]))
                        changed = true;
                }
                break;
            }
            case 's': case 'S':
            {
                if (selected < static_cast<int>(settings.externalEditors.size()))
                {
                    settings.selectedEditor = settings.externalEditors[selected].name;
                    changed = true;
                }
                break;
            }
            case 'd': case 'D':
            case TK_DELETE:
            {
                if (selected < static_cast<int>(settings.externalEditors.size()))
                {
                    string name = settings.externalEditors[selected].name;
                    if (confirmDialog(_("Delete editor '") + name + _("'?")))
                    {
                        settings.externalEditors.erase(
                            settings.externalEditors.begin() + selected);
                        if (settings.selectedEditor == name)
                            settings.selectedEditor.clear();
                        if (selected >= static_cast<int>(settings.externalEditors.size()) + 1)
                            selected = static_cast<int>(settings.externalEditors.size());
                        changed = true;
                    }
                }
                break;
            }
            case TK_ESCAPE:
            case 'q': case 'Q':
            {
                int dlgW2 = g_term->getCols() > 69 ? 65 : g_term->getCols() - 4;
                int totalItems2 = static_cast<int>(settings.externalEditors.size()) + 1;
                int vis2 = std::min(totalItems2, g_term->getRows() - 10);
                if (vis2 < 3) vis2 = 3;
                int dlgH2 = vis2 + 4;
                int dlgY2 = (g_term->getRows() - dlgH2) / 2;
                int dlgX2 = (g_term->getCols() - dlgW2) / 2;
                clearDialogArea(dlgY2, dlgX2, dlgH2, dlgW2);
                return changed;
            }
            default: break;
        }
    }
}
