#ifndef SLYMAIL_MSG_READER_H
#define SLYMAIL_MSG_READER_H

#include "terminal.h"
#include "colors.h"
#include "ui_common.h"
#include "qwk.h"
#include "settings.h"

// Result from the message reader
enum class MsgReadResult
{
    Back,
    Quit,
    NextMsg,
    PrevMsg,
    Reply,
    FirstMsg,
    LastMsg,
    Settings,
};

// Determine color for a message body line
TermAttr getLineAttr(const std::string& line, const Settings& settings);

// Filter and prepare message body lines
std::vector<std::string> prepareBodyLines(const QwkMessage& msg,
                                          const Settings& settings,
                                          int displayWidth);

// Draw the DDMsgReader-style enhanced message header with box-drawing border
// Returns the number of rows used
int drawMessageHeader(const QwkMessage& msg, const std::string& confName,
                      const std::string& groupName,
                      int msgIndex, int totalMsgs);

// Draw the DDMsgReader-style help bar at the bottom
void drawReaderHelpBar(int y);

// Build a list of header information lines for display
std::vector<std::string> buildHeaderInfoLines(const QwkMessage& msg,
                                              const std::string& confName);

// Show message header information in a scrollable view
void showHeaderInfo(const QwkMessage& msg, const std::string& confName,
                    int msgIndex, int totalMsgs, const Settings& settings);

// Show a message in the enhanced reader (DDMsgReader-style scrollable)
MsgReadResult showMessageReader(const QwkMessage& msg,
                                const std::string& confName,
                                int msgIndex, int totalMsgs,
                                Settings& settings);

#endif // SLYMAIL_MSG_READER_H
