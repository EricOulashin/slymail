#ifndef SLYMAIL_VOTING_H
#define SLYMAIL_VOTING_H

// VOTING.DAT Processing
//
// Parses the VOTING.DAT file from QWK packets (Synchronet QWKE extension).
// VOTING.DAT contains poll definitions and vote records in INI format.
//
// Format (from Synchronet source):
//   [hex_offset]
//   [poll:<msgid>]        - Poll definition
//   [vote:<msgid>]        - Vote/ballot response
//   [close:<msgid>]       - Poll closure
//
// Poll sections contain:
//   MaxVotes, Results, Comment0..N, PollAnswer0..N
//   WhenWritten, Sender, Conference
//
// Vote sections contain:
//   Votes (hex bitmask), UpVote/DownVote (bool)
//   WhenWritten, Sender, Conference

#include "settings.h"
#include <string>
#include <vector>

// A single answer option in a poll
struct PollAnswer
{
    std::string text;
    int         votes;      // Number of votes received (tracked locally)

    PollAnswer() : votes(0) {}
    PollAnswer(const std::string& t) : text(t), votes(0) {}
};

// A poll/voting item from VOTING.DAT
struct VotingPoll
{
    std::string msgId;          // Message ID for this poll
    std::string question;       // Poll subject/question
    std::string from;           // Who created the poll
    std::string date;           // When written
    int         conference;     // Conference number
    int         maxVotes;       // Max number of answers a user can select (0 = unlimited)
    std::vector<std::string> comments;   // Comment lines (additional description)
    std::vector<PollAnswer>  answers;    // Poll answer options
    bool        closed;         // Whether the poll has been closed

    VotingPoll() : conference(0), maxVotes(0), closed(false) {}
};

// A vote record from VOTING.DAT
struct VoteRecord
{
    std::string msgId;          // This vote's own message ID
    std::string inReplyTo;      // Message ID of the poll/message being voted on
    std::string voter;          // Who voted
    std::string date;           // When voted
    int         conference;     // Conference number
    uint16_t    votes;          // Bitmask of selected answers
    bool        upVote;         // Up-vote flag
    bool        downVote;       // Down-vote flag

    VoteRecord() : conference(0), votes(0), upVote(false), downVote(false) {}
};

// Container for all voting data from a QWK packet
struct VotingData
{
    std::vector<VotingPoll>  polls;
    std::vector<VoteRecord>  voteRecords;

    bool empty() const { return polls.empty() && voteRecords.empty(); }

    // Tally votes: apply vote records to matching polls
    void tallyVotes();
};

// Parse VOTING.DAT from the extracted packet directory.
// Returns empty VotingData if file not found or empty.
VotingData parseVotingDat(const std::string& extractDir);

// Display a poll in a scrollable dialog.
void showPollDialog(const VotingPoll& poll);

// Show a list of all polls and allow viewing them.
void showVotingList(const VotingData& voting, const Settings& settings);

#endif // SLYMAIL_VOTING_H
