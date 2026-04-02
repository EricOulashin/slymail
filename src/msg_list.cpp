#include "msg_list.h"
#include "search.h"

using std::string;
using std::vector;
using std::pair;

// Show the conference list
ConfListResult showConferenceList(QwkPacket& packet, int& selectedConf,
                                 Settings& settings)
{
    int selected = 0;
    int scrollOffset = 0;
    bool needFullRedraw = true;
    int prevSelected    = -1;
    int prevScrollOffset = -1;

    // Filtered view: maps display index -> actual conference index
    vector<int> filteredIdx;
    bool isFiltered = false;
    string searchLabel; // Shown in title when filtered

    // Helper: rebuild filteredIdx based on onlyShowAreasWithNewMail setting
    auto rebuildFilteredIdx = [&]()
    {
        filteredIdx.clear();
        for (int i = 0; i < static_cast<int>(packet.conferences.size()); ++i)
        {
            if (settings.onlyShowAreasWithNewMail && packet.conferences[i].messages.empty())
                continue;
            filteredIdx.push_back(i);
        }
    };

    // Initialize conference list
    rebuildFilteredIdx();

    // Restore last selected conference position
    if (selectedConf >= 0)
    {
        for (int i = 0; i < static_cast<int>(filteredIdx.size()); ++i)
        {
            if (filteredIdx[i] == selectedConf)
            {
                selected = i;
                break;
            }
        }
        if (selected >= static_cast<int>(filteredIdx.size()) && !filteredIdx.empty())
        {
            selected = static_cast<int>(filteredIdx.size()) - 1;
        }
    }

    while (true)
    {
        int COLS = g_term->getCols();
        int ROWS = g_term->getRows();

        const int numW   = 6;
        const int countW = 8;
        const int newW   = 4;  // "New" column
        int nameW        = COLS - numW - countW - newW - 5;
        const int listTop = 4;
        int listHeight   = ROWS - 6;
        int totalConfs   = static_cast<int>(filteredIdx.size());

        // Keep selected in view
        if (selected < scrollOffset)
            scrollOffset = selected;
        if (selected >= scrollOffset + listHeight)
            scrollOffset = selected - listHeight + 1;

        // ---- Per-row drawing lambda (covers both selected and unselected states).
        // Always calls fillRow so the background is correct for partial updates.
        auto drawRow = [&](int idx) {
            if (idx < 0 || idx >= totalConfs) return;
            if (idx < scrollOffset || idx >= scrollOffset + listHeight) return;
            int y = listTop + (idx - scrollOffset);
            bool isSel = (idx == selected);
            const auto& conf = packet.conferences[filteredIdx[idx]];

            bool hasNew = !conf.messages.empty();
            int msgsCol = COLS - countW - newW - 2;
            int newCol  = COLS - newW - 1;

            if (isSel)
            {
                fillRow(y, tAttr(TC_BLUE, TC_WHITE, false));
                printAt(y, 1, padStr(std::to_string(conf.number), numW),
                        tAttr(TC_RED, TC_WHITE, false));
                printAt(y, 1 + numW + 1,
                        padStr(truncateStr(conf.name, nameW), nameW),
                        tAttr(TC_BLUE, TC_WHITE, false));
                printAt(y, msgsCol,
                        padStr(std::to_string(conf.messages.size()), countW),
                        tAttr(TC_GREEN, TC_WHITE, false));
                if (hasNew)
                {
                    g_term->setAttr(tAttr(TC_GREEN, TC_WHITE, true));
                    g_term->putCP437(y, newCol + 1, CP437_CHECK_MARK);
                }
            }
            else
            {
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false));
                printAt(y, 1, padStr(std::to_string(conf.number), numW),
                        tAttr(TC_YELLOW, TC_BLACK, true));
                printAt(y, 1 + numW + 1,
                        padStr(truncateStr(conf.name, nameW), nameW),
                        tAttr(TC_CYAN, TC_BLACK, false));
                printAt(y, msgsCol,
                        padStr(std::to_string(conf.messages.size()), countW),
                        tAttr(TC_GREEN, TC_BLACK, false));
                if (hasNew)
                {
                    g_term->setAttr(tAttr(TC_GREEN, TC_BLACK, true));
                    g_term->putCP437(y, newCol + 1, CP437_CHECK_MARK);
                }
            }
        };

        // ---- Scrollbar-only redraw (thumb position depends on selected)
        auto drawSB = [&]() {
            if (totalConfs > listHeight)
            {
                drawScrollbar(listTop, listHeight, selected, totalConfs,
                             tAttr(TC_BLACK, TC_BLACK, true),
                             tAttr(TC_WHITE, TC_BLACK, true));
            }
        };

        bool scrollChanged = (scrollOffset != prevScrollOffset);

        if (needFullRedraw || scrollChanged)
        {
            // ---- Full redraw ----
            g_term->clear();

            TermAttr borderAttr = tAttr(TC_BLUE, TC_BLACK, true);
            TermAttr titleAttr  = tAttr(TC_GREEN, TC_BLACK, true);

            g_term->setAttr(borderAttr);
            g_term->putCP437(0, 0, CP437_BOX_DRAWINGS_UPPER_LEFT_SINGLE);
            g_term->drawHLine(0, 1, COLS - 2);
            g_term->putCP437(0, COLS - 1, CP437_BOX_DRAWINGS_UPPER_RIGHT_SINGLE);
            printAt(0, 4, " " + packet.info.bbsName + " ", titleAttr);

            g_term->setAttr(borderAttr);
            g_term->putCP437(1, 0, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
            g_term->putCP437(1, COLS - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
            printAt(1, 2, "Sysop: ", tAttr(TC_CYAN, TC_BLACK, false));
            printAt(1, 9, packet.info.sysopName, tAttr(TC_WHITE, TC_BLACK, true));
            string userInfo = "User: " + packet.info.userName;
            printAt(1, COLS / 2, userInfo, tAttr(TC_CYAN, TC_BLACK, false));
            string totalStr = std::to_string(packet.totalMessages()) + " msgs";
            printAt(1, COLS - static_cast<int>(totalStr.size()) - 3, totalStr,
                    tAttr(TC_GREEN, TC_BLACK, false));

            g_term->setAttr(borderAttr);
            g_term->putCP437(2, 0, CP437_BOX_DRAWINGS_LOWER_LEFT_SINGLE);
            g_term->drawHLine(2, 1, COLS - 2);
            g_term->putCP437(2, COLS - 1, CP437_BOX_DRAWINGS_LOWER_RIGHT_SINGLE);

            TermAttr colAttr = tAttr(TC_CYAN, TC_BLACK, true);
            int msgsColHdr = COLS - countW - newW - 2;
            int newColHdr  = COLS - newW - 1;
            printAt(3, 1, padStr("Conf#", numW), colAttr);
            printAt(3, 1 + numW + 1, padStr("Conference Name", nameW), colAttr);
            printAt(3, msgsColHdr, padStr("Msgs", countW), colAttr);
            printAt(3, newColHdr, padStr("New", newW), colAttr);

            for (int i = 0; i < listHeight && (scrollOffset + i) < totalConfs; ++i)
                drawRow(scrollOffset + i);

            drawSB();

            drawDDHelpBar(ROWS - 1,
                "Up/Dn/PgUp/PgDn/HOME/END, ",
                {{'E', "nter area"}, {'/', "Search"}, {'G', "o to #"},
                 {'O', "pen file"}, {'S', "ettings"}, {'Q', "uit"}, {'?', ""}});

            // Show filter indicator if active
            if (isFiltered)
            {
                string filterMsg = " Filter: \"" + searchLabel + "\" (" +
                    std::to_string(totalConfs) + "/" +
                    std::to_string(packet.conferences.size()) + ") ";
                printAt(ROWS - 2, 0, filterMsg, tAttr(TC_YELLOW, TC_BLACK, true));
            }

            needFullRedraw = false;
        }
        else if (selected != prevSelected)
        {
            // ---- Partial update: only the two changed rows + scrollbar ----
            drawRow(prevSelected);
            drawRow(selected);
            drawSB();
        }

        g_term->refresh();
        prevSelected     = selected;
        prevScrollOffset = scrollOffset;

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
                if (selected < totalConfs - 1)
                {
                    ++selected;
                }
                break;
            case TK_PGUP:
                selected -= listHeight;
                if (selected < 0)
                {
                    selected = 0;
                }
                break;
            case TK_PGDN:
                selected += listHeight;
                if (selected >= totalConfs)
                {
                    selected = totalConfs - 1;
                }
                break;
            case TK_HOME:
                selected = 0;
                break;
            case TK_END:
                selected = totalConfs - 1;
                break;
            case TK_ENTER:
                if (totalConfs > 0)
                {
                    selectedConf = filteredIdx[selected];
                    return ConfListResult::Selected;
                }
                break;
            case '/':
            {
                // Search / filter conferences
                bool clearSearch = false;
                string searchText = showConfSearchPrompt(clearSearch);
                if (clearSearch || searchText.empty())
                {
                    // Clear filter — show all conferences
                    filteredIdx.clear();
                    for (int i = 0; i < static_cast<int>(packet.conferences.size()); ++i)
                    {
                        filteredIdx.push_back(i);
                    }
                    isFiltered = false;
                    searchLabel.clear();
                }
                else
                {
                    auto results = filterConferences(packet.conferences,
                                                     searchText,
                                                     settings.useRegexSearch);
                    if (results.empty())
                    {
                        messageDialog("Search", "No conferences found matching the search.");
                    }
                    else
                    {
                        filteredIdx = results;
                        isFiltered = true;
                        searchLabel = searchText;
                    }
                }
                selected = 0;
                scrollOffset = 0;
                needFullRedraw = true;
                break;
            }
            case 'o':
            case 'O':
            case TK_CTRL_L:
                return ConfListResult::OpenFile;
            case TK_CTRL_R:
                return ConfListResult::RemoteSystems;
            case TK_CTRL_P:
                return ConfListResult::SaveRep;
            case 'v':
            case 'V':
                return ConfListResult::Voting;
            case 's':
            case 'S':
            case TK_CTRL_U:
                return ConfListResult::Settings;
            case 'g':
            case 'G':
            {
                printAt(ROWS - 1, 0, "Go to conf #: ",
                        tAttr(TC_WHITE, TC_BLACK, true));
                g_term->clearToEol();
                string numStr = getNumericInput(ROWS - 1, 14, 8,
                    tAttr(TC_WHITE, TC_BLACK, true));
                if (!numStr.empty())
                {
                    try
                    {
                        int targetNum = std::stoi(numStr);
                        for (int j = 0; j < totalConfs; ++j)
                        {
                            if (packet.conferences[filteredIdx[j]].number == targetNum)
                            {
                                selected = j;
                                break;
                            }
                        }
                    }
                    catch (...)
                    {
                    }
                }
                needFullRedraw = true;
                break;
            }
            case 'q':
            case 'Q':
                if (isFiltered)
                {
                    // Clear the search filter, but still respect onlyShowAreasWithNewMail
                    isFiltered = false;
                    searchLabel.clear();
                    rebuildFilteredIdx();
                    selected = 0;
                    scrollOffset = 0;
                    needFullRedraw = true;
                    break;
                }
                return ConfListResult::Quit;
            case TK_ESCAPE:
            case TK_CTRL_C:
                return ConfListResult::Quit;
            case '?':
            case TK_F1:
            {
                g_term->clear();
                int r = 1;
                drawProgramInfoLine(r++);
                r++;
                printCentered(r++, "Conference List Help",
                    tAttr(TC_CYAN, TC_BLACK, true));
                TermAttr keyC  = tAttr(TC_CYAN, TC_BLACK, true);
                TermAttr descC = tAttr(TC_CYAN, TC_BLACK, false);
                auto helpLine = [&](const string& key, const string& desc)
                {
                    printAt(r, 2, padStr(key, 20), keyC);
                    printAt(r, 24, ": " + desc, descC);
                    ++r;
                };
                helpLine("Up/Down arrow", "Navigate conferences");
                helpLine("Enter", "Open selected conference");
                helpLine("PageUp/PageDown", "Scroll up/down a page");
                helpLine("HOME/END", "Jump to first/last conference");
                helpLine("/", "Search/filter conferences");
                helpLine("V", "View polls/votes in packet");
                helpLine("O / Ctrl-L", "Open a different QWK file");
                helpLine("Ctrl-R", "Remote systems (download QWK)");
                helpLine("Ctrl-P", "Save REP reply packet");
                helpLine("S / Ctrl-U", "Settings");
                helpLine("Q / ESC", "Quit SlyMail");
                helpLine("? / F1", "This help screen");
                r += 2;
                printAt(r, 2, "Press any key to continue...",
                    tAttr(TC_GREEN, TC_BLACK, false));
                g_term->refresh();
                g_term->getKey();
                needFullRedraw = true;   // help screen clobbered the display
                break;
            }
            case TK_RESIZE:
                needFullRedraw = true;
                break;
            default:
                break;
        }
    }
}

// Show the message list for a conference (DDMsgReader-style lightbar)
MsgListResult showMessageList(QwkConference& conf, int& selectedMsg,
                              Settings& settings,
                              const string& bbsName,
                              int lastReadMsgNum)
{
    int selected  = 0;
    int scrollOffset = 0;
    bool needFullRedraw  = true;
    int prevSelected     = -1;
    int prevScrollOffset = -1;

    // Filtered view: maps display index -> actual message index
    vector<int> filteredIdx;
    bool isFiltered = false;
    string searchLabel;

    // Initialize with all messages, excluding vote response messages
    for (int i = 0; i < static_cast<int>(conf.messages.size()); ++i)
    {
        if (!conf.messages[i].isVoteResponse)
        {
            filteredIdx.push_back(i);
        }
    }

    int totalMsgs = static_cast<int>(filteredIdx.size());

    // Restore last selected position: find the filtered index that
    // corresponds to the real message index in selectedMsg
    if (selectedMsg >= 0)
    {
        for (int i = 0; i < totalMsgs; ++i)
        {
            if (filteredIdx[i] == selectedMsg)
            {
                selected = i;
                break;
            }
        }
        if (selected >= totalMsgs && totalMsgs > 0)
        {
            selected = totalMsgs - 1;
        }
    }
    else if (lastReadMsgNum >= 0 && totalMsgs > 0)
    {
        // Position at the first message after the last-read message
        for (int i = 0; i < totalMsgs; ++i)
        {
            if (conf.messages[filteredIdx[i]].number > lastReadMsgNum)
            {
                selected = i;
                break;
            }
        }
    }

    while (true)
    {
        int COLS = g_term->getCols();
        int ROWS = g_term->getRows();

        const int numW  = 5;
        const int fromW = 15;
        const int toW   = 15;
        const int dateW = 10;
        const int timeW = 8;
        int subjW = COLS - numW - fromW - toW - dateW - timeW - 6;
        if (subjW < 10)
            subjW = 10;

        const int listTop  = 1;
        int listHeight     = ROWS - 3;

        // Keep selected in view
        if (selected < scrollOffset)
            scrollOffset = selected;
        if (selected >= scrollOffset + listHeight)
            scrollOffset = selected - listHeight + 1;

        totalMsgs = static_cast<int>(filteredIdx.size());

        // ---- Per-row drawing lambda ----
        auto drawRow = [&](int idx) {
            if (idx < 0 || idx >= totalMsgs) return;
            if (idx < scrollOffset || idx >= scrollOffset + listHeight) return;
            int y = listTop + (idx - scrollOffset);
            bool isSel = (idx == selected);
            const auto& msg = conf.messages[filteredIdx[idx]];

            if (isSel)
            {
                fillRow(y, tAttr(TC_BLUE, TC_WHITE, false));
                printAt(y, 0,
                        padStr(rightAlign(std::to_string(msg.number), numW), numW),
                        tAttr(TC_RED, TC_WHITE, false));
                printAt(y, numW + 1,
                        padStr(truncateStr(msg.from, fromW), fromW),
                        tAttr(TC_BLUE, TC_WHITE, false));
                printAt(y, numW + fromW + 2,
                        padStr(truncateStr(msg.to, toW), toW),
                        tAttr(TC_BLUE, TC_WHITE, false));
                printAt(y, numW + fromW + toW + 3,
                        padStr(truncateStr(msg.subject, subjW), subjW),
                        tAttr(TC_BLACK, TC_WHITE, false));
                printAt(y, COLS - dateW - timeW - 2,
                        padStr(msg.date, dateW),
                        tAttr(TC_GREEN, TC_WHITE, false));
                printAt(y, COLS - timeW - 1,
                        padStr(msg.time, timeW),
                        tAttr(TC_GREEN, TC_WHITE, false));
            }
            else
            {
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false));
                printAt(y, 0,
                        padStr(rightAlign(std::to_string(msg.number), numW), numW),
                        tAttr(TC_YELLOW, TC_BLACK, true));
                printAt(y, numW + 1,
                        padStr(truncateStr(msg.from, fromW), fromW),
                        tAttr(TC_CYAN, TC_BLACK, false));
                printAt(y, numW + fromW + 2,
                        padStr(truncateStr(msg.to, toW), toW),
                        tAttr(TC_CYAN, TC_BLACK, false));
                printAt(y, numW + fromW + toW + 3,
                        padStr(truncateStr(msg.subject, subjW), subjW),
                        tAttr(TC_CYAN, TC_BLACK, true));
                printAt(y, COLS - dateW - timeW - 2,
                        padStr(msg.date, dateW),
                        tAttr(TC_GREEN, TC_BLACK, false));
                printAt(y, COLS - timeW - 1,
                        padStr(msg.time, timeW),
                        tAttr(TC_GREEN, TC_BLACK, false));
            }
        };

        // ---- Scrollbar-only redraw ----
        auto drawSB = [&]() {
            if (totalMsgs > listHeight)
            {
                drawScrollbar(listTop, listHeight, selected, totalMsgs,
                             tAttr(TC_BLACK, TC_BLACK, true),
                             tAttr(TC_WHITE, TC_BLACK, true));
            }
        };

        bool scrollChanged = (scrollOffset != prevScrollOffset);

        if (needFullRedraw || scrollChanged)
        {
            // ---- Full redraw ----
            g_term->clear();

            TermAttr colHdrAttr = tAttr(TC_CYAN, TC_BLACK, true);
            printAt(0, 0, padStr("Msg#", numW), colHdrAttr);
            printAt(0, numW + 1, padStr("From", fromW), colHdrAttr);
            printAt(0, numW + fromW + 2, padStr("To", toW), colHdrAttr);
            printAt(0, numW + fromW + toW + 3, padStr("Subject", subjW), colHdrAttr);
            printAt(0, COLS - dateW - timeW - 2, padStr("Date", dateW), colHdrAttr);
            printAt(0, COLS - timeW - 1, padStr("Time", timeW), colHdrAttr);

            for (int i = 0; i < listHeight && (scrollOffset + i) < totalMsgs; ++i)
                drawRow(scrollOffset + i);

            drawSB();

            // Status line (static for the lifetime of this message list)
            string confTitle = bbsName + " - " + conf.name
                + " (" + std::to_string(totalMsgs) + " msgs)";
            printAt(ROWS - 2, 0, truncateStr(confTitle, COLS),
                    tAttr(TC_CYAN, TC_BLACK, false));

            drawDDHelpBar(ROWS - 1,
                "Up/Dn/PgUp/PgDn/HOME/END, ",
                {{'N', "ew msg"}, {'R', "ead"}, {'/', "Search"},
                 {'G', "o to #"}, {'C', "onf list"}, {'Q', "uit"}, {'?', ""}});

            // Show filter indicator if active
            if (isFiltered)
            {
                string filterMsg = " Filter: \"" + searchLabel + "\" (" +
                    std::to_string(totalMsgs) + "/" +
                    std::to_string(conf.messages.size()) + " msgs) ";
                printAt(ROWS - 2, COLS - static_cast<int>(filterMsg.size()) - 1,
                        filterMsg, tAttr(TC_YELLOW, TC_BLACK, true));
            }

            needFullRedraw = false;
        }
        else if (selected != prevSelected)
        {
            // ---- Partial update: two changed rows + scrollbar ----
            drawRow(prevSelected);
            drawRow(selected);
            drawSB();
        }

        g_term->refresh();
        prevSelected     = selected;
        prevScrollOffset = scrollOffset;

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
                if (selected < totalMsgs - 1)
                {
                    ++selected;
                }
                break;
            case TK_PGUP:
                selected -= listHeight;
                if (selected < 0)
                {
                    selected = 0;
                }
                break;
            case TK_PGDN:
                selected += listHeight;
                if (selected >= totalMsgs)
                {
                    selected = totalMsgs - 1;
                }
                break;
            case TK_HOME:
            case 'f':
            case 'F':
                selected = 0;
                break;
            case TK_END:
            case 'l':
            case 'L':
                selected = totalMsgs > 0 ? totalMsgs - 1 : 0;
                break;
            case TK_ENTER:
                if (totalMsgs > 0)
                {
                    selectedMsg = filteredIdx[selected];
                    return MsgListResult::ReadMessage;
                }
                break;
            case 'r':
            case 'R':
                if (totalMsgs > 0)
                {
                    selectedMsg = filteredIdx[selected];
                    return MsgListResult::ReadMessage;
                }
                break;
            case '/':
            {
                // Search / filter messages
                MsgSearchParams searchParams;
                bool clearSearch = false;
                if (showMsgSearchDialog(searchParams, settings, clearSearch))
                {
                    auto results = filterMessages(conf.messages, searchParams);
                    if (results.empty())
                    {
                        messageDialog("Search", "No messages found matching the search.");
                    }
                    else
                    {
                        filteredIdx = results;
                        isFiltered = true;
                        searchLabel = searchParams.searchText;
                    }
                }
                else if (clearSearch)
                {
                    // Clear filter (still exclude vote responses)
                    filteredIdx.clear();
                    for (int i = 0; i < static_cast<int>(conf.messages.size()); ++i)
                    {
                        if (!conf.messages[i].isVoteResponse)
                        {
                            filteredIdx.push_back(i);
                        }
                    }
                    isFiltered = false;
                    searchLabel.clear();
                }
                selected = 0;
                scrollOffset = 0;
                needFullRedraw = true;
                break;
            }
            case 'n':
            case 'N':
                return MsgListResult::NewMessage;
            case 'c':
            case 'C':
            case TK_ESCAPE:
                return MsgListResult::Back;
            case 's':
            case 'S':
            case TK_CTRL_U:
                return MsgListResult::Settings;
            case TK_CTRL_L:
                return MsgListResult::OpenFile;
            case TK_CTRL_R:
                return MsgListResult::RemoteSystems;
            case TK_CTRL_P:
                return MsgListResult::SaveRep;
            case 'q':
            case 'Q':
                if (isFiltered)
                {
                    // Clear the search filter (still exclude vote responses)
                    filteredIdx.clear();
                    for (int i = 0; i < static_cast<int>(conf.messages.size()); ++i)
                    {
                        if (!conf.messages[i].isVoteResponse)
                        {
                            filteredIdx.push_back(i);
                        }
                    }
                    isFiltered = false;
                    searchLabel.clear();
                    selected = 0;
                    scrollOffset = 0;
                    needFullRedraw = true;
                    break;
                }
                return MsgListResult::Back;
            case TK_CTRL_C:
                return MsgListResult::Quit;
            case 'g':
            case 'G':
            {
                printAt(ROWS - 1, 0, "Go to msg #: ",
                        tAttr(TC_WHITE, TC_BLACK, true));
                g_term->clearToEol();
                string numStr = getNumericInput(ROWS - 1, 14, 8,
                    tAttr(TC_WHITE, TC_BLACK, true));
                if (!numStr.empty())
                {
                    try
                    {
                        int targetNum = std::stoi(numStr);
                        for (int j = 0; j < totalMsgs; ++j)
                        {
                            if (conf.messages[filteredIdx[j]].number == targetNum)
                            {
                                selected = j;
                                break;
                            }
                        }
                    }
                    catch (...)
                    {
                        // Invalid input — just ignore and return to the list
                    }
                }
                needFullRedraw = true;  // help bar was overwritten by input prompt
                break;
            }
            case '?':
            case TK_F1:
            {
                g_term->clear();
                int r = 1;
                drawProgramInfoLine(r++);
                r++;
                printCentered(r++, "Message List Help",
                    tAttr(TC_CYAN, TC_BLACK, true));
                TermAttr keyC  = tAttr(TC_CYAN, TC_BLACK, true);
                TermAttr descC = tAttr(TC_CYAN, TC_BLACK, false);
                auto helpLine = [&](const string& key, const string& desc)
                {
                    printAt(r, 2, padStr(key, 20), keyC);
                    printAt(r, 24, ": " + desc, descC);
                    ++r;
                };
                helpLine("Up/Down arrow", "Navigate messages");
                helpLine("Enter / R", "Read selected message");
                helpLine("N", "Write a new message");
                helpLine("G", "Go to message number");
                helpLine("PageUp/PageDown", "Scroll up/down a page");
                helpLine("HOME/END", "Jump to first/last message");
                helpLine("/", "Search/filter messages");
                helpLine("Ctrl-L", "Open a different QWK file");
                helpLine("Ctrl-R", "Remote systems (download QWK)");
                helpLine("Ctrl-P", "Save REP reply packet");
                helpLine("S / Ctrl-U", "Settings");
                helpLine("C / ESC", "Back to conference list");
                helpLine("Q", "Quit SlyMail");
                helpLine("? / F1", "This help screen");
                r += 2;
                printAt(r, 2, "Press any key to continue...",
                    tAttr(TC_GREEN, TC_BLACK, false));
                g_term->refresh();
                g_term->getKey();
                needFullRedraw = true;   // help screen clobbered the display
                break;
            }
            case TK_RESIZE:
                needFullRedraw = true;
                break;
            default:
                break;
        }
    }
}
