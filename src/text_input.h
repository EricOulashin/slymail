#ifndef SLYMAIL_TEXT_INPUT_H
#define SLYMAIL_TEXT_INPUT_H

// TextBuffer — Standalone text editing engine for the SlyMail message editor.
//
// Provides all text editing operations (insert, delete, cursor movement,
// word wrap, line splitting/merging) with full ANSI color code awareness.
// No UI or terminal dependencies — purely algorithmic, making it testable
// without ncurses or any platform-specific code.
//
// Used by MessageEditor (msg_editor.cpp) for the actual editing logic,
// and by test_text_input.cpp for automated unit testing.

#include <string>
#include <vector>

// A single line of text in the editor
struct TextLine
{
    std::string text;
    bool hardBreak;    // true = user pressed Enter; false = created by word wrap
    bool isQuoteLine;  // true = line was inserted from the quote window

    TextLine() : hardBreak(false), isQuoteLine(false) {}
    TextLine(const std::string& t, bool hb = false) : text(t), hardBreak(hb), isQuoteLine(false) {}
};

// --- ANSI escape sequence helpers ---

// Return the byte length of an ANSI escape sequence starting at pos,
// or 0 if no valid sequence starts there.
int ansiSeqLen(const std::string& text, int pos);

// Calculate the display column (visible character count) for a byte offset,
// skipping over ANSI sequences which have zero display width.
int byteColToDisplayCol(const std::string& text, int byteCol);

// Move forward past one visible character, skipping all adjacent ANSI sequences.
int skipForward(const std::string& text, int pos);

// Move backward past one visible character, skipping all adjacent ANSI sequences.
int skipBackward(const std::string& text, int pos);

// Erase backward: removes one visible character plus adjacent ANSI sequences.
// Returns the new cursor position.
int eraseBackward(std::string& text, int pos);

// Erase forward: removes one visible character plus adjacent ANSI sequences.
void eraseForward(std::string& text, int pos);

// Pull words from the next line up to the current line to fill available space.
// Inverse of word wrap — used after deleting text.
void pullUpWords(std::vector<TextLine>& lines, int row, int editWidth);

// --- TextBuffer: line-based text editor engine ---

class TextBuffer
{
public:
    std::vector<TextLine> lines;  // All lines of text
    int cursorRow;                // Current line (0-based)
    int cursorCol;                // Byte offset within current line
    int editWidth;                // Max visible columns before word wrap
    bool insertMode;              // true=insert, false=overwrite

    TextBuffer();

    // Initialize the buffer with a given edit width.
    // If startWithReset is true, inserts an ANSI reset code at the beginning.
    void init(int width, bool startWithReset = true);

    // --- Editing operations ---

    void inputChar(char ch);       // Type a visible character at cursor
    void doBackspace();            // Delete backward (with word pull-up)
    void doDelete();               // Delete forward (with word pull-up)
    void doEnter();                // Split line at cursor (ANSI-safe)
    void deleteLine();             // Remove current line (Ctrl-D)
    void insertRaw(const std::string& s); // Insert raw bytes (e.g., ANSI codes)

    // --- Cursor movement ---

    void cursorLeft();
    void cursorRight();
    void cursorUp();
    void cursorDown();
    void cursorHome();
    void cursorEnd();

    // --- Accessors ---

    std::string getBody() const;          // All lines joined with newlines
    std::string getLineText(int row) const; // Text of a specific line
    int lineCount() const;                // Number of lines
    int displayCol() const;               // Display column at cursor position

private:
    void wrapLine();       // Word-wrap current line if it exceeds editWidth
    void pullUp();         // Pull words from next line onto current line
    void fixCursorAnsi();  // Skip cursor past any ANSI sequence it's on
    void clampCursor();    // Ensure cursor is within valid bounds
};

#endif // SLYMAIL_TEXT_INPUT_H
