#include "msg_reader.h"

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
    if (msg.isPrivate)
    {
        string pvt = "[Private]";
        printAt(y, w - static_cast<int>(pvt.size()) - 2, pvt,
                tAttr(TC_RED, TC_BLACK, true));
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

    // Row 6: Bottom border
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
                  {{'F', "irst"}, {'L', "ast"}, {'R', "eply"},
                   {'H', "dr"}, {'S', "ettings"}, {'C', "onf"}, {'Q', "uit"}, {'?', ""}});
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
            drawScrollbar(bodyTop, bodyHeight, scrollPos,
                         std::max(1, totalLines - bodyHeight + 1),
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

// Show a message in the enhanced reader (DDMsgReader-style scrollable)
MsgReadResult showMessageReader(const QwkMessage& msg,
                                const string& confName,
                                int msgIndex, int totalMsgs,
                                Settings& settings)
{
    int displayWidth = settings.useScrollbar ? g_term->getCols() - 2 : g_term->getCols() - 1;
    auto bodyLines = prepareBodyLines(msg, settings, displayWidth);
    int totalLines = static_cast<int>(bodyLines.size());
    int scrollPos = 0;

    while (true)
    {
        g_term->clear();

        // Draw header
        int headerHeight = drawMessageHeader(msg, confName, "SlyMail",
                                              msgIndex, totalMsgs);

        // Body area
        int bodyTop = headerHeight;
        int bodyHeight = g_term->getRows() - headerHeight - 1; // -1 for help bar

        for (int i = 0; i < bodyHeight && (scrollPos + i) < totalLines; ++i)
        {
            int lineIdx = scrollPos + i;
            const auto& line = bodyLines[lineIdx];
            int row = bodyTop + i;
            TermAttr lineAttr = getLineAttr(line, settings);
            printAt(row, 0, padStr(line, displayWidth), lineAttr);
        }

        // Scrollbar
        if (settings.useScrollbar && totalLines > bodyHeight)
        {
            drawScrollbar(bodyTop, bodyHeight, scrollPos,
                         std::max(1, totalLines - bodyHeight + 1),
                         tAttr(TC_BLACK, TC_BLACK, true),
                         tAttr(TC_WHITE, TC_BLACK, true));
        }

        // Help bar
        drawReaderHelpBar(g_term->getRows() - 1);

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
                break;  // After closing headers, loop redraws the message
            case 'r': case 'R':
                return MsgReadResult::Reply;
            case 'f': case 'F':
                return MsgReadResult::FirstMsg;
            case 'l': case 'L':
                return MsgReadResult::LastMsg;
            case 's': case 'S':
            case TK_CTRL_U:
                return MsgReadResult::Settings;
            case 'q': case 'Q':
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
                helpLine("H", "Show message header information");
                helpLine("C", "Back to conference list");
                helpLine("Ctrl-U", "Change your user settings");
                helpLine("Q", "Quit SlyMail");
                r += 2;
                printAt(r, 2, "Hit a key", tAttr(TC_GREEN, TC_BLACK, false));
                g_term->refresh();
                g_term->getKey();
                break;
            }
            case TK_ENTER:
                return MsgReadResult::NextMsg;
            default:
                break;
        }
    }
}
