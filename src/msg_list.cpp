#include "msg_list.h"
#include "ui_common.h"
#include "search.h"
#include "msg_editor.h"
#include "i18n.h"
#include <cstdio>
#include <ctime>
#include <fstream>
#include <filesystem>
#include <cstdlib>

using std::string;
using std::vector;
using std::pair;

// Forward declaration — defined in main.cpp file-locally; we implement a
// local helper here for editing an existing pending reply that mirrors the
// external/builtin selection logic used by the reply paths in main.cpp.
namespace {

// Format a time_t as YYYY-MM-DD
static string formatPendingDate(std::time_t t)
{
    if (t == 0) return "";
    std::tm tmv{};
#if defined(_WIN32)
    localtime_s(&tmv, &t);
#else
    localtime_r(&t, &tmv);
#endif
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d",
                  tmv.tm_year + 1900, tmv.tm_mon + 1, tmv.tm_mday);
    return string(buf);
}

// Format a time_t as HH:MM
static string formatPendingTime(std::time_t t)
{
    if (t == 0) return "";
    std::tm tmv{};
#if defined(_WIN32)
    localtime_s(&tmv, &t);
#else
    localtime_r(&t, &tmv);
#endif
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%02d:%02d", tmv.tm_hour, tmv.tm_min);
    return string(buf);
}

// Edit an existing pending reply using the external editor if one is
// configured, else the built-in editor. Populates newTo/newSubject/newBody
// and sets 'changed' if anything differs from the original.
static EditorResult editPendingWithExternalOrBuiltin(
    const string& userName,
    const string& confName,
    const QwkReply& orig,
    string& newTo,
    string& newSubject,
    string& newBody,
    bool& changed,
    Settings& settings,
    const string& baseDir)
{
    changed = false;
    newTo = orig.to;
    newSubject = orig.subject;
    newBody = orig.body;

    const auto* edCfg = (settings.useExternalEditor ? settings.getSelectedEditor() : nullptr);
    if (!edCfg)
    {
        return editExistingMessage(userName, confName,
                                   orig.to, orig.subject, orig.body,
                                   newTo, newSubject, newBody, changed,
                                   settings, baseDir);
    }

    // External editor: write body to a temp file, launch, read back.
    namespace fs = std::filesystem;
    string tmpDir = baseDir.empty() ? string(".") : baseDir;
    string tmpFile = tmpDir + "/slymail_edit_pending.tmp";
    {
        std::ofstream ofs(tmpFile, std::ios::binary | std::ios::trunc);
        if (!ofs)
        {
            messageDialog(_("Error"), _("Could not create temp file for external editor."));
            return EditorResult::Aborted;
        }
        ofs << orig.body;
    }

    // Build the command line by substituting %f with the temp file path.
    string cmd = edCfg->commandLine;
    {
        size_t pos = cmd.find("%f");
        if (pos != string::npos)
        {
            cmd.replace(pos, 2, tmpFile);
        }
        else
        {
            cmd += " " + tmpFile;
        }
    }

    g_term->shutdown();
    int rc = std::system(cmd.c_str());
    (void)rc;
    g_term->init();

    // Read back
    std::ifstream ifs(tmpFile, std::ios::binary);
    if (!ifs)
    {
        messageDialog(_("Error"), _("Could not read back edited file."));
        std::error_code ec;
        fs::remove(tmpFile, ec);
        return EditorResult::Aborted;
    }
    string edited((std::istreambuf_iterator<char>(ifs)),
                  std::istreambuf_iterator<char>());
    ifs.close();
    std::error_code ec;
    fs::remove(tmpFile, ec);

    newBody = edited;

    // Compare, ignoring trailing newline differences
    string a = orig.body, b = edited;
    while (!a.empty() && (a.back() == '\n' || a.back() == '\r')) a.pop_back();
    while (!b.empty() && (b.back() == '\n' || b.back() == '\r')) b.pop_back();
    if (a != b)
    {
        changed = true;
    }
    return EditorResult::Saved;
}

} // namespace

bool showEditPendingMessagesDialog(vector<QwkReply>& pendingReplies,
                                   Settings& settings,
                                   const string& baseDir,
                                   std::function<void()> onMessageSaved)
{
    bool anyChanged = false;

    if (pendingReplies.empty())
    {
        messageDialog(_("Edit Pending"), _("No messages have been written this session."));
        return false;
    }

    int selected = 0;
    int scrollOffset = 0;
    bool needFullRedraw = true;

    while (true)
    {
        int COLS = g_term->getCols();
        int ROWS = g_term->getRows();

        const int toW   = 20;
        const int dateW = 10;
        const int timeW = 5;
        int subjW = COLS - toW - dateW - timeW - 6;
        if (subjW < 10) subjW = 10;

        const int listTop = 3;
        int listHeight = ROWS - 5;
        int total = static_cast<int>(pendingReplies.size());

        if (total == 0)
        {
            return anyChanged;
        }
        if (selected >= total) selected = total - 1;
        if (selected < 0) selected = 0;
        if (selected < scrollOffset) scrollOffset = selected;
        if (selected >= scrollOffset + listHeight)
            scrollOffset = selected - listHeight + 1;

        if (needFullRedraw)
        {
            g_term->clear();

            TermAttr borderAttr = tAttr(TC_BLUE, TC_BLACK, true);
            TermAttr titleAttr  = tAttr(TC_GREEN, TC_BLACK, true);

            g_term->setAttr(borderAttr);
            g_term->putCP437(0, 0, CP437_BOX_DRAWINGS_UPPER_LEFT_SINGLE);
            g_term->drawHLine(0, 1, COLS - 2);
            g_term->putCP437(0, COLS - 1, CP437_BOX_DRAWINGS_UPPER_RIGHT_SINGLE);
            string title = string(" ") + _("Edit Pending Messages") + " (" + std::to_string(total) + ") ";
            printAt(0, 4, title, titleAttr);

            g_term->setAttr(borderAttr);
            g_term->putCP437(1, 0, CP437_BOX_DRAWINGS_LOWER_LEFT_SINGLE);
            g_term->drawHLine(1, 1, COLS - 2);
            g_term->putCP437(1, COLS - 1, CP437_BOX_DRAWINGS_LOWER_RIGHT_SINGLE);

            TermAttr colAttr = tAttr(TC_CYAN, TC_BLACK, true);
            printAt(2, 1, padStr(_("To"), toW), colAttr);
            printAt(2, 1 + toW + 1, padStr(_("Subject"), subjW), colAttr);
            printAt(2, 1 + toW + subjW + 2, padStr(_("Date"), dateW), colAttr);
            printAt(2, 1 + toW + subjW + dateW + 3, padStr(_("Time"), timeW), colAttr);

            for (int i = 0; i < listHeight && (scrollOffset + i) < total; ++i)
            {
                int idx = scrollOffset + i;
                const auto& r = pendingReplies[idx];
                int y = listTop + i;
                bool isSel = (idx == selected);

                string toDisp = r.to;
                {
                    string t = trimStr(toDisp);
                    string tu;
                    for (char c : t) tu.push_back(static_cast<char>(std::toupper((unsigned char)c)));
                    if (t.empty() || tu == "ALL")
                        toDisp = "All";
                }

                string dateStr = formatPendingDate(r.timestamp);
                string timeStr = formatPendingTime(r.timestamp);

                TermAttr bg = isSel
                    ? tAttr(TC_BLUE, TC_WHITE, false)
                    : tAttr(TC_BLACK, TC_BLACK, false);
                fillRow(y, bg);

                TermAttr toC   = isSel ? tAttr(TC_BLUE, TC_WHITE, false) : tAttr(TC_CYAN, TC_BLACK, false);
                TermAttr subC  = isSel ? tAttr(TC_BLACK, TC_WHITE, false) : tAttr(TC_CYAN, TC_BLACK, true);
                TermAttr dtC   = isSel ? tAttr(TC_GREEN, TC_WHITE, false) : tAttr(TC_GREEN, TC_BLACK, false);

                printAt(y, 1, padStr(truncateStr(toDisp, toW), toW), toC);
                printAt(y, 1 + toW + 1, padStr(truncateStr(r.subject, subjW), subjW), subC);
                printAt(y, 1 + toW + subjW + 2, padStr(dateStr, dateW), dtC);
                printAt(y, 1 + toW + subjW + dateW + 3, padStr(timeStr, timeW), dtC);
            }

            if (total > listHeight)
            {
                drawScrollbar(listTop, listHeight, selected, total,
                              tAttr(TC_BLACK, TC_BLACK, true),
                              tAttr(TC_WHITE, TC_BLACK, true));
            }

            drawDDHelpBar(ROWS - 1,
                _("Up/Dn/PgUp/PgDn/HOME/END, "),
                {{'E', _("nter=Edit")}, {'Q', _("uit/ESC")}});

            needFullRedraw = false;
        }

        g_term->refresh();

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:
                if (selected > 0) --selected;
                needFullRedraw = true;
                break;
            case TK_DOWN:
                if (selected < total - 1) ++selected;
                needFullRedraw = true;
                break;
            case TK_PGUP:
                selected -= listHeight;
                if (selected < 0) selected = 0;
                needFullRedraw = true;
                break;
            case TK_PGDN:
                selected += listHeight;
                if (selected >= total) selected = total - 1;
                needFullRedraw = true;
                break;
            case TK_HOME:
                selected = 0;
                needFullRedraw = true;
                break;
            case TK_END:
                selected = total - 1;
                needFullRedraw = true;
                break;
            case TK_ENTER:
            {
                QwkReply& orig = pendingReplies[selected];
                string newTo, newSubject, newBody;
                bool changed = false;
                EditorResult r = editPendingWithExternalOrBuiltin(
                    settings.userName,
                    "",
                    orig,
                    newTo, newSubject, newBody, changed,
                    settings, baseDir);

                if (r == EditorResult::Saved && changed)
                {
                    // The user successfully exited the editor, so save the
                    // updated message back into the pending-replies vector
                    // and notify the caller so the .rep packet can be
                    // re-written if it had previously been saved to disk.
                    orig.to = newTo;
                    orig.subject = newSubject;
                    orig.body = newBody;
                    orig.timestamp = std::time(nullptr);
                    anyChanged = true;
                    if (onMessageSaved)
                        onMessageSaved();
                }
                needFullRedraw = true;
                break;
            }
            case 'q':
            case 'Q':
            case TK_ESCAPE:
                return anyChanged;
            case TK_RESIZE:
                needFullRedraw = true;
                break;
            default:
                break;
        }
    }
}


// Show the conference list
ConfListResult showConferenceList(QwkPacket& packet, int& selectedConf,
                                 Settings& settings,
                                 vector<QwkReply>* pendingReplies,
                                 const string& baseDir,
                                 std::function<void()> onPendingEdited,
                                 std::function<int(int)> getLastReadFn)
{
    // Returns true if the given conference has any messages newer than the
    // last-read pointer for that conference.  If no getLastReadFn is provided,
    // falls back to the old behavior (any messages == new).
    auto confHasNew = [&](const QwkConference& conf) -> bool {
        if (conf.messages.empty()) return false;
        if (!getLastReadFn) return true;
        int lr = getLastReadFn(conf.number);
        if (lr < 0) return true;
        for (const auto& m : conf.messages)
        {
            if (m.number > lr) return true;
        }
        return false;
    };
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
            if (settings.onlyShowAreasWithNewMail && !confHasNew(packet.conferences[i]))
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

            bool hasNew = confHasNew(conf);
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
            printAt(1, 2, _("Sysop: "), tAttr(TC_CYAN, TC_BLACK, false));
            printAt(1, 9, packet.info.sysopName, tAttr(TC_WHITE, TC_BLACK, true));
            string userInfo = string(_("User: ")) + packet.info.userName;
            printAt(1, COLS / 2, userInfo, tAttr(TC_CYAN, TC_BLACK, false));
            string totalStr = std::to_string(packet.totalMessages()) + " " + _("msgs");
            printAt(1, COLS - static_cast<int>(totalStr.size()) - 3, totalStr,
                    tAttr(TC_GREEN, TC_BLACK, false));

            g_term->setAttr(borderAttr);
            g_term->putCP437(2, 0, CP437_BOX_DRAWINGS_LOWER_LEFT_SINGLE);
            g_term->drawHLine(2, 1, COLS - 2);
            g_term->putCP437(2, COLS - 1, CP437_BOX_DRAWINGS_LOWER_RIGHT_SINGLE);

            TermAttr colAttr = tAttr(TC_CYAN, TC_BLACK, true);
            int msgsColHdr = COLS - countW - newW - 2;
            int newColHdr  = COLS - newW - 1;
            printAt(3, 1, padStr(_("Conf#"), numW), colAttr);
            printAt(3, 1 + numW + 1, padStr(_("Conference Name"), nameW), colAttr);
            printAt(3, msgsColHdr, padStr(_("Msgs"), countW), colAttr);
            printAt(3, newColHdr, padStr(_("New"), newW), colAttr);

            for (int i = 0; i < listHeight && (scrollOffset + i) < totalConfs; ++i)
                drawRow(scrollOffset + i);

            drawSB();

            drawDDHelpBar(ROWS - 1,
                _("Up/Dn/PgUp/PgDn/HOME/END, "),
                {{'E', _("dit pending")}, {'/', _("Search")}, {'G', _("o to #")},
                 {'O', _("pen file")}, {'S', _("ettings")}, {'Q', _("uit")}, {'?', ""}});

            // Show filter indicator if active
            if (isFiltered)
            {
                string filterMsg = string(" ") + _("Filter:") + " \"" + searchLabel + "\" (" +
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
                        messageDialog(_("Search"), _("No conferences found matching the search."));
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
            case 'e':
            case 'E':
                if (pendingReplies != nullptr)
                {
                    showEditPendingMessagesDialog(*pendingReplies, settings, baseDir, onPendingEdited);
                    needFullRedraw = true;
                }
                break;
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
                const string goToConfLabel = _("Go to conf #: ");
                printAt(ROWS - 1, 0, goToConfLabel,
                        tAttr(TC_WHITE, TC_BLACK, true));
                g_term->clearToEol();
                string numStr = getNumericInput(ROWS - 1, static_cast<int>(goToConfLabel.size()), 8,
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
                printCentered(r++, _("Conference List Help"),
                    tAttr(TC_CYAN, TC_BLACK, true));
                TermAttr keyC  = tAttr(TC_CYAN, TC_BLACK, true);
                TermAttr descC = tAttr(TC_CYAN, TC_BLACK, false);
                auto helpLine = [&](const string& key, const string& desc)
                {
                    printAt(r, 2, padStr(key, 20), keyC);
                    printAt(r, 24, ": " + desc, descC);
                    ++r;
                };
                helpLine("Up/Down arrow", _("Navigate conferences"));
                helpLine("Enter", _("Open selected conference"));
                helpLine("PageUp/PageDown", _("Scroll up/down a page"));
                helpLine("HOME/END", _("Jump to first/last conference"));
                helpLine("/", _("Search/filter conferences"));
                helpLine("E", _("Edit pending messages (session)"));
                helpLine("V", _("View polls/votes in packet"));
                helpLine("O / Ctrl-L", _("Open a different QWK file"));
                helpLine("Ctrl-R", _("Remote systems (download QWK)"));
                helpLine("Ctrl-P", _("Save REP reply packet"));
                helpLine("S / Ctrl-U", _("Settings"));
                helpLine("Q / ESC", _("Quit SlyMail"));
                helpLine("? / F1", _("This help screen"));
                r += 2;
                printAt(r, 2, _("Press any key to continue..."),
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
                              int lastReadMsgNum,
                              vector<QwkReply>* pendingReplies,
                              const string& baseDir,
                              std::function<void()> onPendingEdited)
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
            printAt(0, 0, padStr(_("Msg#"), numW), colHdrAttr);
            printAt(0, numW + 1, padStr(_("From"), fromW), colHdrAttr);
            printAt(0, numW + fromW + 2, padStr(_("To"), toW), colHdrAttr);
            printAt(0, numW + fromW + toW + 3, padStr(_("Subject"), subjW), colHdrAttr);
            printAt(0, COLS - dateW - timeW - 2, padStr(_("Date"), dateW), colHdrAttr);
            printAt(0, COLS - timeW - 1, padStr(_("Time"), timeW), colHdrAttr);

            for (int i = 0; i < listHeight && (scrollOffset + i) < totalMsgs; ++i)
                drawRow(scrollOffset + i);

            drawSB();

            // Status line (static for the lifetime of this message list)
            string confTitle = bbsName + " - " + conf.name
                + " (" + std::to_string(totalMsgs) + " " + _("msgs") + ")";
            printAt(ROWS - 2, 0, truncateStr(confTitle, COLS),
                    tAttr(TC_CYAN, TC_BLACK, false));

            drawDDHelpBar(ROWS - 1,
                _("Up/Dn/PgUp/PgDn/HOME/END, "),
                {{'N', _("ew msg")}, {'R', _("ead")}, {'E', _("dit pending")}, {'/', _("Search")},
                 {'G', _("o to #")}, {'C', _("onf list")}, {'Q', _("uit")}, {'?', ""}});

            // Show filter indicator if active
            if (isFiltered)
            {
                string filterMsg = string(" ") + _("Filter:") + " \"" + searchLabel + "\" (" +
                    std::to_string(totalMsgs) + "/" +
                    std::to_string(conf.messages.size()) + " " + _("msgs") + ") ";
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
                        messageDialog(_("Search"), _("No messages found matching the search."));
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
            case 'e':
            case 'E':
                if (pendingReplies != nullptr)
                {
                    showEditPendingMessagesDialog(*pendingReplies, settings, baseDir, onPendingEdited);
                    needFullRedraw = true;
                }
                break;
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
                const string goToMsgLabel = _("Go to msg #: ");
                printAt(ROWS - 1, 0, goToMsgLabel,
                        tAttr(TC_WHITE, TC_BLACK, true));
                g_term->clearToEol();
                string numStr = getNumericInput(ROWS - 1, static_cast<int>(goToMsgLabel.size()), 8,
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
                printCentered(r++, _("Message List Help"),
                    tAttr(TC_CYAN, TC_BLACK, true));
                TermAttr keyC  = tAttr(TC_CYAN, TC_BLACK, true);
                TermAttr descC = tAttr(TC_CYAN, TC_BLACK, false);
                auto helpLine = [&](const string& key, const string& desc)
                {
                    printAt(r, 2, padStr(key, 20), keyC);
                    printAt(r, 24, ": " + desc, descC);
                    ++r;
                };
                helpLine("Up/Down arrow", _("Navigate messages"));
                helpLine("Enter / R", _("Read selected message"));
                helpLine("N", _("Write a new message"));
                helpLine("E", _("Edit pending messages (session)"));
                helpLine("G", _("Go to message number"));
                helpLine("PageUp/PageDown", _("Scroll up/down a page"));
                helpLine("HOME/END", _("Jump to first/last message"));
                helpLine("/", _("Search/filter messages"));
                helpLine("Ctrl-L", _("Open a different QWK file"));
                helpLine("Ctrl-R", _("Remote systems (download QWK)"));
                helpLine("Ctrl-P", _("Save REP reply packet"));
                helpLine("S / Ctrl-U", _("Settings"));
                helpLine("C / ESC", _("Back to conference list"));
                helpLine("Q", _("Quit SlyMail"));
                helpLine("? / F1", _("This help screen"));
                r += 2;
                printAt(r, 2, _("Press any key to continue..."),
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
