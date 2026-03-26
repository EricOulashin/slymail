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

#include <ctime>
#include <optional>

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
                                            EditorResult edResult = editReply(
                                                conf.messages[currentMsg],
                                                settings.userName,
                                                conf.name,
                                                reply,
                                                settings,
                                                baseDir);
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
                                EditorResult edResult = editNewMessage(
                                    settings.userName,
                                    conf.name,
                                    conf.number,
                                    reply,
                                    settings,
                                    baseDir);
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
