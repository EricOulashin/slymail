#include "search.h"
#include <cctype>
#include <ctime>
#include <algorithm>

using std::string;
using std::vector;

// ============================================================
// SimpleDate implementation
// ============================================================

string SimpleDate::toQwkStr() const
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d-%02d-%02d", month, day, year % 100);
    return buf;
}

string SimpleDate::toIsoStr() const
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", year, month, day);
    return buf;
}

static const char* monthNames[] = {
    "", "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

static const char* dayNames[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

string SimpleDate::toDisplayStr() const
{
    if (!isValid()) return "";
    int dow = dayOfWeek(year, month, day);
    char buf[64];
    snprintf(buf, sizeof(buf), "%s, %s %d, %d",
             dayNames[dow], monthNames[month], day, year);
    return buf;
}

int SimpleDate::compareTo(const SimpleDate& other) const
{
    if (year != other.year) return year - other.year;
    if (month != other.month) return month - other.month;
    return day - other.day;
}

SimpleDate SimpleDate::fromQwkStr(const string& s)
{
    // Format: "MM-DD-YY" (standard QWK) or variations like "M-DD-YY", "MM-D-YY"
    // Also handles "MM-DD-YYYY" (4-digit year)
    SimpleDate d;
    if (s.empty()) return d;

    // Split on '-' delimiter for robust parsing
    size_t dash1 = s.find('-');
    if (dash1 == string::npos) return d;
    size_t dash2 = s.find('-', dash1 + 1);
    if (dash2 == string::npos) return d;

    try
    {
        string monthStr = s.substr(0, dash1);
        string dayStr = s.substr(dash1 + 1, dash2 - dash1 - 1);
        string yearStr = s.substr(dash2 + 1);

        d.month = std::stoi(monthStr);
        d.day = std::stoi(dayStr);
        int yr = std::stoi(yearStr);

        if (yr >= 100)
        {
            // 4-digit year (e.g., 2026)
            d.year = yr;
        }
        else
        {
            // Two-digit year: 00-79 = 2000-2079, 80-99 = 1980-1999
            d.year = (yr < 80) ? 2000 + yr : 1900 + yr;
        }

        // Validate ranges
        if (d.month < 1 || d.month > 12) { d = SimpleDate(); return d; }
        if (d.day < 1 || d.day > 31) { d = SimpleDate(); return d; }
        if (d.year < 1900 || d.year > 2099) { d = SimpleDate(); return d; }
    }
    catch (...) { d = SimpleDate(); }
    return d;
}

SimpleDate currentDate()
{
    time_t now = std::time(nullptr);
    struct tm* t = localtime(&now);
    return SimpleDate(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
}

int daysInMonth(int month, int year)
{
    static const int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month < 1 || month > 12) return 30;
    if (month == 2)
    {
        bool leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
        return leap ? 29 : 28;
    }
    return days[month];
}

int dayOfWeek(int year, int month, int day)
{
    // Tomohiko Sakamoto's algorithm
    static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    int y = year;
    if (month < 3) --y;
    return (y + y/4 - y/100 + y/400 + t[month - 1] + day) % 7;
}

// ============================================================
// Date Picker Dialog
// ============================================================

bool showDatePicker(SimpleDate& result, const SimpleDate& initial)
{
    int cols = g_term->getCols();
    int rows = g_term->getRows();

    // Initialize with current date or provided initial
    SimpleDate sel = initial.isValid() ? initial : currentDate();
    int focusDay = sel.day;
    int dispMonth = sel.month;
    int dispYear = sel.year;

    // Dialog dimensions (inspired by cxWidgets date picker)
    int dlgW = 38;
    int dlgH = 18;
    int dlgX = (cols - dlgW) / 2;
    int dlgY = (rows - dlgH) / 2;

    TermAttr borderAttr = tAttr(TC_CYAN, TC_BLUE, true);
    TermAttr titleAttr = tAttr(TC_WHITE, TC_BLUE, true);
    TermAttr dateHdrAttr = tAttr(TC_WHITE, TC_BLUE, false);
    TermAttr dayHdrAttr = tAttr(TC_WHITE, TC_BLUE, true);
    TermAttr dayAttr = tAttr(TC_WHITE, TC_BLUE, false);
    TermAttr focusAttr = tAttr(TC_BLUE, TC_WHITE, true);  // Reversed for focused day
    TermAttr btnAttr = tAttr(TC_WHITE, TC_BLUE, true);
    TermAttr btnFocusAttr = tAttr(TC_BLUE, TC_WHITE, true);
    TermAttr navAttr = tAttr(TC_WHITE, TC_BLUE, true);
    TermAttr bgAttr = tAttr(TC_CYAN, TC_BLUE, false);

    // Focus: 0=calendar, 1=prevMonth(<), 2=nextMonth(>), 3=year, 4=OK, 5=Cancel
    int focus = 0;

    while (true)
    {
        // Clamp focusDay
        int maxDay = daysInMonth(dispMonth, dispYear);
        if (focusDay > maxDay) focusDay = maxDay;
        if (focusDay < 1) focusDay = 1;

        // Clear dialog area with blue background
        for (int r = 0; r < dlgH; ++r)
        {
            fillRow(dlgY + r, bgAttr, dlgX, dlgX + dlgW);
        }

        // Border
        g_term->setAttr(borderAttr);
        g_term->drawBox(dlgY, dlgX, dlgH, dlgW);

        // Title: "Select Date"
        string title = " Select Date ";
        int titleX = dlgX + (dlgW - static_cast<int>(title.size())) / 2;
        g_term->setAttr(borderAttr);
        g_term->drawHLine(dlgY, dlgX + 1, dlgW - 2);
        printAt(dlgY, titleX, title, titleAttr);

        // Date header row
        SimpleDate shown(dispYear, dispMonth, focusDay);
        printAt(dlgY + 1, dlgX + 2, shown.toDisplayStr(), dateHdrAttr);

        // Separator line
        g_term->setAttr(borderAttr);
        g_term->putCP437(dlgY + 2, dlgX, CP437_BOX_DRAWINGS_LIGHT_LEFT_T);
        g_term->drawHLine(dlgY + 2, dlgX + 1, dlgW - 2);
        g_term->putCP437(dlgY + 2, dlgX + dlgW - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);

        // Navigation: <  Month         >   Year
        int navY = dlgY + 3;
        printAt(navY, dlgX + 3, "<", (focus == 1) ? btnFocusAttr : navAttr);
        string monName = monthNames[dispMonth];
        printAt(navY, dlgX + 7, monName, titleAttr);
        printAt(navY, dlgX + 24, ">", (focus == 2) ? btnFocusAttr : navAttr);
        string yearStr = std::to_string(dispYear);
        printAt(navY, dlgX + 28, yearStr, (focus == 3) ? btnFocusAttr : titleAttr);

        // Day-of-week headers
        int calY = dlgY + 5;
        printAt(calY, dlgX + 2, "Su Mo Tu We Th Fr Sa", dayHdrAttr);

        // Calendar grid
        int firstDow = dayOfWeek(dispYear, dispMonth, 1);
        int gridY = calY + 1;
        for (int day = 1; day <= maxDay; ++day)
        {
            int pos = firstDow + day - 1;
            int row = gridY + pos / 7;
            int col = dlgX + 2 + (pos % 7) * 3;

            char buf[16];
            snprintf(buf, sizeof(buf), "%2d", day);

            if (focus == 0 && day == focusDay)
            {
                printAt(row, col, buf, focusAttr);
            }
            else
            {
                printAt(row, col, buf, dayAttr);
            }
        }

        // OK and Cancel buttons
        int btnY = dlgY + dlgH - 3;
        int okX = dlgX + dlgW - 20;
        int cancelX = dlgX + dlgW - 11;
        printAt(btnY, okX, " OK ", (focus == 4) ? btnFocusAttr : btnAttr);
        printAt(btnY, cancelX, " Cancel ", (focus == 5) ? btnFocusAttr : btnAttr);

        // Help
        printAt(dlgY + dlgH - 1, dlgX + 2, "Arrows,Tab,Enter,ESC",
                tAttr(TC_CYAN, TC_BLUE, false));

        g_term->refresh();

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_LEFT:
                if (focus == 0)
                {
                    --focusDay;
                    if (focusDay < 1)
                    {
                        --dispMonth;
                        if (dispMonth < 1) { dispMonth = 12; --dispYear; }
                        focusDay = daysInMonth(dispMonth, dispYear);
                    }
                }
                break;
            case TK_RIGHT:
                if (focus == 0)
                {
                    ++focusDay;
                    if (focusDay > maxDay)
                    {
                        ++dispMonth;
                        if (dispMonth > 12) { dispMonth = 1; ++dispYear; }
                        focusDay = 1;
                    }
                }
                break;
            case TK_UP:
                if (focus == 0)
                {
                    focusDay -= 7;
                    if (focusDay < 1)
                    {
                        --dispMonth;
                        if (dispMonth < 1) { dispMonth = 12; --dispYear; }
                        int prevMax = daysInMonth(dispMonth, dispYear);
                        focusDay += prevMax;
                        if (focusDay < 1) focusDay = 1;
                    }
                }
                break;
            case TK_DOWN:
                if (focus == 0)
                {
                    focusDay += 7;
                    if (focusDay > maxDay)
                    {
                        int overflow = focusDay - maxDay;
                        ++dispMonth;
                        if (dispMonth > 12) { dispMonth = 1; ++dispYear; }
                        focusDay = overflow;
                        int newMax = daysInMonth(dispMonth, dispYear);
                        if (focusDay > newMax) focusDay = newMax;
                    }
                }
                break;
            case '<':
            case ',':
                --dispMonth;
                if (dispMonth < 1) { dispMonth = 12; --dispYear; }
                break;
            case '>':
            case '.':
                ++dispMonth;
                if (dispMonth > 12) { dispMonth = 1; ++dispYear; }
                break;
            case TK_TAB:
                focus = (focus + 1) % 6;
                // When entering year field (focus==3), open the year editor
                if (focus == 3)
                {
                    string yrInput = getStringInput(navY, dlgX + 28, 4,
                                                     std::to_string(dispYear),
                                                     btnFocusAttr);
                    if (!yrInput.empty())
                    {
                        // Strip non-digits
                        string digits;
                        for (char c : yrInput)
                        {
                            if (c >= '0' && c <= '9') digits += c;
                        }
                        if (!digits.empty())
                        {
                            int yr = 0;
                            try { yr = std::stoi(digits); } catch (...) {}
                            if (yr >= 1900 && yr <= 2099)
                            {
                                dispYear = yr;
                            }
                        }
                    }
                    // After editing year, return focus to calendar
                    focus = 0;
                }
                break;
            case TK_ENTER:
                switch (focus)
                {
                    case 0: // Calendar: select date
                    case 4: // OK button
                        result = SimpleDate(dispYear, dispMonth, focusDay);
                        return true;
                    case 1: // Prev month
                        --dispMonth;
                        if (dispMonth < 1) { dispMonth = 12; --dispYear; }
                        break;
                    case 2: // Next month
                        ++dispMonth;
                        if (dispMonth > 12) { dispMonth = 1; ++dispYear; }
                        break;
                    case 3: // Year — open editor
                    {
                        string yrInput = getStringInput(navY, dlgX + 28, 4,
                                                         std::to_string(dispYear),
                                                         btnFocusAttr);
                        if (!yrInput.empty())
                        {
                            string digits;
                            for (char c : yrInput)
                            {
                                if (c >= '0' && c <= '9') digits += c;
                            }
                            if (!digits.empty())
                            {
                                int yr = 0;
                                try { yr = std::stoi(digits); } catch (...) {}
                                if (yr >= 1900 && yr <= 2099)
                                {
                                    dispYear = yr;
                                }
                            }
                        }
                        focus = 0;
                        break;
                    }
                    case 5: // Cancel button
                        return false;
                }
                break;
            case TK_ESCAPE:
                return false;
            default:
                break;
        }
    }
}

// ============================================================
// Case-insensitive search helpers
// ============================================================

static string toLowerStr(const string& s)
{
    string result = s;
    for (auto& c : result)
    {
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }
    return result;
}

static bool matchesText(const string& haystack, const string& needle, bool useRegex)
{
    if (needle.empty()) return true;

    if (useRegex)
    {
        try
        {
            std::regex re(needle, std::regex::icase | std::regex::ECMAScript);
            return std::regex_search(haystack, re);
        }
        catch (const std::regex_error&)
        {
            // Invalid regex, fall back to substring
            return toLowerStr(haystack).find(toLowerStr(needle)) != string::npos;
        }
        catch (...)
        {
            // Unexpected error, fall back to substring
            return toLowerStr(haystack).find(toLowerStr(needle)) != string::npos;
        }
    }
    else
    {
        return toLowerStr(haystack).find(toLowerStr(needle)) != string::npos;
    }
}

// ============================================================
// Conference search prompt
// ============================================================

string showConfSearchPrompt(bool& clearSearch)
{
    clearSearch = false;
    int y = g_term->getRows() - 1;
    int cols = g_term->getCols();

    fillRow(y, tAttr(TC_BLACK, TC_BLACK, false));
    printAt(y, 0, "Search: ", tAttr(TC_CYAN, TC_BLACK, true));

    // Use getStringInput — Ctrl-C inside it returns empty string via TK_CTRL_C
    string text = getStringInput(y, 8, cols - 10, "", tAttr(TC_WHITE, TC_BLACK, true));

    // Check if Ctrl-C was pressed (getStringInput returns "" on cancel keys)
    // We distinguish "Ctrl-C to clear" from "ESC to cancel" by checking
    // if the user explicitly wanted to clear
    if (text.empty())
    {
        // Ask: was this a Ctrl-C clear? getStringInput returns "" for both ESC and Ctrl-C.
        // We'll set clearSearch=true if the prompt was dismissed, so the caller
        // always clears on empty return. The user presses "/" then just hits ESC/Ctrl-C.
        clearSearch = true;
        return "";
    }
    return text;
}

// ============================================================
// Message search options dialog
// ============================================================

bool showMsgSearchDialog(MsgSearchParams& params, const Settings& settings,
                         bool& clearSearch)
{
    clearSearch = false;
    int cols = g_term->getCols();
    int rows = g_term->getRows();

    int dlgW = 52;
    if (dlgW > cols - 4) dlgW = cols - 4;
    int dlgH = 14;
    int dlgX = (cols - dlgW) / 2;
    int dlgY = (rows - dlgH) / 2;

    TermAttr borderAttr = tAttr(TC_CYAN, TC_BLACK, true);
    TermAttr labelAttr = tAttr(TC_WHITE, TC_BLACK, true);
    TermAttr selAttr = tAttr(TC_WHITE, TC_BLUE, true);
    TermAttr normalAttr = tAttr(TC_CYAN, TC_BLACK, false);
    TermAttr helpAttr = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr checkAttr = tAttr(TC_GREEN, TC_BLACK, true);

    static const char* fieldNames[] = {
        "Subject only",
        "Body only",
        "Subject and Body",
        "From name",
        "To name"
    };
    int fieldSel = static_cast<int>(params.field);
    int selected = 0; // 0-4=field, 5=search text prompt

    while (true)
    {
        // Draw dialog
        for (int r = 0; r < dlgH; ++r)
        {
            fillRow(dlgY + r, tAttr(TC_BLACK, TC_BLACK, false), dlgX, dlgX + dlgW);
        }
        drawBox(dlgY, dlgX, dlgH, dlgW, borderAttr, "Search Messages", borderAttr);

        printAt(dlgY + 1, dlgX + 2, "Search in:", labelAttr);

        for (int i = 0; i < 5; ++i)
        {
            int y = dlgY + 2 + i;
            bool isSel = (selected == i);
            bool isChecked = (fieldSel == i);

            TermAttr lbl = isSel ? selAttr : normalAttr;
            if (isSel)
            {
                fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
            }

            string checkMark = isChecked ? "(*) " : "( ) ";
            printAt(y, dlgX + 3, checkMark + fieldNames[i], lbl);
        }

        // Search text field
        {
            int y = dlgY + 8;
            bool isSel = (selected == 5);
            TermAttr lbl = isSel ? selAttr : normalAttr;
            printAt(y, dlgX + 2, "Text: ", labelAttr);
            string displayText = params.searchText;
            if (displayText.empty()) displayText = "(enter search text)";
            printAt(y, dlgX + 8, displayText, lbl);
        }

        // Help
        printAt(dlgY + 10, dlgX + 2, "Up/Dn=Select, Space=Toggle field", helpAttr);
        printAt(dlgY + 11, dlgX + 2, "Enter=Search, Ctrl-A=Advanced, ESC/Ctrl-C=Cancel", helpAttr);

        g_term->refresh();

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:
                if (selected > 0) --selected;
                break;
            case TK_DOWN:
                if (selected < 5) ++selected;
                break;
            case ' ':
                if (selected >= 0 && selected <= 4)
                {
                    fieldSel = selected;
                }
                break;
            case TK_ENTER:
            {
                if (selected <= 4)
                {
                    fieldSel = selected;
                }
                // Prompt for search text
                int textY = dlgY + 8;
                printAt(textY, dlgX + 2, "Text: ", labelAttr);
                string text = getStringInput(textY, dlgX + 8, dlgW - 12,
                                              params.searchText,
                                              tAttr(TC_WHITE, TC_BLACK, true));
                if (text.empty())
                {
                    clearSearch = true;
                    return false;
                }
                params.searchText = text;
                params.field = static_cast<MsgSearchField>(fieldSel);
                params.useRegex = settings.useRegexSearch;
                return true;
            }
            case TK_CTRL_A:
            {
                // Advanced search dialog
                // Draw over the current dialog
                int advW = dlgW + 4;
                if (advW > cols - 2) advW = cols - 2;
                int advH = 18;
                int advX = (cols - advW) / 2;
                int advY = (rows - advH) / 2;

                string advText = params.searchText;
                SimpleDate advStartDate = params.startDate;
                SimpleDate advEndDate = params.endDate;
                int advFieldSel = fieldSel;
                int advSelected = 0; // 0=field, 1=text, 2=startDate, 3=endDate, 4=search

                bool advDone = false;
                bool advResult = false;

                while (!advDone)
                {
                    for (int r = 0; r < advH; ++r)
                    {
                        fillRow(advY + r, tAttr(TC_BLACK, TC_BLACK, false), advX, advX + advW);
                    }
                    drawBox(advY, advX, advH, advW, borderAttr, "Advanced Search", borderAttr);

                    // Field selector
                    printAt(advY + 1, advX + 2, "Search in:", labelAttr);
                    for (int i = 0; i < 5; ++i)
                    {
                        bool isChecked = (advFieldSel == i);
                        bool isSel = (advSelected == 0);
                        string radio = isChecked ? "(*)" : "( )";
                        TermAttr lbl = (isSel && advFieldSel == i) ? selAttr : normalAttr;
                        int fx = advX + 3 + i * 10;
                        if (fx + 8 > advX + advW - 2) break;
                        // Short labels for compact display
                        static const char* shortNames[] = {"Subj", "Body", "Both", "From", "To"};
                        printAt(advY + 2, fx, radio, isChecked ? checkAttr : normalAttr);
                        printAt(advY + 2, fx + 4, shortNames[i], lbl);
                    }

                    // Text field
                    {
                        int y = advY + 4;
                        bool isSel = (advSelected == 1);
                        printAt(y, advX + 2, "Search text:", labelAttr);
                        string dt = advText.empty() ? "(enter text)" : advText;
                        printAt(y, advX + 15, dt, isSel ? selAttr : normalAttr);
                    }

                    // Start date
                    {
                        int y = advY + 6;
                        bool isSel = (advSelected == 2);
                        printAt(y, advX + 2, "Start date:", labelAttr);
                        string ds = advStartDate.isEmpty() ? "(none - no limit)" : advStartDate.toIsoStr();
                        printAt(y, advX + 15, ds, isSel ? selAttr : normalAttr);
                        if (isSel)
                        {
                            printAt(y, advX + 36, "[Enter=Pick]", helpAttr);
                        }
                    }

                    // End date
                    {
                        int y = advY + 8;
                        bool isSel = (advSelected == 3);
                        printAt(y, advX + 2, "End date:  ", labelAttr);
                        string ds = advEndDate.isEmpty() ? "(none - no limit)" : advEndDate.toIsoStr();
                        printAt(y, advX + 15, ds, isSel ? selAttr : normalAttr);
                        if (isSel)
                        {
                            printAt(y, advX + 36, "[Enter=Pick]", helpAttr);
                        }
                    }

                    // Search button
                    {
                        int y = advY + 10;
                        bool isSel = (advSelected == 4);
                        printAt(y, advX + (advW - 10) / 2, "[ Search ]",
                                isSel ? selAttr : labelAttr);
                    }

                    // Help
                    printAt(advY + 12, advX + 2, "Up/Dn=Move, Enter=Edit/Pick, Tab=Field", helpAttr);
                    printAt(advY + 13, advX + 2, "Space=Toggle field, Del=Clear date", helpAttr);
                    printAt(advY + 14, advX + 2, "ESC/Ctrl-C=Cancel", helpAttr);

                    g_term->refresh();

                    int ach = g_term->getKey();
                    switch (ach)
                    {
                        case TK_UP:
                            if (advSelected > 0) --advSelected;
                            break;
                        case TK_DOWN:
                            if (advSelected < 4) ++advSelected;
                            break;
                        case TK_TAB:
                            advSelected = (advSelected + 1) % 5;
                            break;
                        case ' ':
                            if (advSelected == 0)
                            {
                                advFieldSel = (advFieldSel + 1) % 5;
                            }
                            break;
                        case TK_DELETE:
                            if (advSelected == 2) advStartDate = SimpleDate();
                            if (advSelected == 3) advEndDate = SimpleDate();
                            break;
                        case TK_ENTER:
                        {
                            if (advSelected == 0)
                            {
                                advFieldSel = (advFieldSel + 1) % 5;
                            }
                            else if (advSelected == 1)
                            {
                                // Edit search text
                                int y = advY + 4;
                                string t = getStringInput(y, advX + 15, advW - 18,
                                                           advText,
                                                           tAttr(TC_WHITE, TC_BLACK, true));
                                if (!t.empty()) advText = t;
                            }
                            else if (advSelected == 2)
                            {
                                // Date picker for start date
                                SimpleDate picked;
                                if (showDatePicker(picked, advStartDate))
                                {
                                    advStartDate = picked;
                                }
                            }
                            else if (advSelected == 3)
                            {
                                // Date picker for end date
                                SimpleDate picked;
                                if (showDatePicker(picked, advEndDate))
                                {
                                    advEndDate = picked;
                                }
                            }
                            else if (advSelected == 4)
                            {
                                // Execute search
                                if (advText.empty())
                                {
                                    messageDialog("Search", "Please enter search text.");
                                }
                                else
                                {
                                    params.searchText = advText;
                                    params.field = static_cast<MsgSearchField>(advFieldSel);
                                    params.startDate = advStartDate;
                                    params.endDate = advEndDate;
                                    params.useRegex = settings.useRegexSearch;
                                    advResult = true;
                                    advDone = true;
                                }
                            }
                            break;
                        }
                        case TK_ESCAPE:
                        case TK_CTRL_C:
                            advDone = true;
                            break;
                        default:
                            break;
                    }
                }

                if (advResult) return true;
                // Cancelled advanced search, redraw basic dialog
                break;
            }
            case TK_ESCAPE:
                return false;
            case TK_CTRL_C:
                clearSearch = true;
                return false;
            default:
                break;
        }
    }
}

// ============================================================
// Conference filtering
// ============================================================

vector<int> filterConferences(const vector<QwkConference>& conferences,
                              const string& searchText,
                              bool useRegex)
{
    vector<int> results;
    for (int i = 0; i < static_cast<int>(conferences.size()); ++i)
    {
        if (matchesText(conferences[i].name, searchText, useRegex))
        {
            results.push_back(i);
        }
    }
    return results;
}

// ============================================================
// Message filtering
// ============================================================

static bool dateInRange(const QwkMessage& msg, const SimpleDate& startDate,
                        const SimpleDate& endDate)
{
    if (startDate.isEmpty() && endDate.isEmpty()) return true;

    SimpleDate msgDate = SimpleDate::fromQwkStr(msg.date);
    if (!msgDate.isValid())
    {
        // Can't parse the message date. If a date range is specified,
        // exclude messages with unparseable dates since we can't verify
        // they fall within the range.
        return false;
    }

    if (!startDate.isEmpty() && msgDate.compareTo(startDate) < 0)
    {
        return false; // Message is before the start date
    }
    if (!endDate.isEmpty() && msgDate.compareTo(endDate) > 0)
    {
        return false; // Message is after the end date
    }
    return true;
}

vector<int> filterMessages(const vector<QwkMessage>& messages,
                           const MsgSearchParams& params)
{
    vector<int> results;
    for (int i = 0; i < static_cast<int>(messages.size()); ++i)
    {
        const auto& msg = messages[i];

        // Check date range first
        if (!dateInRange(msg, params.startDate, params.endDate))
        {
            continue;
        }

        // Check text match
        bool found = false;
        switch (params.field)
        {
            case MsgSearchField::Subject:
                found = matchesText(msg.subject, params.searchText, params.useRegex);
                break;
            case MsgSearchField::Body:
                found = matchesText(msg.body, params.searchText, params.useRegex);
                break;
            case MsgSearchField::SubjectAndBody:
                found = matchesText(msg.subject, params.searchText, params.useRegex) ||
                        matchesText(msg.body, params.searchText, params.useRegex);
                break;
            case MsgSearchField::From:
                found = matchesText(msg.from, params.searchText, params.useRegex);
                break;
            case MsgSearchField::To:
                found = matchesText(msg.to, params.searchText, params.useRegex);
                break;
        }

        if (found)
        {
            results.push_back(i);
        }
    }
    return results;
}
