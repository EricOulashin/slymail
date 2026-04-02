#include "settings.h"
#include "file_dir_utils.h"

using std::string;
using std::vector;
using std::ifstream;
using std::ofstream;

// Settings INI filename
const char* SETTINGS_FILENAME = "slymail.ini";

// Global settings directory — now defaults to the SlyMail data directory
string& settingsDir()
{
    static string dir;
    if (dir.empty())
    {
        dir = getSlyMailDataDir();
    }
    return dir;
}

// Local trim helper
static string settingsTrimStr(const string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos)
    {
        return "";
    }
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Parse ExtQuoteMode from string
static ExtQuoteMode parseExtQuoteMode(const string& val)
{
    if (val == "Always" || val == "always") return ExtQuoteMode::Always;
    if (val == "Never" || val == "never") return ExtQuoteMode::Never;
    return ExtQuoteMode::Prompt;
}

// Parse DropFileType from string
static DropFileType parseDropFileType(const string& val)
{
    if (val == "MSGINF") return DropFileType::MSGINF;
    if (val == "EDITORINF" || val == "EDITOR.INF") return DropFileType::EDITORINF;
    if (val == "DOORSYS" || val == "DOOR.SYS") return DropFileType::DOORSYS;
    if (val == "DOOR32SYS" || val == "DOOR32.SYS") return DropFileType::DOOR32SYS;
    return DropFileType::None;
}

// Constructor with defaults
Settings::Settings()
    : editorStyle(EditorStyle::Ice)
    , insertMode(true)
    , wrapQuoteLines(true)
    , quoteLineWidth(76)
    , quotePrefix("> ")
    , taglines(false)
    , promptSpellCheck(false)
    , spellDictionaries("")
    , quoteWithInitials(false)
    , indentQuoteInitials(true)
    , trimQuoteSpaces(false)
    , showKludgeLines(false)
    , showTearLine(true)
    , useScrollbar(true)
    , onlyShowAreasWithNewMail(false)
    , stripAnsi(false)
    , attrSynchronet(true)
    , attrWWIV(true)
    , attrCelerity(true)
    , attrRenegade(true)
    , attrPCBoard(true)
    , useRegexSearch(false)
    , lightbarMode(true)
    , reverseOrder(false)
    , iceThemeFile("")
    , dctThemeFile("")
    , showSplashScreen(true)
    , userName("")
    , replyDir("")
    , useExternalEditor(false)
    , selectedEditor("")
{
}

string Settings::getSettingsPath()
{
    return settingsDir() + PATH_SEP_STR + SETTINGS_FILENAME;
}

const ExternalEditorConfig* Settings::getSelectedEditor() const
{
    if (selectedEditor.empty()) return nullptr;
    for (const auto& ed : externalEditors)
    {
        if (ed.name == selectedEditor) return &ed;
    }
    return nullptr;
}

bool Settings::load()
{
    string path = getSettingsPath();
    ifstream f(path);
    if (!f.is_open())
    {
        return false;
    }

    sectionOrder.clear();
    externalEditors.clear();

    string currentSection;
    ExternalEditorConfig* currentEditor = nullptr;

    // For backward compatibility: detect old-style externalEditor key
    string legacyEditorPath;
    ExtQuoteMode legacyQuoteMode = ExtQuoteMode::Prompt;

    string line;
    while (std::getline(f, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        // Record section headers in order
        if (line.size() >= 3 && line[0] == '[')
        {
            auto closing = line.find(']');
            if (closing != string::npos)
            {
                string secName = line.substr(1, closing - 1);
                sectionOrder.push_back(secName);
                currentSection = secName;

                // Check for external_editor: sections
                const string prefix = "external_editor:";
                if (secName.substr(0, prefix.size()) == prefix)
                {
                    externalEditors.push_back(ExternalEditorConfig());
                    currentEditor = &externalEditors.back();
                    currentEditor->name = secName.substr(prefix.size());
                }
                else
                {
                    currentEditor = nullptr;
                }
            }
            continue;
        }
        if (line.empty() || line[0] == '#' || line[0] == ';')
        {
            continue;
        }

        auto eq = line.find('=');
        if (eq == string::npos)
        {
            continue;
        }

        string key = settingsTrimStr(line.substr(0, eq));
        string val = settingsTrimStr(line.substr(eq + 1));

        // If we're inside an external_editor: section, parse editor-specific keys
        if (currentEditor)
        {
            if (key == "startupDir") currentEditor->startupDir = val;
            else if (key == "commandLine") currentEditor->commandLine = val;
            else if (key == "wordWrapQuotedText") currentEditor->wordWrapQuotedText = (val == "true" || val == "1");
            else if (key == "wordWrapNumCols") { try { currentEditor->wordWrapNumCols = std::stoi(val); } catch (...) {} }
            else if (key == "autoQuoteMode") currentEditor->autoQuoteMode = parseExtQuoteMode(val);
            else if (key == "dropFileType") currentEditor->dropFileType = parseDropFileType(val);
            else if (key == "stripFidoKludges") currentEditor->stripFidoKludges = (val == "true" || val == "1");
            continue;
        }

        // Standard settings (not inside an external_editor: section)
        if (key == "editorStyle")
        {
            if (val == "Ice" || val == "ice") editorStyle = EditorStyle::Ice;
            else if (val == "Dct" || val == "dct" || val == "DCT") editorStyle = EditorStyle::Dct;
            else if (val == "Random" || val == "random") editorStyle = EditorStyle::Random;
        }
        else if (key == "insertMode") { insertMode = (val == "true" || val == "1"); }
        else if (key == "wrapQuoteLines") { wrapQuoteLines = (val == "true" || val == "1"); }
        else if (key == "quoteLineWidth") { try { quoteLineWidth = std::stoi(val); } catch (...) {} }
        else if (key == "quotePrefix")
        {
            string rawVal = line.substr(eq + 1);
            while (!rawVal.empty() && (rawVal.back() == '\r' || rawVal.back() == '\n'))
                rawVal.pop_back();
            quotePrefix = rawVal;
        }
        else if (key == "taglines")          { taglines = (val == "true" || val == "1"); }
        else if (key == "promptSpellCheck")  { promptSpellCheck = (val == "true" || val == "1"); }
        else if (key == "spellDictionaries") { spellDictionaries = val; }
        else if (key == "quoteWithInitials") { quoteWithInitials = (val == "true" || val == "1"); }
        else if (key == "indentQuoteInitials") { indentQuoteInitials = (val == "true" || val == "1"); }
        else if (key == "trimQuoteSpaces")   { trimQuoteSpaces = (val == "true" || val == "1"); }
        else if (key == "showKludgeLines")   { showKludgeLines = (val == "true" || val == "1"); }
        else if (key == "showTearLine")      { showTearLine = (val == "true" || val == "1"); }
        else if (key == "useScrollbar")      { useScrollbar = (val == "true" || val == "1"); }
        else if (key == "onlyShowAreasWithNewMail") { onlyShowAreasWithNewMail = (val == "true" || val == "1"); }
        else if (key == "stripAnsi")         { stripAnsi = (val == "true" || val == "1"); }
        else if (key == "attrSynchronet")     { attrSynchronet = (val == "true" || val == "1"); }
        else if (key == "attrWWIV")          { attrWWIV = (val == "true" || val == "1"); }
        else if (key == "attrCelerity")      { attrCelerity = (val == "true" || val == "1"); }
        else if (key == "attrRenegade")      { attrRenegade = (val == "true" || val == "1"); }
        else if (key == "attrPCBoard")       { attrPCBoard = (val == "true" || val == "1"); }
        else if (key == "useRegexSearch")    { useRegexSearch = (val == "true" || val == "1"); }
        else if (key == "lightbarMode")      { lightbarMode = (val == "true" || val == "1"); }
        else if (key == "reverseOrder")      { reverseOrder = (val == "true" || val == "1"); }
        else if (key == "iceThemeFile")      { iceThemeFile = val; }
        else if (key == "dctThemeFile")      { dctThemeFile = val; }
        else if (key == "lastDirectory")     { lastDirectory = val; }
        else if (key == "lastQwkFile")       { lastQwkFile = val; }
        else if (key == "showSplashScreen")  { showSplashScreen = (val == "true" || val == "1"); }
        else if (key == "userName")          { userName = val; }
        else if (key == "replyDir")          { replyDir = val; }
        else if (key == "useExternalEditor") { useExternalEditor = (val == "true" || val == "1"); }
        else if (key == "selectedEditor")    { selectedEditor = val; }
        // Backward compatibility: old-style single external editor
        else if (key == "externalEditor")    { legacyEditorPath = val; }
        else if (key == "externalEditorQuoting") { legacyQuoteMode = parseExtQuoteMode(val); }
    }
    f.close();

    // Backward compatibility: migrate old externalEditor setting
    if (externalEditors.empty() && !legacyEditorPath.empty())
    {
        ExternalEditorConfig migrated;
        migrated.name = "Default";
        migrated.commandLine = legacyEditorPath;
        migrated.autoQuoteMode = legacyQuoteMode;
        externalEditors.push_back(migrated);
        selectedEditor = "Default";
    }

    return true;
}

bool Settings::save() const
{
    string path = getSettingsPath();
    ofstream f(path);
    if (!f.is_open())
    {
        return false;
    }

    f << "; SlyMail Configuration File\n";
    f << "; This file is automatically generated. Manual edits are preserved.\n\n";

    // Helper: write a standard section's settings by name
    auto writeSection = [&](const string& sec, bool& needNewline)
    {
        // Skip external_editor: sections here — they're handled separately
        if (sec.substr(0, 16) == "external_editor:")
            return;

        if (needNewline) f << "\n";
        needNewline = true;

        if (sec == "General")
        {
            f << "[General]\n\n";
            f << "; Show the splash screen when SlyMail starts\n";
            f << "showSplashScreen=" << (showSplashScreen ? "true" : "false") << "\n";
            f << "\n; Last directory browsed for QWK files\n";
            f << "lastDirectory=" << lastDirectory << "\n";
            f << "\n; Last QWK file that was opened\n";
            f << "lastQwkFile=" << lastQwkFile << "\n";
            f << "\n; Your name, used as the \"From\" field when writing replies\n";
            f << "userName=" << userName << "\n";
            f << "\n; Directory where REP reply packets are saved\n";
            f << "; If empty, REP files are saved in the same directory as the QWK file\n";
            f << "replyDir=" << replyDir << "\n";
            f << "\n; Use an external editor instead of the built-in editor\n";
            f << "useExternalEditor=" << (useExternalEditor ? "true" : "false") << "\n";
            f << "\n; Name of the selected external editor (must match an [external_editor:NAME] section)\n";
            f << "selectedEditor=" << selectedEditor << "\n";
        }
        else if (sec == "Themes")
        {
            f << "[Themes]\n\n";
            f << "; Color theme file for the editor in Ice mode\n";
            f << "; Theme files are located in the config/ directory\n";
            f << "iceThemeFile=" << iceThemeFile << "\n";
            f << "\n; Color theme file for the editor in DCT mode\n";
            f << "dctThemeFile=" << dctThemeFile << "\n";
        }
        else if (sec == "Reader")
        {
            f << "[Reader]\n\n";
            f << "; Show kludge/control lines (e.g. @MSGID, @REPLY) in the message reader\n";
            f << "showKludgeLines=" << (showKludgeLines ? "true" : "false") << "\n";
            f << "\n; Show tear lines and origin lines in messages\n";
            f << "showTearLine=" << (showTearLine ? "true" : "false") << "\n";
            f << "\n; Show a scrollbar indicator on the right side of the message reader\n";
            f << "useScrollbar=" << (useScrollbar ? "true" : "false") << "\n";
            f << "\n; Only show conferences that have messages (hide empty conferences)\n";
            f << "onlyShowAreasWithNewMail=" << (onlyShowAreasWithNewMail ? "true" : "false") << "\n";
            f << "\n; Strip ANSI escape codes from message text\n";
            f << "stripAnsi=" << (stripAnsi ? "true" : "false") << "\n";
            f << "\n; Attribute code toggles - enable/disable interpreting color/attribute\n";
            f << "; codes from various BBS software packages. ANSI escape codes are always\n";
            f << "; interpreted unless stripAnsi is enabled above.\n";
            f << "; These settings affect both the message reader and the message editor.\n";
            f << "attrSynchronet=" << (attrSynchronet ? "true" : "false") << "\n";
            f << "attrWWIV=" << (attrWWIV ? "true" : "false") << "\n";
            f << "attrCelerity=" << (attrCelerity ? "true" : "false") << "\n";
            f << "attrRenegade=" << (attrRenegade ? "true" : "false") << "\n";
            f << "attrPCBoard=" << (attrPCBoard ? "true" : "false") << "\n";
            f << "\n; Search using regular expressions instead of plain substring matching\n";
            f << "useRegexSearch=" << (useRegexSearch ? "true" : "false") << "\n";
        }
        else if (sec == "MessageList")
        {
            f << "[MessageList]\n\n";
            f << "; Use lightbar-style navigation in the message list\n";
            f << "lightbarMode=" << (lightbarMode ? "true" : "false") << "\n";
            f << "\n; Show messages in reverse order (newest first)\n";
            f << "reverseOrder=" << (reverseOrder ? "true" : "false") << "\n";
        }
        else if (sec == "Editor")
        {
            f << "[Editor]\n\n";
            f << "; Editor style for writing messages: Ice, Dct, or Random\n";
            f << "; Ice = IceEdit/QuikEdit style, Dct = DCT Edit style\n";
            f << "; Random = randomly choose Ice or Dct each time the editor opens\n";
            switch (editorStyle)
            {
                case EditorStyle::Ice:    f << "editorStyle=Ice\n"; break;
                case EditorStyle::Dct:    f << "editorStyle=Dct\n"; break;
                case EditorStyle::Random: f << "editorStyle=Random\n"; break;
            }
            f << "\n; Default text entry mode: true=insert, false=overwrite\n";
            f << "insertMode=" << (insertMode ? "true" : "false") << "\n";
            f << "\n; Whether to word-wrap quoted lines to the terminal width\n";
            f << "wrapQuoteLines=" << (wrapQuoteLines ? "true" : "false") << "\n";
            f << "\n; Maximum width for quote lines (in characters)\n";
            f << "quoteLineWidth=" << quoteLineWidth << "\n";
            f << "\n; Prefix to prepend to quoted lines. Do not use quotes around the value.\n";
            f << "; The trailing space is significant. Example:  quotePrefix= > \n";
            f << "quotePrefix=" << quotePrefix << "\n";
            f << "\n; Enable tagline insertion when saving a message\n";
            f << "; When enabled, prompts to select or randomly choose a tagline from\n";
            f << "; tagline_files/taglines.txt, which is appended to the message with \"...\" prefix\n";
            f << "taglines=" << (taglines ? "true" : "false") << "\n";
            f << "\n; Prompt the user to run the spell checker when saving a message\n";
            f << "promptSpellCheck=" << (promptSpellCheck ? "true" : "false") << "\n";
            f << "\n; Comma-separated list of dictionary filenames for spell checking\n";
            f << "; Dictionary files are located in the dictionary_files/ directory\n";
            f << "spellDictionaries=" << spellDictionaries << "\n";
            f << "\n; Prepend the author's initials to quote line prefixes when replying\n";
            f << "; e.g. if replying to Mike Powell, quote prefix becomes \"MP> \"\n";
            f << "quoteWithInitials=" << (quoteWithInitials ? "true" : "false") << "\n";
            f << "\n; When quoting with author's initials, add a leading space\n";
            f << "; e.g. \" MP> \" instead of \"MP> \"\n";
            f << "indentQuoteInitials=" << (indentQuoteInitials ? "true" : "false") << "\n";
            f << "\n; Strip leading whitespace from lines when quoting a message\n";
            f << "trimQuoteSpaces=" << (trimQuoteSpaces ? "true" : "false") << "\n";
        }
    };

    // Default section order for new files (no INI loaded yet)
    const vector<string> defaultOrder = {
        "General", "Themes", "Reader", "MessageList", "Editor"
    };

    // Use the order from the loaded INI file, or the default if none was loaded
    const vector<string>& order = sectionOrder.empty() ? defaultOrder : sectionOrder;

    // Write sections in the remembered order
    bool needNewline = false;
    vector<bool> written(defaultOrder.size(), false);
    for (const auto& sec : order)
    {
        writeSection(sec, needNewline);
        for (size_t i = 0; i < defaultOrder.size(); ++i)
        {
            if (defaultOrder[i] == sec) written[i] = true;
        }
    }

    // Append any standard sections that weren't in the loaded order
    for (size_t i = 0; i < defaultOrder.size(); ++i)
    {
        if (!written[i])
        {
            writeSection(defaultOrder[i], needNewline);
        }
    }

    // Write external editor sections
    for (const auto& ed : externalEditors)
    {
        if (ed.name.empty()) continue;
        f << "\n[external_editor:" << ed.name << "]\n";
        f << "startupDir=" << ed.startupDir << "\n";
        f << "commandLine=" << ed.commandLine << "\n";
        f << "wordWrapQuotedText=" << (ed.wordWrapQuotedText ? "true" : "false") << "\n";
        f << "wordWrapNumCols=" << ed.wordWrapNumCols << "\n";
        f << "autoQuoteMode=" << extQuoteModeStr(ed.autoQuoteMode) << "\n";
        f << "dropFileType=" << dropFileTypeStr(ed.dropFileType) << "\n";
        f << "stripFidoKludges=" << (ed.stripFidoKludges ? "true" : "false") << "\n";
    }

    f.close();
    return true;
}

string editorStyleStr(EditorStyle s)
{
    switch (s)
    {
        case EditorStyle::Ice:    return "Ice";
        case EditorStyle::Dct:    return "DCT";
        case EditorStyle::Random: return "Random";
    }
    return "Unknown";
}

string dropFileTypeStr(DropFileType t)
{
    switch (t)
    {
        case DropFileType::None:      return "None";
        case DropFileType::MSGINF:    return "MSGINF";
        case DropFileType::EDITORINF: return "EDITOR.INF";
        case DropFileType::DOORSYS:   return "DOOR.SYS";
        case DropFileType::DOOR32SYS: return "DOOR32.SYS";
    }
    return "None";
}

string extQuoteModeStr(ExtQuoteMode m)
{
    switch (m)
    {
        case ExtQuoteMode::Always: return "Always";
        case ExtQuoteMode::Prompt: return "Prompt";
        case ExtQuoteMode::Never:  return "Never";
    }
    return "Prompt";
}
