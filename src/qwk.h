#ifndef SLYMAIL_QWK_H
#define SLYMAIL_QWK_H

#include "terminal.h"
#include <iostream>
#include <optional>

// QWK format constants
constexpr int QWK_BLOCK_LEN = 128;
constexpr uint8_t QWK_NEWLINE = 0xE3;  // Line terminator in QWK messages

// Message status flags
enum class QwkStatus : char
{
    NewPublic     = ' ',
    OldPublic     = '-',
    NewPrivate    = '+',
    OldPrivate    = '*',
    Comment       = '~',
    Vote          = 'V'
};

// A single QWK message
struct QwkMessage
{
    int         number;         // Message number
    std::string from;           // From name (25 chars max)
    std::string to;             // To name (25 chars max)
    std::string subject;        // Subject (25 chars max)
    std::string date;           // Date string "MM-DD-YY"
    std::string time;           // Time string "HH:MM"
    std::string body;           // Message body text
    int         replyTo;        // Reply-to message number (0 = none)
    int         conference;     // Conference number
    QwkStatus   status;         // Message status
    bool        isPrivate;      // Is this a private message?

    QwkMessage()
        : number(0), replyTo(0), conference(0),
          status(QwkStatus::NewPublic), isPrivate(false)
    {
    }
};

// A QWK conference (message area)
struct QwkConference
{
    int         number;         // Conference number
    std::string name;           // Conference name
    std::vector<QwkMessage> messages;   // Messages in this conference

    QwkConference() : number(0)
    {
    }

    QwkConference(int num, const std::string& n) : number(num), name(n)
    {
    }
};

// QWK packet information
struct QwkPacketInfo
{
    std::string bbsName;        // BBS system name
    std::string bbsLocation;    // BBS city/state
    std::string bbsPhone;       // BBS phone number
    std::string sysopName;      // Sysop name
    std::string bbsID;          // BBS QWK ID
    std::string packetDate;     // Packet creation date
    std::string userName;       // User's name/alias
    int         userNumber;     // User's number on the BBS
};

// A complete QWK packet
struct QwkPacket
{
    QwkPacketInfo info;
    std::vector<QwkConference> conferences;
    std::string extractDir;     // Where the packet was extracted
    std::string sourceFile;     // Original .qwk file path

    // Get total message count
    int totalMessages() const
    {
        int count = 0;
        for (const auto& conf : conferences)
        {
            count += static_cast<int>(conf.messages.size());
        }
        return count;
    }

    // Find a conference by number
    QwkConference* findConference(int num)
    {
        for (auto& conf : conferences)
        {
            if (conf.number == num)
            {
                return &conf;
            }
        }
        return nullptr;
    }
};

// A reply message (for REP packet)
struct QwkReply
{
    int         conference;     // Target conference number
    std::string to;             // To name
    std::string from;           // From name
    std::string subject;        // Subject
    std::string body;           // Message body
    int         replyToNum;     // Reply-to message number (0 = new msg)
    std::string editor;         // Editor identifier string (for HEADERS.DAT)

    QwkReply() : conference(0), replyToNum(0)
    {
    }
};

// Parse a QWK packet from a .qwk file
// Returns a QwkPacket on success, or std::nullopt on failure
std::optional<QwkPacket> parseQwkFile(const std::string& qwkFilePath);

// Parse CONTROL.DAT file
bool parseControlDat(const std::string& path, QwkPacketInfo& info,
                     std::vector<QwkConference>& conferences);

// Parse MESSAGES.DAT file and populate conferences with messages
// ndxMap variant uses NDX file data for accurate conference assignment
bool parseMessagesDat(const std::string& path,
                      std::vector<QwkConference>& conferences,
                      const std::map<long, int>& ndxMap);
bool parseMessagesDat(const std::string& path,
                      std::vector<QwkConference>& conferences);

// Create a REP packet from reply messages
bool createRepPacket(const std::string& repFilePath,
                     const std::string& bbsID,
                     const std::string& userName,
                     const std::vector<QwkReply>& replies);

// Utility: trim whitespace from both ends of a string
inline std::string trimStr(const std::string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
    {
        return "";
    }
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Utility: extract a fixed-width field from a buffer
inline std::string extractField(const char* buf, int offset, int len)
{
    std::string s(buf + offset, len);
    return trimStr(s);
}

#endif // SLYMAIL_QWK_H
