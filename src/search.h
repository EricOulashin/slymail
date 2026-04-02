#ifndef SLYMAIL_SEARCH_H
#define SLYMAIL_SEARCH_H

// Search functionality for SlyMail
//
// Provides conference search, message search (with advanced options
// including date range filtering), and a visual date picker dialog.

#include "qwk.h"
#include "settings.h"
#include <string>
#include <vector>
#include <regex>

// ============================================================
// Date Picker
// ============================================================

// A simple date struct (year, month 1-12, day 1-31)
struct SimpleDate
{
    int year;
    int month;  // 1-12
    int day;    // 1-31

    SimpleDate() : year(0), month(0), day(0) {}
    SimpleDate(int y, int m, int d) : year(y), month(m), day(d) {}

    bool isValid() const { return year > 0 && month >= 1 && month <= 12 && day >= 1; }
    bool isEmpty() const { return year == 0 && month == 0 && day == 0; }

    // Format as "MM-DD-YY" (QWK style) or "YYYY-MM-DD" (ISO)
    std::string toQwkStr() const;
    std::string toIsoStr() const;
    std::string toDisplayStr() const; // "Mon, Month DD, YYYY"

    // Compare: negative if this < other, 0 if equal, positive if this > other
    int compareTo(const SimpleDate& other) const;

    // Parse from QWK date string "MM-DD-YY"
    static SimpleDate fromQwkStr(const std::string& s);
};

// Get the current date
SimpleDate currentDate();

// Get the number of days in a given month/year
int daysInMonth(int month, int year);

// Get the day of the week for a date (0=Sunday, 1=Monday, ... 6=Saturday)
int dayOfWeek(int year, int month, int day);

// Show a visual date picker dialog (calendar-style).
// Returns true if the user selected a date (stored in result).
// Returns false if cancelled.
bool showDatePicker(SimpleDate& result, const SimpleDate& initial = SimpleDate());

// ============================================================
// Message Search
// ============================================================

// What fields to search in messages
enum class MsgSearchField
{
    Subject,        // Search message subjects only
    Body,           // Search message bodies only
    SubjectAndBody, // Search both subjects and bodies
    From,           // Search the "From" field
    To,             // Search the "To" field
};

// Search parameters
struct MsgSearchParams
{
    std::string   searchText;
    MsgSearchField field;
    SimpleDate    startDate;    // Empty = no start date limit
    SimpleDate    endDate;      // Empty = no end date limit
    bool          useRegex;     // Use regex matching vs substring

    MsgSearchParams() : field(MsgSearchField::SubjectAndBody), useRegex(false) {}
};

// Show the message search options dialog.
// Returns true if the user wants to search, false if cancelled (or Ctrl-C to clear).
// If clearSearch is set to true, the caller should clear the current filter.
bool showMsgSearchDialog(MsgSearchParams& params, const Settings& settings,
                         bool& clearSearch);

// Show the conference search prompt.
// Returns the search text, or empty string if cancelled.
// If clearSearch is set to true, the caller should clear the current filter.
std::string showConfSearchPrompt(bool& clearSearch);

// Filter conferences by name (case-insensitive substring or regex match).
// Returns indices into the original conferences vector.
std::vector<int> filterConferences(const std::vector<QwkConference>& conferences,
                                   const std::string& searchText,
                                   bool useRegex);

// Filter messages by the given search parameters.
// Returns indices into the original messages vector.
std::vector<int> filterMessages(const std::vector<QwkMessage>& messages,
                                const MsgSearchParams& params);

#endif // SLYMAIL_SEARCH_H
