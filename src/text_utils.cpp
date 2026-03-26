// text_utils.cpp — General-purpose text processing utilities for SlyMail.
// See text_utils.h for the public interface and documentation.

#include "text_utils.h"
#include <cctype>
#include <cstdint>

using std::string;
using std::vector;

// ============================================================
// Quote prefix detection
// ============================================================

string detectQuotePrefix(const string& line)
{
    if (line.empty()) return "";

    // Detect quote prefixes in BBS message text. Common formats:
    //   "> "            — simple single-level quote
    //   " > "           — indented single-level quote
    //   ">> "           — nested quote (two levels)
    //   "> > "          — spaced nested quote (two levels)
    //   "EO> "          — initials-based quote
    //   " EO> "         — indented initials-based quote
    //   " > Vi> "       — outer quote wrapping an initials quote
    //   " > > Vi> "     — multiple nesting levels
    //   " > > > "       — three levels of quoting
    //
    // Strategy: scan for repeating segments of [optional-space][optional-initials]>
    // The prefix ends at the last '>' followed by a space. After each '>'+space,
    // check if another quote segment follows (initials or another '>').

    size_t len = line.size();
    size_t i = 0;
    size_t lastGtPos = string::npos;

    while (i < len)
    {
        // Skip spaces/tabs
        while (i < len && (line[i] == ' ' || line[i] == '\t'))
        {
            ++i;
        }
        if (i >= len) break;

        // Check for '>' directly
        if (line[i] == '>')
        {
            lastGtPos = i;
            ++i;
            continue;
        }

        // Check for alphanumeric initials followed by '>'
        // (e.g., "Vi" in "Vi> " or "EO" in "EO> ")
        size_t initStart = i;
        while (i < len && (std::isalnum(static_cast<unsigned char>(line[i])) || line[i] == '_'))
        {
            ++i;
        }
        if (i > initStart && i < len && line[i] == '>')
        {
            // Found initials + '>' (e.g., "Vi>")
            lastGtPos = i;
            ++i;
            continue;
        }

        // Not a quote pattern — stop scanning.
        // Reset i to initStart since those chars are not part of the prefix.
        break;
    }

    if (lastGtPos == string::npos)
    {
        return ""; // No '>' found
    }

    // The prefix includes everything up to and including the last '>'
    // plus one trailing space if present.
    size_t prefixEnd = lastGtPos + 1;
    if (prefixEnd < len && line[prefixEnd] == ' ')
    {
        ++prefixEnd; // Include trailing space
    }

    // Sanity check: reject prefixes that are unreasonably long.
    // A genuine quote prefix rarely exceeds ~25 characters even with long
    // usernames and multiple nesting levels.
    if (prefixEnd > 25)
    {
        return "";
    }

    return line.substr(0, prefixEnd);
}

// ============================================================
// Tear/origin line detection
// ============================================================

bool isTearOrOrigin(const string& line)
{
    if (line.size() >= 3 && (line.substr(0, 3) == "---" || line.substr(0, 3) == "-!-"))
        return true;
    if (line.find(" * Origin:") != string::npos)
        return true;
    return false;
}

// ============================================================
// Simple word wrap
// ============================================================

vector<string> wordWrapStr(const string& text, int maxWidth)
{
    vector<string> result;
    if (maxWidth <= 0) { result.push_back(text); return result; }

    string remaining = text;
    while (static_cast<int>(remaining.size()) > maxWidth)
    {
        // Find the last space at or before maxWidth
        int wrapPos = maxWidth;
        bool foundSpace = false;
        for (int w = maxWidth; w > maxWidth / 2; --w)
        {
            if (remaining[w] == ' ')
            {
                wrapPos = w;
                foundSpace = true;
                break;
            }
        }
        if (!foundSpace)
        {
            wrapPos = maxWidth; // Force break
        }
        result.push_back(remaining.substr(0, wrapPos));
        remaining = remaining.substr(wrapPos);
        // Strip leading spaces from the remainder
        while (!remaining.empty() && remaining[0] == ' ')
        {
            remaining = remaining.substr(1);
        }
    }
    if (!remaining.empty())
    {
        result.push_back(remaining);
    }
    return result;
}

// ============================================================
// Quote line re-wrapping
// ============================================================

vector<string> wrapQuoteLines(const vector<string>& quoteLines, int maxWidth)
{
    if (quoteLines.empty()) return {};

    vector<string> result;

    // Group consecutive lines with the same quote prefix into paragraphs.
    // Blank lines and tear/origin lines break paragraphs.
    size_t i = 0;
    while (i < quoteLines.size())
    {
        const string& line = quoteLines[i];

        // Blank line — preserve as-is
        if (line.empty() || line.find_first_not_of(" \t") == string::npos)
        {
            result.push_back(line);
            ++i;
            continue;
        }

        // Detect quote prefix for this line
        string prefix = detectQuotePrefix(line);

        // A line that is ONLY a prefix (no text after it) acts as a
        // paragraph separator — preserve as-is
        string withoutPrefix = (prefix.empty()) ? line : line.substr(prefix.size());
        if (!prefix.empty() && (withoutPrefix.empty()
            || withoutPrefix.find_first_not_of(" \t") == string::npos))
        {
            result.push_back(line);
            ++i;
            continue;
        }

        // Tear/origin lines — preserve as-is
        if (isTearOrOrigin(withoutPrefix))
        {
            result.push_back(line);
            ++i;
            continue;
        }

        // Collect consecutive lines with the same prefix into a paragraph
        string paragraph;
        while (i < quoteLines.size())
        {
            const string& curLine = quoteLines[i];

            // Blank line breaks the paragraph
            if (curLine.empty() || curLine.find_first_not_of(" \t") == string::npos)
                break;

            string curPrefix = detectQuotePrefix(curLine);
            if (curPrefix != prefix)
                break; // Different prefix — new section

            string curText = (curPrefix.empty()) ? curLine : curLine.substr(curPrefix.size());

            // Prefix-only line (no text after prefix) breaks the paragraph
            if (!curPrefix.empty() && (curText.empty()
                || curText.find_first_not_of(" \t") == string::npos))
                break;

            // Tear/origin line breaks the paragraph
            if (isTearOrOrigin(curText))
                break;

            // Append to paragraph text
            if (!paragraph.empty())
            {
                if (!paragraph.empty() && paragraph.back() != ' '
                    && !curText.empty() && curText[0] != ' ')
                {
                    paragraph += ' ';
                }
            }
            paragraph += curText;
            ++i;
        }

        // Re-wrap the paragraph to fit within maxWidth
        if (paragraph.empty())
        {
            result.push_back(prefix);
            continue;
        }

        int textWidth = maxWidth - static_cast<int>(prefix.size());
        if (textWidth < 10) textWidth = 10;

        auto wrappedLines = wordWrapStr(paragraph, textWidth);
        for (const auto& wl : wrappedLines)
        {
            if (!wl.empty())
            {
                result.push_back(prefix + wl);
            }
            else
            {
                result.push_back(prefix);
            }
        }
    }

    return result;
}
