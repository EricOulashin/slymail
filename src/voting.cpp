#include "voting.h"
#include "ui_common.h"
#include <filesystem>
#include <cctype>

using std::string;
using std::vector;

// ============================================================
// Simple INI parser for VOTING.DAT
// ============================================================

struct IniSection
{
    string name;
    vector<std::pair<string, string>> keys; // key-value pairs

    string getValue(const string& key, const string& def = "") const
    {
        for (const auto& kv : keys)
        {
            if (kv.first == key) return kv.second;
        }
        return def;
    }

    int getInt(const string& key, int def = 0) const
    {
        string val = getValue(key);
        if (val.empty()) return def;
        try { return std::stoi(val); } catch (...) { return def; }
    }

    uint16_t getHexShort(const string& key, uint16_t def = 0) const
    {
        string val = getValue(key);
        if (val.empty()) return def;
        try { return static_cast<uint16_t>(std::stoul(val, nullptr, 0)); }
        catch (...) { return def; }
    }

    bool getBool(const string& key, bool def = false) const
    {
        string val = getValue(key);
        if (val.empty()) return def;
        return (val == "true" || val == "True" || val == "TRUE" || val == "1");
    }
};

static string iniTrim(const string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static vector<IniSection> parseIniFile(const string& path)
{
    vector<IniSection> sections;
    std::ifstream f(path);
    if (!f.is_open()) return sections;

    IniSection current;
    bool hasSection = false;

    string line;
    while (std::getline(f, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        string trimmed = iniTrim(line);
        if (trimmed.empty() || trimmed[0] == ';' || trimmed[0] == '#')
        {
            continue;
        }

        // Section header
        if (trimmed.front() == '[' && trimmed.back() == ']')
        {
            if (hasSection)
            {
                sections.push_back(current);
            }
            current = IniSection();
            current.name = trimmed.substr(1, trimmed.size() - 2);
            hasSection = true;
            continue;
        }

        // Key = Value or Key: Value
        auto eqPos = trimmed.find('=');
        auto colonPos = trimmed.find(':');
        size_t sepPos = string::npos;
        int sepLen = 1;

        if (eqPos != string::npos && (colonPos == string::npos || eqPos < colonPos))
        {
            sepPos = eqPos;
        }
        else if (colonPos != string::npos)
        {
            sepPos = colonPos;
            // Check for ": " (with space after colon)
            if (colonPos + 1 < trimmed.size() && trimmed[colonPos + 1] == ' ')
            {
                sepLen = 2;
            }
        }

        if (sepPos != string::npos && hasSection)
        {
            string key = iniTrim(trimmed.substr(0, sepPos));
            string val = iniTrim(trimmed.substr(sepPos + sepLen));
            current.keys.emplace_back(key, val);
        }
    }

    if (hasSection)
    {
        sections.push_back(current);
    }

    return sections;
}

// ============================================================
// VOTING.DAT parsing
// ============================================================

VotingData parseVotingDat(const string& extractDir)
{
    namespace fs = std::filesystem;

    VotingData data;

    // Find VOTING.DAT (case-insensitive)
    string votingPath;
    for (const auto& entry : fs::directory_iterator(extractDir))
    {
        string fname = entry.path().filename().string();
        string upper;
        for (char c : fname) upper += static_cast<char>(toupper(static_cast<unsigned char>(c)));
        if (upper == "VOTING.DAT")
        {
            votingPath = entry.path().string();
            break;
        }
    }

    if (votingPath.empty()) return data;

    auto sections = parseIniFile(votingPath);

    // Process sections in pairs: [hex_offset] followed by [poll:id] or [vote:id] or [close:id]
    // The hex offset section is informational; the actual data is in the typed section.
    for (size_t i = 0; i < sections.size(); ++i)
    {
        const auto& sec = sections[i];

        // Poll definition
        if (sec.name.size() > 5 && sec.name.substr(0, 5) == "poll:")
        {
            VotingPoll poll;
            poll.msgId = sec.name.substr(5);
            poll.from = sec.getValue("Sender");
            poll.date = sec.getValue("WhenWritten");
            poll.conference = sec.getInt("Conference");
            poll.maxVotes = sec.getInt("MaxVotes");
            poll.question = sec.getValue("Subject");

            // Collect Comment0, Comment1, ... and PollAnswer0, PollAnswer1, ...
            for (int n = 0; n < 100; ++n)
            {
                string key = "Comment" + std::to_string(n);
                string val = sec.getValue(key);
                if (val.empty()) break;
                poll.comments.push_back(val);
            }

            for (int n = 0; n < 100; ++n)
            {
                string key = "PollAnswer" + std::to_string(n);
                string val = sec.getValue(key);
                if (val.empty()) break;
                poll.answers.emplace_back(val);
            }

            data.polls.push_back(poll);
        }
        // Vote record
        else if (sec.name.size() > 5 && sec.name.substr(0, 5) == "vote:")
        {
            VoteRecord vote;
            vote.msgId = sec.name.substr(5);
            vote.inReplyTo = sec.getValue("In-Reply-To");
            vote.voter = sec.getValue("Sender");
            vote.date = sec.getValue("WhenWritten");
            vote.conference = sec.getInt("Conference");
            vote.votes = sec.getHexShort("Votes");
            vote.upVote = sec.getBool("UpVote") || sec.getBool("upvote");
            vote.downVote = sec.getBool("DownVote") || sec.getBool("downvote");

            data.voteRecords.push_back(vote);
        }
        // Poll closure
        else if (sec.name.size() > 6 && sec.name.substr(0, 6) == "close:")
        {
            string closedId = sec.name.substr(6);
            // Mark matching poll as closed
            for (auto& poll : data.polls)
            {
                if (poll.msgId == closedId)
                {
                    poll.closed = true;
                    break;
                }
            }
        }
    }

    // Tally votes
    data.tallyVotes();

    return data;
}

// ============================================================
// Vote tallying
// ============================================================

void VotingData::tallyVotes()
{
    for (auto& poll : polls)
    {
        // Reset vote counts
        for (auto& answer : poll.answers)
        {
            answer.votes = 0;
        }

        // Apply each vote record whose In-Reply-To matches this poll's msgId
        for (const auto& vote : voteRecords)
        {
            if (vote.inReplyTo != poll.msgId) continue;

            // votes is a bitmask: bit 0 = answer 0, bit 1 = answer 1, etc.
            for (size_t a = 0; a < poll.answers.size() && a < 16; ++a)
            {
                if (vote.votes & (1 << a))
                {
                    ++poll.answers[a].votes;
                }
            }
        }
    }
}

// ============================================================
// Poll display
// ============================================================

void showPollDialog(const VotingPoll& poll)
{
    int cols = g_term->getCols();
    int rows = g_term->getRows();

    // Build display lines
    vector<string> displayLines;
    displayLines.push_back("Poll: " + poll.question);
    displayLines.push_back("From: " + poll.from);
    if (!poll.date.empty())
    {
        displayLines.push_back("Date: " + poll.date);
    }
    if (poll.maxVotes > 0)
    {
        displayLines.push_back("Max selections: " + std::to_string(poll.maxVotes));
    }
    if (poll.closed)
    {
        displayLines.push_back("[CLOSED]");
    }
    displayLines.push_back("");

    // Comments
    for (const auto& comment : poll.comments)
    {
        displayLines.push_back("  " + comment);
    }
    if (!poll.comments.empty())
    {
        displayLines.push_back("");
    }

    // Calculate total votes for percentage
    int totalVotes = 0;
    for (const auto& answer : poll.answers)
    {
        totalVotes += answer.votes;
    }

    // Answers with vote counts and bar chart
    displayLines.push_back("Answers:");
    displayLines.push_back(string(40, '-'));
    int maxBarWidth = cols - 30;
    if (maxBarWidth < 10) maxBarWidth = 10;

    for (size_t a = 0; a < poll.answers.size(); ++a)
    {
        const auto& answer = poll.answers[a];
        string line = "  " + std::to_string(a + 1) + ". " + answer.text;
        displayLines.push_back(line);

        // Vote count and bar
        int barLen = 0;
        if (totalVotes > 0 && answer.votes > 0)
        {
            barLen = (answer.votes * maxBarWidth) / totalVotes;
            if (barLen < 1) barLen = 1;
        }
        string bar(barLen, '#');
        string pct;
        if (totalVotes > 0)
        {
            int pctVal = (answer.votes * 100) / totalVotes;
            pct = " (" + std::to_string(pctVal) + "%)";
        }
        displayLines.push_back("     " + std::to_string(answer.votes)
                                + " vote(s)" + pct + "  " + bar);
    }

    displayLines.push_back("");
    displayLines.push_back("Total votes: " + std::to_string(totalVotes));

    // Scrollable display
    int totalLines = static_cast<int>(displayLines.size());
    int scrollPos = 0;

    TermAttr titleAttr = tAttr(TC_CYAN, TC_BLACK, true);
    TermAttr textAttr = tAttr(TC_WHITE, TC_BLACK, false);
    TermAttr barAttr = tAttr(TC_GREEN, TC_BLACK, true);
    TermAttr helpAttr = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr borderAttr = tAttr(TC_BLUE, TC_BLACK, true);

    while (true)
    {
        g_term->clear();

        // Title
        printAt(0, 0, " Poll/Vote Results ", titleAttr);
        g_term->setAttr(borderAttr);
        g_term->drawHLine(1, 0, cols);

        int bodyTop = 2;
        int bodyHeight = rows - 3;

        for (int i = 0; i < bodyHeight && (scrollPos + i) < totalLines; ++i)
        {
            int lineIdx = scrollPos + i;
            const string& line = displayLines[lineIdx];
            int row = bodyTop + i;

            TermAttr lineAttr = textAttr;
            if (line.find("Poll:") == 0 || line.find("Answers:") == 0)
            {
                lineAttr = titleAttr;
            }
            else if (!line.empty() && line[0] == ' ' && line.find('#') != string::npos
                     && line.find("vote(s)") != string::npos)
            {
                lineAttr = barAttr;
            }
            else if (line.find("---") == 0)
            {
                lineAttr = borderAttr;
            }

            string padded = line;
            if (static_cast<int>(padded.size()) < cols)
            {
                padded += string(cols - padded.size(), ' ');
            }
            else if (static_cast<int>(padded.size()) > cols)
            {
                padded = padded.substr(0, cols);
            }
            printAt(row, 0, padded, lineAttr);
        }

        // Help bar
        fillRow(rows - 1, tAttr(TC_BLACK, TC_BLACK, false));
        printAt(rows - 1, 1, "Up/Dn/PgUp/PgDn=Scroll, Q/ESC=Close", helpAttr);

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
            case 'q': case 'Q': case TK_ESCAPE:
                return;
            default:
                break;
        }
    }
}

// ============================================================
// Voting list display
// ============================================================

void showVotingList(const VotingData& voting, const Settings& /* settings */)
{
    if (voting.polls.empty())
    {
        messageDialog("Voting", "No polls found in this packet.");
        return;
    }

    int cols = g_term->getCols();
    int rows = g_term->getRows();
    int selected = 0;
    int scrollPos = 0;
    int totalPolls = static_cast<int>(voting.polls.size());

    TermAttr titleAttr = tAttr(TC_CYAN, TC_BLACK, true);
    TermAttr normalAttr = tAttr(TC_WHITE, TC_BLACK, false);
    TermAttr selectedAttr = tAttr(TC_WHITE, TC_BLUE, true);
    TermAttr helpAttr = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr borderAttr = tAttr(TC_BLUE, TC_BLACK, true);
    TermAttr closedAttr = tAttr(TC_RED, TC_BLACK, false);

    while (true)
    {
        g_term->clear();

        // Title
        printAt(0, 0, " Polls/Votes in Packet ", titleAttr);
        g_term->setAttr(borderAttr);
        g_term->drawHLine(1, 0, cols);

        // Column headers
        printAt(2, 1, "#", titleAttr);
        printAt(2, 5, "Question", titleAttr);
        printAt(2, cols - 20, "From", titleAttr);
        printAt(2, cols - 8, "Status", titleAttr);

        g_term->setAttr(borderAttr);
        g_term->drawHLine(3, 0, cols);

        int bodyTop = 4;
        int bodyHeight = rows - 5;

        for (int i = 0; i < bodyHeight && (scrollPos + i) < totalPolls; ++i)
        {
            int idx = scrollPos + i;
            const auto& poll = voting.polls[idx];
            int row = bodyTop + i;

            bool isSel = (idx == selected);
            TermAttr lineAttr = isSel ? selectedAttr : normalAttr;

            fillRow(row, lineAttr, 0, cols);
            printAt(row, 1, std::to_string(idx + 1), lineAttr);

            string question = poll.question;
            int maxQLen = cols - 30;
            if (static_cast<int>(question.size()) > maxQLen)
            {
                question = question.substr(0, maxQLen - 3) + "...";
            }
            printAt(row, 5, question, lineAttr);

            string from = poll.from;
            if (static_cast<int>(from.size()) > 12)
            {
                from = from.substr(0, 12);
            }
            printAt(row, cols - 20, from, lineAttr);

            if (poll.closed)
            {
                printAt(row, cols - 8, "Closed", isSel ? lineAttr : closedAttr);
            }
            else
            {
                printAt(row, cols - 8, "Open", lineAttr);
            }
        }

        // Help bar
        fillRow(rows - 1, tAttr(TC_BLACK, TC_BLACK, false));
        printAt(rows - 1, 1, "Up/Dn=Select, Enter=View, Q/ESC=Close", helpAttr);

        g_term->refresh();

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:
                if (selected > 0) --selected;
                if (selected < scrollPos) scrollPos = selected;
                break;
            case TK_DOWN:
                if (selected < totalPolls - 1) ++selected;
                if (selected >= scrollPos + bodyHeight) scrollPos = selected - bodyHeight + 1;
                break;
            case TK_PGUP:
                selected -= bodyHeight;
                if (selected < 0) selected = 0;
                scrollPos = selected;
                break;
            case TK_PGDN:
                selected += bodyHeight;
                if (selected >= totalPolls) selected = totalPolls - 1;
                if (selected >= scrollPos + bodyHeight) scrollPos = selected - bodyHeight + 1;
                break;
            case TK_ENTER:
                if (selected >= 0 && selected < totalPolls)
                {
                    showPollDialog(voting.polls[selected]);
                }
                break;
            case 'q': case 'Q': case TK_ESCAPE:
                return;
            default:
                break;
        }
    }
}
