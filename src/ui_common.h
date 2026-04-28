#ifndef SLYMAIL_UI_COMMON_H
#define SLYMAIL_UI_COMMON_H

// Common UI helper functions that route through g_term (ITerminal).
// All functions use Allman brace style.

#include "terminal.h"
#include "colors.h"
#include "program_info.h"
#include <string>

// ---- Text output helpers ----

void printAt(int y, int x, const std::string& text, const TermAttr& attr);

void printCentered(int y, const std::string& text, const TermAttr& attr);

// Draw the standard program info header line centered on a given row.
// Format: "PROGRAM_NAME version PROGRAM_VERSION (PROGRAM_DATE)"
// with specific colors for each part.
void drawProgramInfoLine(int y);

void fillRow(int y, const TermAttr& attr, int startX = 0, int endX = -1);

// Fill a dialog's complete rectangular area (border + interior) with a solid
// background so that no previous terminal content shows through.  Call this
// at the top of any dialog's full-redraw path before drawing the box/content,
// and also before returning from a sub-dialog so it erases its own footprint.
void clearDialogArea(int y, int x, int h, int w);

void fillBullets(int y, int x, int len, const TermAttr& attr);

// ---- Box-drawing helpers ----

void drawHLine(int y, int x, int len, const TermAttr& attr,
               const std::string& label = "", const TermAttr& labelAttr = TermAttr());

void drawBox(int y, int x, int h, int w, const TermAttr& attr,
             const std::string& title = "", const TermAttr& titleAttr = TermAttr());

// ---- DDMsgReader-style help bar ----
// Green text for navigation, red hotkey letters: "F)irst, L)ast, R)eply"

void drawDDHelpBar(int y, const std::string& prefix,
                   const std::vector<std::pair<char, std::string>>& items);

// ---- Scrollbar ----

void drawScrollbar(int topY, int height, int position, int total,
                   const TermAttr& bgAttr, const TermAttr& blockAttr);

// ---- String helpers ----

std::string truncateStr(const std::string& s, int maxLen);

std::string padStr(const std::string& s, int width);

std::string rightAlign(const std::string& s, int width);

// ---- Input helpers ----

std::string getStringInput(int y, int x, int maxLen,
                           const std::string& initial,
                           const TermAttr& attr);

// Like getStringInput, but only allows digit characters (0-9).
// Returns the entered string, or empty string if cancelled.
std::string getNumericInput(int y, int x, int maxLen,
                            const TermAttr& attr);

// Like getStringInput, but displays '*' characters instead of the actual text.
// Returns the actual (unmasked) text, or empty string if cancelled.
std::string getPasswordInput(int y, int x, int maxLen,
                             const std::string& initial,
                             const TermAttr& attr);

// ---- Dialogs ----

bool confirmDialog(const std::string& prompt);

void messageDialog(const std::string& title, const std::string& msg);

// ---- Word wrap ----

std::vector<std::string> wordWrap(const std::string& text, int maxWidth);

// trimStr() and extractField() are defined in qwk.h

#endif // SLYMAIL_UI_COMMON_H
