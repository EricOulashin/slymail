// SlyMail - QWK Offline Mail Reader
// A cross-platform text-based QWK offline mail reader
// UI inspired by DDMsgReader.js (reading) and SlyEdit.js (editing)
//
// Copyright (C) 2024 - Licensed under MIT License

#include "terminal.h"
#include "qwk.h"
#include "settings.h"
#include "ui_common.h"
#include "file_browser.h"
#include "msg_list.h"
#include "msg_reader.h"
#include "msg_editor.h"
#include "settings_dialog.h"
#include "program_info.h"
#include "voting.h"
#include "remote_systems.h"
#include "text_utils.h"

#include <ctime>
#include <optional>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <filesystem>

using std::string;
using std::vector;
using std::optional;

// Show the splash/title screen
void showSplashScreen()
{
    g_term->clear();

    int centerY = g_term->getRows() / 2 - 6;
    if (centerY < 1)
    {
        centerY = 1;
    }

    // Title art
    printCentered(centerY,     "  ____  _       __  __       _ _ ",
        tAttr(TC_CYAN, TC_BLACK, true));
    printCentered(centerY + 1, " / ___|| |_   _|  \\/  | __ _(_) |",
        tAttr(TC_CYAN, TC_BLACK, true));
    printCentered(centerY + 2, " \\___ \\| | | | | |\\/| |/ _` | | |",
        tAttr(TC_CYAN, TC_BLACK, true));
    printCentered(centerY + 3, "  ___) | | |_| | |  | | (_| | | |",
        tAttr(TC_CYAN, TC_BLACK, false));
    printCentered(centerY + 4, " |____/|_|\\__, |_|  |_|\\__,_|_|_|",
        tAttr(TC_CYAN, TC_BLACK, false));
    printCentered(centerY + 5, "          |___/                   ",
        tAttr(TC_CYAN, TC_BLACK, false));

    printCentered(centerY + 7,
        "QWK Offline Mail Reader v" + string(PROGRAM_VERSION),
        tAttr(TC_WHITE, TC_BLACK, true));

    printCentered(centerY + 9,
        "Reading UI inspired by DDMsgReader  |  Editor UI inspired by SlyEdit",
        tAttr(TC_GREEN, TC_BLACK, false));

    printCentered(centerY + 11,
        "Press any key to continue...",
        tAttr(TC_YELLOW, TC_BLACK, false));

    g_term->refresh();
    g_term->getKey();
}

// Print command-line help and exit
static void showCommandLineHelp()
{
    printf("%s v%s (%s)\n", PROGRAM_NAME, PROGRAM_VERSION, PROGRAM_DATE);
    printf("QWK Offline Mail Reader\n\n");
    printf("Usage: slymail [options] [qwk_file]\n\n");
    printf("Options:\n");
    printf("  -qwk_file=<path>   Open the specified QWK packet file on startup\n");
    printf("  -v, --version      Show version information and exit\n");
    printf("  -?, -help, --help  Show this help message and exit\n\n");
    printf("Examples:\n");
    printf("  slymail                              Launch with file browser\n");
    printf("  slymail MYBBS.qwk                    Open a QWK packet directly\n");
    printf("  slymail -qwk_file=/path/to/FILE.qwk  Open a QWK packet by path\n\n");
    printf("Data directory: ~/.slymail\n");
    printf("  Settings:    ~/.slymail/slymail.ini\n");
    printf("  QWK files:   ~/.slymail/QWK/\n");
    printf("  Remote sys:  ~/.slymail/remote_systems.json\n");
}

// Check if an argument is a help flag
static bool isHelpArg(const char* arg)
{
    return strcmp(arg, "-?") == 0
        || strcmp(arg, "--?") == 0
        || strcmp(arg, "/?") == 0
        || strcmp(arg, "-help") == 0
        || strcmp(arg, "--help") == 0
        || strcmp(arg, "/help") == 0;
}

// Check if an argument is a version flag
static bool isVersionArg(const char* arg)
{
    return strcmp(arg, "-v") == 0
        || strcmp(arg, "--version") == 0;
}

// Build quote lines from a message for the external editor.
// Replicates the quoting logic from MessageEditor::prepareQuotes.
static string buildQuoteText(const QwkMessage& msg, const Settings& settings)
{
    // Build quote prefix
    string prefix;
    if (settings.quoteWithInitials)
    {
        string initials;
        if (!msg.from.empty())
        {
            auto spacePos = msg.from.rfind(' ');
            if (spacePos != string::npos && spacePos + 1 < msg.from.size())
            {
                initials += static_cast<char>(toupper(static_cast<unsigned char>(msg.from[0])));
                initials += static_cast<char>(toupper(static_cast<unsigned char>(msg.from[spacePos + 1])));
            }
            else
            {
                initials += static_cast<char>(toupper(static_cast<unsigned char>(msg.from[0])));
                if (msg.from.size() >= 2)
                    initials += static_cast<char>(toupper(static_cast<unsigned char>(msg.from[1])));
            }
        }
        if (settings.indentQuoteInitials)
            prefix = " " + initials + "> ";
        else
            prefix = initials + "> ";
    }
    else
    {
        prefix = settings.quotePrefix;
    }

    // Build quote lines with prefix
    vector<string> rawQuoteLines;
    std::istringstream stream(msg.body);
    string line;
    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        // Skip kludge lines
        if (!line.empty() && line[0] == '@') continue;
        if (line.find("SEEN-BY:") == 0) continue;
        if (line.find("PATH:") == 0) continue;
        // Trim leading spaces if setting enabled
        if (settings.trimQuoteSpaces)
        {
            size_t start = line.find_first_not_of(" \t");
            if (start != string::npos)
                line = line.substr(start);
            else
                line.clear();
        }
        rawQuoteLines.push_back(prefix + line);
    }

    // Re-wrap quote lines to fit within the configured width, matching
    // the built-in editor's prepareQuotes behavior
    vector<string> finalLines;
    int quoteMaxWidth = settings.quoteLineWidth;
    if (quoteMaxWidth <= 0) quoteMaxWidth = 79;
    if (settings.wrapQuoteLines)
        finalLines = wrapQuoteLines(rawQuoteLines, quoteMaxWidth);
    else
        finalLines = rawQuoteLines;

    // Join into a single string
    string result;
    for (size_t i = 0; i < finalLines.size(); ++i)
    {
        result += finalLines[i];
        result += '\n';
    }
    // Add an empty line after the quote block for the user to start typing
    if (!result.empty())
        result += '\n';
    return result;
}

// Run an external editor to compose a message.
// If initialContent is non-empty, it is written to the temp file before launching the editor.
// Returns the message body text, or empty string if aborted.
// Result from running an external editor
struct ExtEditorResult
{
    string body;           // Message body text (empty if aborted)
    string resultSubject;  // Subject from RESULT.ED line 2 (may be empty)
    string editorDetails;  // Editor details from RESULT.ED line 3 (may be empty)
};

static ExtEditorResult runExternalEditor(const ExternalEditorConfig& edCfg,
                                         const string& initialContent = "")
{
    namespace fs = std::filesystem;
    ExtEditorResult result;

    // Create a temporary file in the .slymail directory
    string dataDir = getSlyMailDataDir();
    string tmpFile = dataDir + PATH_SEP_STR + "slymail_edit.tmp";
    string resultEdFile = dataDir + PATH_SEP_STR + "RESULT.ED";

    // Remove any stale RESULT.ED from a previous session
    {
        std::error_code ec;
        fs::remove(resultEdFile, ec);
    }

    // Create the temp file with optional initial content (e.g. quote lines)
    {
        std::ofstream ofs(tmpFile, std::ios::trunc);
        if (!ofs.is_open())
        {
            messageDialog("Error", "Failed to create temporary file.");
            return result;
        }
        if (!initialContent.empty())
        {
            ofs << initialContent;
        }
    }

    // Build the command line from the editor config
    // Concatenate startupDir + PATH_SEP + commandLine, and substitute %f
    string editorCmd = edCfg.commandLine;
    string fullPath;
    if (!edCfg.startupDir.empty())
    {
        fullPath = edCfg.startupDir;
        // Only add a separator if startupDir doesn't already end with one
        if (fullPath.back() != PATH_SEP)
        {
            fullPath += PATH_SEP_STR;
        }
        fullPath += editorCmd;
    }
    else
    {
        fullPath = editorCmd;
    }

    // Replace %f with the quoted temp file path
    string quotedTmpFile = "\"" + tmpFile + "\"";
    string cmd;
    auto fPos = fullPath.find("%f");
    if (fPos != string::npos)
    {
        cmd = fullPath.substr(0, fPos) + quotedTmpFile + fullPath.substr(fPos + 2);
    }
    else
    {
        // No %f — append the temp file as argument
        cmd = "\"" + fullPath + "\" " + quotedTmpFile;
    }

    // Suspend the terminal so the external editor can use it
    g_term->shutdown();

    int exitCode = std::system(cmd.c_str());

    // Restore the terminal
    g_term->init();

    if (exitCode == 0)
    {
        // Read the temp file contents
        std::ifstream ifs(tmpFile);
        if (ifs.is_open())
        {
            string line;
            while (std::getline(ifs, line))
            {
                if (!result.body.empty())
                {
                    result.body += '\n';
                }
                result.body += line;
            }
        }
        if (result.body.empty())
        {
            messageDialog("Message Aborted", "The message file was empty. Message not posted.");
        }
    }
    else
    {
        messageDialog("Message Aborted", "The external editor did not exit successfully. Message not posted.");
    }

    // Read RESULT.ED if it exists (Synchronet-compatible editor result file)
    // Line 1: ignored
    // Line 2: new subject (if editor changed it)
    // Line 3: editor details/identifier string
    {
        std::ifstream rif(resultEdFile);
        if (rif.is_open())
        {
            string line1, line2, line3;
            if (std::getline(rif, line1))     // Line 1 — ignored
            {
                if (std::getline(rif, line2)) // Line 2 — subject
                {
                    while (!line2.empty() && (line2.back() == '\r' || line2.back() == ' '))
                        line2.pop_back();
                    if (!line2.empty())
                        result.resultSubject = line2;

                    if (std::getline(rif, line3)) // Line 3 — editor details
                    {
                        while (!line3.empty() && (line3.back() == '\r' || line3.back() == ' '))
                            line3.pop_back();
                        if (!line3.empty())
                            result.editorDetails = line3;
                    }
                }
            }
        }
        // Clean up RESULT.ED
        std::error_code ec;
        fs::remove(resultEdFile, ec);
    }

    // Clean up temp file
    std::error_code ec;
    fs::remove(tmpFile, ec);

    return result;
}

// Try external editor; if it fails to launch, offer built-in editor fallback.
// Returns EditorResult and populates reply.
static EditorResult tryExternalOrBuiltinReply(
    const QwkMessage& origMsg,
    const string& userName,
    const string& confName,
    QwkReply& reply,
    Settings& settings,
    const string& baseDir)
{
    const auto* edCfg = settings.getSelectedEditor();
    if (!edCfg)
    {
        // No editor selected — fall back to built-in
        return editReply(origMsg, userName, confName, reply, settings, baseDir);
    }

    // Determine whether to include quote lines based on editor's auto-quote mode
    string initialContent;
    if (edCfg->autoQuoteMode == ExtQuoteMode::Always)
    {
        initialContent = buildQuoteText(origMsg, settings);
    }
    else if (edCfg->autoQuoteMode == ExtQuoteMode::Prompt)
    {
        if (confirmDialog("Include quoted text from original message?"))
        {
            initialContent = buildQuoteText(origMsg, settings);
        }
    }

    auto edResult = runExternalEditor(*edCfg, initialContent);
    if (edResult.body.empty())
    {
        return EditorResult::Aborted;
    }

    // Build the reply from the external editor output
    reply.conference = origMsg.conference;
    reply.to = origMsg.from;
    reply.from = userName;
    // Use subject from RESULT.ED if the editor provided one, otherwise derive from original
    if (!edResult.resultSubject.empty())
    {
        reply.subject = edResult.resultSubject;
    }
    else
    {
        reply.subject = origMsg.subject;
        if (reply.subject.substr(0, 4) != "Re: " &&
            reply.subject.substr(0, 3) != "Re:")
        {
            if (reply.subject.size() > 21)
            {
                reply.subject = reply.subject.substr(0, 21);
            }
            reply.subject = "Re: " + reply.subject;
        }
    }
    reply.body = edResult.body;
    reply.replyToNum = origMsg.number;
    // Use editor details from RESULT.ED if provided
    if (!edResult.editorDetails.empty())
        reply.editor = edResult.editorDetails;
    else
        reply.editor = string(PROGRAM_NAME) + " " + PROGRAM_VERSION
                     + " (" + PROGRAM_DATE + ") (external editor: " + edCfg->name + ")";
    return EditorResult::Saved;
}

static EditorResult tryExternalOrBuiltinNewMsg(
    const string& userName,
    const string& confName,
    int confNumber,
    QwkReply& reply,
    Settings& settings,
    const string& baseDir)
{
    // Prompt for To and Subject first
    int cols = g_term->getCols();
    int rows = g_term->getRows();
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

    printAt(boxY + 5, boxX + 3, "Subject: ", tAttr(TC_CYAN, TC_BLACK, true));
    string subj = getStringInput(boxY + 5, boxX + 12, 25, "",
        tAttr(TC_WHITE, TC_BLACK, true));
    if (subj.empty())
    {
        return EditorResult::Aborted;
    }

    const auto* edCfg = settings.getSelectedEditor();
    if (!edCfg)
    {
        return editNewMessage(userName, confName, confNumber, reply, settings, baseDir);
    }

    auto edResult = runExternalEditor(*edCfg);
    if (edResult.body.empty())
    {
        return EditorResult::Aborted;
    }

    reply.conference = confNumber;
    reply.to = to;
    reply.from = userName;
    // Use subject from RESULT.ED if the editor provided one, otherwise use what user entered
    reply.subject = !edResult.resultSubject.empty() ? edResult.resultSubject : subj;
    reply.body = edResult.body;
    reply.replyToNum = 0;
    if (!edResult.editorDetails.empty())
        reply.editor = edResult.editorDetails;
    else
        reply.editor = string(PROGRAM_NAME) + " " + PROGRAM_VERSION
                     + " (" + PROGRAM_DATE + ") (external editor: " + edCfg->name + ")";
    return EditorResult::Saved;
}

// Main application loop
int main(int argc, char* argv[])
{
    // Check for help/version flags before initializing anything
    for (int i = 1; i < argc; ++i)
    {
        if (isHelpArg(argv[i]))
        {
            showCommandLineHelp();
            return 0;
        }
        if (isVersionArg(argv[i]))
        {
            printf("%s v%s (%s)\n", PROGRAM_NAME, PROGRAM_VERSION, PROGRAM_DATE);
            return 0;
        }
    }

    // Seed random for Random editor style
    srand(static_cast<unsigned>(time(nullptr)));

    // Determine base directory (where the executable lives)
    // Theme files, dictionaries, and taglines are relative to this
    string baseDir = ".";
    if (argc > 0 && argv[0] != nullptr)
    {
        fs::path exePath = fs::path(argv[0]).parent_path();
        if (!exePath.empty())
        {
            baseDir = exePath.string();
        }
    }

    // Settings are stored in the SlyMail data directory (~/.slymail)
    // getSlyMailDataDir() creates it and the QWK subdirectory if needed
    string dataDir = getSlyMailDataDir();
    settingsDir() = dataDir;

    // Initialize terminal
    auto terminal = createTerminal();
    g_term = terminal.get();
    g_term->init();

    // Load settings
    Settings settings;
    settings.load();

    // Parse command-line parameters before showing splash screen
    // Supports: -name=value format (e.g., -qwk_file=/path/to/file.qwk)
    // Also supports a bare positional argument for backward compatibility
    string initialFile;
    for (int i = 1; i < argc; ++i)
    {
        string arg = argv[i];
        if (arg.size() > 1 && arg[0] == '-')
        {
            // Named parameter: -name=value
            size_t eqPos = arg.find('=');
            if (eqPos != string::npos)
            {
                string name = arg.substr(1, eqPos - 1);
                string value = arg.substr(eqPos + 1);
                if (name == "qwk_file")
                {
                    initialFile = value;
                }
            }
        }
        else if (initialFile.empty())
        {
            // Bare positional argument (backward compatibility)
            initialFile = arg;
        }
    }

    // Show splash screen if enabled and no QWK file was specified on the command line
    if (initialFile.empty() && settings.showSplashScreen)
    {
        showSplashScreen();
    }

    // Main application state
    optional<QwkPacket> currentPacket;
    vector<QwkReply> pendingReplies;
    vector<PendingVote> pendingVotes;
    bool running = true;
    bool repPacketSaved = true; // true = no unsaved messages since last REP save

    // Helper: save the REP packet from current pending replies/votes
    auto saveRepPacket = [&]() -> bool
    {
        if (pendingReplies.empty() && pendingVotes.empty())
        {
            messageDialog("REP Packet", "No pending messages or votes to save.");
            return false;
        }
        if (!currentPacket.has_value())
        {
            messageDialog("Error", "No QWK packet loaded.");
            return false;
        }
        string repDir = settings.replyDir;
        if (repDir.empty()) repDir = dataDir + PATH_SEP_STR + "REP";
        string repFile = repDir + PATH_SEP_STR + currentPacket->info.bbsID + ".rep";
        if (createRepPacket(repFile, currentPacket->info.bbsID,
                            settings.userName, pendingReplies, pendingVotes))
        {
            messageDialog("REP Saved", "Reply packet saved: " + repFile);
            repPacketSaved = true;
            return true;
        }
        else
        {
            messageDialog("Error", "Failed to create REP packet.");
            return false;
        }
    };

    while (running)
    {
        // If no packet is open, show the file browser
        if (!currentPacket.has_value())
        {
            string qwkFile;
            if (!initialFile.empty())
            {
                qwkFile = initialFile;
                initialFile.clear(); // Only use once
            }
            else
            {
                // Default browse directory is the QWK subdirectory in the data dir
                string browseDir = settings.lastDirectory;
                if (browseDir.empty())
                {
                    browseDir = dataDir + PATH_SEP_STR + "QWK";
                }
                qwkFile = showFileBrowser(browseDir, settings.lastQwkFile);
            }

            if (qwkFile.empty())
            {
                running = false;
                break;
            }

            // Remember the directory and filename
            settings.lastDirectory = fs::path(qwkFile).parent_path().string();
            settings.lastQwkFile = qwkFile;
            settings.save();

            // Show loading message
            g_term->clear();
            printCentered(g_term->getRows() / 2, "Loading QWK packet...",
                tAttr(TC_CYAN, TC_BLACK, true));
            printCentered(g_term->getRows() / 2 + 1, qwkFile,
                tAttr(TC_WHITE, TC_BLACK, false));
            g_term->refresh();

            // Parse the QWK file
            currentPacket = parseQwkFile(qwkFile);
            if (!currentPacket.has_value())
            {
                messageDialog("Error", "Failed to open or parse QWK file.");
                continue;
            }

            // Set username from packet if not already set
            if (settings.userName.empty())
            {
                settings.userName = currentPacket->info.userName;
            }

            // Clear pending replies for new packet
            pendingReplies.clear();


        }

        // Conference list loop
        bool inConfList = true;
        int selectedConf = 0;
        while (inConfList && running)
        {
            ConfListResult confResult = showConferenceList(*currentPacket,
                                                           selectedConf, settings);

            switch (confResult)
            {
                case ConfListResult::Selected:
                {
                    // Enter the selected conference
                    auto& conf = currentPacket->conferences[selectedConf];
                    bool inMsgList = true;
                    int selectedMsg = 0;

                    while (inMsgList && running)
                    {
                        MsgListResult msgResult = showMessageList(conf, selectedMsg,
                            settings, currentPacket->info.bbsName);

                        switch (msgResult)
                        {
                            case MsgListResult::ReadMessage:
                            {
                                // Read messages starting from selected
                                int currentMsg = selectedMsg;
                                bool inReader = true;

                                while (inReader && running)
                                {
                                    if (currentMsg < 0)
                                    {
                                        currentMsg = 0;
                                    }
                                    if (currentMsg >= static_cast<int>(conf.messages.size()))
                                    {
                                        currentMsg = static_cast<int>(conf.messages.size()) - 1;
                                    }

                                    if (conf.messages.empty())
                                    {
                                        inReader = false;
                                        break;
                                    }

                                    PendingVote lastVote;
                                    MsgReadResult readResult = showMessageReader(
                                        conf.messages[currentMsg],
                                        conf.name,
                                        currentMsg,
                                        static_cast<int>(conf.messages.size()),
                                        settings,
                                        currentPacket->extractDir,
                                        &currentPacket->voting,
                                        &lastVote);

                                    switch (readResult)
                                    {
                                        case MsgReadResult::NextMsg:
                                            if (currentMsg < static_cast<int>(conf.messages.size()) - 1)
                                            {
                                                ++currentMsg;
                                            }
                                            break;
                                        case MsgReadResult::PrevMsg:
                                            if (currentMsg > 0)
                                            {
                                                --currentMsg;
                                            }
                                            break;
                                        case MsgReadResult::FirstMsg:
                                            currentMsg = 0;
                                            break;
                                        case MsgReadResult::LastMsg:
                                            currentMsg = static_cast<int>(conf.messages.size()) - 1;
                                            break;
                                        case MsgReadResult::Reply:
                                        {
                                            QwkReply reply;
                                            EditorResult edResult;
                                            if (settings.useExternalEditor && settings.getSelectedEditor())
                                            {
                                                edResult = tryExternalOrBuiltinReply(
                                                    conf.messages[currentMsg],
                                                    settings.userName,
                                                    conf.name,
                                                    reply,
                                                    settings,
                                                    baseDir);
                                            }
                                            else
                                            {
                                                edResult = editReply(
                                                    conf.messages[currentMsg],
                                                    settings.userName,
                                                    conf.name,
                                                    reply,
                                                    settings,
                                                    baseDir);
                                            }
                                            if (edResult == EditorResult::Saved)
                                            {
                                                pendingReplies.push_back(reply);
                                                repPacketSaved = false;
                                                messageDialog("Reply Saved",
                                                    "Reply queued. " +
                                                    std::to_string(pendingReplies.size()) +
                                                    " pending reply(s).");
                                                if (confirmDialog("Save REP packet now?"))
                                                {
                                                    saveRepPacket();
                                                }
                                            }
                                            break;
                                        }
                                        case MsgReadResult::Vote:
                                        {
                                            pendingVotes.push_back(lastVote);
                                            repPacketSaved = false;
                                            // Update the message's userVoted flag locally
                                            auto& votedMsg = conf.messages[currentMsg];
                                            if (lastVote.upVote)
                                            {
                                                votedMsg.userVoted = 1;
                                                ++votedMsg.upvotes;
                                            }
                                            else if (lastVote.downVote)
                                            {
                                                votedMsg.userVoted = 2;
                                                ++votedMsg.downvotes;
                                            }
                                            else if (lastVote.votes != 0)
                                            {
                                                votedMsg.userVoted = lastVote.votes;
                                            }
                                            string voteDesc;
                                            if (lastVote.upVote)
                                                voteDesc = "Up-vote";
                                            else if (lastVote.downVote)
                                                voteDesc = "Down-vote";
                                            else
                                                voteDesc = "Poll vote";
                                            messageDialog("Vote Queued",
                                                voteDesc + " queued. " +
                                                std::to_string(pendingVotes.size()) +
                                                " pending vote(s).");
                                            break;
                                        }
                                        case MsgReadResult::Settings:
                                            showSettingsDialog(settings, baseDir);
                                            break;
                                        case MsgReadResult::Back:
                                            inReader = false;
                                            break;
                                        case MsgReadResult::Quit:
                                            inReader = false;
                                            inMsgList = false;
                                            inConfList = false;
                                            running = false;
                                            break;
                                    }
                                }
                                break;
                            }
                            case MsgListResult::NewMessage:
                            {
                                QwkReply reply;
                                EditorResult edResult;
                                if (settings.useExternalEditor && settings.getSelectedEditor())
                                {
                                    edResult = tryExternalOrBuiltinNewMsg(
                                        settings.userName,
                                        conf.name,
                                        conf.number,
                                        reply,
                                        settings,
                                        baseDir);
                                }
                                else
                                {
                                    edResult = editNewMessage(
                                        settings.userName,
                                        conf.name,
                                        conf.number,
                                        reply,
                                        settings,
                                        baseDir);
                                }
                                if (edResult == EditorResult::Saved)
                                {
                                    pendingReplies.push_back(reply);
                                    repPacketSaved = false;
                                    messageDialog("Message Saved",
                                        "Message queued. " +
                                        std::to_string(pendingReplies.size()) +
                                        " pending message(s).");
                                    if (confirmDialog("Save REP packet now?"))
                                    {
                                        saveRepPacket();
                                    }
                                }
                                break;
                            }
                            case MsgListResult::Back:
                                inMsgList = false;
                                break;
                            case MsgListResult::Settings:
                                showSettingsDialog(settings, baseDir);
                                // If onlyShowAreasWithNewMail was enabled and this
                                // conference has 0 messages, back out to the conference list
                                if (settings.onlyShowAreasWithNewMail && conf.messages.empty())
                                {
                                    inMsgList = false;
                                }
                                break;
                            case MsgListResult::OpenFile:
                                inMsgList = false;
                                inConfList = false;
                                // Will fall through to OpenFile handling
                                // by resetting the packet
                                if (!pendingReplies.empty())
                                {
                                    if (confirmDialog("Save pending replies before opening new file?"))
                                    {
                                        string repDir = settings.replyDir;
                                        if (repDir.empty())
                                        {
                                            repDir = dataDir + PATH_SEP_STR + "REP";
                                        }
                                        string repFile = repDir + PATH_SEP_STR
                                            + currentPacket->info.bbsID + ".rep";
                                        createRepPacket(repFile, currentPacket->info.bbsID,
                                                        settings.userName, pendingReplies,
                                                        pendingVotes);
                                    }
                                    pendingReplies.clear();
                                    pendingVotes.clear();
                                }
                                if (currentPacket.has_value())
                                {
                                    cleanupTempDir(currentPacket->extractDir);
                                }
                                currentPacket.reset();
                                break;
                            case MsgListResult::RemoteSystems:
                            {
                                string downloadDir = dataDir + PATH_SEP_STR + "QWK";
                                string downloaded = showRemoteSystems(dataDir, downloadDir);
                                if (!downloaded.empty())
                                {
                                    if (!pendingReplies.empty() || !pendingVotes.empty())
                                    {
                                        if (confirmDialog("Save pending items before opening new file?"))
                                        {
                                            string repDir = settings.replyDir;
                                            if (repDir.empty()) repDir = dataDir + PATH_SEP_STR + "REP";
                                            string repFile = repDir + PATH_SEP_STR + currentPacket->info.bbsID + ".rep";
                                            createRepPacket(repFile, currentPacket->info.bbsID,
                                                            settings.userName, pendingReplies, pendingVotes);
                                        }
                                        pendingReplies.clear();
                                        pendingVotes.clear();
                                    }
                                    if (currentPacket.has_value())
                                    {
                                        cleanupTempDir(currentPacket->extractDir);
                                    }
                                    currentPacket.reset();
                                    initialFile = downloaded;
                                    inMsgList = false;
                                    inConfList = false;
                                }
                                break;
                            }
                            case MsgListResult::SaveRep:
                                saveRepPacket();
                                break;
                            case MsgListResult::Quit:
                                inMsgList = false;
                                inConfList = false;
                                running = false;
                                break;
                        }
                    }
                    break;
                }
                case ConfListResult::OpenFile:
                    // Save pending replies first if any
                    if (!pendingReplies.empty())
                    {
                        if (confirmDialog("Save pending replies before opening new file?"))
                        {
                            string repDir = settings.replyDir;
                            if (repDir.empty())
                            {
                                repDir = dataDir + PATH_SEP_STR + "REP";
                            }
                            string repFile = repDir + PATH_SEP_STR
                                + currentPacket->info.bbsID + ".rep";
                            if (createRepPacket(repFile, currentPacket->info.bbsID,
                                               settings.userName, pendingReplies,
                                               pendingVotes))
                            {
                                messageDialog("REP Saved",
                                    "Reply packet saved: " + repFile);
                            }
                            else
                            {
                                messageDialog("Error", "Failed to create REP packet.");
                            }
                        }
                        pendingReplies.clear();
                    }
                    // Clean up current packet
                    if (currentPacket.has_value())
                    {
                        cleanupTempDir(currentPacket->extractDir);
                    }
                    currentPacket.reset();
                    inConfList = false;
                    break;
                case ConfListResult::RemoteSystems:
                {
                    string downloadDir = dataDir + PATH_SEP_STR + "QWK";
                    string downloaded = showRemoteSystems(dataDir, downloadDir);
                    if (!downloaded.empty())
                    {
                        // Save pending replies first
                        if (!pendingReplies.empty() || !pendingVotes.empty())
                        {
                            if (confirmDialog("Save pending items before opening new file?"))
                            {
                                string repDir = settings.replyDir;
                                if (repDir.empty()) repDir = dataDir + PATH_SEP_STR + "REP";
                                string repFile = repDir + PATH_SEP_STR + currentPacket->info.bbsID + ".rep";
                                createRepPacket(repFile, currentPacket->info.bbsID,
                                                settings.userName, pendingReplies, pendingVotes);
                            }
                            pendingReplies.clear();
                            pendingVotes.clear();
                        }
                        if (currentPacket.has_value())
                        {
                            cleanupTempDir(currentPacket->extractDir);
                        }
                        currentPacket.reset();
                        initialFile = downloaded;
                        inConfList = false;
                    }
                    break;
                }
                case ConfListResult::Voting:
                    if (!currentPacket->voting.empty())
                    {
                        showVotingList(currentPacket->voting, settings);
                    }
                    else
                    {
                        messageDialog("Voting", "No polls/votes found in this packet.");
                    }
                    break;
                case ConfListResult::Settings:
                    showSettingsDialog(settings, baseDir);
                    break;
                case ConfListResult::SaveRep:
                    saveRepPacket();
                    break;
                case ConfListResult::Quit:
                    inConfList = false;
                    running = false;
                    break;
            }
        }

        // Before exiting or opening new file, offer to save replies
        if (!running && !repPacketSaved && (!pendingReplies.empty() || !pendingVotes.empty()))
        {
            int totalPending = static_cast<int>(pendingReplies.size() + pendingVotes.size());
            if (confirmDialog("Save " + std::to_string(totalPending) + " unsaved item(s) to REP packet?"))
            {
                string repDir = settings.replyDir;
                if (repDir.empty())
                {
                    repDir = dataDir + PATH_SEP_STR + "REP";
                }

                string bbsID = currentPacket.has_value()
                    ? currentPacket->info.bbsID : "REPLY";
                string repFile = repDir + PATH_SEP_STR + bbsID + ".rep";

                if (createRepPacket(repFile, bbsID, settings.userName, pendingReplies, pendingVotes))
                {
                    // Brief message before exit
                    printCentered(g_term->getRows() / 2, "REP packet saved: " + repFile,
                        tAttr(TC_GREEN, TC_BLACK, true));
                    g_term->refresh();
                    g_term->napMillis(1500);
                }
                else
                {
                    messageDialog("Error", "Failed to create REP packet.");
                }
            }
        }
    }

    // Save settings
    settings.save();

    // Clean up temp directory
    if (currentPacket.has_value())
    {
        cleanupTempDir(currentPacket->extractDir);
    }

    // Shutdown terminal
    g_term->shutdown();

    return 0;
}
