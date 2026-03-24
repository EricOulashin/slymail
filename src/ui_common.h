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

// ---- Dialogs ----

bool confirmDialog(const std::string& prompt);

void messageDialog(const std::string& title, const std::string& msg);

// ---- Word wrap ----

std::vector<std::string> wordWrap(const std::string& text, int maxWidth);

// trimStr() and extractField() are defined in qwk.h

#endif // SLYMAIL_UI_COMMON_H
