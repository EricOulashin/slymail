#ifndef SLYMAIL_TEXT_UTILS_H
#define SLYMAIL_TEXT_UTILS_H

// Text utility functions for SlyMail.
// General-purpose text processing that is not tied to interactive text input.

#include <string>
#include <vector>

// Detect the quote prefix at the start of a line.
// Matches patterns like "> ", " > ", "EO> ", " EO> ", ">> ", etc.
// Returns the prefix (including trailing space) or empty string if none.
std::string detectQuotePrefix(const std::string& line);

// Check if a line is a tear line ("---") or origin line ("* Origin:").
bool isTearOrOrigin(const std::string& line);

// Word-wrap a single string to maxWidth, returning a vector of lines.
std::vector<std::string> wordWrapStr(const std::string& text, int maxWidth);

// Re-wrap quote lines so they fit within maxWidth (default 79).
// Groups consecutive lines with the same quote prefix into paragraphs,
// joins each paragraph into a single string, and word-wraps it to
// (maxWidth - prefixLength) characters, then re-applies the prefix.
// Blank lines and tear/origin lines are preserved as-is.
std::vector<std::string> wrapQuoteLines(const std::vector<std::string>& quoteLines,
                                         int maxWidth = 79);

#endif // SLYMAIL_TEXT_UTILS_H
