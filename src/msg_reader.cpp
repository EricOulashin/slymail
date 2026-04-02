#include "msg_reader.h"
#include "bbs_colors.h"
#include "utf8_util.h"
#include "ansi_render.h"

using std::string;
using std::vector;

// Determine color for a message body line
TermAttr getLineAttr(const string& line, const Settings& settings)
{
    if (line.empty())
    {
        return tAttr(TC_WHITE, TC_BLACK, false);
    }

    // Quote lines: check for "> " or "XX> " patterns
    {
        int depth = 0;
        bool hasQuote = false;
        for (size_t i = 0; i < line.size() && i < 6; ++i)
        {
            if (line[i] == '>')
            {
                ++depth;
                hasQuote = true;
            }
            else if (line[i] != ' ' && !hasQuote)
            {
                continue;
            }
            else if (line[i] == ' ' && hasQuote)
            {
                break;
            }
            else if (!hasQuote)
            {
                break;
            }
        }
        if (depth >= 2)
        {
            return tAttr(TC_GREEN, TC_BLACK, false);
        }
        if (depth >= 1)
        {
            return tAttr(TC_CYAN, TC_BLACK, false);
        }
    }

    // Tear line
    if (line.size() >= 3 && (line.substr(0, 3) == "---" || line.substr(0, 3) == "-!-"))
    {
        if (settings.showTearLine)
        {
            return tAttr(TC_YELLOW, TC_BLACK, false);
        }
        return TermAttr();
    }

    // Origin line
    if (line.find(" * Origin:") == 0)
    {
        return tAttr(TC_GREEN, TC_BLACK, false);
    }

    // Kludge lines
    if (line[0] == '@' || line.find("SEEN-BY:") == 0 || line.find("PATH:") == 0)
    {
        if (!settings.showKludgeLines)
        {
            return TermAttr();
        }
        return tAttr(TC_MAGENTA, TC_BLACK, false);
    }

    return tAttr(TC_WHITE, TC_BLACK, false);
}

// Build Synchronet-style poll result display lines.
// Each line is stored as plain text; the rendering function handles the
// backfill color effect based on embedded markers.
static vector<string> buildPollDisplayLines(const QwkMessage& msg,
                                             const VotingData* votingData,
                                             int displayWidth)
{
    vector<string> lines;

    if (!votingData || msg.pollIndex < 0 ||
        msg.pollIndex >= static_cast<int>(votingData->polls.size()))
    {
        lines.push_back("(Poll data not available)");
        return lines;
    }

    const VotingPoll& poll = votingData->polls[msg.pollIndex];

    // Poll question
    lines.push_back(poll.question);
    lines.push_back("");

    // Comments
    for (const auto& comment : poll.comments)
    {
        lines.push_back("  " + comment);
    }
    if (!poll.comments.empty())
    {
        lines.push_back("");
    }

    // Calculate total votes
    int totalVotes = 0;
    for (const auto& answer : poll.answers)
    {
        totalVotes += answer.votes;
    }

    // Find the longest answer text for alignment
    int maxAnswerLen = 0;
    for (const auto& answer : poll.answers)
    {
        int len = static_cast<int>(answer.text.size());
        if (len > maxAnswerLen) maxAnswerLen = len;
    }
    // Cap the answer display width
    int answerWidth = maxAnswerLen;
    if (answerWidth > displayWidth - 20) answerWidth = displayWidth - 20;
    if (answerWidth < 10) answerWidth = 10;

    // Display each answer with vote count and percentage bar
    for (size_t i = 0; i < poll.answers.size(); ++i)
    {
        const auto& answer = poll.answers[i];
        float pct = (totalVotes > 0) ? (static_cast<float>(answer.votes) / totalVotes) * 100.0f : 0.0f;

        // Format: " N: Answer text         [count  pct%] checkmark"
        char numBuf[8];
        snprintf(numBuf, sizeof(numBuf), "%2d", static_cast<int>(i + 1));

        string ansText = answer.text;
        if (static_cast<int>(ansText.size()) > answerWidth)
        {
            ansText = ansText.substr(0, answerWidth - 3) + "...";
        }
        // Pad answer to answerWidth
        while (static_cast<int>(ansText.size()) < answerWidth)
        {
            ansText += ' ';
        }

        char statBuf[32];
        snprintf(statBuf, sizeof(statBuf), " [%-4d %3.0f%%]", answer.votes, pct);

        string checkMark;
        if (msg.userVoted & (1 << i))
        {
            checkMark = " *"; // Indicates the user voted for this answer
        }

        // The line includes a special marker for the backfill renderer:
        // \x01P<pct>\x01 at the start indicates this is a poll answer line
        // with the given percentage for backfill coloring.
        char pctMarker[16];
        snprintf(pctMarker, sizeof(pctMarker), "\x01P%03d\x01", static_cast<int>(pct));
        string fullLine = string(pctMarker) + string(numBuf) + ": " + ansText + statBuf + checkMark;

        lines.push_back(fullLine);
    }

    lines.push_back("");
    lines.push_back("Total votes: " + std::to_string(totalVotes));

    if (poll.closed)
    {
        lines.push_back("[Poll is closed]");
    }

    if (poll.maxVotes > 1)
    {
        lines.push_back("(Up to " + std::to_string(poll.maxVotes) + " selections allowed)");
    }

    return lines;
}

// Filter and prepare message body lines
vector<string> prepareBodyLines(const QwkMessage& msg,
                                const Settings& settings,
                                int displayWidth)
{
    vector<string> result;
    std::istringstream stream(msg.body);
    string line;

    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        if (!settings.showKludgeLines)
        {
            if (!line.empty() && line[0] == '@')
            {
                continue;
            }
            if (line.find("SEEN-BY:") == 0)
            {
                continue;
            }
            if (line.find("PATH:") == 0)
            {
                continue;
            }
        }
        if (!settings.showTearLine)
        {
            if (line.size() >= 3 && (line.substr(0, 3) == "---" || line.substr(0, 3) == "-!-"))
            {
                continue;
            }
            if (line.find(" * Origin:") == 0)
            {
                continue;
            }
        }

        if (static_cast<int>(line.size()) > displayWidth)
        {
            auto wrapped = wordWrap(line, displayWidth);
            for (auto& wl : wrapped)
            {
                result.push_back(wl);
            }
        }
        else
        {
            result.push_back(line);
        }
    }
    return result;
}

// Draw the DDMsgReader-style enhanced message header with box-drawing border
// Matches the screenshot: bordered box with group/area names in top border,
// labeled fields inside (Msg#, From, To, Subj, Date)
// Returns the number of rows used
int drawMessageHeader(const QwkMessage& msg, const string& confName,
                      const string& groupName,
                      int msgIndex, int totalMsgs)
{
    TermAttr borderAttr = tAttr(TC_BLUE, TC_BLACK, true);
    TermAttr labelAttr  = tAttr(TC_CYAN, TC_BLACK, false);
    TermAttr valueAttr  = tAttr(TC_WHITE, TC_BLACK, true);
    TermAttr areaAttr   = tAttr(TC_GREEN, TC_BLACK, true);
    int y = 0;
    int w = g_term->getCols();

    // Row 0: Top border with embedded group and area names
    g_term->setAttr(borderAttr);
    g_term->putCP437(y, 0, CP437_BOX_DRAWINGS_UPPER_LEFT_SINGLE);
    g_term->drawHLine(y, 1, w - 2);
    g_term->putCP437(y, w - 1, CP437_BOX_DRAWINGS_UPPER_RIGHT_SINGLE);

    // Embed group name near left
    if (!groupName.empty())
    {
        printAt(y, 5, " " + groupName + " ", areaAttr);
    }
    // Embed conference/area name centered
    if (!confName.empty())
    {
        int confX = (w - static_cast<int>(confName.size())) / 2;
        if (confX < 20)
        {
            confX = 20;
        }
        printAt(y, confX, " " + confName + " ", areaAttr);
    }
    ++y;

    // Side borders for header lines
    auto drawSideBorders = [&](int row)
    {
        g_term->setAttr(borderAttr);
        g_term->putCP437(row, 0, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
        g_term->putCP437(row, w - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
    };

    // Row 1: Msg#: NN/TOTAL
    drawSideBorders(y);
    printAt(y, 1, "Msg#: ", labelAttr);
    string msgNumStr = std::to_string(msgIndex + 1) + "/" + std::to_string(totalMsgs);
    printAt(y, 7, msgNumStr, valueAttr);
    // Show indicators on the right side
    int indicatorX = w - 2;
    if (msg.isPrivate)
    {
        string pvt = "[Private]";
        indicatorX -= static_cast<int>(pvt.size());
        printAt(y, indicatorX, pvt, tAttr(TC_RED, TC_BLACK, true));
    }
    if (msg.hasAttachment)
    {
        string att = "[ATT]";
        indicatorX -= static_cast<int>(att.size()) + 1;
        printAt(y, indicatorX, att, tAttr(TC_YELLOW, TC_BLACK, true));
    }
    if (msg.utf8)
    {
        string u8 = "[UTF8]";
        indicatorX -= static_cast<int>(u8.size()) + 1;
        printAt(y, indicatorX, u8, tAttr(TC_GREEN, TC_BLACK, false));
    }
    ++y;

    // Row 2: From:
    drawSideBorders(y);
    printAt(y, 1, "From: ", labelAttr);
    printAt(y, 7, msg.from, valueAttr);
    ++y;

    // Row 3: To  :
    drawSideBorders(y);
    printAt(y, 1, "To  : ", labelAttr);
    printAt(y, 7, msg.to, valueAttr);
    ++y;

    // Row 4: Subj:
    drawSideBorders(y);
    printAt(y, 1, "Subj: ", labelAttr);
    printAt(y, 7, truncateStr(msg.subject, w - 9), valueAttr);
    ++y;

    // Row 5: Date:
    drawSideBorders(y);
    printAt(y, 1, "Date: ", labelAttr);
    printAt(y, 7, msg.date + "  " + msg.time, valueAttr);
    if (msg.replyTo > 0)
    {
        string replyStr = "Reply to #" + std::to_string(msg.replyTo);
        printAt(y, w - static_cast<int>(replyStr.size()) - 2, replyStr,
                tAttr(TC_YELLOW, TC_BLACK, false));
    }
    ++y;

    // Row 6 (optional): Vote tally or poll indicator
    if (msg.isPoll || msg.upvotes > 0 || msg.downvotes > 0)
    {
        drawSideBorders(y);
        if (msg.isPoll)
        {
            printAt(y, 1, "Poll: ", labelAttr);
            string pollInfo = "Press V to vote";
            if (msg.userVoted != 0)
            {
                pollInfo = "You have voted on this poll";
            }
            printAt(y, 7, pollInfo, tAttr(TC_YELLOW, TC_BLACK, true));
        }
        else
        {
            // Show upvote/downvote tally like Synchronet
            printAt(y, 1, "Votes:", labelAttr);
            string voteStr = " Up " + std::to_string(msg.upvotes);
            if (msg.userVoted == 1)
            {
                voteStr += " *";
            }
            voteStr += "  Down " + std::to_string(msg.downvotes);
            if (msg.userVoted == 2)
            {
                voteStr += " *";
            }
            int score = static_cast<int>(msg.upvotes) - static_cast<int>(msg.downvotes);
            voteStr += "  (Score: " + std::to_string(score) + ")";
            printAt(y, 7, voteStr, tAttr(TC_GREEN, TC_BLACK, false));
        }
        ++y;
    }

    // Bottom border
    g_term->setAttr(borderAttr);
    g_term->putCP437(y, 0, CP437_BOX_DRAWINGS_LOWER_LEFT_SINGLE);
    g_term->drawHLine(y, 1, w - 2);
    g_term->putCP437(y, w - 1, CP437_BOX_DRAWINGS_LOWER_RIGHT_SINGLE);
    ++y;

    return y;
}

// Draw the DDMsgReader-style help bar at the bottom
void drawReaderHelpBar(int y)
{
    drawDDHelpBar(y, "Up/Dn/<-/->/PgUp/PgDn, ",
                  {{'F', "irst"}, {'L', "ast"}, {'R', "eply"}, {'V', "ote"},
                   {'D', "wnld"}, {'H', "dr"}, {'S', "et"}, {'C', "onf"}, {'Q', "uit"}, {'?', ""}});
}

// Build a list of header information lines for display
vector<string> buildHeaderInfoLines(const QwkMessage& msg,
                                    const string& confName)
{
    vector<string> lines;

    lines.push_back("Message Header Information");
    lines.push_back(string(40, '-'));
    lines.push_back("");
    lines.push_back("    Msg Num: " + std::to_string(msg.number));
    lines.push_back("       From: " + msg.from);
    lines.push_back("         To: " + msg.to);
    lines.push_back("    Subject: " + msg.subject);
    lines.push_back("       Date: " + msg.date);
    lines.push_back("       Time: " + msg.time);
    lines.push_back(" Conference: " + confName + " (#" + std::to_string(msg.conference) + ")");
    lines.push_back("   Reply To: " + (msg.replyTo > 0 ? std::to_string(msg.replyTo) : "(none)"));

    // Status
    string statusStr;
    switch (msg.status)
    {
        case QwkStatus::NewPublic:    statusStr = "New (public)"; break;
        case QwkStatus::OldPublic:    statusStr = "Read (public)"; break;
        case QwkStatus::NewPrivate:   statusStr = "New (private)"; break;
        case QwkStatus::OldPrivate:   statusStr = "Read (private)"; break;
        case QwkStatus::Comment:      statusStr = "Comment"; break;
        case QwkStatus::Vote:         statusStr = "Vote/Poll"; break;
        default:                      statusStr = "Unknown"; break;
    }
    lines.push_back("     Status: " + statusStr);
    lines.push_back("    Private: " + string(msg.isPrivate ? "Yes" : "No"));

    // Add blank line then show any kludge lines from the body
    lines.push_back("");
    lines.push_back("Kludge Lines");
    lines.push_back(string(40, '-'));

    bool foundKludge = false;
    std::istringstream stream(msg.body);
    string line;
    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        if (!line.empty() && line[0] == '@')
        {
            lines.push_back(line);
            foundKludge = true;
        }
        else if (line.find("SEEN-BY:") == 0 || line.find("PATH:") == 0)
        {
            lines.push_back(line);
            foundKludge = true;
        }
    }
    if (!foundKludge)
    {
        lines.push_back("(none)");
    }

    return lines;
}

// Show message header information in a scrollable view
// Uses the same body area as the message reader
void showHeaderInfo(const QwkMessage& msg, const string& confName,
                    int msgIndex, int totalMsgs, const Settings& settings)
{
    auto headerLines = buildHeaderInfoLines(msg, confName);
    int totalLines = static_cast<int>(headerLines.size());
    int scrollPos = 0;

    TermAttr labelAttr = tAttr(TC_CYAN, TC_BLACK, true);
    TermAttr valueAttr = tAttr(TC_WHITE, TC_BLACK, false);
    TermAttr kludgeAttr = tAttr(TC_MAGENTA, TC_BLACK, false);
    TermAttr separatorAttr = tAttr(TC_BLUE, TC_BLACK, true);
    TermAttr helpAttr = tAttr(TC_GREEN, TC_BLACK, false);

    int displayWidth = settings.useScrollbar ? g_term->getCols() - 2 : g_term->getCols() - 1;

    while (true)
    {
        g_term->clear();

        // Draw the same message header at the top
        int headerHeight = drawMessageHeader(msg, confName, "SlyMail",
                                              msgIndex, totalMsgs);

        // Body area shows header info
        int bodyTop = headerHeight;
        int bodyHeight = g_term->getRows() - headerHeight - 1;

        for (int i = 0; i < bodyHeight && (scrollPos + i) < totalLines; ++i)
        {
            int lineIdx = scrollPos + i;
            const string& line = headerLines[lineIdx];
            int row = bodyTop + i;

            // Color based on line content
            TermAttr lineAttr = valueAttr;
            if (line.size() > 0 && line[0] == '-')
            {
                lineAttr = separatorAttr;
            }
            else if (line.find("Message Header") == 0 || line.find("Kludge Lines") == 0)
            {
                lineAttr = labelAttr;
            }
            else if (line.size() > 0 && line[0] == '@')
            {
                lineAttr = kludgeAttr;
            }
            else if (line.find("SEEN-BY:") == 0 || line.find("PATH:") == 0)
            {
                lineAttr = kludgeAttr;
            }
            else if (line.size() > 4 && line.find(": ") != string::npos
                     && line[0] == ' ')
            {
                // Header field line - color the label part cyan, value part white
                size_t colonPos = line.find(": ");
                if (colonPos != string::npos)
                {
                    string lbl = line.substr(0, colonPos + 2);
                    string val = line.substr(colonPos + 2);
                    printAt(row, 0, lbl, labelAttr);
                    printAt(row, static_cast<int>(lbl.size()), padStr(val, displayWidth - static_cast<int>(lbl.size())), valueAttr);
                    continue;
                }
            }

            printAt(row, 0, padStr(line, displayWidth), lineAttr);
        }

        // Scrollbar
        if (settings.useScrollbar && totalLines > bodyHeight)
        {
            int scrollMax = std::max(1, totalLines - bodyHeight);
            int sbPos = (totalLines <= 1) ? 0
                      : (scrollPos * (totalLines - 1)) / scrollMax;
            drawScrollbar(bodyTop, bodyHeight, sbPos, totalLines,
                         tAttr(TC_BLACK, TC_BLACK, true),
                         tAttr(TC_WHITE, TC_BLACK, true));
        }

        // Help bar
        fillRow(g_term->getRows() - 1, tAttr(TC_BLACK, TC_BLACK, false));
        printAt(g_term->getRows() - 1, 1,
                "Up/Dn/PgUp/PgDn=Scroll, Q/ESC=Close headers",
                helpAttr);

        g_term->refresh();

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:
                if (scrollPos > 0) --scrollPos;
                break;
            case TK_DOWN:
                if (scrollPos < totalLines - bodyHeight && totalLines > bodyHeight)
                    ++scrollPos;
                break;
            case TK_PGUP:
                scrollPos -= bodyHeight;
                if (scrollPos < 0) scrollPos = 0;
                break;
            case TK_PGDN:
                scrollPos += bodyHeight;
                if (scrollPos > totalLines - bodyHeight)
                    scrollPos = std::max(0, totalLines - bodyHeight);
                break;
            case TK_HOME:
                scrollPos = 0;
                break;
            case TK_END:
                scrollPos = std::max(0, totalLines - bodyHeight);
                break;
            case 'q': case 'Q':
            case 'h': case 'H':
            case TK_ESCAPE:
                return;  // Close header view, caller will redraw message
            default:
                break;
        }
    }
}

// Save attachments from a message
void downloadAttachments(const QwkMessage& msg, const string& extractDir)
{
    if (!msg.hasAttachment || msg.attachmentFiles.empty())
    {
        messageDialog("Attachments", "No file attachments found.");
        return;
    }

    // Show list of attachments and prompt for save location
    int cols = g_term->getCols();
    int rows = g_term->getRows();

    int dlgH = static_cast<int>(msg.attachmentFiles.size()) + 6;
    if (dlgH > rows - 4) dlgH = rows - 4;
    int dlgW = cols - 10;
    if (dlgW > 60) dlgW = 60;
    int dlgY = (rows - dlgH) / 2;
    int dlgX = (cols - dlgW) / 2;

    TermAttr borderAttr = tAttr(TC_CYAN, TC_BLACK, true);
    TermAttr textAttr = tAttr(TC_WHITE, TC_BLACK, true);
    TermAttr fileAttr = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr helpAttr = tAttr(TC_CYAN, TC_BLACK, false);

    // Clear dialog area
    for (int r = 0; r < dlgH; ++r)
    {
        fillRow(dlgY + r, tAttr(TC_WHITE, TC_BLACK, false), dlgX, dlgX + dlgW);
    }
    drawBox(dlgY, dlgX, dlgH, dlgW, borderAttr, "File Attachments", borderAttr);

    printAt(dlgY + 1, dlgX + 2, "Attached files:", textAttr);

    int fileRow = dlgY + 2;
    for (size_t i = 0; i < msg.attachmentFiles.size() && fileRow < dlgY + dlgH - 3; ++i)
    {
        string fname = msg.attachmentFiles[i];
        // Show file size if file exists
        string filePath = extractDir + PATH_SEP_STR + fname;
        string sizeStr;
        if (fs::exists(filePath))
        {
            auto sz = fs::file_size(filePath);
            if (sz >= 1048576)
                sizeStr = " (" + std::to_string(sz / 1048576) + " MB)";
            else if (sz >= 1024)
                sizeStr = " (" + std::to_string(sz / 1024) + " KB)";
            else
                sizeStr = " (" + std::to_string(sz) + " bytes)";
        }
        printAt(fileRow, dlgX + 4, fname + sizeStr, fileAttr);
        ++fileRow;
    }

    printAt(dlgY + dlgH - 2, dlgX + 2, "Save to directory (Enter for home):", helpAttr);

    // Get destination directory
    string homeDir = ".";
    const char* home = getenv("HOME");
    if (home) homeDir = home;
#ifdef _WIN32
    const char* userProfile = getenv("USERPROFILE");
    if (userProfile) homeDir = userProfile;
#endif

    string destDir = getStringInput(dlgY + dlgH - 1, dlgX + 2, dlgW - 4, homeDir,
                                     tAttr(TC_WHITE, TC_BLACK, true));

    if (destDir.empty())
    {
        return; // Cancelled
    }

    // Copy files
    int copied = 0;
    for (const auto& fname : msg.attachmentFiles)
    {
        string srcPath = extractDir + PATH_SEP_STR + fname;
        string dstPath = destDir + PATH_SEP_STR + fname;

        if (fs::exists(srcPath))
        {
            try
            {
                fs::copy_file(srcPath, dstPath, fs::copy_options::overwrite_existing);
                ++copied;
            }
            catch (const fs::filesystem_error& e)
            {
                messageDialog("Copy Error",
                    "Failed to copy " + fname + ": " + string(e.what()));
            }
            catch (...)
            {
                messageDialog("Copy Error", "Failed to copy " + fname);
            }
        }
    }

    if (copied > 0)
    {
        messageDialog("Download", std::to_string(copied) + " file(s) saved to " + destDir);
    }
    else
    {
        messageDialog("Download", "No files could be saved. Files may not exist in packet.");
    }
}

// ============================================================
// Voting UI
// ============================================================

// Show up/down vote prompt for a regular (non-poll) message.
// Modeled on Synchronet's VoteMsgUpDownOrQuit prompt.
static bool showUpDownVoteUI(const QwkMessage& msg, const string& userName,
                              PendingVote& voteOut)
{
    int rows = g_term->getRows();
    int y = rows - 1;

    TermAttr promptAttr = tAttr(TC_CYAN, TC_BLACK, true);
    TermAttr keyAttr = tAttr(TC_WHITE, TC_BLACK, true);

    // Show vote tally if any
    if (msg.upvotes > 0 || msg.downvotes > 0)
    {
        fillRow(y - 1, tAttr(TC_BLACK, TC_BLACK, false));
        string tally = "Current: Up " + std::to_string(msg.upvotes)
                     + ", Down " + std::to_string(msg.downvotes)
                     + " (Score: " + std::to_string(static_cast<int>(msg.upvotes) - static_cast<int>(msg.downvotes)) + ")";
        if (msg.userVoted == 1)
        {
            tally += " [You voted Up]";
        }
        else if (msg.userVoted == 2)
        {
            tally += " [You voted Down]";
        }
        printAt(y - 1, 1, tally, tAttr(TC_GREEN, TC_BLACK, false));
    }

    fillRow(y, tAttr(TC_BLACK, TC_BLACK, false));
    printAt(y, 1, "Vote for message: ", promptAttr);
    int x = 19;
    printAt(y, x, "U", keyAttr);
    printAt(y, x + 1, "p, ", promptAttr);
    x += 4;
    printAt(y, x, "D", keyAttr);
    printAt(y, x + 1, "own, or ", promptAttr);
    x += 9;
    printAt(y, x, "Q", keyAttr);
    printAt(y, x + 1, "uit: ", promptAttr);

    g_term->refresh();

    while (true)
    {
        int ch = g_term->getKey();
        if (ch == 'u' || ch == 'U')
        {
            voteOut.msgId = msg.msgId;
            voteOut.voter = userName;
            voteOut.conference = msg.conference;
            voteOut.upVote = true;
            voteOut.downVote = false;
            voteOut.votes = 0;
            return true;
        }
        else if (ch == 'd' || ch == 'D')
        {
            voteOut.msgId = msg.msgId;
            voteOut.voter = userName;
            voteOut.conference = msg.conference;
            voteOut.upVote = false;
            voteOut.downVote = true;
            voteOut.votes = 0;
            return true;
        }
        else if (ch == 'q' || ch == 'Q' || ch == TK_ESCAPE)
        {
            return false;
        }
    }
}

// Show poll ballot UI. Lets user toggle answer selections and cast vote.
// Modeled on Synchronet's mselect() ballot interface.
static bool showPollBallotUI(const QwkMessage& msg, const VotingData* votingData,
                              const string& userName, PendingVote& voteOut)
{
    if (!votingData || msg.pollIndex < 0 ||
        msg.pollIndex >= static_cast<int>(votingData->polls.size()))
    {
        messageDialog("Vote", "No poll data available for this message.");
        return false;
    }

    const VotingPoll& poll = votingData->polls[msg.pollIndex];

    if (poll.closed)
    {
        messageDialog("Vote", "This poll is closed.");
        return false;
    }

    if (poll.answers.empty())
    {
        messageDialog("Vote", "This poll has no answer options.");
        return false;
    }

    int cols = g_term->getCols();
    int rows = g_term->getRows();
    int maxVotes = (poll.maxVotes > 0) ? poll.maxVotes : 1;

    // Selections bitmask (toggle-able)
    uint16_t selections = 0;
    int selected = 0; // Currently highlighted answer
    int numAnswers = static_cast<int>(poll.answers.size());

    // Dialog sizing
    int dlgW = cols - 8;
    if (dlgW > 70) dlgW = 70;
    int dlgH = numAnswers + 8;
    if (dlgH > rows - 4) dlgH = rows - 4;
    int dlgX = (cols - dlgW) / 2;
    int dlgY = (rows - dlgH) / 2;

    TermAttr borderAttr = tAttr(TC_CYAN, TC_BLACK, true);
    TermAttr titleAttr = tAttr(TC_WHITE, TC_BLACK, true);
    TermAttr normalAttr = tAttr(TC_CYAN, TC_BLACK, false);
    TermAttr selectedAttr = tAttr(TC_WHITE, TC_BLUE, true);
    TermAttr checkedAttr = tAttr(TC_GREEN, TC_BLACK, true);
    TermAttr helpAttr = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr infoAttr = tAttr(TC_YELLOW, TC_BLACK, false);

    while (true)
    {
        // Count current selections
        int numSelected = 0;
        for (int i = 0; i < numAnswers && i < 16; ++i)
        {
            if (selections & (1 << i))
            {
                ++numSelected;
            }
        }

        // Draw dialog
        for (int r = 0; r < dlgH; ++r)
        {
            fillRow(dlgY + r, tAttr(TC_BLACK, TC_BLACK, false), dlgX, dlgX + dlgW);
        }
        drawBox(dlgY, dlgX, dlgH, dlgW, borderAttr, "Ballot", borderAttr);

        // Poll question
        string questionStr = poll.question;
        if (static_cast<int>(questionStr.size()) > dlgW - 4)
        {
            questionStr = questionStr.substr(0, dlgW - 7) + "...";
        }
        printAt(dlgY + 1, dlgX + 2, questionStr, titleAttr);

        // Comments (if any, show first line)
        if (!poll.comments.empty())
        {
            string comment = poll.comments[0];
            if (static_cast<int>(comment.size()) > dlgW - 4)
            {
                comment = comment.substr(0, dlgW - 7) + "...";
            }
            printAt(dlgY + 2, dlgX + 2, comment, infoAttr);
        }

        // Answers
        int answerTop = dlgY + 3;
        int maxVisible = dlgH - 6;
        int scrollOff = 0;
        if (selected >= maxVisible)
        {
            scrollOff = selected - maxVisible + 1;
        }

        for (int i = 0; i < maxVisible && (i + scrollOff) < numAnswers; ++i)
        {
            int ansIdx = i + scrollOff;
            int y = answerTop + i;
            bool isChecked = (selections & (1 << ansIdx)) != 0;
            bool isSel = (ansIdx == selected);

            string checkMark = isChecked ? "[*] " : "[ ] ";
            string numStr = std::to_string(ansIdx + 1) + ". ";
            string ansText = poll.answers[ansIdx].text;

            int maxAnsLen = dlgW - 12;
            if (static_cast<int>(ansText.size()) > maxAnsLen)
            {
                ansText = ansText.substr(0, maxAnsLen - 3) + "...";
            }

            string fullLine = checkMark + numStr + ansText;
            // Pad to fill width
            if (static_cast<int>(fullLine.size()) < dlgW - 4)
            {
                fullLine += string(dlgW - 4 - fullLine.size(), ' ');
            }

            TermAttr lineAttr;
            if (isSel)
            {
                lineAttr = selectedAttr;
            }
            else if (isChecked)
            {
                lineAttr = checkedAttr;
            }
            else
            {
                lineAttr = normalAttr;
            }

            printAt(y, dlgX + 2, fullLine, lineAttr);
        }

        // Vote count and help info
        int bottomY = dlgY + dlgH - 2;
        string selectInfo = "Selected: " + std::to_string(numSelected) + "/" + std::to_string(maxVotes);
        printAt(bottomY, dlgX + 2, selectInfo, infoAttr);

        string helpStr = "Up/Dn=Move, Space/Enter=Toggle, C=Cast vote, Q=Quit";
        if (static_cast<int>(helpStr.size()) > dlgW - 4)
        {
            helpStr = helpStr.substr(0, dlgW - 4);
        }
        printAt(bottomY, dlgX + dlgW - static_cast<int>(helpStr.size()) - 2, helpStr, helpAttr);

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
                if (selected < numAnswers - 1)
                {
                    ++selected;
                }
                break;
            case TK_PGUP:
                selected -= maxVisible;
                if (selected < 0) selected = 0;
                break;
            case TK_PGDN:
                selected += maxVisible;
                if (selected >= numAnswers) selected = numAnswers - 1;
                break;
            case ' ':
            case TK_ENTER:
            {
                // Toggle selection
                uint16_t bit = static_cast<uint16_t>(1 << selected);
                if (selections & bit)
                {
                    // Deselect
                    selections &= ~bit;
                }
                else
                {
                    // Check if we'd exceed maxVotes
                    if (numSelected < maxVotes)
                    {
                        selections |= bit;
                    }
                    else if (maxVotes == 1)
                    {
                        // Single-select: clear all and select this one
                        selections = bit;
                    }
                    // else: at limit, don't add more
                }
                break;
            }
            case 'c': case 'C':
            {
                // Cast vote
                if (selections == 0)
                {
                    messageDialog("Vote", "No answers selected. Select at least one.");
                    break;
                }
                voteOut.msgId = msg.msgId;
                voteOut.voter = userName;
                voteOut.conference = msg.conference;
                voteOut.votes = selections;
                voteOut.upVote = false;
                voteOut.downVote = false;
                return true;
            }
            case 'q': case 'Q': case TK_ESCAPE:
                return false;
            default:
                // Number keys 1-9 for quick selection
                if (ch >= '1' && ch <= '9')
                {
                    int idx = ch - '1';
                    if (idx < numAnswers)
                    {
                        selected = idx;
                        // Toggle
                        uint16_t bit = static_cast<uint16_t>(1 << idx);
                        if (selections & bit)
                        {
                            selections &= ~bit;
                        }
                        else
                        {
                            if (numSelected < maxVotes)
                            {
                                selections |= bit;
                            }
                            else if (maxVotes == 1)
                            {
                                selections = bit;
                            }
                        }
                    }
                }
                break;
        }
    }
}

// Main vote UI dispatcher
bool showVoteUI(const QwkMessage& msg, const VotingData* votingData,
                const string& userName, PendingVote& voteOut)
{
    if (msg.msgId.empty())
    {
        messageDialog("Vote", "This message has no Message-ID. Voting requires QWKE headers.");
        return false;
    }

    if (msg.isPoll)
    {
        return showPollBallotUI(msg, votingData, userName, voteOut);
    }
    else
    {
        return showUpDownVoteUI(msg, userName, voteOut);
    }
}

// Show a message in the enhanced reader (DDMsgReader-style scrollable)
MsgReadResult showMessageReader(const QwkMessage& msg,
                                const string& confName,
                                int msgIndex, int totalMsgs,
                                Settings& settings,
                                const string& extractDir,
                                const VotingData* votingData,
                                PendingVote* lastVote)
{
    int displayWidth = settings.useScrollbar ? g_term->getCols() - 2 : g_term->getCols() - 1;

    // Check if this message is ANSI art (uses cursor positioning codes)
    bool isAnsi = !settings.stripAnsi && isAnsiArt(msg.body);
    vector<AnsiLine> ansiScreenLines; // Only populated for ANSI art
    // ANSI art uses a standard 80-column virtual screen
    int ansiRenderWidth = 80;

    // For poll messages, show Synchronet-style poll results instead of body text
    vector<string> bodyLines;
    if (msg.isPoll && votingData)
    {
        bodyLines = buildPollDisplayLines(msg, votingData, displayWidth);
    }
    else if (isAnsi)
    {
        // Render ANSI art through the virtual screen renderer at 80 cols
        ansiScreenLines = renderAnsiToScreen(msg.body, ansiRenderWidth);
        // Create placeholder bodyLines for scrolling (one per screen row)
        for (size_t r = 0; r < ansiScreenLines.size(); ++r)
        {
            bodyLines.push_back(""); // Actual rendering uses ansiScreenLines
        }
    }
    else
    {
        bodyLines = prepareBodyLines(msg, settings, displayWidth);
    }
    int totalLines = static_cast<int>(bodyLines.size());
    int scrollPos = 0;

    // Build attribute code flags from settings
    AttrCodeFlags attrFlags;
    attrFlags.synchronet = settings.attrSynchronet;
    attrFlags.wwiv = settings.attrWWIV;
    attrFlags.celerity = settings.attrCelerity;
    attrFlags.renegade = settings.attrRenegade;
    attrFlags.pcboard = settings.attrPCBoard;

    // Clear the screen immediately on entry so no remnants of the
    // message list are visible while the reader draws its content.
    g_term->clear();
    g_term->refresh();

    bool needFullRedraw = true;  // header + help bar need drawing
    int cachedHeaderHeight = 0;

    while (true)
    {
        int COLS = g_term->getCols();
        int ROWS = g_term->getRows();

        if (needFullRedraw)
        {
            g_term->clear();
            cachedHeaderHeight = drawMessageHeader(msg, confName, "SlyMail",
                                                    msgIndex, totalMsgs);
            drawReaderHelpBar(ROWS - 1);
            needFullRedraw = false;
        }

        // Body area
        int headerHeight = cachedHeaderHeight;
        int bodyTop = headerHeight;
        int bodyHeight = ROWS - headerHeight - 1; // -1 for help bar

        for (int i = 0; i < bodyHeight; ++i)
        {
            int lineIdx = scrollPos + i;
            int row = bodyTop + i;

            if (lineIdx >= totalLines)
            {
                // Past end of content: clear this row
                g_term->setAttr(tAttr(TC_BLACK, TC_BLACK, false));
                g_term->fillRegion(row, 0, COLS, ' ');
                continue;
            }

            // ANSI art: render from pre-rendered screen buffer cell-by-cell
            if (isAnsi && lineIdx < static_cast<int>(ansiScreenLines.size()))
            {
                const auto& ansiLine = ansiScreenLines[lineIdx];
                int renderCols = std::min(displayWidth, static_cast<int>(ansiLine.cells.size()));
                for (int c = 0; c < renderCols; ++c)
                {
                    const auto& cell = ansiLine.cells[c];
                    g_term->setAttr(cell.attr);
                    // Use putCP437 for high bytes (128-255) since ANSI art
                    // uses CP437 block/shade characters extensively
                    uint8_t uch = static_cast<uint8_t>(cell.ch);
                    if (uch >= 128)
                    {
                        g_term->putCP437(row, c, uch);
                    }
                    else
                    {
                        g_term->putCh(row, c, cell.ch);
                    }
                }
                // Pad any remaining columns
                if (renderCols < displayWidth)
                {
                    g_term->setAttr(tAttr(TC_WHITE, TC_BLACK, false));
                    g_term->fillRegion(row, renderCols, displayWidth, ' ');
                }
                continue;
            }

            const auto& line = bodyLines[lineIdx];

            // Check for poll answer backfill marker: \x01P<pct>\x01
            if (line.size() > 6 && line[0] == '\x01' && line[1] == 'P' && line[5] == '\x01')
            {
                // Extract percentage and the display text
                int pct = 0;
                try { pct = std::stoi(line.substr(2, 3)); } catch (...) {}
                string displayText = line.substr(6);
                if (static_cast<int>(displayText.size()) > displayWidth)
                {
                    displayText = displayText.substr(0, displayWidth);
                }

                // Render with Synchronet-style backfill: the portion of the
                // text covered by the percentage is drawn in a "full" color,
                // the remainder in an "empty" color.
                TermAttr fullAttr = tAttr(TC_WHITE, TC_BLUE, true);   // Bright white on blue
                TermAttr emptyAttr = tAttr(TC_CYAN, TC_BLACK, false); // Dim cyan on black

                int textLen = static_cast<int>(displayText.size());
                int fillLen = (textLen > 0) ? (pct * textLen / 100) : 0;

                for (int c = 0; c < textLen && c < displayWidth; ++c)
                {
                    g_term->setAttr(c < fillLen ? fullAttr : emptyAttr);
                    g_term->putCh(row, c, displayText[c]);
                }
                // Pad remaining with spaces
                if (textLen < displayWidth)
                {
                    g_term->setAttr(emptyAttr);
                    g_term->fillRegion(row, textLen, displayWidth, ' ');
                }
            }
            else
            {
                TermAttr lineAttr = getLineAttr(line, settings);

                if (!settings.stripAnsi)
                {
                    // Parse BBS color codes and render segment by segment
                    TermAttr attr = lineAttr;
                    auto segments = parseBBSColors(line, attr, attrFlags);
                    int x = 0;
                    for (const auto& seg : segments)
                    {
                        if (x >= displayWidth) break;
                        string text = seg.text;
                        if (x + static_cast<int>(text.size()) > displayWidth)
                        {
                            text = text.substr(0, displayWidth - x);
                        }
                        printAt(row, x, text, seg.attr);
                        x += static_cast<int>(text.size());
                    }
                    // Pad remaining with spaces
                    if (x < displayWidth)
                    {
                        g_term->setAttr(lineAttr);
                        g_term->fillRegion(row, x, displayWidth, ' ');
                    }
                }
                else
                {
                    // Strip all color codes and display plain
                    string plainLine = stripBBSColors(line, attrFlags);
                    printAt(row, 0, padStr(plainLine, displayWidth), lineAttr);
                }
            }
        }

        // Scrollbar
        if (settings.useScrollbar && totalLines > bodyHeight)
        {
            int scrollMax = std::max(1, totalLines - bodyHeight);
            int sbPos = (totalLines <= 1) ? 0
                      : (scrollPos * (totalLines - 1)) / scrollMax;
            drawScrollbar(bodyTop, bodyHeight, sbPos, totalLines,
                         tAttr(TC_BLACK, TC_BLACK, true),
                         tAttr(TC_WHITE, TC_BLACK, true));
        }

        g_term->refresh();

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:
                if (scrollPos > 0)
                {
                    --scrollPos;
                }
                break;
            case TK_DOWN:
                if (scrollPos < totalLines - bodyHeight && totalLines > bodyHeight)
                {
                    ++scrollPos;
                }
                break;
            case TK_PGUP:
                scrollPos -= bodyHeight;
                if (scrollPos < 0)
                {
                    scrollPos = 0;
                }
                break;
            case TK_PGDN:
                scrollPos += bodyHeight;
                if (scrollPos > totalLines - bodyHeight)
                {
                    scrollPos = std::max(0, totalLines - bodyHeight);
                }
                break;
            case TK_HOME:
                scrollPos = 0;
                break;
            case TK_END:
                scrollPos = std::max(0, totalLines - bodyHeight);
                break;
            case TK_LEFT:
                return MsgReadResult::PrevMsg;
            case TK_RIGHT:
                return MsgReadResult::NextMsg;
            case 'h': case 'H':
                showHeaderInfo(msg, confName, msgIndex, totalMsgs, settings);
                needFullRedraw = true;
                break;
            case 'r': case 'R':
                return MsgReadResult::Reply;
            case 'f': case 'F':
                return MsgReadResult::FirstMsg;
            case 'l': case 'L':
                return MsgReadResult::LastMsg;
            case 'd': case 'D': case TK_CTRL_D:
                // Download file attachments
                if (msg.hasAttachment)
                {
                    downloadAttachments(msg, extractDir);
                }
                else
                {
                    messageDialog("Attachments", "This message has no file attachments.");
                }
                needFullRedraw = true;
                break;
            case 'v': case 'V':
            {
                // Vote on this message (up/down or poll ballot)
                PendingVote vote;
                if (showVoteUI(msg, votingData, settings.userName, vote))
                {
                    if (lastVote)
                    {
                        *lastVote = vote;
                    }
                    return MsgReadResult::Vote;
                }
                needFullRedraw = true;
                break;
            }
            case 's': case 'S':
            case TK_CTRL_U:
                return MsgReadResult::Settings;
            case 'q': case 'Q':
            case 'm': case 'M':
            case 'c': case 'C': case TK_ESCAPE:
                return MsgReadResult::Back;
            case TK_CTRL_C:
                return MsgReadResult::Quit;
            case '?':
            case TK_F1:
            {
                // Help screen (DDMsgReader3 style)
                g_term->clear();
                int r = 1;
                drawProgramInfoLine(r++);
                r++;
                printCentered(r++, "Enhanced reader mode keys",
                    tAttr(TC_GREEN, TC_BLACK, true));
                r++;
                TermAttr keyC  = tAttr(TC_CYAN, TC_BLACK, true);
                TermAttr descC = tAttr(TC_CYAN, TC_BLACK, false);
                auto helpLine = [&](const string& key, const string& desc)
                {
                    printAt(r, 2, padStr(key, 20), keyC);
                    printAt(r, 24, ": " + desc, descC);
                    ++r;
                };
                helpLine("Down/Up arrow", "Scroll down/up in the message");
                helpLine("Left/Right arrow", "Go to the previous/next message");
                helpLine("Enter", "Go to the next message");
                helpLine("PageUp/PageDown", "Scroll up/down a page in the message");
                helpLine("HOME", "Go to the top of the message");
                helpLine("END", "Go to the bottom of the message");
                helpLine("F", "First message in conference");
                helpLine("L", "Last message in conference");
                helpLine("R", "Reply to the message");
                helpLine("V", "Vote (up/down or poll ballot)");
                helpLine("D / Ctrl-D", "Download file attachments");
                helpLine("H", "Show message header information");
                helpLine("C", "Back to conference list");
                helpLine("Ctrl-U", "Change your user settings");
                helpLine("Q", "Quit SlyMail");
                r += 2;
                printAt(r, 2, "Hit a key", tAttr(TC_GREEN, TC_BLACK, false));
                g_term->refresh();
                g_term->getKey();
                needFullRedraw = true;
                break;
            }
            case TK_ENTER:
                return MsgReadResult::NextMsg;
            case TK_RESIZE:
            {
                // Recompute body lines for new terminal width
                int newDisplayWidth = settings.useScrollbar
                    ? g_term->getCols() - 2 : g_term->getCols() - 1;
                displayWidth = newDisplayWidth;
                if (!isAnsi && !(msg.isPoll && votingData))
                {
                    bodyLines = prepareBodyLines(msg, settings, displayWidth);
                    totalLines = static_cast<int>(bodyLines.size());
                    if (scrollPos > totalLines - 1) scrollPos = std::max(0, totalLines - 1);
                }
                needFullRedraw = true;
                break;
            }
            default:
                break;
        }
    }
}
