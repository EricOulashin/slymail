#ifndef SLYMAIL_SETTINGS_DIALOG_H
#define SLYMAIL_SETTINGS_DIALOG_H

#include "settings.h"

// ============================================================
// Editor User Settings Dialog
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
// Reader settings dialog (Ctrl-U in reader mode)
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
    SET_ONLY_AREAS_WITH_MAIL,
    SET_STRIP_ANSI,
    SET_ATTR_CODE_TOGGLES,
    SET_REGEX_SEARCH,
    SET_REVERSE_ORDER,
    SET_SPLASH_SCREEN,
    SET_USER_NAME,
    SET_REPLY_DIR,
    SET_USE_EXTERNAL_EDITOR,
    SET_EXTERNAL_EDITORS_LIST,
    SET_SELECT_EXTERNAL_EDITOR,
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
bool showAttrCodeToggles(Settings& settings);

// External editor dialogs
DropFileType showDropFileTypeSelector(DropFileType current);
bool showExternalEditorConfig(ExternalEditorConfig& editor);
bool showExternalEditorsList(Settings& settings);

#endif // SLYMAIL_SETTINGS_DIALOG_H
