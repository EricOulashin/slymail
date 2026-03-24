#ifndef SLYMAIL_SETTINGS_DIALOG_H
#define SLYMAIL_SETTINGS_DIALOG_H

#include "terminal.h"
#include "colors.h"
#include "ui_common.h"
#include "settings.h"

// ============================================================
// Editor User Settings Dialog
// Matches SlyEdit's ChoiceScrollbox user settings dialog:
//   +--| Setting |-------------------------- Enabled - Page 1 of 1 -+
//   | Choose UI mode                                                 |
//   | Taglines                                              [ ]      |
//   | Spell-check dictionary/dictionaries                            |
//   | Prompt for spell checker on save                      [ ]      |
//   | Wrap quote lines to terminal width                    [v]      |
//   | Quote with author's initials                          [ ]      |
//   | Indent quote lines containing initials                [ ]      |
//   | Trim spaces from quote lines                          [ ]      |
//   | Select theme file                                              |
//   +-|Up, Dn, Enter=Select/toggle, ?=Help, ESC/Q/Ctrl-U=Close|----+
//   +---------------------------------------------------------------|
// ============================================================

// Setting item types
enum class SettingType
{
    Toggle,      // Boolean toggle with checkbox [v] or [ ]
    Choice,      // Opens a selection dialog
    SubMenu,     // Opens a sub-dialog (like dictionary selection)
};

struct EditorSettingItem
{
    std::string label;
    SettingType type;
    int         id;
};

// Editor setting IDs
enum EditorSettingID
{
    ESET_UI_MODE = 0,
    ESET_TAGLINES,
    ESET_SPELL_DICT,
    ESET_PROMPT_SPELL,
    ESET_WRAP_QUOTES,
    ESET_QUOTE_INITIALS,
    ESET_INDENT_INITIALS,
    ESET_TRIM_QUOTE_SPACES,
    ESET_THEME_FILE,
    ESET_COUNT
};

// ============================================================
// showSettingsDialog - Full application settings dialog
// (accessible from main menu)
// ============================================================

// Setting IDs for the full settings dialog
enum SettingID
{
    SET_EDITOR_STYLE = 0,
    SET_INSERT_MODE,
    SET_WRAP_QUOTES,
    SET_QUOTE_WIDTH,
    SET_QUOTE_PREFIX,
    SET_TAGLINES,
    SET_PROMPT_SPELL,
    SET_SPELL_DICT,
    SET_QUOTE_INITIALS,
    SET_INDENT_INITIALS,
    SET_TRIM_QUOTE_SPACES,
    SET_SHOW_KLUDGE,
    SET_SHOW_TEAR,
    SET_USE_SCROLLBAR,
    SET_STRIP_ANSI,
    SET_REVERSE_ORDER,
    SET_USER_NAME,
    SET_REPLY_DIR,
    SET_COUNT
};

struct SettingItem
{
    std::string label;
    std::string value;
    bool isBool;
    bool isChoice;
    int id;
};

std::vector<std::string> findDictionaries(const std::string& baseDir);
std::string showDictionarySelector(const std::string& baseDir, const std::string& currentSelection);
EditorStyle showUIModeDialog(EditorStyle currentStyle);
std::vector<std::string> findThemeFiles(const std::string& baseDir, const std::string& prefix);
std::string showThemeSelector(const std::string& baseDir, EditorStyle currentStyle);
bool showEditorSettings(Settings& settings, const std::string& baseDir);
bool showSettingsDialog(Settings& settings, const std::string& baseDir = "");

#endif // SLYMAIL_SETTINGS_DIALOG_H
