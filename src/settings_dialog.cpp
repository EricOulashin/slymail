#include "settings_dialog.h"

using std::string;
using std::vector;

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

    for (;;)
    {
        // Draw box
        g_term->setAttr(borderAttr);
        g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

        // Title in top border
        string title = " UI Mode ";
        int titleX = dlgX + (dlgW - static_cast<int>(title.size()) - 2) / 2;
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, titleX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        g_term->setAttr(titleAttr);
        g_term->printStr(dlgY, titleX + 1, title);
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, titleX + static_cast<int>(title.size()) + 1,
                         CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

        // Draw items
        for (int i = 0; i < numChoices; ++i)
        {
            int y = dlgY + 1 + i;
            bool isSel = (i == cursor);

            if (isSel)
            {
                fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
            }
            else
            {
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
            }

            TermAttr lbl = isSel ? selAttr : itemAttr;
            printAt(y, dlgX + 2, labels[i], lbl);
        }

        // Fill any empty rows between items and help separator
        for (int r = dlgY + 1 + numChoices; r < dlgY + dlgH - 2; ++r)
        {
            fillRow(r, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
            // Draw side borders
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
        string helpText = "Enter=OK, ESC=Cancel";
        int helpX = dlgX + (dlgW - static_cast<int>(helpText.size())) / 2;
        g_term->setAttr(titleAttr);
        g_term->printStr(helpY, helpX, helpText);

        g_term->refresh();

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
                switch (cursor)
                {
                    case 0: return EditorStyle::Ice;
                    case 1: return EditorStyle::Dct;
                    case 2: return EditorStyle::Random;
                }
                return currentStyle;
            }
            case TK_ESCAPE:
                return currentStyle;
            default:
                break;
        }
    }
}

// Find available dictionary files in the dictionaries directory
vector<string> findDictionaries(const string& baseDir)
{
    vector<string> dicts;
    string dictDir = baseDir + PATH_SEP_STR + "dictionary_files";
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
    string configDir = baseDir + PATH_SEP_STR + "config_files";
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
        messageDialog("Themes", "No theme files found in config/");
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

    for (;;)
    {
        // Ensure cursor is visible
        if (cursor < scrollOffset)
        {
            scrollOffset = cursor;
        }
        if (cursor >= scrollOffset + visibleItems)
        {
            scrollOffset = cursor - visibleItems + 1;
        }

        // Draw box
        g_term->setAttr(borderAttr);
        g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

        // Title in top border
        string title = " Select Theme ";
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
        g_term->printStr(headerY, dlgX + 2, "Theme File");

        // Separator
        g_term->setAttr(borderAttr);
        g_term->putCP437(headerY + 1, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
        g_term->drawHLine(headerY + 1, dlgX + 1, dlgW - 2);
        g_term->putCP437(headerY + 1, dlgX + dlgW - 1,
                         CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);

        // Items
        for (int i = 0; i < visibleItems && i + scrollOffset < static_cast<int>(themes.size()); ++i)
        {
            int idx = i + scrollOffset;
            int y = dlgY + 3 + i;
            bool isSel = (idx == cursor);

            if (isSel)
            {
                fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
            }
            else
            {
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
            }

            TermAttr lbl = isSel ? selAttr : itemAttr;

            // Strip prefix and .ini for display
            string displayName = themes[idx];
            if (displayName.size() > 4)
            {
                displayName = displayName.substr(0, displayName.size() - 4);
            }
            // Strip EditorIceColors_ or EditorDCTColors_ prefix
            if (displayName.substr(0, 16) == "EditorIceColors_")
            {
                displayName = "Ice: " + displayName.substr(16);
            }
            else if (displayName.substr(0, 16) == "EditorDCTColors_")
            {
                displayName = "DCT: " + displayName.substr(16);
            }
            printAt(y, dlgX + 2, truncateStr(displayName, dlgW - 4), lbl);
        }

        // Scrollbar if content overflows
        int totalThemes = static_cast<int>(themes.size());
        if (totalThemes > visibleItems)
        {
            drawScrollbar(dlgY + 3, visibleItems, cursor, totalThemes,
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
        string helpText = "Up, Dn, Enter=Select, ESC/Q=Cancel";
        int helpX = dlgX + (dlgW - static_cast<int>(helpText.size())) / 2;
        g_term->setAttr(titleAttr);
        g_term->printStr(helpY, helpX, helpText);

        g_term->refresh();

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
                return themes[cursor];
            case TK_ESCAPE:
            case 'q':
            case 'Q':
                return "";
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
        messageDialog("Dictionaries", "No dictionary files found in dictionary_files/");
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

    for (;;)
    {
        // Ensure cursor is visible
        if (cursor < scrollOffset)
        {
            scrollOffset = cursor;
        }
        if (cursor >= scrollOffset + visibleItems)
        {
            scrollOffset = cursor - visibleItems + 1;
        }

        // Draw box
        g_term->setAttr(borderAttr);
        g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

        // Title in top border
        string title = " Select Dictionaries ";
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
        g_term->printStr(headerY, dlgX + 2, "Dictionary");
        g_term->printStr(headerY, dlgX + dlgW - 12, "Selected");

        // Separator
        g_term->setAttr(borderAttr);
        g_term->putCP437(headerY + 1, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
        g_term->drawHLine(headerY + 1, dlgX + 1, dlgW - 2);
        g_term->putCP437(headerY + 1, dlgX + dlgW - 1,
                         CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);

        // Items
        int checkCol = dlgX + dlgW - 8;
        for (int i = 0; i < visibleItems && i + scrollOffset < static_cast<int>(dicts.size()); ++i)
        {
            int idx = i + scrollOffset;
            int y = dlgY + 3 + i;
            bool isSel = (idx == cursor);

            if (isSel)
            {
                fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
            }
            else
            {
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
            }

            TermAttr lbl = isSel ? selAttr : itemAttr;
            TermAttr chk = isSel ? selCheckAt : checkAttr;

            // Dictionary name (strip dictionary_ prefix and .txt suffix for display)
            string displayName = dicts[idx];
            if (displayName.size() > 4)
            {
                displayName = displayName.substr(0, displayName.size() - 4);
            }
            if (displayName.substr(0, 11) == "dictionary_")
            {
                displayName = displayName.substr(11);
            }
            printAt(y, dlgX + 2, truncateStr(displayName, dlgW - 16), lbl);

            // Checkbox - use putCP437 for check mark to fix ncurses display
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
        }

        // Scrollbar if content overflows
        int totalDicts = static_cast<int>(dicts.size());
        if (totalDicts > visibleItems)
        {
            drawScrollbar(dlgY + 3, visibleItems, cursor, totalDicts,
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
        string helpText = "Up, Dn, Enter/Space=Toggle, ESC/Q=Done";
        int helpX = dlgX + (dlgW - static_cast<int>(helpText.size())) / 2;
        g_term->setAttr(titleAttr);
        g_term->printStr(helpY, helpX, helpText);

        g_term->refresh();

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
                return result;
            }
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
        {"Choose UI mode",                        SettingType::Choice,  ESET_UI_MODE},
        {"Taglines",                              SettingType::Toggle,  ESET_TAGLINES},
        {"Spell-check dictionary/dictionaries",   SettingType::SubMenu, ESET_SPELL_DICT},
        {"Prompt for spell checker on save",      SettingType::Toggle,  ESET_PROMPT_SPELL},
        {"Wrap quote lines to terminal width",    SettingType::Toggle,  ESET_WRAP_QUOTES},
        {"Quote with author's initials",          SettingType::Toggle,  ESET_QUOTE_INITIALS},
        {"Indent quote lines containing initials", SettingType::Toggle, ESET_INDENT_INITIALS},
        {"Trim spaces from quote lines",          SettingType::Toggle,  ESET_TRIM_QUOTE_SPACES},
        {"Select theme file",                     SettingType::SubMenu, ESET_THEME_FILE},
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

    for (;;)
    {
        // Draw outer box
        g_term->setAttr(borderAttr);
        g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

        // Title bar: +--|Setting|---------------------- Enabled - Page 1 of 1 --+
        // T-characters around "Setting" text in top border
        string settingTitle = " Setting ";
        int settingTitleX = dlgX + 3;
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, settingTitleX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        g_term->setAttr(titleAttr);
        g_term->printStr(dlgY, settingTitleX + 1, settingTitle);
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, settingTitleX + static_cast<int>(settingTitle.size()) + 1,
                         CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

        // "Enabled" and "Page 1 of 1" in top border
        string enabledText = " Enabled ";
        int enabledX = dlgX + dlgW - 24;
        g_term->setAttr(titleAttr);
        g_term->printStr(dlgY, enabledX, enabledText);

        string pageText = " Page 1 of 1 ";
        g_term->printStr(dlgY, dlgX + dlgW - static_cast<int>(pageText.size()) - 1, pageText);

        // Draw items
        for (int i = 0; i < itemCount; ++i)
        {
            int y = dlgY + 1 + i;
            bool isSel = (i == selected);

            // Clear the row interior
            if (isSel)
            {
                fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
            }
            else
            {
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
            }

            TermAttr lbl = isSel ? selAttr : itemAttr;

            // Item label
            printAt(y, dlgX + 2, truncateStr(items[i].label, 46), lbl);

            // For Choice type, show current value after label
            if (items[i].type == SettingType::Choice && items[i].id == ESET_UI_MODE)
            {
                // Do nothing extra - label says "Choose UI mode"
            }

            // For Toggle type, show checkbox using putCP437 for check mark
            if (items[i].type == SettingType::Toggle)
            {
                bool val = false;
                switch (items[i].id)
                {
                    case ESET_TAGLINES:        val = settings.taglines; break;
                    case ESET_PROMPT_SPELL:     val = settings.promptSpellCheck; break;
                    case ESET_WRAP_QUOTES:      val = settings.wrapQuoteLines; break;
                    case ESET_QUOTE_INITIALS:   val = settings.quoteWithInitials; break;
                    case ESET_INDENT_INITIALS:  val = settings.indentQuoteInitials; break;
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
        }

        // Bottom separator with help text
        int helpY = dlgY + dlgH - 2;
        g_term->setAttr(borderAttr);
        g_term->putCP437(helpY, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
        g_term->drawHLine(helpY, dlgX + 1, dlgW - 2);
        g_term->putCP437(helpY, dlgX + dlgW - 1,
                         CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);

        // Help text with T-characters around it
        string helpText = "Up, Dn, Enter=Select/toggle, ?=Help, ESC/Q/Ctrl-U=Close";
        int helpTextX = dlgX + 2;
        g_term->setAttr(borderAttr);
        g_term->putCP437(helpY, helpTextX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        g_term->setAttr(titleAttr);
        g_term->printStr(helpY, helpTextX + 1, helpText);
        g_term->setAttr(borderAttr);
        g_term->putCP437(helpY, helpTextX + static_cast<int>(helpText.size()) + 1,
                         CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

        g_term->refresh();

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
                        // Clear screen area to ensure clean redraw after sub-dialog
                        for (int r = 0; r < g_term->getRows(); ++r)
                        {
                            fillRow(r, tAttr(TC_BLACK, TC_BLACK, false), 0, g_term->getCols());
                        }
                        break;
                    }
                    case ESET_TAGLINES:
                        settings.taglines = !settings.taglines;
                        changed = true;
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
                        // Clear screen area to ensure clean redraw after sub-dialog
                        for (int r = 0; r < g_term->getRows(); ++r)
                        {
                            fillRow(r, tAttr(TC_BLACK, TC_BLACK, false), 0, g_term->getCols());
                        }
                        break;
                    }
                    case ESET_PROMPT_SPELL:
                        settings.promptSpellCheck = !settings.promptSpellCheck;
                        changed = true;
                        break;
                    case ESET_WRAP_QUOTES:
                        settings.wrapQuoteLines = !settings.wrapQuoteLines;
                        changed = true;
                        break;
                    case ESET_QUOTE_INITIALS:
                        settings.quoteWithInitials = !settings.quoteWithInitials;
                        changed = true;
                        break;
                    case ESET_INDENT_INITIALS:
                        settings.indentQuoteInitials = !settings.indentQuoteInitials;
                        changed = true;
                        break;
                    case ESET_TRIM_QUOTE_SPACES:
                        settings.trimQuoteSpaces = !settings.trimQuoteSpaces;
                        changed = true;
                        break;
                    case ESET_THEME_FILE:
                    {
                        string themeName = showThemeSelector(baseDir, settings.editorStyle);
                        if (!themeName.empty())
                        {
                            if (settings.editorStyle == EditorStyle::Ice)
                            {
                                settings.iceThemeFile = themeName;
                            }
                            else if (settings.editorStyle == EditorStyle::Dct)
                            {
                                settings.dctThemeFile = themeName;
                            }
                            else
                            {
                                // Random mode: determine which type by prefix
                                if (themeName.substr(0, 16) == "EditorIceColors_")
                                {
                                    settings.iceThemeFile = themeName;
                                }
                                else
                                {
                                    settings.dctThemeFile = themeName;
                                }
                            }
                            changed = true;
                        }
                        // Clear screen area to ensure clean redraw after sub-dialog
                        for (int r = 0; r < g_term->getRows(); ++r)
                        {
                            fillRow(r, tAttr(TC_BLACK, TC_BLACK, false), 0, g_term->getCols());
                        }
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

    while (true)
    {
        vector<SettingItem> items;
        // Reader-only settings
        items.push_back({"Show kludge lines",
                         settings.showKludgeLines ? "Y" : "N", true, false, SET_SHOW_KLUDGE});
        items.push_back({"Show tear/origin lines",
                         settings.showTearLine ? "Y" : "N", true, false, SET_SHOW_TEAR});
        items.push_back({"Scrollbar in reader",
                         settings.useScrollbar ? "Y" : "N", true, false, SET_USE_SCROLLBAR});
        items.push_back({"Strip ANSI codes",
                         settings.stripAnsi ? "Y" : "N", true, false, SET_STRIP_ANSI});
        items.push_back({"List messages in reversed",
                         settings.reverseOrder ? "Y" : "N", true, false, SET_REVERSE_ORDER});
        items.push_back({"Reply packet directory",
                         settings.replyDir.empty() ? "(current dir)" : settings.replyDir,
                         false, false, SET_REPLY_DIR});

        int itemCount = static_cast<int>(items.size());

        // Dialog dimensions
        int dlgW = 73;
        if (dlgW > g_term->getCols() - 4)
        {
            dlgW = g_term->getCols() - 4;
        }
        int visibleItems = g_term->getRows() - 10;
        if (visibleItems > itemCount)
        {
            visibleItems = itemCount;
        }
        int dlgH = visibleItems + 5;
        int dlgY = (g_term->getRows() - dlgH) / 2;
        int dlgX = (g_term->getCols() - dlgW) / 2;

        // Scrolling
        static int scrollOffset = 0;
        if (selected < scrollOffset)
        {
            scrollOffset = selected;
        }
        if (selected >= scrollOffset + visibleItems)
        {
            scrollOffset = selected - visibleItems + 1;
        }

        // Colors matching SlyEdit's ChoiceScrollbox
        TermAttr borderAttr = tAttr(TC_GREEN, TC_BLACK, false);
        TermAttr titleAttr  = tAttr(TC_BLUE, TC_BLACK, true);
        TermAttr itemAttr   = tAttr(TC_CYAN, TC_BLACK, false);
        TermAttr checkAttr  = tAttr(TC_GREEN, TC_BLACK, true);
        TermAttr selAttr    = tAttr(TC_BLUE, TC_WHITE, false);
        int checkCol = dlgX + 48;

        // Draw box
        g_term->setAttr(borderAttr);
        g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

        // Title with T-characters
        string title = " Reader Settings ";
        int titleX = dlgX + 3;
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, titleX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        g_term->setAttr(titleAttr);
        g_term->printStr(dlgY, titleX + 1, title);
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY, titleX + static_cast<int>(title.size()) + 1,
                         CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

        // "Enabled" and page info
        string enabledText = " Enabled ";
        int enabledX = dlgX + dlgW - 24;
        g_term->setAttr(titleAttr);
        g_term->printStr(dlgY, enabledX, enabledText);

        int totalPages = (itemCount + visibleItems - 1) / visibleItems;
        int currentPage = (scrollOffset / visibleItems) + 1;
        string pageText = " Page " + std::to_string(currentPage) + " of "
                              + std::to_string(totalPages) + " ";
        g_term->printStr(dlgY, dlgX + dlgW - static_cast<int>(pageText.size()) - 1, pageText);

        // Draw items
        for (int i = 0; i < visibleItems; ++i)
        {
            int idx = i + scrollOffset;
            if (idx >= itemCount)
            {
                break;
            }
            int y = dlgY + 1 + i;
            bool isSel = (idx == selected);

            if (isSel)
            {
                fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
            }
            else
            {
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
            }

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
        }

        // Scrollbar if content overflows
        if (itemCount > visibleItems)
        {
            drawScrollbar(dlgY + 1, visibleItems, selected, itemCount,
                         tAttr(TC_BLACK, TC_BLACK, true),
                         tAttr(TC_WHITE, TC_BLACK, true));
        }

        // Bottom separator with help text
        int helpY = dlgY + dlgH - 2;
        g_term->setAttr(borderAttr);
        g_term->putCP437(helpY, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
        g_term->drawHLine(helpY, dlgX + 1, dlgW - 2);
        g_term->putCP437(helpY, dlgX + dlgW - 1,
                         CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);

        string helpText = "Up, Dn, Enter=Select/toggle, S=Save, ESC/Q=Close";
        int helpTextX = dlgX + 2;
        g_term->setAttr(borderAttr);
        g_term->putCP437(helpY, helpTextX, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
        g_term->setAttr(titleAttr);
        g_term->printStr(helpY, helpTextX + 1, helpText);
        g_term->setAttr(borderAttr);
        g_term->putCP437(helpY, helpTextX + static_cast<int>(helpText.size()) + 1,
                         CP437_BOX_DRAWINGS_LIGHT_LEFT_T);

        g_term->refresh();

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
                scrollOffset = 0;
                break;
            case TK_END:
                selected = itemCount - 1;
                break;
            case TK_PGUP:
                selected -= visibleItems;
                if (selected < 0)
                {
                    selected = 0;
                }
                break;
            case TK_PGDN:
                selected += visibleItems;
                if (selected >= itemCount)
                {
                    selected = itemCount - 1;
                }
                break;
            case TK_ENTER:
            case ' ':
                switch (items[selected].id)
                {
                    case SET_SHOW_KLUDGE:
                        settings.showKludgeLines = !settings.showKludgeLines;
                        changed = true;
                        break;
                    case SET_SHOW_TEAR:
                        settings.showTearLine = !settings.showTearLine;
                        changed = true;
                        break;
                    case SET_USE_SCROLLBAR:
                        settings.useScrollbar = !settings.useScrollbar;
                        changed = true;
                        break;
                    case SET_STRIP_ANSI:
                        settings.stripAnsi = !settings.stripAnsi;
                        changed = true;
                        break;
                    case SET_REVERSE_ORDER:
                        settings.reverseOrder = !settings.reverseOrder;
                        changed = true;
                        break;
                    case SET_REPLY_DIR:
                    {
                        int y = dlgY + 1 + (selected - scrollOffset);
                        string val = getStringInput(y, dlgX + 2, dlgW - 6,
                            settings.replyDir,
                            tAttr(TC_WHITE, TC_BLACK, true));
                        if (!val.empty())
                        {
                            settings.replyDir = val;
                            changed = true;
                        }
                        break;
                    }
                }
                break;
            case 's':
            case 'S':
                if (changed)
                {
                    settings.save();
                    messageDialog("Settings", "Settings saved successfully.");
                }
                return changed;
            case TK_ESCAPE:
            case 'q':
            case 'Q':
                if (changed)
                {
                    if (confirmDialog("Save changes before closing?"))
                    {
                        settings.save();
                    }
                }
                return changed;
            default:
                break;
        }
    }
}
