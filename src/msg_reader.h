#ifndef SLYMAIL_MSG_READER_H
#define SLYMAIL_MSG_READER_H

#include "terminal.h"
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
    Vote,       // User cast a vote (check lastVote for details)
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
// extractDir is the packet extract directory (for attachment downloads)
// votingData is the packet's voting data (for poll display)
// lastVote is set if the user casts a vote (caller should queue it)
MsgReadResult showMessageReader(const QwkMessage& msg,
                                const std::string& confName,
                                int msgIndex, int totalMsgs,
                                Settings& settings,
                                const std::string& extractDir = "",
                                const VotingData* votingData = nullptr,
                                PendingVote* lastVote = nullptr);

// Save file attachments from a message to a destination directory
void downloadAttachments(const QwkMessage& msg, const std::string& extractDir);

// Show the voting UI for a message. Returns true if user cast a vote.
// For polls: shows answer list and lets user toggle selections.
// For regular messages: prompts for Up, Down, or Quit.
bool showVoteUI(const QwkMessage& msg, const VotingData* votingData,
                const std::string& userName, PendingVote& voteOut);

#endif // SLYMAIL_MSG_READER_H
