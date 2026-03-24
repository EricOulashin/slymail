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

#include <ctime>
#include <optional>

using std::string;
using std::vector;
using std::optional;

// Application version
constexpr const char* SLYMAIL_VERSION = "1.00";

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
        "QWK Offline Mail Reader v" + std::string(SLYMAIL_VERSION),
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

// Main application loop
int main(int argc, char* argv[])
{
    // Seed random for Random editor style
    srand(static_cast<unsigned>(time(nullptr)));

    // Determine base directory (where the executable lives)
    // Settings file, config/, and dictionaries/ are relative to this
    std::string baseDir = ".";
    if (argc > 0 && argv[0] != nullptr)
    {
        fs::path exePath = fs::path(argv[0]).parent_path();
        if (!exePath.empty())
        {
            baseDir = exePath.string();
        }
    }
    settingsDir() = baseDir;

    // Initialize terminal
    auto terminal = createTerminal();
    g_term = terminal.get();
    g_term->init();

    // Load settings
    Settings settings;
    settings.load();

    // Show splash screen
    showSplashScreen();

    // Main application state
    std::optional<QwkPacket> currentPacket;
    std::vector<QwkReply> pendingReplies;
    bool running = true;

    // Check if a QWK file was passed as command line argument
    std::string initialFile;
    if (argc > 1)
    {
        initialFile = argv[1];
    }

    while (running)
    {
        // If no packet is open, show the file browser
        if (!currentPacket.has_value())
        {
            std::string qwkFile;
            if (!initialFile.empty())
            {
                qwkFile = initialFile;
                initialFile.clear(); // Only use once
            }
            else
            {
                qwkFile = showFileBrowser(settings.lastDirectory, settings.lastQwkFile);
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
        while (inConfList && running)
        {
            int selectedConf = 0;
            ConfListResult confResult = showConferenceList(*currentPacket,
                                                           selectedConf, settings);

            switch (confResult)
            {
                case ConfListResult::Selected:
                {
                    // Enter the selected conference
                    auto& conf = currentPacket->conferences[selectedConf];
                    bool inMsgList = true;

                    while (inMsgList && running)
                    {
                        int selectedMsg = 0;
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

                                    MsgReadResult readResult = showMessageReader(
                                        conf.messages[currentMsg],
                                        conf.name,
                                        currentMsg,
                                        static_cast<int>(conf.messages.size()),
                                        settings);

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
                                                messageDialog("Reply Saved",
                                                    "Reply queued. " +
                                                    std::to_string(pendingReplies.size()) +
                                                    " pending reply(s).");
                                            }
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
                                    messageDialog("Message Saved",
                                        "Message queued. " +
                                        std::to_string(pendingReplies.size()) +
                                        " pending message(s).");
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
                                        std::string repDir = settings.replyDir;
                                        if (repDir.empty())
                                        {
                                            repDir = fs::path(currentPacket->sourceFile).parent_path().string();
                                        }
                                        std::string repFile = repDir + PATH_SEP_STR
                                            + currentPacket->info.bbsID + ".rep";
                                        createRepPacket(repFile, currentPacket->info.bbsID,
                                                        settings.userName, pendingReplies);
                                    }
                                    pendingReplies.clear();
                                }
                                if (currentPacket.has_value())
                                {
                                    cleanupTempDir(currentPacket->extractDir);
                                }
                                currentPacket.reset();
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
                            std::string repDir = settings.replyDir;
                            if (repDir.empty())
                            {
                                repDir = fs::path(currentPacket->sourceFile).parent_path().string();
                            }
                            std::string repFile = repDir + PATH_SEP_STR
                                + currentPacket->info.bbsID + ".rep";
                            if (createRepPacket(repFile, currentPacket->info.bbsID,
                                               settings.userName, pendingReplies))
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
                case ConfListResult::Settings:
                    showSettingsDialog(settings, baseDir);
                    break;
                case ConfListResult::Quit:
                    inConfList = false;
                    running = false;
                    break;
            }
        }

        // Before exiting or opening new file, offer to save replies
        if (!running && !pendingReplies.empty())
        {
            if (confirmDialog("Save " + std::to_string(pendingReplies.size())
                              + " pending reply(s) to REP packet?"))
            {
                std::string repDir = settings.replyDir;
                if (repDir.empty() && currentPacket.has_value())
                {
                    repDir = fs::path(currentPacket->sourceFile).parent_path().string();
                }
                if (repDir.empty())
                {
                    char* cwd = getcwd(nullptr, 0);
                    if (cwd)
                    {
                        repDir = cwd;
                        free(cwd);
                    }
                    else
                    {
                        repDir = ".";
                    }
                }

                std::string bbsID = currentPacket.has_value()
                    ? currentPacket->info.bbsID : "REPLY";
                std::string repFile = repDir + PATH_SEP_STR + bbsID + ".rep";

                if (createRepPacket(repFile, bbsID, settings.userName, pendingReplies))
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
