// text_input.cpp — Implementation of the TextBuffer text editing engine.
// See text_input.h for the public interface and documentation.
// This file has no UI or terminal dependencies.

#include "text_input.h"
#include <cstdint>
#include <cctype>

using std::string;
using std::vector;

// ============================================================
// ANSI sequence helpers
// ============================================================

int ansiSeqLen(const string& text, int pos)
{
    if (pos < 0 || pos >= static_cast<int>(text.size())) return 0;
    if (static_cast<uint8_t>(text[pos]) != 0x1B) return 0;
    if (pos + 1 >= static_cast<int>(text.size()) || text[pos + 1] != '[') return 0;
    int i = pos + 2;
    while (i < static_cast<int>(text.size()))
    {
        char ch = text[i];
        if (ch >= 0x40 && ch <= 0x7E) return i - pos + 1;
        if ((ch >= '0' && ch <= '9') || ch == ';') { ++i; continue; }
        break;
    }
    return 0;
}

int byteColToDisplayCol(const string& text, int byteCol)
{
    int displayCol = 0;
    int i = 0;
    while (i < byteCol && i < static_cast<int>(text.size()))
    {
        int seqLen = ansiSeqLen(text, i);
        if (seqLen > 0) i += seqLen;
        else { ++displayCol; ++i; }
    }
    return displayCol;
}

int skipForward(const string& text, int pos)
{
    int len = static_cast<int>(text.size());
    if (pos >= len) return pos;
    while (pos < len) { int sl = ansiSeqLen(text, pos); if (sl > 0) pos += sl; else break; }
    if (pos < len) ++pos;
    while (pos < len) { int sl = ansiSeqLen(text, pos); if (sl > 0) pos += sl; else break; }
    return pos;
}

static int skipOneSeqBackward(const string& text, int pos)
{
    if (pos <= 0) return pos;
    int endByte = pos - 1;
    char ch = text[endByte];
    if (ch >= 0x40 && ch <= 0x7E)
    {
        int scan = endByte - 1;
        while (scan >= 0)
        {
            if (static_cast<uint8_t>(text[scan]) == 0x1B && scan + 1 < static_cast<int>(text.size())
                && text[scan + 1] == '[')
            {
                int seqLen = ansiSeqLen(text, scan);
                if (seqLen > 0 && scan + seqLen == pos) return scan;
                break;
            }
            char sc = text[scan];
            if ((sc >= '0' && sc <= '9') || sc == ';' || sc == '[') { --scan; continue; }
            break;
        }
    }
    return pos;
}

int skipBackward(const string& text, int pos)
{
    if (pos <= 0) return 0;
    while (pos > 0) { int np = skipOneSeqBackward(text, pos); if (np < pos) pos = np; else break; }
    if (pos > 0) --pos;
    while (pos > 0) { int np = skipOneSeqBackward(text, pos); if (np < pos) pos = np; else break; }
    return pos;
}

int eraseBackward(string& text, int pos)
{
    if (pos <= 0) return 0;
    int newPos = skipBackward(text, pos);
    int count = pos - newPos;
    text.erase(newPos, count);
    return newPos;
}

void eraseForward(string& text, int pos)
{
    if (pos >= static_cast<int>(text.size())) return;
    int endPos = skipForward(text, pos);
    if (endPos > pos) text.erase(pos, endPos - pos);
}

// ============================================================
// pullUpWords (standalone helper)
// ============================================================

void pullUpWords(vector<TextLine>& lines, int row, int editWidth)
{
    if (row < 0 || row + 1 >= static_cast<int>(lines.size())) return;
    string& curLine = lines[row].text;
    string& nextLine = lines[row + 1].text;
    if (nextLine.empty()) return;
    int curDispW = byteColToDisplayCol(curLine, static_cast<int>(curLine.size()));
    if (curDispW >= editWidth) return;

    size_t pos = 0;
    while (pos < nextLine.size())
    {
        size_t wordEnd = nextLine.find(' ', pos);
        if (wordEnd == string::npos) wordEnd = nextLine.size(); else ++wordEnd;
        string word = nextLine.substr(pos, wordEnd - pos);
        string separator;
        if (!curLine.empty() && curLine.back() != ' ' && pos == 0 && !word.empty() && word[0] != ' ')
            separator = " ";
        string candidate = curLine + separator + word;
        int candidateDispW = byteColToDisplayCol(candidate, static_cast<int>(candidate.size()));
        if (candidateDispW > editWidth) break;
        curLine = candidate;
        pos = wordEnd;
    }
    if (pos > 0)
    {
        nextLine = nextLine.substr(pos);
        while (!nextLine.empty() && nextLine[0] == ' ') nextLine = nextLine.substr(1);
        if (nextLine.empty())
        {
            // Transfer the hard break flag before removing the empty line
            lines[row].hardBreak = lines[row + 1].hardBreak;
            lines.erase(lines.begin() + row + 1);
        }
    }
    while (!curLine.empty() && curLine.back() == ' ') curLine.pop_back();
}

// ============================================================
// TextBuffer implementation
// ============================================================

TextBuffer::TextBuffer()
    : cursorRow(0), cursorCol(0), editWidth(78), insertMode(true)
{
}

void TextBuffer::init(int width, bool startWithReset)
{
    editWidth = width;
    lines.clear();
    cursorRow = 0;
    cursorCol = 0;
    if (startWithReset)
    {
        lines.push_back(TextLine{"\x1b[0m"});
        cursorCol = static_cast<int>(lines[0].text.size());
    }
    else
    {
        lines.push_back(TextLine{""});
    }
}

void TextBuffer::fixCursorAnsi()
{
    if (cursorRow >= 0 && cursorRow < static_cast<int>(lines.size()) && cursorCol > 0)
    {
        int sq = ansiSeqLen(lines[cursorRow].text, cursorCol);
        if (sq > 0) cursorCol += sq;
    }
}

void TextBuffer::clampCursor()
{
    if (cursorRow < 0) cursorRow = 0;
    if (cursorRow >= static_cast<int>(lines.size())) cursorRow = static_cast<int>(lines.size()) - 1;
    if (cursorCol < 0) cursorCol = 0;
    if (cursorRow >= 0 && cursorRow < static_cast<int>(lines.size()))
    {
        if (cursorCol > static_cast<int>(lines[cursorRow].text.size()))
            cursorCol = static_cast<int>(lines[cursorRow].text.size());
        if (cursorCol > 0)
        {
            int sq = ansiSeqLen(lines[cursorRow].text, cursorCol);
            if (sq > 0) cursorCol += sq;
            if (cursorCol > static_cast<int>(lines[cursorRow].text.size()))
                cursorCol = static_cast<int>(lines[cursorRow].text.size());
        }
    }
}

void TextBuffer::wrapLine()
{
    int lineDisplayWidth = byteColToDisplayCol(
        lines[cursorRow].text, static_cast<int>(lines[cursorRow].text.size()));
    if (lineDisplayWidth <= editWidth) return;

    // Find wrap position
    int wrapPos = static_cast<int>(lines[cursorRow].text.size());
    for (int w = wrapPos; w > 0; --w)
    {
        if (byteColToDisplayCol(lines[cursorRow].text, w) <= editWidth
            && lines[cursorRow].text[w - 1] == ' ')
        {
            wrapPos = w;
            break;
        }
    }
    if (wrapPos == static_cast<int>(lines[cursorRow].text.size()))
    {
        for (int w = 0; w < static_cast<int>(lines[cursorRow].text.size()); ++w)
        {
            if (byteColToDisplayCol(lines[cursorRow].text, w) >= editWidth)
            {
                wrapPos = w;
                break;
            }
        }
    }
    // ANSI safety
    {
        const string& lt = lines[cursorRow].text;
        for (int sc = wrapPos - 1; sc >= 0; --sc)
        {
            if (static_cast<uint8_t>(lt[sc]) == 0x1B)
            {
                int sl = ansiSeqLen(lt, sc);
                if (sl > 0 && sc + sl > wrapPos) wrapPos = sc;
                break;
            }
            char c = lt[sc];
            if (!((c >= '0' && c <= '9') || c == ';' || c == '[')) break;
        }
    }

    string overflow = lines[cursorRow].text.substr(wrapPos);
    lines[cursorRow].text = lines[cursorRow].text.substr(0, wrapPos);
    while (!lines[cursorRow].text.empty() && lines[cursorRow].text.back() == ' ')
        lines[cursorRow].text.pop_back();
    int strippedLeading = 0;
    while (!overflow.empty() && overflow[0] == ' ')
    {
        overflow = overflow.substr(1);
        ++strippedLeading;
    }

    int nextLineIdx = cursorRow + 1;
    if (nextLineIdx < static_cast<int>(lines.size()) && !overflow.empty())
    {
        string& nextText = lines[nextLineIdx].text;
        if (!nextText.empty())
        {
            bool needSpace = true;
            if (!overflow.empty() && overflow.back() == ' ') needSpace = false;
            if (!nextText.empty() && nextText[0] == ' ') needSpace = false;
            nextText = overflow + (needSpace ? " " : "") + nextText;
        }
        else
        {
            nextText = overflow;
        }
    }
    else if (!overflow.empty())
    {
        TextLine newLine;
        newLine.text = overflow;
        if (nextLineIdx < static_cast<int>(lines.size()))
            lines.insert(lines.begin() + nextLineIdx, newLine);
        else
            lines.push_back(newLine);
    }

    // Cursor adjustment
    if (cursorCol > static_cast<int>(lines[cursorRow].text.size()))
    {
        int offset = cursorCol - static_cast<int>(lines[cursorRow].text.size());
        offset -= strippedLeading;
        if (offset < 0) offset = 0;
        ++cursorRow;
        cursorCol = offset;
        fixCursorAnsi();
        if (cursorCol > static_cast<int>(lines[cursorRow].text.size()))
            cursorCol = static_cast<int>(lines[cursorRow].text.size());
    }

    // Cascade wrap
    for (int cr = cursorRow + 1; cr < static_cast<int>(lines.size()); ++cr)
    {
        int cDispW = byteColToDisplayCol(lines[cr].text, static_cast<int>(lines[cr].text.size()));
        if (cDispW <= editWidth) break;
        int nwp = static_cast<int>(lines[cr].text.size());
        for (int w = nwp; w > 0; --w)
        {
            if (byteColToDisplayCol(lines[cr].text, w) <= editWidth && lines[cr].text[w - 1] == ' ')
            { nwp = w; break; }
        }
        if (nwp >= static_cast<int>(lines[cr].text.size())) break;
        string cOverflow = lines[cr].text.substr(nwp);
        lines[cr].text = lines[cr].text.substr(0, nwp);
        while (!lines[cr].text.empty() && lines[cr].text.back() == ' ') lines[cr].text.pop_back();
        while (!cOverflow.empty() && cOverflow[0] == ' ') cOverflow = cOverflow.substr(1);
        if (cOverflow.empty()) break;
        int afterIdx = cr + 1;
        if (afterIdx < static_cast<int>(lines.size()))
        {
            string& afterText = lines[afterIdx].text;
            if (!afterText.empty())
            {
                bool ns = (!cOverflow.empty() && cOverflow.back() != ' ' && afterText[0] != ' ');
                afterText = cOverflow + (ns ? " " : "") + afterText;
            }
            else afterText = cOverflow;
        }
        else
        {
            lines.push_back({cOverflow});
        }
    }
}

void TextBuffer::pullUp()
{
    pullUpWords(lines, cursorRow, editWidth);
}

void TextBuffer::inputChar(char ch)
{
    if (insertMode)
    {
        lines[cursorRow].text.insert(cursorCol, 1, ch);
        ++cursorCol;
    }
    else
    {
        int writePos = cursorCol;
        int seqLen = ansiSeqLen(lines[cursorRow].text, writePos);
        if (seqLen > 0)
        {
            lines[cursorRow].text.insert(writePos, 1, ch);
        }
        else if (writePos < static_cast<int>(lines[cursorRow].text.size()))
        {
            lines[cursorRow].text[writePos] = ch;
        }
        else
        {
            lines[cursorRow].text += ch;
        }
        ++cursorCol;
    }
    // Skip over ANSI sequences after cursor advance
    {
        int sq = ansiSeqLen(lines[cursorRow].text, cursorCol);
        if (sq > 0) cursorCol += sq;
    }
    wrapLine();
    clampCursor();
}

void TextBuffer::doBackspace()
{
    if (cursorCol > 0)
    {
        cursorCol = eraseBackward(lines[cursorRow].text, cursorCol);
        pullUp();
    }
    else if (cursorRow > 0)
    {
        cursorCol = static_cast<int>(lines[cursorRow - 1].text.size());
        if (!lines[cursorRow - 1].text.empty() && !lines[cursorRow].text.empty()
            && lines[cursorRow - 1].text.back() != ' ' && lines[cursorRow].text[0] != ' ')
        {
            lines[cursorRow - 1].text += ' ';
        }
        lines[cursorRow - 1].text += lines[cursorRow].text;
        // Inherit the hard break from the removed line
        lines[cursorRow - 1].hardBreak = lines[cursorRow].hardBreak;
        lines.erase(lines.begin() + cursorRow);
        --cursorRow;
        // Merged line may be too long — wrap if needed
        wrapLine();
    }
    clampCursor();
}

void TextBuffer::doDelete()
{
    if (cursorCol < static_cast<int>(lines[cursorRow].text.size()))
    {
        eraseForward(lines[cursorRow].text, cursorCol);
        pullUp();
    }
    else if (cursorRow < static_cast<int>(lines.size()) - 1)
    {
        if (!lines[cursorRow].text.empty() && !lines[cursorRow + 1].text.empty()
            && lines[cursorRow].text.back() != ' ' && lines[cursorRow + 1].text[0] != ' ')
        {
            lines[cursorRow].text += ' ';
        }
        lines[cursorRow].text += lines[cursorRow + 1].text;
        // Inherit the hard break from the removed line
        lines[cursorRow].hardBreak = lines[cursorRow + 1].hardBreak;
        lines.erase(lines.begin() + cursorRow + 1);
        // Merged line may be too long — wrap if needed
        wrapLine();
    }
    clampCursor();
}

void TextBuffer::doEnter()
{
    int splitPos = cursorCol;
    int seqAt = ansiSeqLen(lines[cursorRow].text, splitPos);
    if (seqAt > 0) splitPos += seqAt;
    if (splitPos > 0 && splitPos < static_cast<int>(lines[cursorRow].text.size()))
    {
        for (int sc = splitPos - 1; sc >= 0; --sc)
        {
            if (static_cast<uint8_t>(lines[cursorRow].text[sc]) == 0x1B)
            {
                int sl = ansiSeqLen(lines[cursorRow].text, sc);
                if (sl > 0 && sc + sl > splitPos) splitPos = sc + sl;
                break;
            }
            char c = lines[cursorRow].text[sc];
            if (!((c >= '0' && c <= '9') || c == ';' || c == '[')) break;
        }
    }
    string remainder = lines[cursorRow].text.substr(splitPos);
    lines[cursorRow].text = lines[cursorRow].text.substr(0, splitPos);
    // Mark this line as having a hard break (user pressed Enter)
    lines[cursorRow].hardBreak = true;
    TextLine newLine;
    newLine.text = remainder;
    newLine.hardBreak = false; // The new line inherits no break yet
    lines.insert(lines.begin() + cursorRow + 1, newLine);
    ++cursorRow;
    cursorCol = 0;
}

void TextBuffer::cursorLeft()
{
    if (cursorCol > 0)
    {
        cursorCol = skipBackward(lines[cursorRow].text, cursorCol);
    }
    else if (cursorRow > 0)
    {
        --cursorRow;
        cursorCol = static_cast<int>(lines[cursorRow].text.size());
    }
}

void TextBuffer::cursorRight()
{
    if (cursorCol < static_cast<int>(lines[cursorRow].text.size()))
    {
        cursorCol = skipForward(lines[cursorRow].text, cursorCol);
    }
    else if (cursorRow < static_cast<int>(lines.size()) - 1)
    {
        ++cursorRow;
        cursorCol = 0;
    }
}

void TextBuffer::cursorUp()
{
    if (cursorRow > 0)
    {
        --cursorRow;
        clampCursor();
    }
}

void TextBuffer::cursorDown()
{
    if (cursorRow < static_cast<int>(lines.size()) - 1)
    {
        ++cursorRow;
        clampCursor();
    }
}

void TextBuffer::cursorHome()
{
    cursorCol = 0;
}

void TextBuffer::cursorEnd()
{
    cursorCol = static_cast<int>(lines[cursorRow].text.size());
}

void TextBuffer::deleteLine()
{
    if (cursorRow < static_cast<int>(lines.size()) - 1)
    {
        lines.erase(lines.begin() + cursorRow);
    }
    else
    {
        lines[cursorRow].text.clear();
    }
    cursorCol = 0;
    clampCursor();
}

void TextBuffer::insertRaw(const string& s)
{
    lines[cursorRow].text.insert(cursorCol, s);
    cursorCol += static_cast<int>(s.size());
    clampCursor();
}

string TextBuffer::getBody() const
{
    // Join lines for saving: soft-wrapped lines (from word wrap) are joined
    // with a space to form a single paragraph. Hard breaks (from Enter) emit
    // a newline to separate paragraphs.
    string body;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        body += lines[i].text;
        if (i + 1 < lines.size())
        {
            if (lines[i].hardBreak)
            {
                body += "\n"; // User pressed Enter — paragraph boundary
            }
            else
            {
                // Soft wrap — join with space (same paragraph)
                // Only add a space if the current line doesn't end with one
                // and the next line doesn't start with one
                if (!lines[i].text.empty() && lines[i].text.back() != ' '
                    && !lines[i + 1].text.empty() && lines[i + 1].text[0] != ' ')
                {
                    body += " ";
                }
            }
        }
    }
    return body;
}

string TextBuffer::getLineText(int row) const
{
    if (row >= 0 && row < static_cast<int>(lines.size()))
        return lines[row].text;
    return "";
}

int TextBuffer::lineCount() const
{
    return static_cast<int>(lines.size());
}

int TextBuffer::displayCol() const
{
    if (cursorRow >= 0 && cursorRow < static_cast<int>(lines.size()))
        return byteColToDisplayCol(lines[cursorRow].text, cursorCol);
    return 0;
}
