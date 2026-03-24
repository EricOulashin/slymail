#ifndef SLYMAIL_COLORS_H
#define SLYMAIL_COLORS_H

// Color scheme definitions for SlyMail UI elements.
// Uses TermAttr from terminal.h for platform-independent color specs.

#include "terminal.h"

// Predefined color schemes as inline functions returning TermAttr

// --- DDMsgReader-style message list colors ---
namespace MsgListColors
{
    inline TermAttr header()       { return tAttr(TC_CYAN, TC_BLACK, true); }
    inline TermAttr headerBg()     { return tAttr(TC_WHITE, TC_BLUE, true); }
    inline TermAttr msgNum()       { return tAttr(TC_YELLOW, TC_BLACK, true); }
    inline TermAttr from()         { return tAttr(TC_CYAN, TC_BLACK, false); }
    inline TermAttr to()           { return tAttr(TC_CYAN, TC_BLACK, false); }
    inline TermAttr subject()      { return tAttr(TC_CYAN, TC_BLACK, true); }
    inline TermAttr date()         { return tAttr(TC_GREEN, TC_BLACK, false); }
    inline TermAttr selected()     { return tAttr(TC_WHITE, TC_BLUE, true); }
    inline TermAttr selMsgNum()    { return tAttr(TC_YELLOW, TC_BLUE, true); }
    inline TermAttr selFrom()      { return tAttr(TC_CYAN, TC_BLUE, true); }
    inline TermAttr selTo()        { return tAttr(TC_CYAN, TC_BLUE, true); }
    inline TermAttr selSubject()   { return tAttr(TC_WHITE, TC_BLUE, true); }
    inline TermAttr selDate()      { return tAttr(TC_GREEN, TC_BLUE, true); }
    inline TermAttr normal()       { return tAttr(TC_WHITE, TC_BLACK, false); }
}

// --- Message reader colors ---
namespace ReaderColors
{
    inline TermAttr headerLabel()  { return tAttr(TC_CYAN, TC_BLACK, false); }
    inline TermAttr headerValue()  { return tAttr(TC_WHITE, TC_BLACK, true); }
    inline TermAttr headerBorder() { return tAttr(TC_BLUE, TC_BLACK, true); }
    inline TermAttr msgBody()      { return tAttr(TC_WHITE, TC_BLACK, false); }
    inline TermAttr quoteLine()    { return tAttr(TC_CYAN, TC_BLACK, false); }
    inline TermAttr quote2Line()   { return tAttr(TC_GREEN, TC_BLACK, false); }
    inline TermAttr tearLine()     { return tAttr(TC_YELLOW, TC_BLACK, false); }
    inline TermAttr originLine()   { return tAttr(TC_GREEN, TC_BLACK, false); }
    inline TermAttr kludgeLine()   { return tAttr(TC_MAGENTA, TC_BLACK, false); }
}

// --- ICE editor colors ---
namespace IceColors
{
    inline TermAttr borderBright() { return tAttr(TC_BLUE, TC_BLACK, true); }
    inline TermAttr borderNorm()   { return tAttr(TC_BLUE, TC_BLACK, false); }
    inline TermAttr topLabel()     { return tAttr(TC_CYAN, TC_BLACK, true); }
    inline TermAttr topColon()     { return tAttr(TC_BLUE, TC_BLACK, true); }
    inline TermAttr topValue()     { return tAttr(TC_WHITE, TC_BLACK, true); }
    inline TermAttr topTime()      { return tAttr(TC_GREEN, TC_BLACK, true); }
    inline TermAttr editMode()     { return tAttr(TC_CYAN, TC_BLACK, true); }
    inline TermAttr editText()     { return tAttr(TC_WHITE, TC_BLACK, false); }
    inline TermAttr quoteText()    { return tAttr(TC_CYAN, TC_BLACK, false); }
    inline TermAttr quoteHl()      { return tAttr(TC_CYAN, TC_BLUE, true); }
    inline TermAttr quoteBorder()  { return tAttr(TC_CYAN, TC_BLACK, true); }
    inline TermAttr statusBar()    { return tAttr(TC_CYAN, TC_BLACK, true); }
    inline TermAttr statusText()   { return tAttr(TC_WHITE, TC_BLACK, true); }
    inline TermAttr keyLabel()     { return tAttr(TC_CYAN, TC_BLACK, true); }
    inline TermAttr keyLetter()    { return tAttr(TC_WHITE, TC_BLACK, false); }
}

// --- DCT editor colors ---
namespace DctColors
{
    inline TermAttr topBorderN()   { return tAttr(TC_RED, TC_BLACK, false); }
    inline TermAttr topBorderB()   { return tAttr(TC_RED, TC_BLACK, true); }
    inline TermAttr editBorderN()  { return tAttr(TC_GREEN, TC_BLACK, false); }
    inline TermAttr editBorderB()  { return tAttr(TC_GREEN, TC_BLACK, true); }
    inline TermAttr topLabel()     { return tAttr(TC_BLUE, TC_BLACK, true); }
    inline TermAttr topFrom()      { return tAttr(TC_CYAN, TC_BLACK, true); }
    inline TermAttr topFromFill()  { return tAttr(TC_CYAN, TC_BLACK, false); }
    inline TermAttr topTo()        { return tAttr(TC_CYAN, TC_BLACK, true); }
    inline TermAttr topToFill()    { return tAttr(TC_CYAN, TC_BLACK, false); }
    inline TermAttr topSubj()      { return tAttr(TC_WHITE, TC_BLACK, true); }
    inline TermAttr topSubjFill()  { return tAttr(TC_WHITE, TC_BLACK, false); }
    inline TermAttr topArea()      { return tAttr(TC_GREEN, TC_BLACK, true); }
    inline TermAttr topAreaFill()  { return tAttr(TC_GREEN, TC_BLACK, false); }
    inline TermAttr topTime()      { return tAttr(TC_YELLOW, TC_BLACK, true); }
    inline TermAttr bracket()      { return tAttr(TC_MAGENTA, TC_BLACK, false); }
    inline TermAttr editModeB()    { return tAttr(TC_BLACK, TC_BLACK, true); }
    inline TermAttr editModeT()    { return tAttr(TC_WHITE, TC_BLACK, false); }
    inline TermAttr editText()     { return tAttr(TC_WHITE, TC_BLACK, false); }
    inline TermAttr bottomBrk()    { return tAttr(TC_BLACK, TC_BLACK, true); }
    inline TermAttr bottomKey()    { return tAttr(TC_RED, TC_BLACK, true); }
    inline TermAttr bottomDesc()   { return tAttr(TC_CYAN, TC_BLACK, false); }
    inline TermAttr quoteText()    { return tAttr(TC_WHITE, TC_BLACK, false); }
    inline TermAttr quoteHl()      { return tAttr(TC_RED, TC_BLUE, true); }
    inline TermAttr quoteBorderC() { return tAttr(TC_BLACK, TC_WHITE, true); }
    inline TermAttr quoteTitleC()  { return tAttr(TC_RED, TC_WHITE, false); }
}

#endif // SLYMAIL_COLORS_H
