#include "msg_list.h"

using std::string;
using std::vector;
using std::pair;

// Show the conference list
ConfListResult showConferenceList(QwkPacket& packet, int& selectedConf,
                                 const Settings& /* settings */)
{
    int selected = 0;
    int scrollOffset = 0;

    while (true)
    {
        g_term->clear();

        // Title: DDMsgReader-style bordered header
        TermAttr borderAttr = tAttr(TC_BLUE, TC_BLACK, true);
        TermAttr titleAttr  = tAttr(TC_GREEN, TC_BLACK, true);

        // Top border with BBS name
        g_term->setAttr(borderAttr);
        g_term->putCP437(0, 0, CP437_BOX_DRAWINGS_UPPER_LEFT_SINGLE);
        g_term->drawHLine(0, 1, g_term->getCols() - 2);
        g_term->putCP437(0, g_term->getCols() - 1, CP437_BOX_DRAWINGS_UPPER_RIGHT_SINGLE);
        printAt(0, 4, " " + packet.info.bbsName + " ", titleAttr);

        // Packet info inside border
        g_term->setAttr(borderAttr);
        g_term->putCP437(1, 0, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
        g_term->putCP437(1, g_term->getCols() - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
        printAt(1, 2, "Sysop: ", tAttr(TC_CYAN, TC_BLACK, false));
        printAt(1, 9, packet.info.sysopName, tAttr(TC_WHITE, TC_BLACK, true));
        string userInfo = "User: " + packet.info.userName;
        printAt(1, g_term->getCols() / 2, userInfo, tAttr(TC_CYAN, TC_BLACK, false));
        string totalStr = std::to_string(packet.totalMessages()) + " msgs";
        printAt(1, g_term->getCols() - static_cast<int>(totalStr.size()) - 3, totalStr,
                tAttr(TC_GREEN, TC_BLACK, false));

        // Bottom of info area
        g_term->setAttr(borderAttr);
        g_term->putCP437(2, 0, CP437_BOX_DRAWINGS_LOWER_LEFT_SINGLE);
        g_term->drawHLine(2, 1, g_term->getCols() - 2);
        g_term->putCP437(2, g_term->getCols() - 1, CP437_BOX_DRAWINGS_LOWER_RIGHT_SINGLE);

        // Column headers (DDMsgReader style)
        int headerY = 3;
        int numW = 6;
        int countW = 8;
        int nameW = g_term->getCols() - numW - countW - 4;

        TermAttr colAttr = tAttr(TC_CYAN, TC_BLACK, true);
        printAt(headerY, 1, padStr("Conf#", numW), colAttr);
        printAt(headerY, 1 + numW + 1, padStr("Conference Name", nameW), colAttr);
        printAt(headerY, g_term->getCols() - countW - 1, padStr("Msgs", countW), colAttr);

        // Conference list
        int listTop = 4;
        int listHeight = g_term->getRows() - 6;
        int totalConfs = static_cast<int>(packet.conferences.size());

        if (selected < scrollOffset)
        {
            scrollOffset = selected;
        }
        if (selected >= scrollOffset + listHeight)
        {
            scrollOffset = selected - listHeight + 1;
        }

        for (int i = 0; i < listHeight && (scrollOffset + i) < totalConfs; ++i)
        {
            int idx = scrollOffset + i;
            const auto& conf = packet.conferences[idx];
            int y = listTop + i;
            bool isSel = (idx == selected);

            if (isSel)
            {
                fillRow(y, tAttr(TC_BLUE, TC_WHITE, false));
                printAt(y, 1, padStr(std::to_string(conf.number), numW),
                        tAttr(TC_RED, TC_WHITE, false));
                printAt(y, 1 + numW + 1,
                        padStr(truncateStr(conf.name, nameW), nameW),
                        tAttr(TC_BLUE, TC_WHITE, false));
                printAt(y, g_term->getCols() - countW - 1,
                        padStr(std::to_string(conf.messages.size()), countW),
                        tAttr(TC_GREEN, TC_WHITE, false));
            }
            else
            {
                printAt(y, 1, padStr(std::to_string(conf.number), numW),
                        tAttr(TC_YELLOW, TC_BLACK, true));
                printAt(y, 1 + numW + 1,
                        padStr(truncateStr(conf.name, nameW), nameW),
                        tAttr(TC_CYAN, TC_BLACK, false));
                printAt(y, g_term->getCols() - countW - 1,
                        padStr(std::to_string(conf.messages.size()), countW),
                        tAttr(TC_GREEN, TC_BLACK, false));
            }
        }

        // Scrollbar
        if (totalConfs > listHeight)
        {
            drawScrollbar(listTop, listHeight, selected, totalConfs,
                         tAttr(TC_BLACK, TC_BLACK, true),
                         tAttr(TC_WHITE, TC_BLACK, true));
        }

        // Help bar (DDMsgReader style)
        drawDDHelpBar(g_term->getRows() - 1,
            "Up/Dn/PgUp/PgDn/HOME/END, ",
            {{'E', "nter area"}, {'O', "pen file"},
             {'S', "ettings"}, {'Q', "uit"}, {'?', ""}});

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
                    selectedConf = selected;
                    return ConfListResult::Selected;
                }
                break;
            case 'o':
            case 'O':
            case TK_CTRL_L:
                return ConfListResult::OpenFile;
            case 's':
            case 'S':
            case TK_CTRL_U:
                return ConfListResult::Settings;
            case 'q':
            case 'Q':
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
                helpLine("O / Ctrl-L", "Open a different QWK file");
                helpLine("S / Ctrl-U", "Settings");
                helpLine("Q / ESC", "Quit SlyMail");
                r += 2;
                printAt(r, 2, "Press any key to continue...",
                    tAttr(TC_GREEN, TC_BLACK, false));
                g_term->refresh();
                g_term->getKey();
                break;
            }
            default:
                break;
        }
    }
}

// Show the message list for a conference (DDMsgReader-style lightbar)
MsgListResult showMessageList(QwkConference& conf, int& selectedMsg,
                              const Settings& /* settings */,
                              const string& bbsName)
{
    int selected = 0;
    int scrollOffset = 0;
    int totalMsgs = static_cast<int>(conf.messages.size());

    while (true)
    {
        g_term->clear();

        // Column header row (DDMsgReader style)
        // Msg#  From          To           Subject          Date        Time
        int numW = 5;
        int fromW = 15;
        int toW = 15;
        int dateW = 10;
        int timeW = 8;
        int subjW = g_term->getCols() - numW - fromW - toW - dateW - timeW - 6;
        if (subjW < 10)
        {
            subjW = 10;
        }

        int headerY = 0;
        TermAttr colHdrAttr = tAttr(TC_CYAN, TC_BLACK, true);
        printAt(headerY, 0, padStr("Msg#", numW), colHdrAttr);
        printAt(headerY, numW + 1, padStr("From", fromW), colHdrAttr);
        printAt(headerY, numW + fromW + 2, padStr("To", toW), colHdrAttr);
        printAt(headerY, numW + fromW + toW + 3, padStr("Subject", subjW), colHdrAttr);
        printAt(headerY, g_term->getCols() - dateW - timeW - 2, padStr("Date", dateW), colHdrAttr);
        printAt(headerY, g_term->getCols() - timeW - 1, padStr("Time", timeW), colHdrAttr);

        // Message list
        int listTop = 1;
        int listHeight = g_term->getRows() - 3;

        if (selected < scrollOffset)
        {
            scrollOffset = selected;
        }
        if (selected >= scrollOffset + listHeight)
        {
            scrollOffset = selected - listHeight + 1;
        }

        for (int i = 0; i < listHeight && (scrollOffset + i) < totalMsgs; ++i)
        {
            int idx = scrollOffset + i;
            const auto& msg = conf.messages[idx];
            int y = listTop + i;
            bool isSel = (idx == selected);

            if (isSel)
            {
                fillRow(y, tAttr(TC_BLUE, TC_WHITE, false));
                printAt(y, 0, padStr(rightAlign(std::to_string(msg.number), numW), numW),
                        tAttr(TC_RED, TC_WHITE, false));
                printAt(y, numW + 1, padStr(truncateStr(msg.from, fromW), fromW),
                        tAttr(TC_BLUE, TC_WHITE, false));
                printAt(y, numW + fromW + 2, padStr(truncateStr(msg.to, toW), toW),
                        tAttr(TC_BLUE, TC_WHITE, false));
                printAt(y, numW + fromW + toW + 3,
                        padStr(truncateStr(msg.subject, subjW), subjW),
                        tAttr(TC_BLACK, TC_WHITE, false));
                printAt(y, g_term->getCols() - dateW - timeW - 2,
                        padStr(msg.date, dateW),
                        tAttr(TC_GREEN, TC_WHITE, false));
                printAt(y, g_term->getCols() - timeW - 1,
                        padStr(msg.time, timeW),
                        tAttr(TC_GREEN, TC_WHITE, false));
            }
            else
            {
                printAt(y, 0, padStr(rightAlign(std::to_string(msg.number), numW), numW),
                        tAttr(TC_YELLOW, TC_BLACK, true));
                printAt(y, numW + 1, padStr(truncateStr(msg.from, fromW), fromW),
                        tAttr(TC_CYAN, TC_BLACK, false));
                printAt(y, numW + fromW + 2, padStr(truncateStr(msg.to, toW), toW),
                        tAttr(TC_CYAN, TC_BLACK, false));
                printAt(y, numW + fromW + toW + 3,
                        padStr(truncateStr(msg.subject, subjW), subjW),
                        tAttr(TC_CYAN, TC_BLACK, true));
                printAt(y, g_term->getCols() - dateW - timeW - 2,
                        padStr(msg.date, dateW),
                        tAttr(TC_GREEN, TC_BLACK, false));
                printAt(y, g_term->getCols() - timeW - 1,
                        padStr(msg.time, timeW),
                        tAttr(TC_GREEN, TC_BLACK, false));
            }
        }

        // Scrollbar
        if (totalMsgs > listHeight)
        {
            drawScrollbar(listTop, listHeight, selected, totalMsgs,
                         tAttr(TC_BLACK, TC_BLACK, true),
                         tAttr(TC_WHITE, TC_BLACK, true));
        }

        // Status line
        string confTitle = bbsName + " - " + conf.name
            + " (" + std::to_string(totalMsgs) + " msgs)";
        printAt(g_term->getRows() - 2, 0, truncateStr(confTitle, g_term->getCols()),
                tAttr(TC_CYAN, TC_BLACK, false));

        // Help bar (DDMsgReader style)
        drawDDHelpBar(g_term->getRows() - 1,
            "Up/Dn/PgUp/PgDn/HOME/END, ",
            {{'N', "ew msg"}, {'R', "eply"}, {'F', "irst"}, {'L', "ast"},
             {'G', "o to #"}, {'C', "onf list"}, {'Q', "uit"}, {'?', ""}});

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
                    selectedMsg = selected;
                    return MsgListResult::ReadMessage;
                }
                break;
            case 'r':
            case 'R':
                if (totalMsgs > 0)
                {
                    selectedMsg = selected;
                    return MsgListResult::ReadMessage;
                }
                break;
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
            case 'q':
            case 'Q':
                return MsgListResult::Back;
            case TK_CTRL_C:
                return MsgListResult::Quit;
            case 'g':
            case 'G':
            {
                printAt(g_term->getRows() - 1, 0, "Go to msg #: ",
                        tAttr(TC_WHITE, TC_BLACK, true));
                g_term->moveTo(g_term->getRows() - 1, 13);
                g_term->clearToEol();
                string numStr = getStringInput(g_term->getRows() - 1, 14, 8, "",
                    tAttr(TC_WHITE, TC_BLACK, true));
                if (!numStr.empty())
                {
                    try
                    {
                        int targetNum = std::stoi(numStr);
                        for (int j = 0; j < totalMsgs; ++j)
                        {
                            if (conf.messages[j].number == targetNum)
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
                helpLine("Ctrl-L", "Open a different QWK file");
                helpLine("S / Ctrl-U", "Settings");
                helpLine("C / ESC", "Back to conference list");
                helpLine("Q", "Quit SlyMail");
                r += 2;
                printAt(r, 2, "Press any key to continue...",
                    tAttr(TC_GREEN, TC_BLACK, false));
                g_term->refresh();
                g_term->getKey();
                break;
            }
            default:
                break;
        }
    }
}
