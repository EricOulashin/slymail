#ifndef SLYMAIL_SETTINGS_H
#define SLYMAIL_SETTINGS_H

#include "terminal.h"

// Editor style choices
enum class EditorStyle
{
    Ice,
    Dct,
    Random   // Randomly choose Ice or Dct on each edit
};

// External editor quoting mode for replies
enum class ExtQuoteMode
{
    Always,  // Always include quote lines when replying with external editor
    Prompt,  // Ask the user whether to include quote lines
    Never    // Never include quote lines when replying with external editor
};

// Drop file types for external message editors
enum class DropFileType
{
    None,       // No drop file
    MSGINF,     // QuickBBS-style MSGINF (used by SlyEdit)
    EDITORINF,  // EDITOR.INF (Synchronet standard)
    DOORSYS,    // DOOR.SYS
    DOOR32SYS,  // DOOR32.SYS
};

// Configuration for a single external editor
struct ExternalEditorConfig
{
    std::string  name;
    std::string  startupDir;
    std::string  commandLine;        // %f = temp filename substitution
    bool         wordWrapQuotedText;  // default true
    int          wordWrapNumCols;     // default 79
    ExtQuoteMode autoQuoteMode;       // Always/Prompt/Never
    DropFileType dropFileType;        // default None
    bool         stripFidoKludges;    // default false

    ExternalEditorConfig()
        : wordWrapQuotedText(true)
        , wordWrapNumCols(79)
        , autoQuoteMode(ExtQuoteMode::Prompt)
        , dropFileType(DropFileType::None)
        , stripFidoKludges(false)
    {}
};

// The settings INI filename
extern const char* SETTINGS_FILENAME;

// Global: directory where the executable lives (set in main())
// Settings file is stored here alongside the executable.
std::string& settingsDir();

// User settings for SlyMail
struct Settings
{
    // Editor settings
    EditorStyle editorStyle;
    bool        insertMode;         // Default insert mode (vs overwrite)
    bool        wrapQuoteLines;     // Wrap long quote lines
    int         quoteLineWidth;     // Max width for quote lines
    std::string quotePrefix;        // Quote line prefix (default "> ")
    bool        taglines;           // Enable tagline insertion on save
    bool        promptSpellCheck;   // Prompt for spell-check on save
    std::string spellDictionaries;  // Comma-separated dictionary filenames
    bool        quoteWithInitials;  // Prepend author's initials to quote prefix
    bool        indentQuoteInitials; // Indent quote lines with initials (space before)
    bool        trimQuoteSpaces;    // Trim leading whitespace from quoted lines

    // Reader settings
    bool        showKludgeLines;    // Show kludge/control lines
    bool        showTearLine;       // Show tear/origin lines
    bool        useScrollbar;       // Show scrollbar in reader
    bool        onlyShowAreasWithNewMail; // Only show conferences with messages

    // Attribute code toggles (ANSI is always enabled unless stripAnsi is set)
    bool        stripAnsi;          // Strip all ANSI codes from messages
    bool        attrSynchronet;     // Interpret Synchronet Ctrl-A codes
    bool        attrWWIV;           // Interpret WWIV heart codes
    bool        attrCelerity;       // Interpret Celerity pipe codes
    bool        attrRenegade;       // Interpret Renegade pipe codes
    bool        attrPCBoard;        // Interpret PCBoard/Wildcat @X codes

    // Search settings
    bool        useRegexSearch;     // Treat search text as regex (vs substring)

    // Message list settings
    bool        lightbarMode;       // Use lightbar (vs traditional) list
    bool        reverseOrder;       // Show newest messages first

    // Theme settings
    std::string iceThemeFile;       // Selected Ice theme filename
    std::string dctThemeFile;       // Selected DCT theme filename

    // General settings
    bool        showSplashScreen;   // Show the splash screen on startup
    std::string lastDirectory;      // Last browsed directory
    std::string lastQwkFile;        // Last opened QWK file
    std::string userName;           // User's name for replies
    std::string replyDir;           // Directory for REP packets
    bool        useExternalEditor;  // Use external editor instead of built-in
    std::string selectedEditor;     // Name of the active external editor
    std::string language;           // UI language code ("" = OS default)

    // External editor configurations
    std::vector<ExternalEditorConfig> externalEditors;

    // Section order as read from the INI file (preserved on save)
    std::vector<std::string> sectionOrder;

    // Constructor
    Settings();

    // Get the full path to the settings file
    static std::string getSettingsPath();

    // Load settings from INI file (next to executable)
    bool load();

    // Save settings to INI file (next to executable) with descriptive comments
    bool save() const;

    // Look up the selected external editor config (or nullptr if none)
    const ExternalEditorConfig* getSelectedEditor() const;
};

// Helper to convert EditorStyle to display string
std::string editorStyleStr(EditorStyle s);

// Helper to convert DropFileType to display string
std::string dropFileTypeStr(DropFileType t);

// Helper to convert ExtQuoteMode to display string
std::string extQuoteModeStr(ExtQuoteMode m);

#endif // SLYMAIL_SETTINGS_H
