#include "ui_common.h"
#include "i18n.h"

using std::string;
using std::vector;
using std::pair;

// ---- Text output helpers ----

void printAt(int y, int x, const string& text, const TermAttr& attr)
{
    g_term->setAttr(attr);
    g_term->printStr(y, x, text);
}

void printCentered(int y, const string& text, const TermAttr& attr)
{
    int x = (g_term->getCols() - static_cast<int>(text.size())) / 2;
    if (x < 0)
    {
        x = 0;
    }
    g_term->setAttr(attr);
    g_term->printStr(y, x, text);
}

void drawProgramInfoLine(int y)
{
    string name = PROGRAM_NAME;
    string ver  = PROGRAM_VERSION;
    string date = PROGRAM_DATE;

    // Calculate total width: name + " version " + ver + " (" + date + ")"
    int totalLen = static_cast<int>(name.size()) + 9 + static_cast<int>(ver.size())
                 + 2 + static_cast<int>(date.size()) + 1;
    int x = (g_term->getCols() - totalLen) / 2;
    if (x < 0)
    {
        x = 0;
    }

    printAt(y, x, name, tAttr(TC_CYAN, TC_BLACK, true));
    x += static_cast<int>(name.size());
    printAt(y, x, _(" version "), tAttr(TC_CYAN, TC_BLACK, false));
    x += 9;
    printAt(y, x, ver, tAttr(TC_GREEN, TC_BLACK, false));
    x += static_cast<int>(ver.size());
    printAt(y, x, _(" ("), tAttr(TC_WHITE, TC_BLACK, true));
    x += 2;
    printAt(y, x, date, tAttr(TC_BLUE, TC_BLACK, true));
    x += static_cast<int>(date.size());
    printAt(y, x, _(")"), tAttr(TC_WHITE, TC_BLACK, true));
}

void fillRow(int y, const TermAttr& attr, int startX, int endX)
{
    if (endX < 0)
    {
        endX = g_term->getCols();
    }
    g_term->setAttr(attr);
    g_term->fillRegion(y, startX, endX, ' ');
}

void fillBullets(int y, int x, int len, const TermAttr& attr)
{
    g_term->setAttr(attr);
    for (int i = 0; i < len; ++i)
    {
        g_term->putCP437(y, x + i, CP437_BULLET_OPERATOR);
    }
}

// ---- Box-drawing helpers ----

void drawHLine(int y, int x, int len, const TermAttr& attr,
               const string& label, const TermAttr& labelAttr)
{
    g_term->setAttr(attr);
    g_term->drawHLine(y, x, len);
    if (!label.empty())
    {
        int labelX = x + (len - static_cast<int>(label.size())) / 2;
        g_term->setAttr(labelAttr);
        g_term->printStr(y, labelX, " " + label + " ");
    }
}

void drawBox(int y, int x, int h, int w, const TermAttr& attr,
             const string& title, const TermAttr& titleAttr)
{
    g_term->setAttr(attr);
    g_term->drawBox(y, x, h, w);
    if (!title.empty())
    {
        int titleX = x + (w - static_cast<int>(title.size()) - 2) / 2;
        g_term->setAttr(titleAttr);
        g_term->printStr(y, titleX, " " + title + " ");
    }
}

// ---- DDMsgReader-style help bar ----

void drawDDHelpBar(int y, const string& prefix,
                   const vector<pair<char, string>>& items)
{
    // Background: grey (white non-bright)
    // Hotkey letters: normal red on white bg
    // Slashes, commas, regular text: normal blue on white bg
    // ')' characters: normal magenta on white bg
    int bg = TC_WHITE; // White background = grey
    TermAttr keyAttr  = tAttr(TC_RED, bg, false);
    TermAttr textAttr = tAttr(TC_BLUE, bg, false);
    TermAttr parenAttr = tAttr(TC_MAGENTA, bg, false);
    TermAttr bgAttr   = tAttr(TC_BLUE, bg, false);

    // Fill entire row with the background color
    fillRow(y, bgAttr);

    // First pass: calculate total width of content
    int totalWidth = static_cast<int>(prefix.size());
    for (size_t i = 0; i < items.size(); ++i)
    {
        totalWidth += 1; // hotkey character
        if (!items[i].second.empty())
        {
            totalWidth += 1; // ')' only when there is description text
            totalWidth += static_cast<int>(items[i].second.size());
        }
        if (i + 1 < items.size())
        {
            totalWidth += 2; // ", "
        }
    }

    // Center horizontally
    int x = (g_term->getCols() - totalWidth) / 2;
    if (x < 0)
    {
        x = 0;
    }

    // Render the prefix character by character with appropriate colors
    for (size_t ci = 0; ci < prefix.size(); ++ci)
    {
        char c = prefix[ci];
        if (c == '/' || c == ',' || c == ' ' || c == '-' || c == '<' || c == '>')
        {
            g_term->setAttr(textAttr);
        }
        else
        {
            // Letters in the prefix (like "Up", "Dn", "PgUp") are hotkey-colored
            g_term->setAttr(keyAttr);
        }
        g_term->putCh(y, x, c);
        ++x;
    }

    // Render each item
    for (size_t i = 0; i < items.size(); ++i)
    {
        if (x >= g_term->getCols())
        {
            break;
        }

        // Hotkey letter (red)
        g_term->setAttr(keyAttr);
        g_term->putCh(y, x, items[i].first);
        ++x;

        // ')' and description text — only if description is non-empty
        const string& desc = items[i].second;
        if (!desc.empty())
        {
            // ')' in magenta
            g_term->setAttr(parenAttr);
            g_term->putCh(y, x, ')');
            ++x;

            // Description text (blue)
            for (size_t ci = 0; ci < desc.size(); ++ci)
            {
                if (x >= g_term->getCols()) break;
                g_term->setAttr(textAttr);
                g_term->putCh(y, x, desc[ci]);
                ++x;
            }
        }

        // Separator ", " (blue) between items
        if (i + 1 < items.size())
        {
            if (x < g_term->getCols())
            {
                g_term->setAttr(textAttr);
                g_term->putCh(y, x, ',');
                ++x;
            }
            if (x < g_term->getCols())
            {
                g_term->putCh(y, x, ' ');
                ++x;
            }
        }
    }
}

// ---- Scrollbar ----

void drawScrollbar(int topY, int height, int position, int total,
                   const TermAttr& bgAttr, const TermAttr& blockAttr)
{
    int col = g_term->getCols() - 1;
    if (total <= 0 || height <= 0)
    {
        return;
    }

    // Draw the track background
    g_term->setAttr(bgAttr);
    for (int i = 0; i < height; ++i)
    {
        g_term->putCP437(topY + i, col, CP437_MEDIUM_SHADE);
    }

    // Thumb size is proportional to the visible portion of the content.
    // If the viewport shows half the content, the thumb covers half the track.
    int thumbSize = (height * height) / total;
    if (thumbSize < 1)
    {
        thumbSize = 1;
    }
    // When content is scrollable, the thumb must be at least 1 cell shorter
    // than the track so it has room to move and indicate scroll position.
    if (total > height && thumbSize >= height)
    {
        thumbSize = height - 1;
    }

    // Thumb position: map the scroll position onto the available track range
    int trackRange = height - thumbSize;
    int thumbPos = 0;
    if (total > 1 && trackRange > 0)
    {
        thumbPos = (position * trackRange) / (total - 1);
    }
    if (thumbPos + thumbSize > height)
    {
        thumbPos = height - thumbSize;
    }

    // Draw the thumb
    g_term->setAttr(blockAttr);
    for (int i = 0; i < thumbSize; ++i)
    {
        g_term->putCP437(topY + thumbPos + i, col, CP437_FULL_BLOCK);
    }
}

// ---- String helpers ----

string truncateStr(const string& s, int maxLen)
{
    if (maxLen <= 0)
    {
        return "";
    }
    if (static_cast<int>(s.size()) <= maxLen)
    {
        return s;
    }
    if (maxLen <= 3)
    {
        return s.substr(0, maxLen);
    }
    return s.substr(0, maxLen - 3) + "...";
}

string padStr(const string& s, int width)
{
    if (static_cast<int>(s.size()) >= width)
    {
        return s.substr(0, width);
    }
    return s + string(width - s.size(), ' ');
}

string rightAlign(const string& s, int width)
{
    if (static_cast<int>(s.size()) >= width)
    {
        return s.substr(0, width);
    }
    return string(width - s.size(), ' ') + s;
}

// ---- Input helpers ----

string getStringInput(int y, int x, int maxLen,
                      const string& initial,
                      const TermAttr& attr)
{
    string result = initial;
    int cursorPos = static_cast<int>(result.size());

    g_term->setCursorVisible(true);
    for (;;)
    {
        g_term->setAttr(attr);
        string display = padStr(result, maxLen);
        g_term->printStr(y, x, display);
        g_term->moveTo(y, x + cursorPos);
        g_term->refresh();

        int ch = g_term->getKey();
        if (ch == TK_ENTER)
        {
            break;
        }
        else if (ch == TK_ESCAPE || ch == TK_CTRL_A || ch == TK_CTRL_C)
        {
            g_term->setCursorVisible(false);
            return "";
        }
        else if (ch == TK_BACKSPACE || ch == TK_BACKSPACE_8)
        {
            if (cursorPos > 0)
            {
                result.erase(cursorPos - 1, 1);
                --cursorPos;
            }
        }
        else if (ch == TK_DELETE)
        {
            if (cursorPos < static_cast<int>(result.size()))
            {
                result.erase(cursorPos, 1);
            }
        }
        else if (ch == TK_LEFT)
        {
            if (cursorPos > 0)
            {
                --cursorPos;
            }
        }
        else if (ch == TK_RIGHT)
        {
            if (cursorPos < static_cast<int>(result.size()))
            {
                ++cursorPos;
            }
        }
        else if (ch == TK_HOME)
        {
            cursorPos = 0;
        }
        else if (ch == TK_END)
        {
            cursorPos = static_cast<int>(result.size());
        }
        else if (ch >= 32 && ch < 127 && static_cast<int>(result.size()) < maxLen)
        {
            result.insert(cursorPos, 1, static_cast<char>(ch));
            ++cursorPos;
        }
    }
    g_term->setCursorVisible(false);
    return result;
}

string getNumericInput(int y, int x, int maxLen,
                       const TermAttr& attr)
{
    string result;
    int cursorPos = 0;

    g_term->setCursorVisible(true);
    for (;;)
    {
        g_term->setAttr(attr);
        string display = padStr(result, maxLen);
        g_term->printStr(y, x, display);
        g_term->moveTo(y, x + cursorPos);
        g_term->refresh();

        int ch = g_term->getKey();
        if (ch == TK_ENTER)
        {
            break;
        }
        else if (ch == TK_ESCAPE || ch == TK_CTRL_C)
        {
            g_term->setCursorVisible(false);
            return "";
        }
        else if (ch == TK_BACKSPACE || ch == TK_BACKSPACE_8)
        {
            if (cursorPos > 0)
            {
                result.erase(cursorPos - 1, 1);
                --cursorPos;
            }
        }
        else if (ch == TK_DELETE)
        {
            if (cursorPos < static_cast<int>(result.size()))
            {
                result.erase(cursorPos, 1);
            }
        }
        else if (ch == TK_LEFT)
        {
            if (cursorPos > 0) --cursorPos;
        }
        else if (ch == TK_RIGHT)
        {
            if (cursorPos < static_cast<int>(result.size())) ++cursorPos;
        }
        else if (ch == TK_HOME)
        {
            cursorPos = 0;
        }
        else if (ch == TK_END)
        {
            cursorPos = static_cast<int>(result.size());
        }
        else if (ch >= '0' && ch <= '9' && static_cast<int>(result.size()) < maxLen)
        {
            result.insert(cursorPos, 1, static_cast<char>(ch));
            ++cursorPos;
        }
    }
    g_term->setCursorVisible(false);
    return result;
}

string getPasswordInput(int y, int x, int maxLen,
                        const string& initial,
                        const TermAttr& attr)
{
    string result = initial;
    int cursorPos = static_cast<int>(result.size());

    g_term->setCursorVisible(true);
    for (;;)
    {
        g_term->setAttr(attr);
        // Display masked version: '*' for each character, padded with spaces
        string masked(result.size(), '*');
        string display = padStr(masked, maxLen);
        g_term->printStr(y, x, display);
        g_term->moveTo(y, x + cursorPos);
        g_term->refresh();

        int ch = g_term->getKey();
        if (ch == TK_ENTER)
        {
            break;
        }
        else if (ch == TK_ESCAPE || ch == TK_CTRL_A || ch == TK_CTRL_C)
        {
            g_term->setCursorVisible(false);
            return "";
        }
        else if (ch == TK_BACKSPACE || ch == TK_BACKSPACE_8)
        {
            if (cursorPos > 0)
            {
                result.erase(cursorPos - 1, 1);
                --cursorPos;
            }
        }
        else if (ch == TK_DELETE)
        {
            if (cursorPos < static_cast<int>(result.size()))
            {
                result.erase(cursorPos, 1);
            }
        }
        else if (ch == TK_LEFT)
        {
            if (cursorPos > 0)
            {
                --cursorPos;
            }
        }
        else if (ch == TK_RIGHT)
        {
            if (cursorPos < static_cast<int>(result.size()))
            {
                ++cursorPos;
            }
        }
        else if (ch == TK_HOME)
        {
            cursorPos = 0;
        }
        else if (ch == TK_END)
        {
            cursorPos = static_cast<int>(result.size());
        }
        else if (ch >= 32 && ch < 127 && static_cast<int>(result.size()) < maxLen)
        {
            result.insert(cursorPos, 1, static_cast<char>(ch));
            ++cursorPos;
        }
    }
    g_term->setCursorVisible(false);
    return result;
}

// ---- Dialogs ----

bool confirmDialog(const string& prompt)
{
    int w = static_cast<int>(prompt.size()) + 10;
    if (w < 30)
    {
        w = 30;
    }
    if (w > g_term->getCols() - 4)
    {
        w = g_term->getCols() - 4;
    }
    int h = 5;
    int y = (g_term->getRows() - h) / 2;
    int x = (g_term->getCols() - w) / 2;

    TermAttr borderAttr = tAttr(TC_CYAN, TC_BLACK, true);
    TermAttr textAttr   = tAttr(TC_WHITE, TC_BLACK, true);
    TermAttr promptAttr = tAttr(TC_CYAN, TC_BLACK, false);

    // Shadow
    g_term->setAttr(tAttr(TC_BLACK, TC_BLACK, true));
    for (int r = 1; r < h + 1; ++r)
    {
        for (int c = 2; c < w + 2; ++c)
        {
            if (y + r < g_term->getRows() && x + c < g_term->getCols())
            {
                g_term->putCh(y + r, x + c, ' ');
            }
        }
    }

    // Box interior
    for (int r = 0; r < h; ++r)
    {
        fillRow(y + r, tAttr(TC_WHITE, TC_BLACK, false), x, x + w);
    }

    drawBox(y, x, h, w, borderAttr, "Confirm", borderAttr);
    printAt(y + 2, x + 3, prompt, textAttr);
    printAt(y + 3, x + 3, "[Y/N]", promptAttr);
    g_term->refresh();

    for (;;)
    {
        int ch = g_term->getKey();
        if (ch == 'y' || ch == 'Y')
        {
            return true;
        }
        if (ch == 'n' || ch == 'N' || ch == TK_ESCAPE)
        {
            return false;
        }
    }
}

void messageDialog(const string& title, const string& msg)
{
    int w = std::max(static_cast<int>(msg.size()) + 6,
                     static_cast<int>(title.size()) + 6);
    if (w < 30)
    {
        w = 30;
    }
    if (w > g_term->getCols() - 4)
    {
        w = g_term->getCols() - 4;
    }
    int h = 5;
    int y = (g_term->getRows() - h) / 2;
    int x = (g_term->getCols() - w) / 2;

    TermAttr borderAttr = tAttr(TC_CYAN, TC_BLACK, true);

    g_term->setAttr(tAttr(TC_BLACK, TC_BLACK, true));
    for (int r = 1; r < h + 1; ++r)
    {
        for (int c = 2; c < w + 2; ++c)
        {
            if (y + r < g_term->getRows() && x + c < g_term->getCols())
            {
                g_term->putCh(y + r, x + c, ' ');
            }
        }
    }

    for (int r = 0; r < h; ++r)
    {
        fillRow(y + r, tAttr(TC_WHITE, TC_BLACK, false), x, x + w);
    }
    drawBox(y, x, h, w, borderAttr, title, borderAttr);
    printAt(y + 2, x + 3, truncateStr(msg, w - 6), tAttr(TC_WHITE, TC_BLACK, true));
    printAt(y + 3, x + 3, "Press any key...", tAttr(TC_GREEN, TC_BLACK, false));
    g_term->refresh();
    g_term->getKey();
}

// ---- Word wrap ----

vector<string> wordWrap(const string& text, int maxWidth)
{
    vector<string> lines;
    std::istringstream stream(text);
    string paragraph;

    while (std::getline(stream, paragraph))
    {
        if (paragraph.empty())
        {
            lines.push_back("");
            continue;
        }
        if (static_cast<int>(paragraph.size()) <= maxWidth)
        {
            lines.push_back(paragraph);
            continue;
        }
        string currentLine;
        std::istringstream words(paragraph);
        string word;
        while (words >> word)
        {
            if (currentLine.empty())
            {
                currentLine = word;
            }
            else if (static_cast<int>(currentLine.size() + 1 + word.size()) <= maxWidth)
            {
                currentLine += " " + word;
            }
            else
            {
                lines.push_back(currentLine);
                currentLine = word;
            }
        }
        if (!currentLine.empty())
        {
            lines.push_back(currentLine);
        }
    }
    return lines;
}
