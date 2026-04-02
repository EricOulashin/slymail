#include "qwk.h"
#include "utf8_util.h"
#include "file_dir_utils.h"
#include <filesystem>
#include <cctype>
#include <cstdio>


using std::string;
using std::vector;
using std::ifstream;
using std::ofstream;
using std::map;
using std::istringstream;

namespace fs = std::filesystem;

// Parse CONTROL.DAT file
bool parseControlDat(const std::string& path, QwkPacketInfo& info,
                     std::vector<QwkConference>& conferences)
{
    std::ifstream f(path);
    if (!f.is_open())
    {
        return false;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(f, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        lines.push_back(line);
    }
    f.close();

    if (lines.size() < 11)
    {
        return false;
    }

    info.bbsName     = trimStr(lines[0]);
    info.bbsLocation = trimStr(lines[1]);
    info.bbsPhone    = trimStr(lines[2]);
    info.sysopName   = trimStr(lines[3]);

    // Line 4: "0000,BBSID"
    std::string idLine = lines[4];
    auto commaPos = idLine.find(',');
    if (commaPos != std::string::npos)
    {
        info.bbsID = trimStr(idLine.substr(commaPos + 1));
    }
    else
    {
        info.bbsID = trimStr(idLine);
    }

    info.packetDate = trimStr(lines[5]);
    info.userName   = trimStr(lines[6]);

    if (lines.size() > 7)
    {
        try
        {
            info.userNumber = std::stoi(trimStr(lines[7]));
        }
        catch (...)
        {
            info.userNumber = 0;
        }
    }

    // Conferences start at line 11, in pairs: number, name
    conferences.clear();
    size_t lineIdx = 11;
    while (lineIdx + 1 < lines.size())
    {
        std::string numStr = trimStr(lines[lineIdx]);
        std::string name = trimStr(lines[lineIdx + 1]);

        if (numStr == "HELLO" || numStr == "BBSNEWS" || numStr == "GOODBYE")
        {
            break;
        }

        try
        {
            int confNum = std::stoi(numStr);
            conferences.emplace_back(confNum, name);
        }
        catch (...)
        {
            break;
        }
        lineIdx += 2;
    }

    return true;
}

// Convert a 4-byte Microsoft Basic MBF (Micro Binary Format) float to an integer
// NDX files use this format for block offsets
static int mbfToInt(const uint8_t* bytes)
{
    // MBF single-precision: byte[3]=exponent, bytes[0-2]=mantissa (big-endian)
    // Format: [mantissa_lo] [mantissa_mid] [mantissa_hi | sign] [exponent]
    uint8_t exponent = bytes[3];
    if (exponent == 0)
    {
        return 0;
    }

    // Build 24-bit mantissa with implicit leading 1
    uint32_t mantissa = (static_cast<uint32_t>(bytes[2] & 0x7F) << 16) |
                        (static_cast<uint32_t>(bytes[1]) << 8) |
                        static_cast<uint32_t>(bytes[0]);
    mantissa |= 0x800000; // Set implicit bit

    // Bias is 128 for MBF; mantissa is 24 bits including implicit bit
    int shift = exponent - 128 - 24;

    int result;
    if (shift >= 0)
    {
        result = static_cast<int>(mantissa << shift);
    }
    else
    {
        result = static_cast<int>(mantissa >> (-shift));
    }

    // Check sign bit
    if (bytes[2] & 0x80)
    {
        result = -result;
    }

    return result;
}

// Build a map of block_offset -> conference_number from NDX files
static std::map<long, int> buildNdxMap(const std::string& extractDir)
{
    std::map<long, int> ndxMap;

    for (const auto& entry : fs::directory_iterator(extractDir))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }

        std::string fname = entry.path().filename().string();
        std::string upper;
        for (char c : fname)
        {
            upper += static_cast<char>(toupper(static_cast<unsigned char>(c)));
        }

        // Check for .NDX extension
        if (upper.size() < 5 || upper.substr(upper.size() - 4) != ".NDX")
        {
            continue;
        }

        // Skip PERSONAL.NDX (we handle it separately if needed)
        if (upper == "PERSONAL.NDX")
        {
            continue;
        }

        // Conference number is the filename without extension
        std::string numPart = upper.substr(0, upper.size() - 4);
        int confNum = 0;
        try
        {
            confNum = std::stoi(numPart);
        }
        catch (...)
        {
            continue;
        }

        // Read NDX file: 5-byte records (4-byte MBF float offset + 1 byte conf)
        FILE* ndxFile = fopen(entry.path().string().c_str(), "rb");
        if (!ndxFile)
        {
            continue;
        }

        uint8_t record[5];
        while (fread(record, 1, 5, ndxFile) == 5)
        {
            int blockOffset = mbfToInt(record);
            if (blockOffset > 0)
            {
                ndxMap[static_cast<long>(blockOffset)] = confNum;
            }
        }
        fclose(ndxFile);
    }

    return ndxMap;
}

// Parse MESSAGES.DAT file using NDX map for conference assignment
bool parseMessagesDat(const std::string& path,
                      std::vector<QwkConference>& conferences,
                      const std::map<long, int>& ndxMap)
{
    FILE* f = fopen(path.c_str(), "rb");
    if (!f)
    {
        return false;
    }

    // Skip the first 128-byte block (packet header)
    char block[QWK_BLOCK_LEN];
    if (fread(block, 1, QWK_BLOCK_LEN, f) != QWK_BLOCK_LEN)
    {
        fclose(f);
        return false;
    }

    // Current block position (1-based, block 1 is the header we just read)
    long currentBlock = 2; // Next block to read is block 2

    while (fread(block, 1, QWK_BLOCK_LEN, f) == QWK_BLOCK_LEN)
    {
        QwkMessage msg;

        // Record the byte offset of this message header (for HEADERS.DAT matching)
        msg.msgOffset = (currentBlock - 1) * QWK_BLOCK_LEN;

        // Status flag
        msg.status = static_cast<QwkStatus>(block[0]);
        msg.isPrivate = (msg.status == QwkStatus::NewPrivate ||
                         msg.status == QwkStatus::OldPrivate);
        // Vote/ballot response messages (status 'V') should be hidden from message lists
        msg.isVoteResponse = (msg.status == QwkStatus::Vote);

        // Message number (bytes 1-7)
        msg.number = 0;
        try
        {
            msg.number = std::stoi(extractField(block, 1, 7));
        }
        catch (...)
        {
        }

        // Date (bytes 8-15, "MM-DD-YY")
        msg.date = extractField(block, 8, 8);

        // Time (bytes 16-20, "HH:MM")
        msg.time = extractField(block, 16, 5);

        // To (bytes 21-45, 25 chars)
        msg.to = extractField(block, 21, 25);

        // From (bytes 46-70, 25 chars)
        msg.from = extractField(block, 46, 25);

        // Subject (bytes 71-95, 25 chars)
        msg.subject = extractField(block, 71, 25);

        // Reply-to message number (bytes 108-115, 8 chars)
        try
        {
            msg.replyTo = std::stoi(extractField(block, 108, 8));
        }
        catch (...)
        {
            msg.replyTo = 0;
        }

        // Number of 128-byte blocks (bytes 116-121, 6 chars) - includes header
        int numBlocks = 1;
        try
        {
            numBlocks = std::stoi(extractField(block, 116, 6));
        }
        catch (...)
        {
            numBlocks = 1;
        }

        // Determine conference from NDX map using current block position
        auto ndxIt = ndxMap.find(currentBlock);
        if (ndxIt != ndxMap.end())
        {
            msg.conference = ndxIt->second;
        }
        else
        {
            // Fallback: bytes 123-124 (0-indexed) contain conference number
            // Byte 122 is reserved (0xE3), conference is at 123-124
            msg.conference = static_cast<uint8_t>(block[123]) |
                            (static_cast<uint8_t>(block[124]) << 8);
        }

        // Read message body (remaining blocks)
        int bodyBlocks = numBlocks - 1;
        if (bodyBlocks > 0 && bodyBlocks < 10000)
        {
            std::vector<char> bodyBuf(bodyBlocks * QWK_BLOCK_LEN);
            size_t bytesRead = fread(bodyBuf.data(), 1, bodyBuf.size(), f);

            msg.body.reserve(bytesRead);
            for (size_t i = 0; i < bytesRead; ++i)
            {
                uint8_t ch = static_cast<uint8_t>(bodyBuf[i]);
                if (ch == QWK_NEWLINE)
                {
                    // 0xE3 could be a QWK newline or a UTF-8 lead byte.
                    // If followed by valid UTF-8 continuation bytes, it's UTF-8.
                    bool isUtf8LeadByte = false;
                    if (i + 2 < bytesRead)
                    {
                        uint8_t b1 = static_cast<uint8_t>(bodyBuf[i + 1]);
                        uint8_t b2 = static_cast<uint8_t>(bodyBuf[i + 2]);
                        if ((b1 & 0xC0) == 0x80 && (b2 & 0xC0) == 0x80)
                        {
                            isUtf8LeadByte = true;
                        }
                    }
                    if (isUtf8LeadByte)
                    {
                        // Treat as UTF-8 3-byte sequence
                        msg.body += static_cast<char>(ch);
                        msg.utf8 = true;
                    }
                    else
                    {
                        msg.body += '\n';
                    }
                }
                else if (ch == '\n')
                {
                    // Standard newline (used in UTF-8 mode QWK packets)
                    msg.body += '\n';
                }
                else if (ch >= 32 || ch == '\t' || ch == 0x01 || ch == 0x03 || ch == 0x1B)
                {
                    // Allow through: printable chars, tab, Ctrl-A (Synchronet),
                    // Ctrl-C (WWIV heart codes), ESC (ANSI sequences)
                    msg.body += static_cast<char>(ch);
                }
                else if (ch >= 0x80)
                {
                    // High bytes: could be CP437 or UTF-8 continuation
                    msg.body += static_cast<char>(ch);
                }
            }

            while (!msg.body.empty() &&
                   (msg.body.back() == ' ' || msg.body.back() == '\n'))
            {
                msg.body.pop_back();
            }

            // Scan for attachment kludge lines
            {
                std::istringstream bodyStream(msg.body);
                string bodyLine;
                while (std::getline(bodyStream, bodyLine))
                {
                    if (bodyLine.find("@ATTACH:") == 0)
                    {
                        string attachFile = trimStr(bodyLine.substr(8));
                        if (!attachFile.empty())
                        {
                            msg.hasAttachment = true;
                            msg.attachmentFiles.push_back(attachFile);
                        }
                    }
                }
            }
        }

        // Advance block counter: header block + body blocks
        currentBlock += numBlocks;

        // Find the conference and add this message
        bool found = false;
        for (auto& conf : conferences)
        {
            if (conf.number == msg.conference)
            {
                conf.messages.push_back(std::move(msg));
                found = true;
                break;
            }
        }
        if (!found)
        {
            QwkConference newConf(msg.conference,
                "Conference " + std::to_string(msg.conference));
            newConf.messages.push_back(std::move(msg));
            conferences.push_back(std::move(newConf));
        }
    }

    fclose(f);
    return true;
}

// Legacy overload without NDX map (for backward compatibility)
bool parseMessagesDat(const std::string& path,
                      std::vector<QwkConference>& conferences)
{
    std::map<long, int> emptyMap;
    return parseMessagesDat(path, conferences, emptyMap);
}

// Parse a complete QWK packet
std::optional<QwkPacket> parseQwkFile(const std::string& qwkFilePath)
{
    QwkPacket packet;
    packet.sourceFile = qwkFilePath;

    packet.extractDir = getTempDir();

    std::string extractResult = extractQwkPacket(qwkFilePath, packet.extractDir);
    if (extractResult.empty())
    {
        cleanupTempDir(packet.extractDir);
        return std::nullopt;
    }

    // Find CONTROL.DAT, MESSAGES.DAT, HEADERS.DAT, and VOTING.DAT (case-insensitive)
    std::string controlPath, messagesPath, headersPath;
    for (const auto& entry : fs::directory_iterator(packet.extractDir))
    {
        std::string fname = entry.path().filename().string();
        std::string upper;
        for (char c : fname)
        {
            upper += static_cast<char>(toupper(static_cast<unsigned char>(c)));
        }

        if (upper == "CONTROL.DAT")
        {
            controlPath = entry.path().string();
        }
        else if (upper == "MESSAGES.DAT")
        {
            messagesPath = entry.path().string();
        }
        else if (upper == "HEADERS.DAT")
        {
            headersPath = entry.path().string();
        }
    }

    if (controlPath.empty() || messagesPath.empty())
    {
        cleanupTempDir(packet.extractDir);
        return std::nullopt;
    }

    // Parse CONTROL.DAT
    if (!parseControlDat(controlPath, packet.info, packet.conferences))
    {
        cleanupTempDir(packet.extractDir);
        return std::nullopt;
    }

    // Build NDX map for conference assignment
    auto ndxMap = buildNdxMap(packet.extractDir);

    // Parse MESSAGES.DAT using NDX map
    if (!parseMessagesDat(messagesPath, packet.conferences, ndxMap))
    {
        cleanupTempDir(packet.extractDir);
        return std::nullopt;
    }


    // Parse HEADERS.DAT (QWKE extended headers) to override from/to/subject
    // with extended versions that may exceed the 25-char limit.
    // Uses byte offset matching for accurate message identification.
    if (!headersPath.empty())
    {
        // Build a map from byte offset -> message pointer for fast lookup
        std::map<long, QwkMessage*> offsetMap;
        for (auto& conf : packet.conferences)
        {
            for (auto& msg : conf.messages)
            {
                if (msg.msgOffset > 0)
                {
                    offsetMap[msg.msgOffset] = &msg;
                }
            }
        }

        std::ifstream hf(headersPath);
        if (hf.is_open())
        {
            // HEADERS.DAT is INI-format with hex offset sections
            // [hex_offset]
            // Sender: Full Name
            // Recipient: Full Name
            // Subject: Full Subject
            // Utf8: true/false
            std::string line;
            long currentOffset = -1;
            std::string hdrSender, hdrRecipient, hdrSubject, hdrMsgId;
            bool hdrUtf8 = false;

            auto applyHeaders = [&]()
            {
                if (currentOffset < 0)
                {
                    return;
                }
                // Find message by exact byte offset
                auto it = offsetMap.find(currentOffset);
                if (it != offsetMap.end())
                {
                    QwkMessage* msg = it->second;
                    if (!hdrSender.empty())
                    {
                        msg->from = hdrSender;
                    }
                    if (!hdrRecipient.empty())
                    {
                        msg->to = hdrRecipient;
                    }
                    if (!hdrSubject.empty())
                    {
                        msg->subject = hdrSubject;
                    }
                    if (hdrUtf8)
                    {
                        msg->utf8 = true;
                    }
                    if (!hdrMsgId.empty())
                    {
                        msg->msgId = hdrMsgId;
                    }
                }
            };

            while (std::getline(hf, line))
            {
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }
                if (line.empty())
                {
                    continue;
                }

                // Section header: [hex_offset]
                if (line.front() == '[' && line.back() == ']')
                {
                    // Apply previous section's headers
                    applyHeaders();
                    // Parse new offset
                    std::string hexStr = line.substr(1, line.size() - 2);
                    try
                    {
                        currentOffset = std::stol(hexStr, nullptr, 16);
                    }
                    catch (...)
                    {
                        currentOffset = -1;
                    }
                    hdrSender.clear();
                    hdrRecipient.clear();
                    hdrSubject.clear();
                    hdrMsgId.clear();
                    hdrUtf8 = false;
                    continue;
                }

                // Key: Value (with ": " separator)
                auto colonPos = line.find(": ");
                if (colonPos != std::string::npos)
                {
                    std::string key = line.substr(0, colonPos);
                    std::string val = line.substr(colonPos + 2);
                    if (key == "Sender")
                    {
                        hdrSender = val;
                    }
                    else if (key == "Recipient")
                    {
                        hdrRecipient = val;
                    }
                    else if (key == "Subject")
                    {
                        hdrSubject = val;
                    }
                    else if (key == "Utf8")
                    {
                        hdrUtf8 = (val == "true" || val == "True" || val == "TRUE");
                    }
                    else if (key == "RFC822MSGID")
                    {
                        hdrMsgId = val;
                    }
                }
                // Key=Value (with "=" separator)
                else
                {
                    auto eqPos = line.find('=');
                    if (eqPos != std::string::npos)
                    {
                        std::string key = trimStr(line.substr(0, eqPos));
                        std::string val = trimStr(line.substr(eqPos + 1));
                        if (key == "Sender")
                        {
                            hdrSender = val;
                        }
                        else if (key == "Recipient")
                        {
                            hdrRecipient = val;
                        }
                        else if (key == "Subject")
                        {
                            hdrSubject = val;
                        }
                        else if (key == "Utf8")
                        {
                            hdrUtf8 = (val == "true" || val == "True" || val == "TRUE");
                        }
                        else if (key == "RFC822MSGID")
                        {
                            hdrMsgId = val;
                        }
                    }
                }
            }
            // Apply last section
            applyHeaders();
            hf.close();
        }
    }

    // Also check for QWKE-style extended headers as kludge lines in message bodies
    // (To:, From:, Subject: at the start of message body)
    for (auto& conf : packet.conferences)
    {
        for (auto& msg : conf.messages)
        {
            std::istringstream bodyStream(msg.body);
            string bodyLine;
            string newBody;
            bool firstLines = true;

            while (std::getline(bodyStream, bodyLine))
            {
                if (!bodyLine.empty() && bodyLine.back() == '\r')
                {
                    bodyLine.pop_back();
                }

                if (firstLines)
                {
                    if (bodyLine.substr(0, 4) == "To: " && bodyLine.size() > 4)
                    {
                        msg.to = trimStr(bodyLine.substr(4));
                        continue;
                    }
                    else if (bodyLine.substr(0, 6) == "From: " && bodyLine.size() > 6)
                    {
                        msg.from = trimStr(bodyLine.substr(6));
                        continue;
                    }
                    else if (bodyLine.substr(0, 9) == "Subject: " && bodyLine.size() > 9)
                    {
                        msg.subject = trimStr(bodyLine.substr(9));
                        continue;
                    }
                    else
                    {
                        firstLines = false;
                    }
                }

                if (!newBody.empty())
                {
                    newBody += '\n';
                }
                newBody += bodyLine;
            }

            if (firstLines == false || newBody != msg.body)
            {
                // Only update body if we actually stripped QWKE headers
                if (newBody.size() < msg.body.size())
                {
                    msg.body = newBody;
                }
            }
        }
    }

    // Check for file attachments in the extracted directory
    for (auto& conf : packet.conferences)
    {
        for (auto& msg : conf.messages)
        {
            // Check for files matching the message (common attachment patterns)
            // Attachments can be stored at the top level of the packet
            if (!msg.attachmentFiles.empty())
            {
                // Verify each referenced file exists
                for (const auto& fname : msg.attachmentFiles)
                {
                    string attachPath = packet.extractDir + PATH_SEP_STR + fname;
                    if (fs::exists(attachPath))
                    {
                        msg.hasAttachment = true;
                    }
                }
            }
        }
    }

    // Parse VOTING.DAT (Synchronet QWKE extension)
    packet.voting = parseVotingDat(packet.extractDir);

    // Associate poll data from VOTING.DAT with messages.
    // Polls in VOTING.DAT may or may not have a corresponding message in
    // MESSAGES.DAT. For polls that don't, we create synthetic messages so
    // they appear in the conference's message list.
    if (!packet.voting.empty())
    {
        // Try to match each poll to an existing message by msgId or conference+from+subject
        std::vector<bool> pollMatched(packet.voting.polls.size(), false);

        for (size_t pi = 0; pi < packet.voting.polls.size(); ++pi)
        {
            const auto& poll = packet.voting.polls[pi];
            for (auto& conf : packet.conferences)
            {
                if (conf.number != poll.conference) continue;
                for (auto& msg : conf.messages)
                {
                    // Match by msgId (most reliable) or by from+subject
                    bool match = false;
                    if (!poll.msgId.empty() && !msg.msgId.empty() &&
                        poll.msgId == msg.msgId)
                    {
                        match = true;
                    }
                    else if (msg.from == poll.from && msg.subject == poll.question)
                    {
                        match = true;
                    }

                    if (match)
                    {
                        msg.isPoll = true;
                        msg.pollIndex = static_cast<int>(pi);
                        if (!poll.msgId.empty() && msg.msgId.empty())
                        {
                            msg.msgId = poll.msgId;
                        }
                        pollMatched[pi] = true;
                    }
                }
            }
        }

        // Create synthetic messages for polls that have no corresponding
        // message in MESSAGES.DAT. This is common with Synchronet, which
        // puts poll definitions only in VOTING.DAT.
        for (size_t pi = 0; pi < packet.voting.polls.size(); ++pi)
        {
            if (pollMatched[pi]) continue;

            const auto& poll = packet.voting.polls[pi];

            QwkMessage synMsg;
            synMsg.number = 0;
            synMsg.from = poll.from;
            synMsg.to = "All";
            synMsg.subject = poll.question;
            synMsg.conference = poll.conference;
            synMsg.isPoll = true;
            synMsg.pollIndex = static_cast<int>(pi);
            synMsg.msgId = poll.msgId;
            synMsg.status = QwkStatus::NewPublic;

            // Parse date from VOTING.DAT WhenWritten field (ISO format: YYYYMMDDHHMMSS)
            if (poll.date.size() >= 8)
            {
                // Try to extract MM-DD-YY from the ISO-ish date
                std::string wd = poll.date;
                // Remove spaces and timezone info
                auto spPos = wd.find(' ');
                if (spPos != std::string::npos) wd = wd.substr(0, spPos);
                if (wd.size() >= 8)
                {
                    // Format: YYYYMMDDHHMMSS
                    std::string yyyy = wd.substr(0, 4);
                    std::string mm = wd.substr(4, 2);
                    std::string dd = wd.substr(6, 2);
                    synMsg.date = mm + "-" + dd + "-" + yyyy.substr(2, 2);
                    if (wd.size() >= 12)
                    {
                        synMsg.time = wd.substr(8, 2) + ":" + wd.substr(10, 2);
                    }
                }
            }

            // Build a body that describes the poll (for display if the poll
            // renderer doesn't kick in for some reason)
            synMsg.body = "Poll: " + poll.question + "\n";
            for (const auto& comment : poll.comments)
            {
                synMsg.body += comment + "\n";
            }
            synMsg.body += "\n";
            for (size_t a = 0; a < poll.answers.size(); ++a)
            {
                synMsg.body += std::to_string(a + 1) + ". " + poll.answers[a].text + "\n";
            }

            // Add to the appropriate conference, inserting at the correct
            // chronological position based on date so polls appear among
            // the messages from the same time period.
            QwkConference* conf = packet.findConference(poll.conference);
            if (!conf)
            {
                // Conference doesn't exist yet — create it
                QwkConference newConf(poll.conference,
                    "Conference " + std::to_string(poll.conference));
                packet.conferences.push_back(std::move(newConf));
                conf = &packet.conferences.back();
            }

            // Find the insertion point by comparing dates.
            // Messages in MESSAGES.DAT are generally in chronological order.
            // Insert just after the last message with a date <= the poll's date.
            auto toSortable = [](const std::string& d) -> std::string
            {
                // d is "MM-DD-YY" — convert to "YYMMDD" for comparison
                if (d.empty()) return "";
                size_t dash1 = d.find('-');
                if (dash1 == std::string::npos) return "";
                size_t dash2 = d.find('-', dash1 + 1);
                if (dash2 == std::string::npos) return "";
                std::string mm = d.substr(0, dash1);
                std::string dd = d.substr(dash1 + 1, dash2 - dash1 - 1);
                std::string yy = d.substr(dash2 + 1);
                if (mm.size() == 1) mm = "0" + mm;
                if (dd.size() == 1) dd = "0" + dd;
                if (yy.size() == 1) yy = "0" + yy;
                return yy + mm + dd;
            };

            std::string synSort = toSortable(synMsg.date);
            size_t insertPos = conf->messages.size(); // default: append at end
            if (!synSort.empty())
            {
                for (size_t mi = 0; mi < conf->messages.size(); ++mi)
                {
                    std::string existSort = toSortable(conf->messages[mi].date);
                    if (!existSort.empty() && existSort > synSort)
                    {
                        insertPos = mi;
                        break;
                    }
                }
            }
            conf->messages.insert(conf->messages.begin() + static_cast<long>(insertPos),
                                   std::move(synMsg));
        }

        // Mark messages with Vote status as polls (even if not in VOTING.DAT)
        for (auto& conf : packet.conferences)
        {
            for (auto& msg : conf.messages)
            {
                if (msg.status == QwkStatus::Vote && !msg.isPoll)
                {
                    msg.isVoteResponse = true;
                }
            }
        }

        // Tally up/down votes from vote records on non-poll messages
        for (const auto& vr : packet.voting.voteRecords)
        {
            for (auto& conf : packet.conferences)
            {
                if (conf.number != vr.conference) continue;
                for (auto& msg : conf.messages)
                {
                    if (!msg.msgId.empty() && msg.msgId == vr.inReplyTo && !msg.isPoll)
                    {
                        if (vr.upVote)
                        {
                            ++msg.upvotes;
                        }
                        else if (vr.downVote)
                        {
                            ++msg.downvotes;
                        }
                    }
                }
            }
        }
    }

    return packet;
}

// Create a REP packet
bool createRepPacket(const std::string& repFilePath,
                     const std::string& bbsID,
                     const std::string& /* userName */,
                     const std::vector<QwkReply>& replies,
                     const std::vector<PendingVote>& pendingVotes)
{
    if (replies.empty() && pendingVotes.empty())
    {
        return false;
    }

    std::string repDir = getTempDir() + "_rep";
    fs::create_directories(repDir);

    std::string idUpper = bbsID;
    for (auto& c : idUpper)
    {
        c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
    }

    // Only write the .MSG file if there are actual message replies
    if (!replies.empty())
    {

    std::string msgPath = repDir + PATH_SEP_STR + idUpper + ".MSG";
    FILE* f = fopen(msgPath.c_str(), "wb");
    if (!f)
    {
        cleanupTempDir(repDir);
        return false;
    }

    // Write header block
    char headerBlock[QWK_BLOCK_LEN];
    memset(headerBlock, ' ', QWK_BLOCK_LEN);
    size_t idLen = std::min(idUpper.size(), (size_t)QWK_BLOCK_LEN);
    memcpy(headerBlock, idUpper.c_str(), idLen);
    fwrite(headerBlock, 1, QWK_BLOCK_LEN, f);

    for (const auto& reply : replies)
    {
        // Build the message body with QWK newlines
        std::string fullBody = reply.body;

        std::string qwkBody;
        for (char c : fullBody)
        {
            if (c == '\n')
            {
                qwkBody += static_cast<char>(QWK_NEWLINE);
            }
            else
            {
                qwkBody += c;
            }
        }

        int bodyLen = static_cast<int>(qwkBody.size());
        int totalBytes = QWK_BLOCK_LEN + bodyLen;
        int numBlocks = (totalBytes + QWK_BLOCK_LEN - 1) / QWK_BLOCK_LEN;

        char hdr[QWK_BLOCK_LEN];
        memset(hdr, ' ', QWK_BLOCK_LEN);

        hdr[0] = ' ';
        // In REP packets, bytes 1-7 contain the CONFERENCE NUMBER (not msg number)
        // MUST be LEFT-justified (%-7d) so trailing spaces separate it from the
        // date field at byte 8. Synchronet uses atol(block+1) which reads until
        // a non-digit, so right-justified numbers would run into date digits.
        {
            char confBuf[8];
            snprintf(confBuf, sizeof(confBuf), "%-7d", reply.conference);
            memcpy(hdr + 1, confBuf, 7);
        }

        time_t now = std::time(nullptr);
        struct tm* tm = localtime(&now);
        {
            char dateBuf[16], timeBuf[16];
            snprintf(dateBuf, sizeof(dateBuf), "%02d-%02d-%02d",
                     (tm->tm_mon + 1) % 100, tm->tm_mday % 100, tm->tm_year % 100);
            memcpy(hdr + 8, dateBuf, 8);
            snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", tm->tm_hour, tm->tm_min);
            memcpy(hdr + 16, timeBuf, 5);
        }

        memset(hdr + 21, ' ', 25);
        size_t toLen = std::min(reply.to.size(), (size_t)25);
        memcpy(hdr + 21, reply.to.c_str(), toLen);

        memset(hdr + 46, ' ', 25);
        size_t fromLen = std::min(reply.from.size(), (size_t)25);
        memcpy(hdr + 46, reply.from.c_str(), fromLen);

        memset(hdr + 71, ' ', 25);
        size_t subjLen = std::min(reply.subject.size(), (size_t)25);
        memcpy(hdr + 71, reply.subject.c_str(), subjLen);

        memset(hdr + 96, ' ', 12);
        snprintf(hdr + 108, 9, "%8d", reply.replyToNum);

        {
            char blockBuf[16];
            snprintf(blockBuf, sizeof(blockBuf), "%6d", numBlocks);
            memcpy(hdr + 116, blockBuf, 6);
        }

        // Byte 122 is reserved (0xE3), conference number at bytes 123-124
        hdr[122] = static_cast<char>(QWK_NEWLINE);  // Reserved byte
        hdr[123] = static_cast<char>(reply.conference & 0xFF);
        hdr[124] = static_cast<char>((reply.conference >> 8) & 0xFF);
        memset(hdr + 125, ' ', 3);

        for (int i = 0; i < QWK_BLOCK_LEN; ++i)
        {
            if (hdr[i] == '\0')
            {
                hdr[i] = ' ';
            }
        }

        fwrite(hdr, 1, QWK_BLOCK_LEN, f);

        if (!qwkBody.empty())
        {
            fwrite(qwkBody.c_str(), 1, qwkBody.size(), f);
            int padding = (numBlocks * QWK_BLOCK_LEN) - totalBytes;
            if (padding > 0)
            {
                std::string pad(padding, ' ');
                fwrite(pad.c_str(), 1, padding, f);
            }
        }
    }

    fclose(f);

    } // end if (!replies.empty())

    // Generate HEADERS.DAT for QWKE support (extended fields > 25 chars)
    {
        bool needHeaders = false;
        for (const auto& reply : replies)
        {
            if (reply.to.size() > 25 || reply.from.size() > 25 ||
                reply.subject.size() > 25 || !reply.editor.empty())
            {
                needHeaders = true;
                break;
            }
        }

        if (needHeaders)
        {
            string headersPath = repDir + PATH_SEP_STR + "HEADERS.DAT";
            std::ofstream hf(headersPath);
            if (hf.is_open())
            {
                // Track offset: header block (128 bytes) + per message
                long offset = QWK_BLOCK_LEN; // Skip BBS ID header block
                for (const auto& reply : replies)
                {
                    std::string fullBody = reply.body;
                    int bodyLen = static_cast<int>(fullBody.size());
                    int totalBytes = QWK_BLOCK_LEN + bodyLen;
                    int numBlocks = (totalBytes + QWK_BLOCK_LEN - 1) / QWK_BLOCK_LEN;

                    hf << "[" << std::hex << offset << "]\n" << std::dec;
                    if (reply.from.size() > 25)
                    {
                        hf << "Sender: " << reply.from << "\n";
                    }
                    if (reply.to.size() > 25)
                    {
                        hf << "Recipient: " << reply.to << "\n";
                    }
                    if (reply.subject.size() > 25)
                    {
                        hf << "Subject: " << reply.subject << "\n";
                    }
                    if (!reply.editor.empty())
                    {
                        hf << "Editor: " << reply.editor << "\n";
                    }
                    hf << "\n";

                    offset += numBlocks * QWK_BLOCK_LEN;
                }
                hf.close();
            }
        }
    }

    // Generate VOTING.DAT for any pending votes
    if (!pendingVotes.empty())
    {
        string votingPath = repDir + PATH_SEP_STR + "VOTING.DAT";
        std::ofstream vf(votingPath);
        if (vf.is_open())
        {
            // Get current time for WhenWritten
            time_t now = std::time(nullptr);
            struct tm* tm = localtime(&now);
            char timeBuf[64];
            snprintf(timeBuf, sizeof(timeBuf), "%04d-%02d-%02dT%02d:%02d:%02d",
                     tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                     tm->tm_hour, tm->tm_min, tm->tm_sec);

            int voteIdx = 0;
            for (const auto& vote : pendingVotes)
            {
                // Write offset section (dummy offset, just needs to be unique)
                vf << "[" << std::hex << (0x1000 + voteIdx * 0x100) << "]\n" << std::dec;

                // Write vote section
                vf << "[vote:" << vote.msgId << "]\n";
                if (vote.upVote)
                {
                    vf << "UpVote = true\n";
                }
                else if (vote.downVote)
                {
                    vf << "DownVote = true\n";
                }
                else if (vote.votes != 0)
                {
                    vf << "Votes = 0x" << std::hex << vote.votes << std::dec << "\n";
                }
                vf << "WhenWritten: " << timeBuf << "\n";
                vf << "Sender: " << vote.voter << "\n";
                vf << "Conference: " << vote.conference << "\n";
                vf << "\n";

                ++voteIdx;
            }
            vf.close();
        }
    }

    bool ok = createZipArchive(repFilePath, repDir);
    cleanupTempDir(repDir);
    return ok;
}
