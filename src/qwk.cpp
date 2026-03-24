#include "qwk.h"

using std::string;
using std::vector;
using std::ifstream;
using std::ofstream;
using std::map;
using std::istringstream;

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
            upper += toupper(c);
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

        // Status flag
        msg.status = static_cast<QwkStatus>(block[0]);
        msg.isPrivate = (msg.status == QwkStatus::NewPrivate ||
                         msg.status == QwkStatus::OldPrivate);

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
                    msg.body += '\n';
                }
                else if (ch >= 32 || ch == '\t')
                {
                    msg.body += static_cast<char>(ch);
                }
            }

            while (!msg.body.empty() &&
                   (msg.body.back() == ' ' || msg.body.back() == '\n'))
            {
                msg.body.pop_back();
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

    // Find CONTROL.DAT, MESSAGES.DAT, and HEADERS.DAT (case-insensitive)
    std::string controlPath, messagesPath, headersPath;
    for (const auto& entry : fs::directory_iterator(packet.extractDir))
    {
        std::string fname = entry.path().filename().string();
        std::string upper;
        for (char c : fname)
        {
            upper += toupper(c);
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
    // with extended versions that may exceed the 25-char limit
    if (!headersPath.empty())
    {
        std::ifstream hf(headersPath);
        if (hf.is_open())
        {
            // HEADERS.DAT is INI-format with hex offset sections
            // [hex_offset]
            // Sender: Full Name
            // Recipient: Full Name
            // Subject: Full Subject
            std::string line;
            long currentOffset = -1;
            std::string hdrSender, hdrRecipient, hdrSubject;

            auto applyHeaders = [&]()
            {
                if (currentOffset < 0)
                {
                    return;
                }
                // Find the message at this offset
                // Offset is in bytes from start of MESSAGES.DAT
                // Block number = offset / 128; message block index = blockNum - 1 (skip header)
                for (auto& conf : packet.conferences)
                {
                    for (auto& msg : conf.messages)
                    {
                        // Match by message number (approximate - use conference + sequential)
                        // The offset corresponds to the message's position in MESSAGES.DAT
                        // Since we don't track exact offsets, match by checking if the
                        // extended headers have values that differ from the 25-char truncated ones
                        if (!hdrSender.empty() && !msg.from.empty()
                            && hdrSender.find(msg.from.substr(0, std::min(msg.from.size(), (size_t)10))) == 0)
                        {
                            msg.from = hdrSender;
                        }
                        if (!hdrRecipient.empty() && !msg.to.empty()
                            && hdrRecipient.find(msg.to.substr(0, std::min(msg.to.size(), (size_t)10))) == 0)
                        {
                            msg.to = hdrRecipient;
                        }
                        if (!hdrSubject.empty() && !msg.subject.empty()
                            && hdrSubject.find(msg.subject.substr(0, std::min(msg.subject.size(), (size_t)10))) == 0)
                        {
                            msg.subject = hdrSubject;
                        }
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
                    continue;
                }

                // Key: Value
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
                }
            }
            // Apply last section
            applyHeaders();
            hf.close();
        }
    }

    return packet;
}

// Create a REP packet
bool createRepPacket(const std::string& repFilePath,
                     const std::string& bbsID,
                     const std::string& /* userName */,
                     const std::vector<QwkReply>& replies)
{
    if (replies.empty())
    {
        return false;
    }

    std::string repDir = getTempDir() + "_rep";
    fs::create_directories(repDir);

    std::string idUpper = bbsID;
    for (auto& c : idUpper)
    {
        c = toupper(c);
    }

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
        // Append editor tear line if editor info is set
        std::string fullBody = reply.body;
        if (!reply.editor.empty())
        {
            fullBody += "\n---\n";
            fullBody += " * " + reply.editor + "\n";
        }

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

    bool ok = createZipArchive(repFilePath, repDir);
    cleanupTempDir(repDir);
    return ok;
}
