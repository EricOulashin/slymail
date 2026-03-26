// test_wrap_quote_lines.cpp — Unit tests for quote prefix detection and
// quote line re-wrapping. Tests the functions in text_utils.h/cpp.
//
// Usage: ./test_wrap_quote_lines
// Exit code: 0 = all tests passed, 1 = one or more failed

#include "text_utils.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using std::string;
using std::vector;

static int passed = 0;
static int failed = 0;

static void check(bool cond, const char* testName, const char* detail = nullptr)
{
    if (cond) { ++passed; }
    else
    {
        ++failed;
        printf("  FAIL: %s", testName);
        if (detail) printf(" — %s", detail);
        printf("\n");
    }
}

// ============================================================
// Quote prefix detection tests
// ============================================================

static void testPrefixSimpleGt()
{
    printf("Test: Prefix - simple '> '\n");
    check(detectQuotePrefix("> Hello") == "> ", "bare > prefix");
}

static void testPrefixIndentedGt()
{
    printf("Test: Prefix - indented ' > '\n");
    check(detectQuotePrefix(" > Hello") == " > ", "indented > prefix");
}

static void testPrefixTwoCharInitials()
{
    printf("Test: Prefix - two-char initials 'Vi> '\n");
    check(detectQuotePrefix("Vi> Hello") == "Vi> ", "Vi> prefix");
}

static void testPrefixIndentedTwoCharInitials()
{
    printf("Test: Prefix - indented ' Vi> '\n");
    check(detectQuotePrefix(" Vi> Hello") == " Vi> ", "indented Vi> prefix");
}

static void testPrefixSingleCharInitial()
{
    printf("Test: Prefix - single-char initial 'V> '\n");
    check(detectQuotePrefix("V> Hello") == "V> ", "V> prefix");
}

static void testPrefixThreeCharInitials()
{
    printf("Test: Prefix - three-char initials 'ESC> '\n");
    check(detectQuotePrefix("ESC> Hello") == "ESC> ", "ESC> prefix");
}

static void testPrefixLongName()
{
    printf("Test: Prefix - long name 'Nightfox> '\n");
    check(detectQuotePrefix("Nightfox> Hello") == "Nightfox> ", "Nightfox> prefix");
}

static void testPrefixIndentedLongName()
{
    printf("Test: Prefix - indented long name ' Nightfox> '\n");
    check(detectQuotePrefix(" Nightfox> Hello") == " Nightfox> ", "indented Nightfox>");
}

static void testPrefixNestedBare()
{
    printf("Test: Prefix - nested bare '>> '\n");
    check(detectQuotePrefix(">> Hello") == ">> ", ">> prefix");
}

static void testPrefixSpacedNested()
{
    printf("Test: Prefix - spaced nested '> > '\n");
    check(detectQuotePrefix("> > Hello") == "> > ", "> > prefix");
}

static void testPrefixTripleNested()
{
    printf("Test: Prefix - triple nested '> > > '\n");
    check(detectQuotePrefix("> > > Hello") == "> > > ", "> > > prefix");
}

static void testPrefixNestedWithInitials()
{
    printf("Test: Prefix - nested with initials ' > Vi> '\n");
    check(detectQuotePrefix(" > Vi> Hello") == " > Vi> ", "> Vi> prefix");
}

static void testPrefixNestedWithLongInitials()
{
    printf("Test: Prefix - nested with long initials ' > Nightfox> '\n");
    check(detectQuotePrefix(" > Nightfox> Hello") == " > Nightfox> ", "> Nightfox> prefix");
}

static void testPrefixNestedWithESC()
{
    printf("Test: Prefix - nested with ESC ' > ESC> '\n");
    check(detectQuotePrefix(" > ESC> Hello") == " > ESC> ", "> ESC> prefix");
}

static void testPrefixTripleWithInitials()
{
    printf("Test: Prefix - triple nested with initials ' > Vi> ESC> '\n");
    check(detectQuotePrefix(" > Vi> ESC> Hello") == " > Vi> ESC> ", "> Vi> ESC> prefix");
}

static void testPrefixDoubleNestedBare()
{
    printf("Test: Prefix - double nested with space ' > > '\n");
    check(detectQuotePrefix(" > > text here") == " > > ", " > > prefix");
}

static void testPrefixNoPrefix()
{
    printf("Test: Prefix - no prefix on normal text\n");
    check(detectQuotePrefix("Hello World") == "", "no prefix");
}

static void testPrefixEmpty()
{
    printf("Test: Prefix - empty line\n");
    check(detectQuotePrefix("") == "", "empty");
}

static void testPrefixGtInMiddleOfText()
{
    printf("Test: Prefix - > in middle of sentence\n");
    check(detectQuotePrefix("The value 5 > 3 is true") == "", "no false positive");
}

static void testPrefixDefinitelyGt()
{
    printf("Test: Prefix - 'definitely > 5' not a prefix\n");
    check(detectQuotePrefix("The value is definitely > 5") == "", "no false positive");
}

static void testPrefixBareGtNoSpace()
{
    printf("Test: Prefix - bare > without trailing space\n");
    string r = detectQuotePrefix(">Hello");
    check(r == ">", "bare > without space");
}

static void testPrefixOnlyPrefix()
{
    printf("Test: Prefix - line is only the prefix\n");
    check(detectQuotePrefix("> ") == "> ", "only prefix > ");
    check(detectQuotePrefix(" > ") == " > ", "only prefix  > ");
}

static void testPrefixGtAtEnd()
{
    printf("Test: Prefix - > at end of line\n");
    check(detectQuotePrefix(">") == ">", "> at end");
}

// ============================================================
// Quote wrapping tests
// ============================================================

static void testWrapBasicLongLine()
{
    printf("Test: Wrap - basic long quoted line\n");
    vector<string> lines = {
        " > This is a very long quoted line that definitely exceeds the seventy-nine character width limit and needs wrapping"
    };
    auto result = wrapQuoteLines(lines, 79);
    for (const auto& l : result)
    {
        check(static_cast<int>(l.size()) <= 79, "Line fits in 79", l.c_str());
        if (!l.empty()) check(l.find(" > ") == 0, "Prefix preserved", l.c_str());
    }
    check(result.size() >= 2, "Wrapped to multiple lines");
}

static void testWrapInitialsPrefix()
{
    printf("Test: Wrap - initials prefix\n");
    vector<string> lines = {
        " EO> This is a quoted line with initials prefix that is long enough to need wrapping to fit within limits"
    };
    auto result = wrapQuoteLines(lines, 79);
    for (const auto& l : result)
    {
        check(static_cast<int>(l.size()) <= 79, "Fits in 79", l.c_str());
        if (!l.empty()) check(l.find(" EO> ") == 0, "EO prefix preserved", l.c_str());
    }
}

static void testWrapPreservesBlankLines()
{
    printf("Test: Wrap - blank lines preserved between paragraphs\n");
    vector<string> lines = {" > First paragraph.", "", " > Second paragraph."};
    auto result = wrapQuoteLines(lines, 79);
    bool hasBlank = false;
    for (const auto& l : result)
    {
        if (l.empty()) hasBlank = true;
    }
    check(hasBlank, "Blank line preserved");
}

static void testWrapPreservesPrefixOnlyLines()
{
    printf("Test: Wrap - prefix-only lines preserved as paragraph separators\n");
    vector<string> lines = {
        " > First paragraph text.",
        " > ",
        " > Second paragraph text."
    };
    auto result = wrapQuoteLines(lines, 79);
    bool hasPrefixOnly = false;
    for (const auto& l : result)
    {
        string pfx = detectQuotePrefix(l);
        string body = (!pfx.empty() && pfx.size() < l.size()) ? l.substr(pfx.size()) : "";
        if (!pfx.empty() && (body.empty() || body.find_first_not_of(" \t") == string::npos))
        {
            hasPrefixOnly = true;
        }
    }
    check(hasPrefixOnly, "Prefix-only separator preserved");
}

static void testWrapPreservesTearLine()
{
    printf("Test: Wrap - tear and origin lines not wrapped\n");
    vector<string> lines = {
        " > Some text.",
        "--- SlyMail v0.52",
        " * Origin: My BBS (1:234/567)"
    };
    auto result = wrapQuoteLines(lines, 79);
    bool hasTear = false, hasOrigin = false;
    for (const auto& l : result)
    {
        if (l.find("---") == 0) hasTear = true;
        if (l.find("* Origin:") != string::npos) hasOrigin = true;
    }
    check(hasTear, "Tear line preserved");
    check(hasOrigin, "Origin line preserved");
}

static void testWrapGroupsSamePrefixLines()
{
    printf("Test: Wrap - groups consecutive same-prefix lines\n");
    vector<string> lines = {
        " > Line one of paragraph",
        " > continues on line two",
        " > and line three"
    };
    auto result = wrapQuoteLines(lines, 50);
    // Check all text present
    string all;
    for (const auto& l : result)
    {
        string pfx = detectQuotePrefix(l);
        all += (pfx.empty()) ? l : l.substr(pfx.size());
        all += " ";
    }
    check(all.find("Line one") != string::npos, "Line one present");
    check(all.find("line three") != string::npos, "Line three present");
}

static void testWrapNestedQuotesSeparately()
{
    printf("Test: Wrap - nested quotes wrapped independently\n");
    vector<string> lines = {
        " > First level quote that is long enough to need wrapping around the width",
        " > second part of first level quote",
        " > Vi> Second level nested that is also quite long and will need wrapping",
        " > Vi> more of second level"
    };
    auto result = wrapQuoteLines(lines, 50);
    bool hasLevel1 = false, hasLevel2 = false;
    for (const auto& l : result)
    {
        if (!l.empty())
        {
            check(static_cast<int>(l.size()) <= 50, "Fits in 50", l.c_str());
        }
        if (l.find(" > Vi> ") == 0) hasLevel2 = true;
        else if (l.find(" > ") == 0 && l.find(" > Vi>") != 0) hasLevel1 = true;
    }
    check(hasLevel1, "Level 1 present");
    check(hasLevel2, "Level 2 present");
}

static void testWrapRealWorldScenario()
{
    printf("Test: Wrap - real-world nested quote scenario from screenshot\n");
    vector<string> lines = {
        " > Vi> A lot of cats I've met from Chicago",
        " > Vi> despair of the fact that they're",
        " > Vi> known for deep-dish. I've heard it",
        " > Vi> said that it is a tourist pizza.",
        " > ",
        " > My San Francisco friends serve clam chowder in a bowl, not a hollowed out",
        " > loaf of sourdough bread.",
        " > ",
        " > You can take the loaf home and put it in the dishwasher to use again -",
        " > or so I've heard.",
        " > ",
        " > ... What is the reality of the situation?",
    };
    auto result = wrapQuoteLines(lines, 79);

    // Vi> section should have the full prefix
    int viLines = 0;
    for (const auto& l : result)
    {
        if (l.find(" > Vi> ") == 0) ++viLines;
    }
    check(viLines >= 2, "Vi> lines present with correct prefix");

    // All lines should fit
    for (const auto& l : result)
    {
        if (!l.empty())
        {
            check(static_cast<int>(l.size()) <= 79, "Fits in 79", l.c_str());
        }
    }

    // Paragraph separators preserved
    int sepCount = 0;
    for (const auto& l : result)
    {
        if (l == " > " || l.empty()) ++sepCount;
    }
    check(sepCount >= 3, "Paragraph separators preserved");

    // All text preserved
    string all;
    for (const auto& l : result) all += l + " ";
    check(all.find("Chicago") != string::npos, "Chicago present");
    check(all.find("tourist pizza") != string::npos, "tourist pizza present");
    check(all.find("clam chowder") != string::npos, "clam chowder present");
    check(all.find("dishwasher") != string::npos, "dishwasher present");
    check(all.find("reality") != string::npos, "reality present");
}

static void testWrapNarrowWidth()
{
    printf("Test: Wrap - very narrow max width\n");
    vector<string> lines = {" > Hello World this is a long quote"};
    auto result = wrapQuoteLines(lines, 20);
    for (const auto& l : result)
    {
        if (!l.empty())
        {
            check(static_cast<int>(l.size()) <= 20, "Fits in 20", l.c_str());
        }
    }
    check(result.size() >= 2, "Wrapped");
}

static void testWrapShortLinesJoined()
{
    printf("Test: Wrap - short lines joined into paragraph\n");
    vector<string> lines = {" > Short.", " > Also short."};
    auto result = wrapQuoteLines(lines, 79);
    // Should be joined (both fit in one line)
    check(result.size() == 1, "Joined into one line");
    string all;
    for (const auto& l : result)
    {
        string pfx = detectQuotePrefix(l);
        all += (!pfx.empty()) ? l.substr(pfx.size()) : l;
    }
    check(all.find("Short.") != string::npos, "Short present");
    check(all.find("Also short.") != string::npos, "Also short present");
}

static void testWrapDifferentPrefixSections()
{
    printf("Test: Wrap - different prefix sections not merged\n");
    vector<string> lines = {
        " > Section one text",
        " EO> Section two text"
    };
    auto result = wrapQuoteLines(lines, 79);
    // These should remain separate since prefixes differ
    bool hasGt = false, hasEO = false;
    for (const auto& l : result)
    {
        if (l.find(" > ") == 0 && l.find(" EO>") != 0) hasGt = true;
        if (l.find(" EO> ") == 0) hasEO = true;
    }
    check(hasGt, "Section with > present");
    check(hasEO, "Section with EO> present");
}

static void testWrapLongNamePrefix()
{
    printf("Test: Wrap - long username prefix\n");
    vector<string> lines = {
        " > Nightfox> This is a quoted line from Nightfox that is quite long and needs wrapping within limits",
        " > Nightfox> And this continues the same paragraph from Nightfox"
    };
    auto result = wrapQuoteLines(lines, 60);
    for (const auto& l : result)
    {
        if (!l.empty())
        {
            check(l.find(" > Nightfox> ") == 0, "Nightfox prefix preserved", l.c_str());
            check(static_cast<int>(l.size()) <= 60, "Fits in 60", l.c_str());
        }
    }
}

static void testWrapEmptyInput()
{
    printf("Test: Wrap - empty input\n");
    vector<string> lines;
    auto result = wrapQuoteLines(lines, 79);
    check(result.empty(), "Empty result");
}

static void testWrapSingleShortLine()
{
    printf("Test: Wrap - single short line unchanged\n");
    vector<string> lines = {" > Short."};
    auto result = wrapQuoteLines(lines, 79);
    check(result.size() == 1, "Still one line");
    check(result[0] == " > Short.", "Unchanged");
}

// ============================================================
// Main
// ============================================================

int main()
{
    printf("=== SlyMail Quote Wrapping Unit Tests ===\n\n");

    // Prefix detection
    testPrefixSimpleGt();
    testPrefixIndentedGt();
    testPrefixTwoCharInitials();
    testPrefixIndentedTwoCharInitials();
    testPrefixSingleCharInitial();
    testPrefixThreeCharInitials();
    testPrefixLongName();
    testPrefixIndentedLongName();
    testPrefixNestedBare();
    testPrefixSpacedNested();
    testPrefixTripleNested();
    testPrefixNestedWithInitials();
    testPrefixNestedWithLongInitials();
    testPrefixNestedWithESC();
    testPrefixTripleWithInitials();
    testPrefixDoubleNestedBare();
    testPrefixNoPrefix();
    testPrefixEmpty();
    testPrefixGtInMiddleOfText();
    testPrefixDefinitelyGt();
    testPrefixBareGtNoSpace();
    testPrefixOnlyPrefix();
    testPrefixGtAtEnd();

    // Quote wrapping
    testWrapBasicLongLine();
    testWrapInitialsPrefix();
    testWrapPreservesBlankLines();
    testWrapPreservesPrefixOnlyLines();
    testWrapPreservesTearLine();
    testWrapGroupsSamePrefixLines();
    testWrapNestedQuotesSeparately();
    testWrapRealWorldScenario();
    testWrapNarrowWidth();
    testWrapShortLinesJoined();
    testWrapDifferentPrefixSections();
    testWrapLongNamePrefix();
    testWrapEmptyInput();
    testWrapSingleShortLine();

    printf("\n=== Results: %d passed, %d failed ===\n", passed, failed);
    return (failed > 0) ? 1 : 0;
}
