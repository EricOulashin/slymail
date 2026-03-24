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
    bool        stripAnsi;          // Strip ANSI codes from messages

    // Message list settings
    bool        lightbarMode;       // Use lightbar (vs traditional) list
    bool        reverseOrder;       // Show newest messages first

    // Theme settings
    std::string iceThemeFile;       // Selected Ice theme filename
    std::string dctThemeFile;       // Selected DCT theme filename

    // General settings
    std::string lastDirectory;      // Last browsed directory
    std::string lastQwkFile;        // Last opened QWK file
    std::string userName;           // User's name for replies
    std::string replyDir;           // Directory for REP packets

    // Constructor
    Settings();

    // Get the full path to the settings file
    static std::string getSettingsPath();

    // Load settings from INI file (next to executable)
    bool load();

    // Save settings to INI file (next to executable) with descriptive comments
    bool save() const;
};

// Helper to convert EditorStyle to display string
std::string editorStyleStr(EditorStyle s);

#endif // SLYMAIL_SETTINGS_H
