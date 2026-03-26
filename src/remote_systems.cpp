#include "remote_systems.h"
#include "file_browser.h"
#include <cctype>
#include <ctime>
#include <fstream>
#include <sstream>
#include <algorithm>

using std::string;
using std::vector;

// ============================================================
// Cross-platform home directory
// ============================================================

string getHomeDir()
{
#if SLYMAIL_WINDOWS
    // Try USERPROFILE first (most reliable on Windows)
    const char* userProfile = getenv("USERPROFILE");
    if (userProfile && userProfile[0] != '\0')
    {
        return userProfile;
    }
    // Fallback: HOMEDRIVE + HOMEPATH
    const char* homeDrive = getenv("HOMEDRIVE");
    const char* homePath = getenv("HOMEPATH");
    if (homeDrive && homePath)
    {
        return string(homeDrive) + string(homePath);
    }
    return "C:\\";
#else
    // POSIX: Linux, macOS, BSD, other Unix
    const char* home = getenv("HOME");
    if (home && home[0] != '\0')
    {
        return home;
    }
    return "/tmp";
#endif
}

// ============================================================
// SlyMail data directory
// ============================================================

string getSlyMailDataDir()
{
    string home = getHomeDir();
    string dataDir = home + PATH_SEP_STR + ".slymail";
    try
    {
        fs::create_directories(dataDir);
        // Also create the QWK and REP subdirectories
        fs::create_directories(dataDir + PATH_SEP_STR + "QWK");
        fs::create_directories(dataDir + PATH_SEP_STR + "REP");
    }
    catch (const fs::filesystem_error& e)
    {
        fprintf(stderr, "Warning: Could not create data directory: %s\n", e.what());
    }
    catch (...)
    {
        fprintf(stderr, "Warning: Could not create data directory: %s\n", dataDir.c_str());
    }
    return dataDir;
}

// ============================================================
// Password obfuscation (XOR + base64)
// This is not cryptographically secure — it prevents the password
// from being immediately readable in the JSON file.
// ============================================================

static const char* XOR_KEY = "SlyMail_QWK_2026";

static string xorCipher(const string& input, const string& key)
{
    string output;
    output.reserve(input.size());
    for (size_t i = 0; i < input.size(); ++i)
    {
        output += static_cast<char>(input[i] ^ key[i % key.size()]);
    }
    return output;
}

static const char* BASE64_CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static string base64Encode(const string& input)
{
    string output;
    int val = 0;
    int bits = 0;
    for (uint8_t c : input)
    {
        val = (val << 8) | c;
        bits += 8;
        while (bits >= 6)
        {
            bits -= 6;
            output += BASE64_CHARS[(val >> bits) & 0x3F];
        }
    }
    if (bits > 0)
    {
        output += BASE64_CHARS[(val << (6 - bits)) & 0x3F];
    }
    while (output.size() % 4 != 0)
    {
        output += '=';
    }
    return output;
}

static string base64Decode(const string& input)
{
    string output;
    int val = 0;
    int bits = 0;
    for (char c : input)
    {
        if (c == '=' || c == '\0') break;
        const char* p = strchr(BASE64_CHARS, c);
        if (!p) continue;
        val = (val << 6) | static_cast<int>(p - BASE64_CHARS);
        bits += 6;
        if (bits >= 8)
        {
            bits -= 8;
            output += static_cast<char>((val >> bits) & 0xFF);
        }
    }
    return output;
}

static string encryptPassword(const string& plaintext)
{
    if (plaintext.empty()) return "";
    return base64Encode(xorCipher(plaintext, XOR_KEY));
}

static string decryptPassword(const string& encoded)
{
    if (encoded.empty()) return "";
    return xorCipher(base64Decode(encoded), XOR_KEY);
}

// ============================================================
// Minimal JSON helpers (no external dependency)
// ============================================================

// Escape a string for JSON output
static string jsonEscape(const string& s)
{
    string result;
    result.reserve(s.size() + 8);
    for (char c : s)
    {
        switch (c)
        {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:   result += c; break;
        }
    }
    return result;
}

// Unescape a JSON string value (basic)
static string jsonUnescape(const string& s)
{
    string result;
    result.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '\\' && i + 1 < s.size())
        {
            switch (s[i + 1])
            {
                case '"':  result += '"'; ++i; break;
                case '\\': result += '\\'; ++i; break;
                case 'n':  result += '\n'; ++i; break;
                case 'r':  result += '\r'; ++i; break;
                case 't':  result += '\t'; ++i; break;
                default:   result += s[i]; break;
            }
        }
        else
        {
            result += s[i];
        }
    }
    return result;
}

// Trim whitespace
static string jTrim(const string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Extract a JSON string value: find "key": "value" and return value
static string jsonGetString(const string& json, const string& key)
{
    string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == string::npos) return "";
    pos = json.find(':', pos + needle.size());
    if (pos == string::npos) return "";
    pos = json.find('"', pos + 1);
    if (pos == string::npos) return "";
    ++pos; // skip opening quote
    string result;
    while (pos < json.size() && json[pos] != '"')
    {
        if (json[pos] == '\\' && pos + 1 < json.size())
        {
            result += json[pos];
            result += json[pos + 1];
            pos += 2;
        }
        else
        {
            result += json[pos];
            ++pos;
        }
    }
    return jsonUnescape(result);
}

// Extract a JSON integer value
static int jsonGetInt(const string& json, const string& key, int def = 0)
{
    string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == string::npos) return def;
    pos = json.find(':', pos + needle.size());
    if (pos == string::npos) return def;
    ++pos;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) ++pos;
    try { return std::stoi(json.substr(pos)); } catch (...) { return def; }
}

// Extract a JSON boolean value
static bool jsonGetBool(const string& json, const string& key, bool def = false)
{
    string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == string::npos) return def;
    pos = json.find(':', pos + needle.size());
    if (pos == string::npos) return def;
    string rest = jTrim(json.substr(pos + 1));
    if (rest.substr(0, 4) == "true") return true;
    if (rest.substr(0, 5) == "false") return false;
    return def;
}

// ============================================================
// Load / Save remote systems
// ============================================================

static const char* REMOTE_SYSTEMS_FILE = "remote_systems.json";

vector<RemoteSystem> loadRemoteSystems(const string& dataDir)
{
    vector<RemoteSystem> systems;
    string path = dataDir + PATH_SEP_STR + REMOTE_SYSTEMS_FILE;
    std::ifstream f(path);
    if (!f.is_open()) return systems;

    // Read entire file
    string content((std::istreambuf_iterator<char>(f)),
                    std::istreambuf_iterator<char>());
    f.close();

    // Parse array of objects (simple: split by "{" ... "}")
    size_t pos = 0;
    while (pos < content.size())
    {
        size_t objStart = content.find('{', pos);
        if (objStart == string::npos) break;
        // Find matching closing brace (no nested objects)
        size_t objEnd = content.find('}', objStart);
        if (objEnd == string::npos) break;
        string obj = content.substr(objStart, objEnd - objStart + 1);

        RemoteSystem sys;
        sys.name = jsonGetString(obj, "name");
        string typeStr = jsonGetString(obj, "type");
        sys.type = (typeStr == "SSH") ? RemoteConnType::SSH : RemoteConnType::FTP;
        sys.host = jsonGetString(obj, "host");
        sys.port = jsonGetInt(obj, "port", 0);
        sys.username = jsonGetString(obj, "username");
        sys.password = decryptPassword(jsonGetString(obj, "password"));
        sys.passiveFTP = jsonGetBool(obj, "passiveFTP", true);
        sys.remotePath = jsonGetString(obj, "remotePath");
        sys.lastConnected = jsonGetString(obj, "lastConnected");

        if (!sys.name.empty() || !sys.host.empty())
        {
            systems.push_back(sys);
        }
        pos = objEnd + 1;
    }
    return systems;
}

bool saveRemoteSystems(const string& dataDir,
                       const vector<RemoteSystem>& systems)
{
    string path = dataDir + PATH_SEP_STR + REMOTE_SYSTEMS_FILE;
    std::ofstream f(path);
    if (!f.is_open()) return false;

    f << "[\n";
    for (size_t i = 0; i < systems.size(); ++i)
    {
        const auto& s = systems[i];
        f << "  {\n";
        f << "    \"name\": \"" << jsonEscape(s.name) << "\",\n";
        f << "    \"type\": \"" << (s.type == RemoteConnType::SSH ? "SSH" : "FTP") << "\",\n";
        f << "    \"host\": \"" << jsonEscape(s.host) << "\",\n";
        f << "    \"port\": " << s.port << ",\n";
        f << "    \"username\": \"" << jsonEscape(s.username) << "\",\n";
        f << "    \"password\": \"" << jsonEscape(encryptPassword(s.password)) << "\",\n";
        f << "    \"passiveFTP\": " << (s.passiveFTP ? "true" : "false") << ",\n";
        f << "    \"remotePath\": \"" << jsonEscape(s.remotePath) << "\",\n";
        f << "    \"lastConnected\": \"" << jsonEscape(s.lastConnected) << "\"\n";
        f << "  }" << (i + 1 < systems.size() ? "," : "") << "\n";
    }
    f << "]\n";
    f.close();
    return true;
}

// ============================================================
// Get current timestamp as ISO string
// ============================================================
static string currentTimestamp()
{
    time_t now = std::time(nullptr);
    struct tm* tm = localtime(&now);
    char buf[64];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d",
             tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
             tm->tm_hour, tm->tm_min);
    return buf;
}

// ============================================================
// Remote system editor dialog
// ============================================================

bool editRemoteSystem(RemoteSystem& sys)
{
    int cols = g_term->getCols();
    int rows = g_term->getRows();

    int dlgW = 62;
    if (dlgW > cols - 4) dlgW = cols - 4;
    int dlgH = 16;
    int dlgX = (cols - dlgW) / 2;
    int dlgY = (rows - dlgH) / 2;

    TermAttr borderAttr = tAttr(TC_CYAN, TC_BLACK, true);
    TermAttr labelAttr = tAttr(TC_CYAN, TC_BLACK, false);
    TermAttr valueAttr = tAttr(TC_WHITE, TC_BLACK, true);
    TermAttr helpAttr = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr selAttr = tAttr(TC_WHITE, TC_BLUE, true);

    // Editable fields
    string name = sys.name;
    int typeIdx = (sys.type == RemoteConnType::SSH) ? 1 : 0;
    string host = sys.host;
    // Default port based on connection type if not set
    int defaultPort = (sys.type == RemoteConnType::SSH) ? 22 : 21;
    string portStr = (sys.port > 0) ? std::to_string(sys.port) : std::to_string(defaultPort);
    string username = sys.username;
    string password = sys.password;
    bool passiveFTP = sys.passiveFTP;
    string remotePath = sys.remotePath;

    int selected = 0;
    const int fieldCount = 8;

    while (true)
    {
        // Draw dialog
        for (int r = 0; r < dlgH; ++r)
        {
            fillRow(dlgY + r, tAttr(TC_BLACK, TC_BLACK, false), dlgX, dlgX + dlgW);
        }
        drawBox(dlgY, dlgX, dlgH, dlgW, borderAttr, "Edit Remote System", borderAttr);

        int y = dlgY + 1;
        int labelX = dlgX + 2;
        int valX = dlgX + 18;
        int valW = dlgW - 20;

        auto drawField = [&](int idx, const string& label, const string& value)
        {
            bool isSel = (idx == selected);
            TermAttr lbl = isSel ? selAttr : labelAttr;
            TermAttr val = isSel ? selAttr : valueAttr;
            // Fill the entire row with background color first
            if (isSel)
            {
                fillRow(y, selAttr, dlgX + 1, dlgX + dlgW - 1);
            }
            else
            {
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
            }
            printAt(y, labelX, label, lbl);
            string displayVal = value;
            if (static_cast<int>(displayVal.size()) > valW)
            {
                displayVal = displayVal.substr(0, valW);
            }
            printAt(y, valX, displayVal, val);
            ++y;
        };

        drawField(0, "Name:       ", name);
        drawField(1, "Type:       ", typeIdx == 0 ? "FTP" : "SSH");
        drawField(2, "Host:       ", host);
        drawField(3, "Port:       ", portStr);
        drawField(4, "Username:   ", username);
        drawField(5, "Password:   ", string(password.size(), '*'));
        if (typeIdx == 0)
        {
            drawField(6, "Passive FTP:", passiveFTP ? "Yes" : "No");
        }
        else
        {
            TermAttr dimAttr = tAttr(TC_BLACK, TC_BLACK, true);
            fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), dlgX + 1, dlgX + dlgW - 1);
            printAt(y, labelX, "Passive FTP:", dimAttr);
            printAt(y, valX, "(N/A for SSH)", dimAttr);
            ++y;
        }
        drawField(7, "Remote Path:", remotePath.empty() ? "/" : remotePath);

        // Help
        printAt(dlgY + dlgH - 3, labelX, "Up/Dn=Move, Enter=Edit field, Space=Toggle", helpAttr);
        printAt(dlgY + dlgH - 2, labelX, "Ctrl-S=Save & close, ESC/Ctrl-C/Q=Cancel", helpAttr);

        g_term->refresh();

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:
                if (selected > 0) --selected;
                // Skip passive FTP field when in SSH mode
                if (selected == 6 && typeIdx != 0) --selected;
                break;
            case TK_DOWN:
                if (selected < fieldCount - 1) ++selected;
                if (selected == 6 && typeIdx != 0) ++selected;
                break;
            case ' ':
                if (selected == 1)
                {
                    typeIdx = (typeIdx == 0) ? 1 : 0;
                    // Update port to match new type's default if it was the old default
                    if (portStr == "21" || portStr == "22")
                    {
                        portStr = (typeIdx == 1) ? "22" : "21";
                    }
                }
                else if (selected == 6 && typeIdx == 0)
                {
                    passiveFTP = !passiveFTP;
                }
                break;
            case TK_ENTER:
            {
                int fieldY = dlgY + 1 + selected;
                switch (selected)
                {
                    case 0:
                        name = getStringInput(fieldY, valX, valW, name, valueAttr);
                        if (name.empty()) name = sys.name; // revert if cancelled
                        break;
                    case 1:
                        typeIdx = (typeIdx == 0) ? 1 : 0;
                        if (portStr == "21" || portStr == "22")
                        {
                            portStr = (typeIdx == 1) ? "22" : "21";
                        }
                        break;
                    case 2:
                        host = getStringInput(fieldY, valX, valW, host, valueAttr);
                        if (host.empty()) host = sys.host;
                        break;
                    case 3:
                    {
                        string ps = getStringInput(fieldY, valX, valW, portStr, valueAttr);
                        if (!ps.empty())
                        {
                            // Strip non-digit characters
                            string digits;
                            for (char c : ps)
                            {
                                if (c >= '0' && c <= '9') digits += c;
                            }
                            portStr = digits;
                        }
                        break;
                    }
                    case 4:
                        username = getStringInput(fieldY, valX, valW, username, valueAttr);
                        if (username.empty()) username = sys.username;
                        break;
                    case 5:
                        password = getPasswordInput(fieldY, valX, valW, password, valueAttr);
                        if (password.empty()) password = sys.password;
                        break;
                    case 6:
                        if (typeIdx == 0) passiveFTP = !passiveFTP;
                        break;
                    case 7:
                        remotePath = getStringInput(fieldY, valX, valW, remotePath, valueAttr);
                        break;
                }
                break;
            }
            case TK_CTRL_S:
            {
                // Save
                sys.name = name;
                sys.type = (typeIdx == 1) ? RemoteConnType::SSH : RemoteConnType::FTP;
                sys.host = host;
                sys.port = 0;
                if (!portStr.empty())
                {
                    try { sys.port = std::stoi(portStr); } catch (...) {}
                }
                sys.username = username;
                sys.password = password;
                sys.passiveFTP = passiveFTP;
                sys.remotePath = remotePath;
                return true;
            }
            case TK_ESCAPE:
            case TK_CTRL_C:
            case 'q': case 'Q':
                return false;
            default:
                break;
        }
    }
}

// ============================================================
// FTP/SFTP directory listing via curl/sftp commands
// ============================================================

// Platform-compatible popen/pclose
#if SLYMAIL_WINDOWS
    #define sm_popen  _popen
    #define sm_pclose _pclose
    #define SM_SUPPRESS_STDERR " 2>NUL"
#else
    #define sm_popen  popen
    #define sm_pclose pclose
    #define SM_SUPPRESS_STDERR " 2>/dev/null"
#endif

// Execute a command and capture stdout into a string
static string execCapture(const string& cmd)
{
    string result;
    FILE* pipe = sm_popen(cmd.c_str(), "r");
    if (!pipe) return result;
    char buf[4096];
    while (fgets(buf, sizeof(buf), pipe) != nullptr)
    {
        result += buf;
    }
    sm_pclose(pipe);
    return result;
}

// Parse FTP directory listing (LIST format) into file entries
static vector<FileEntry> parseFtpListing(const string& listing)
{
    vector<FileEntry> entries;
    std::istringstream stream(listing);
    string line;
    while (std::getline(stream, line))
    {
        if (line.empty()) continue;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        FileEntry fe;
        fe.isDirectory = (line[0] == 'd');
        fe.fileSize = 0;

        // Parse Unix-style listing: drwxr-xr-x ... name
        // Find the filename: it's everything after the last space-separated token
        // Simplification: split by whitespace, take token index 8+ as name
        vector<string> tokens;
        std::istringstream ts(line);
        string token;
        while (ts >> token)
        {
            tokens.push_back(token);
        }
        if (tokens.size() < 9) continue;

        // Name is everything from token 8 onwards (may contain spaces)
        string name;
        for (size_t i = 8; i < tokens.size(); ++i)
        {
            if (!name.empty()) name += " ";
            name += tokens[i];
        }
        // Skip . and ..
        if (name == "." || name == "..") continue;
        // Skip symlink targets
        auto arrowPos = name.find(" -> ");
        if (arrowPos != string::npos)
        {
            name = name.substr(0, arrowPos);
        }

        fe.name = name;
        fe.fullPath = name; // relative name; caller will prepend path
        try { fe.fileSize = std::stoull(tokens[4]); } catch (...) {}

        // Date from tokens 5-7
        if (tokens.size() >= 8)
        {
            fe.dateStr = tokens[5] + " " + tokens[6] + " " + tokens[7];
        }

        entries.push_back(fe);
    }

    // Sort: directories first, then alphabetical
    std::sort(entries.begin(), entries.end());
    return entries;
}

// Build the FTP base URL. For curl FTP, the path after the host is relative
// to the login directory unless we use %2F for absolute paths.
// We use the simpler approach: CWD-relative paths.
static string ftpBaseUrl(const RemoteSystem& sys)
{
    int port = (sys.port > 0) ? sys.port : 21;
    return "ftp://" + sys.host + ":" + std::to_string(port);
}

// Build an FTP URL for a given remote path. Ensures no double slashes.
static string ftpUrl(const RemoteSystem& sys, const string& remotePath)
{
    string base = ftpBaseUrl(sys);
    if (remotePath.empty() || remotePath == "/")
    {
        return base + "/";
    }
    // Ensure exactly one slash between base and path
    if (remotePath[0] == '/')
    {
        return base + remotePath;
    }
    return base + "/" + remotePath;
}

// List remote directory via FTP using curl
static vector<FileEntry> ftpListDir(const RemoteSystem& sys, const string& remotePath)
{
    string url = ftpUrl(sys, remotePath);
    if (!url.empty() && url.back() != '/') url += "/";

    string cmd = "curl -s --user \"" + sys.username + ":" + sys.password + "\"";
    if (sys.passiveFTP) cmd += " --ftp-pasv";
    cmd += " --connect-timeout 10 --max-time 30";
    cmd += " \"" + url + "\" " SM_SUPPRESS_STDERR;

    string listing = execCapture(cmd);
    return parseFtpListing(listing);
}

// Build an SFTP URL for a given remote path.
static string sftpUrl(const RemoteSystem& sys, const string& remotePath)
{
    int port = (sys.port > 0) ? sys.port : 22;
    string base = "sftp://" + sys.host + ":" + std::to_string(port);
    if (remotePath.empty() || remotePath == "/")
    {
        return base + "/";
    }
    if (remotePath[0] == '/')
    {
        return base + remotePath;
    }
    return base + "/" + remotePath;
}

// List remote directory via SFTP using curl (curl supports sftp://)
// Returns empty vector on failure. Sets errMsg if provided.
static vector<FileEntry> sftpListDir(const RemoteSystem& sys, const string& remotePath,
                                      string* errMsg = nullptr)
{
    string url = sftpUrl(sys, remotePath);
    if (!url.empty() && url.back() != '/') url += "/";

    // Capture both stdout and stderr so we can detect SFTP subsystem errors
    string cmd = "curl -s --user \"" + sys.username + ":" + sys.password + "\""
               + " --insecure --connect-timeout 15 --max-time 30"
               + " \"" + url + "\" 2>&1";

    string output = execCapture(cmd);

    // Check for common SFTP errors in the output
    if (output.find("Failure initializing sftp") != string::npos ||
        output.find("SFTP subsystem") != string::npos)
    {
        if (errMsg)
        {
            *errMsg = "The remote system does not support SFTP file transfers. "
                      "Try connecting via FTP instead.";
        }
        return {};
    }
    if (output.find("curl:") != string::npos ||
        output.find("Connection refused") != string::npos)
    {
        if (errMsg)
        {
            *errMsg = "Failed to connect via SSH/SFTP to " + sys.host;
        }
        return {};
    }

    return parseFtpListing(output);
}

// ============================================================
// Curl exit code to human-readable error message
// ============================================================

// Extract the curl exit code from system() return value.
// On POSIX, system() returns the status from waitpid; the actual exit code
// is in the upper bits. On Windows, system() returns the exit code directly.
static int curlExitCode(int systemRet)
{
#if SLYMAIL_WINDOWS
    return systemRet;
#else
    // WEXITSTATUS equivalent
    if (systemRet >= 0 && (systemRet & 0x7F) == 0)
    {
        return (systemRet >> 8) & 0xFF;
    }
    return systemRet; // signal or other
#endif
}

static string curlErrorMessage(int exitCode)
{
    switch (exitCode)
    {
        case 1:  return "Unsupported protocol";
        case 2:  return "Failed to initialize curl";
        case 3:  return "Malformed URL";
        case 5:  return "Could not resolve proxy";
        case 6:  return "Could not resolve host (check hostname)";
        case 7:  return "Could not connect to server (connection refused or host unreachable)";
        case 8:  return "Server returned an unexpected response";
        case 9:  return "Access denied to remote resource";
        case 10: return "FTP user or password incorrect";
        case 11: return "FTP unexpected server response";
        case 13: return "FTP unknown response to PASV/EPSV";
        case 14: return "FTP unknown response format during transfer";
        case 16: return "HTTP/2 error";
        case 17: return "FTP could not set binary transfer mode";
        case 18: return "Partial file received (transfer interrupted)";
        case 19: return "FTP could not download the file (RETR failed)";
        case 21: return "FTP quote command error";
        case 22: return "HTTP server returned an error";
        case 23: return "Could not write data to local file (disk full or permission denied)";
        case 25: return "FTP could not STOR file on server";
        case 26: return "Read error from local file";
        case 27: return "Out of memory";
        case 28: return "Connection timed out";
        case 33: return "Requested range not satisfiable";
        case 34: return "HTTP POST error";
        case 35: return "SSL/TLS handshake failed";
        case 36: return "FTP could not resume transfer (bad download/resume)";
        case 37: return "Could not read the specified file (permission denied)";
        case 38: return "LDAP bind failed";
        case 43: return "Internal error (function not found)";
        case 45: return "Interface error (could not use specified outgoing interface)";
        case 47: return "Too many redirects";
        case 51: return "Server's SSL certificate or SSH fingerprint was not OK";
        case 52: return "Server returned nothing (empty response)";
        case 53: return "SSL crypto engine not found";
        case 54: return "Cannot set SSL crypto engine as default";
        case 55: return "Error sending data to server";
        case 56: return "Failure in receiving network data (connection reset)";
        case 58: return "Problem with the local SSL certificate";
        case 59: return "Could not use the specified SSL cipher";
        case 60: return "SSL certificate problem (unable to verify; server may use self-signed cert)";
        case 61: return "Unrecognized transfer encoding";
        case 63: return "Maximum file size exceeded";
        case 67: return "Login denied (username/password rejected by server)";
        case 73: return "SSH encryption or key exchange error";
        case 77: return "SSL CA certificate problem (missing or inaccessible)";
        case 78: return "Remote file not found on server";
        case 79: return "SSH session error (unexpected disconnect)";
        case 80: return "Failed to shut down the SSL connection";
        case 82: return "Could not load CRL file";
        case 83: return "Issuer check against certificate failed";
        case 84: return "FTP PRET command failed";
        case 85: return "RTSP CSeq mismatch or invalid";
        case 86: return "RTSP session error";
        case 87: return "Cannot parse FTP file list";
        case 88: return "FTP chunk callback error";
        case 89: return "No connection available (connection pool empty)";
        case 90: return "SSL client certificate required by server";
        case 92: return "Stream error in HTTP/2 framing layer";
        default:
            return "Transfer failed (curl error code " + std::to_string(exitCode) + ")";
    }
}

// Build a user-friendly error message from curl output and exit code.
// Checks stderr output for known patterns first, then falls back to exit code.
static string buildDownloadErrorMsg(const string& stderrOutput, int systemRet,
                                     const string& host, bool isFtp)
{
    int code = curlExitCode(systemRet);

    // Check for BBS-specific messages in verbose output
    if (stderrOutput.find("No QWK packet") != string::npos ||
        stderrOutput.find("no new messages") != string::npos ||
        stderrOutput.find("No new messages") != string::npos)
    {
        return "No new messages available (no QWK packet created).";
    }

    // Check for SFTP-specific errors
    if (!isFtp)
    {
        if (stderrOutput.find("Failure initializing sftp") != string::npos ||
            stderrOutput.find("SFTP subsystem") != string::npos)
        {
            return "The remote system does not support SFTP file transfers.";
        }
    }

    // Check for authentication failure patterns in FTP verbose output
    if (stderrOutput.find("530") != string::npos)
    {
        return "Login denied by " + host + " (incorrect username or password).";
    }

    // Check for permission denied
    if (stderrOutput.find("550") != string::npos &&
        stderrOutput.find("denied") != string::npos)
    {
        return "Permission denied by " + host + " for the requested file.";
    }

    // Use the curl exit code for a human-readable message
    if (code != 0)
    {
        return curlErrorMessage(code);
    }

    return "Downloaded file is empty or missing.";
}

// ============================================================
// File download functions
// ============================================================

// Run a system command while showing a spinning indicator on screen.
// Uses popen to detect when the process finishes while animating.
// Returns the process exit code (or -1 on error).
static int systemWithSpinner(const string& cmd, int spinRow, int spinCol)
{
    static const char spinChars[] = "|/-\\";
    TermAttr spinAttr = tAttr(TC_YELLOW, TC_BLACK, true);
    int spinIdx = 0;

    // Show initial spinner frame
    g_term->setAttr(spinAttr);
    g_term->putCh(spinRow, spinCol, spinChars[0]);
    g_term->refresh();

    // Use popen to run the command. For download commands, stdout is empty
    // (file goes to -o, errors to stderr file), but popen lets us detect
    // when the process finishes while we animate the spinner.
    // We append a known marker to stdout so we get at least one line to read.
#if SLYMAIL_WINDOWS
    string wrappedCmd = cmd + " & echo __DONE";
#else
    string wrappedCmd = cmd + " ; echo __DONE";
#endif

    FILE* pipe = sm_popen(wrappedCmd.c_str(), "r");
    if (!pipe)
    {
        return system(cmd.c_str()); // Fallback to blocking system()
    }

    // Read output line by line, updating spinner each time we check
    char buf[256];
    while (fgets(buf, sizeof(buf), pipe) != nullptr)
    {
        // Animate spinner
        g_term->setAttr(spinAttr);
        g_term->putCh(spinRow, spinCol, spinChars[spinIdx % 4]);
        g_term->refresh();
        ++spinIdx;
    }

    int ret = sm_pclose(pipe);

    // Clear spinner
    g_term->putCh(spinRow, spinCol, ' ');
    g_term->refresh();

#if SLYMAIL_WINDOWS
    return ret;
#else
    // POSIX: extract exit code
    if (ret >= 0 && (ret & 0x7F) == 0)
    {
        return (ret >> 8) & 0xFF;
    }
    return ret;
#endif
}

// Read stderr from a file, then remove the file
static string readAndRemoveStderr(const string& path)
{
    string output;
    std::ifstream sf(path);
    if (sf.is_open())
    {
        output.assign((std::istreambuf_iterator<char>(sf)),
                       std::istreambuf_iterator<char>());
        sf.close();
    }
    try { fs::remove(path); } catch (...) {}
    return output;
}

// Download a file via FTP using curl.
// Captures stderr to a temp file for error detection.
// errMsg is set on failure with a descriptive message.
static bool ftpDownload(const RemoteSystem& sys, const string& remoteFilePath,
                        const string& localPath, string& errMsg)
{
    string url = ftpUrl(sys, remoteFilePath);
    string stderrFile = localPath + ".stderr";

    // Use -v (verbose) so that FTP server response messages (like 550 errors)
    // are captured in stderr alongside curl's own error messages
    string cmd = "curl -sS -v --user \"" + sys.username + ":" + sys.password + "\"";
    if (sys.passiveFTP) cmd += " --ftp-pasv";
    cmd += " --connect-timeout 10 --max-time 300";
    cmd += " -o \"" + localPath + "\" \"" + url + "\"";
    cmd += " 2>\"" + stderrFile + "\"";

    int rows = g_term->getRows();
    int cols = g_term->getCols();
    int ret = systemWithSpinner(cmd, rows / 2, cols / 2 + 12);
    string stderrOutput = readAndRemoveStderr(stderrFile);

    if (ret == 0 && fs::exists(localPath) && fs::file_size(localPath) > 0)
    {
        return true;
    }

    errMsg = buildDownloadErrorMsg(stderrOutput, ret, sys.host, true);
    return false;
}

// Download a file via SFTP using curl.
// errMsg is set on failure with a descriptive message.
static bool sftpDownload(const RemoteSystem& sys, const string& remoteFilePath,
                         const string& localPath, string& errMsg)
{
    string url = sftpUrl(sys, remoteFilePath);
    string stderrFile = localPath + ".stderr";

    string cmd = "curl -sS -v --user \"" + sys.username + ":" + sys.password + "\""
               + " --insecure --connect-timeout 15 --max-time 300"
               + " -o \"" + localPath + "\" \"" + url + "\""
               + " 2>\"" + stderrFile + "\"";

    int rows = g_term->getRows();
    int cols = g_term->getCols();
    int ret = systemWithSpinner(cmd, rows / 2, cols / 2 + 12);
    string stderrOutput = readAndRemoveStderr(stderrFile);

    if (ret == 0 && fs::exists(localPath) && fs::file_size(localPath) > 0)
    {
        return true;
    }

    errMsg = buildDownloadErrorMsg(stderrOutput, ret, sys.host, false);
    return false;
}

// ============================================================
// Remote file browser
// ============================================================

string connectAndBrowse(RemoteSystem& sys, const string& downloadDir)
{
    int cols = g_term->getCols();
    int rows = g_term->getRows();

    string currentPath = sys.remotePath.empty() ? "/" : sys.remotePath;

    // Show connecting message
    g_term->clear();
    printCentered(rows / 2, "Connecting to " + sys.name + " (" + sys.host + ")...",
                  tAttr(TC_CYAN, TC_BLACK, true));
    g_term->refresh();

    // List the initial directory
    vector<FileEntry> entries;
    string connErrMsg;
    if (sys.type == RemoteConnType::FTP)
    {
        entries = ftpListDir(sys, currentPath);
    }
    else
    {
        entries = sftpListDir(sys, currentPath, &connErrMsg);
    }

    if (entries.empty())
    {
        // Try listing root as fallback (skip if we got a specific SFTP error)
        if (connErrMsg.empty() && currentPath != "/")
        {
            currentPath = "/";
            if (sys.type == RemoteConnType::FTP)
                entries = ftpListDir(sys, currentPath);
            else
                entries = sftpListDir(sys, currentPath, &connErrMsg);
        }
        if (entries.empty())
        {
            if (!connErrMsg.empty())
            {
                messageDialog("Connection Error", connErrMsg);
            }
            else
            {
                messageDialog("Connection Error",
                    "Failed to connect or list directory on " + sys.host);
            }
            return "";
        }
    }

    // Update last connected time
    sys.lastConnected = currentTimestamp();

    int selected = 0;
    int scrollOffset = 0;

    TermAttr borderAttr = tAttr(TC_BLUE, TC_BLACK, true);
    TermAttr titleAttr = tAttr(TC_GREEN, TC_BLACK, true);
    TermAttr dirAttr = tAttr(TC_YELLOW, TC_BLACK, true);
    TermAttr fileAttr = tAttr(TC_CYAN, TC_BLACK, false);
    TermAttr selAttr = tAttr(TC_WHITE, TC_BLUE, true);
    TermAttr pathAttr = tAttr(TC_WHITE, TC_BLACK, true);
    TermAttr statusAttr = tAttr(TC_CYAN, TC_BLACK, false);

    while (true)
    {
        g_term->clear();

        // Title bar
        string title = " " + sys.name + " - " + sys.host + " ";
        printAt(0, 0, string(cols, ' '), tAttr(TC_WHITE, TC_BLUE, true));
        printCentered(0, title, tAttr(TC_WHITE, TC_BLUE, true));

        // Current path
        printAt(1, 0, " Path: " + currentPath, pathAttr);

        // Column headers
        g_term->setAttr(borderAttr);
        g_term->drawHLine(2, 0, cols);
        printAt(2, 1, " Name", titleAttr);
        printAt(2, cols - 20, "Size", titleAttr);

        int listTop = 3;
        int listHeight = rows - 5;
        int totalEntries = static_cast<int>(entries.size());

        // Add virtual ".." entry at the top for going up
        bool hasParent = (currentPath != "/" && currentPath.size() > 1);

        int totalItems = totalEntries + (hasParent ? 1 : 0);

        if (selected < scrollOffset) scrollOffset = selected;
        if (selected >= scrollOffset + listHeight) scrollOffset = selected - listHeight + 1;

        for (int i = 0; i < listHeight && (scrollOffset + i) < totalItems; ++i)
        {
            int idx = scrollOffset + i;
            int y = listTop + i;
            bool isSel = (idx == selected);

            if (isSel)
            {
                fillRow(y, selAttr, 0, cols);
            }
            else
            {
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), 0, cols);
            }

            if (hasParent && idx == 0)
            {
                // ".." entry
                printAt(y, 1, "[..]", isSel ? selAttr : dirAttr);
            }
            else
            {
                int entryIdx = idx - (hasParent ? 1 : 0);
                if (entryIdx >= 0 && entryIdx < totalEntries)
                {
                    const auto& fe = entries[entryIdx];
                    TermAttr nameAttr = isSel ? selAttr : (fe.isDirectory ? dirAttr : fileAttr);
                    string displayName = fe.name;
                    if (fe.isDirectory)
                    {
                        displayName = "[" + displayName + "]";
                    }
                    int maxNameLen = cols - 22;
                    if (static_cast<int>(displayName.size()) > maxNameLen)
                    {
                        displayName = displayName.substr(0, maxNameLen - 3) + "...";
                    }
                    printAt(y, 1, displayName, nameAttr);
                    if (!fe.isDirectory)
                    {
                        printAt(y, cols - 20, formatSize(fe.fileSize),
                                isSel ? selAttr : statusAttr);
                    }
                }
            }
        }

        // Scrollbar
        if (totalItems > listHeight)
        {
            drawScrollbar(listTop, listHeight, selected, totalItems,
                         tAttr(TC_BLACK, TC_BLACK, true),
                         tAttr(TC_WHITE, TC_BLACK, true));
        }

        // Help bar
        drawDDHelpBar(rows - 1, "Up/Dn/PgUp/PgDn, ",
                      {{'E', "nter=Select"}, {'U', "pload REP"}, {'/', "=Root"}, {'Q', "uit"}, {'?', ""}});

        // Status
        printAt(rows - 2, 0, string(cols, ' '), statusAttr);
        string statusMsg = std::to_string(totalEntries) + " item(s)";
        printAt(rows - 2, 1, statusMsg, statusAttr);

        g_term->refresh();

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:
                if (selected > 0) --selected;
                break;
            case TK_DOWN:
                if (selected < totalItems - 1) ++selected;
                break;
            case TK_PGUP:
                selected -= listHeight;
                if (selected < 0) selected = 0;
                break;
            case TK_PGDN:
                selected += listHeight;
                if (selected >= totalItems) selected = totalItems - 1;
                break;
            case TK_HOME:
                selected = 0;
                scrollOffset = 0;
                break;
            case TK_END:
                selected = totalItems - 1;
                break;
            case '/':
            {
                // Go to root
                currentPath = "/";
                g_term->clear();
                printCentered(rows / 2, "Loading /...", tAttr(TC_CYAN, TC_BLACK, true));
                g_term->refresh();
                if (sys.type == RemoteConnType::FTP)
                    entries = ftpListDir(sys, currentPath);
                else
                    entries = sftpListDir(sys, currentPath);
                selected = 0;
                scrollOffset = 0;
                break;
            }
            case TK_ENTER:
            {
                if (hasParent && selected == 0)
                {
                    // Go up one directory
                    fs::path p(currentPath);
                    currentPath = p.parent_path().string();
                    if (currentPath.empty()) currentPath = "/";
                    g_term->clear();
                    printCentered(rows / 2, "Loading " + currentPath + "...",
                                  tAttr(TC_CYAN, TC_BLACK, true));
                    g_term->refresh();
                    if (sys.type == RemoteConnType::FTP)
                        entries = ftpListDir(sys, currentPath);
                    else
                        entries = sftpListDir(sys, currentPath);
                    selected = 0;
                    scrollOffset = 0;
                }
                else
                {
                    int entryIdx = selected - (hasParent ? 1 : 0);
                    if (entryIdx >= 0 && entryIdx < totalEntries)
                    {
                        const auto& fe = entries[entryIdx];
                        if (fe.isDirectory)
                        {
                            // Navigate into directory
                            if (currentPath.back() != '/')
                                currentPath += "/";
                            currentPath += fe.name;
                            g_term->clear();
                            printCentered(rows / 2, "Loading " + currentPath + "...",
                                          tAttr(TC_CYAN, TC_BLACK, true));
                            g_term->refresh();
                            if (sys.type == RemoteConnType::FTP)
                                entries = ftpListDir(sys, currentPath);
                            else
                                entries = sftpListDir(sys, currentPath);
                            selected = 0;
                            scrollOffset = 0;
                        }
                        else
                        {
                            // Download file
                            // Build the remote path — avoid double slashes
                            string remoteFull;
                            if (currentPath == "/" || currentPath.empty())
                            {
                                remoteFull = "/" + fe.name;
                            }
                            else
                            {
                                remoteFull = currentPath;
                                if (remoteFull.back() != '/') remoteFull += "/";
                                remoteFull += fe.name;
                            }

                            string localPath = downloadDir + PATH_SEP_STR + fe.name;

                            g_term->clear();
                            printCentered(rows / 2,
                                "Downloading " + fe.name + "...",
                                tAttr(TC_CYAN, TC_BLACK, true));
                            g_term->refresh();

                            string dlErrMsg;
                            bool ok;
                            if (sys.type == RemoteConnType::FTP)
                                ok = ftpDownload(sys, remoteFull, localPath, dlErrMsg);
                            else
                                ok = sftpDownload(sys, remoteFull, localPath, dlErrMsg);

                            if (ok)
                            {
                                return localPath;
                            }
                            else
                            {
                                // Clean up any empty/partial file
                                try { fs::remove(localPath); } catch (...) {}
                                messageDialog("Download Error", dlErrMsg);
                            }
                        }
                    }
                }
                break;
            }
            case 'u': case 'U':
            {
                // Upload a reply packet
                string repDir = getSlyMailDataDir() + PATH_SEP_STR + "REP";
                string repFile = showFileBrowser(repDir, "", ".rep");
                if (!repFile.empty())
                {
                    // Build the remote destination path
                    string remoteFile = currentPath;
                    if (!remoteFile.empty() && remoteFile.back() != '/') remoteFile += "/";
                    remoteFile += fs::path(repFile).filename().string();

                    g_term->clear();
                    printCentered(rows / 2,
                        "Uploading " + fs::path(repFile).filename().string() + "...",
                        tAttr(TC_CYAN, TC_BLACK, true));
                    g_term->refresh();

                    string uploadErr;
                    if (uploadFileToRemote(sys, repFile, remoteFile, uploadErr))
                    {
                        messageDialog("Upload Complete",
                            fs::path(repFile).filename().string() + " uploaded successfully.");
                        if (confirmDialog("Delete local file " + fs::path(repFile).filename().string() + "?"))
                        {
                            try
                            {
                                fs::remove(repFile);
                            }
                            catch (const fs::filesystem_error& e)
                            {
                                messageDialog("Delete Error",
                                    "Failed to delete file: " + string(e.what()));
                            }
                            catch (...)
                            {
                                messageDialog("Delete Error",
                                    "Failed to delete " + fs::path(repFile).filename().string());
                            }
                        }
                    }
                    else
                    {
                        messageDialog("Upload Error", uploadErr);
                    }
                }
                break;
            }
            case '?':
            case TK_F1:
            {
                g_term->clear();
                int r = 1;
                drawProgramInfoLine(r++);
                r++;
                printCentered(r++, "Remote File Browser Help",
                    tAttr(TC_GREEN, TC_BLACK, true));
                r++;
                TermAttr keyC  = tAttr(TC_CYAN, TC_BLACK, true);
                TermAttr descC = tAttr(TC_CYAN, TC_BLACK, false);
                auto helpLine = [&](const string& key, const string& desc)
                {
                    printAt(r, 2, padStr(key, 20), keyC);
                    printAt(r, 24, ": " + desc, descC);
                    ++r;
                };
                helpLine("Up/Down arrow", "Navigate files and directories");
                helpLine("PageUp/PageDown", "Scroll up/down a page");
                helpLine("HOME/END", "Jump to first/last entry");
                helpLine("Enter", "Open directory or download file");
                helpLine("U", "Upload a reply packet (.rep)");
                helpLine("/", "Go to root directory");
                helpLine("Q / ESC / Ctrl-C", "Disconnect and go back");
                helpLine("? / F1", "Show this help screen");
                r += 2;
                string connInfo = "Connected to: " + sys.name + " (" + sys.host + ")";
                printAt(r++, 2, connInfo, tAttr(TC_WHITE, TC_BLACK, false));
                string typeInfo = "Type: " + string(sys.type == RemoteConnType::SSH ? "SSH/SFTP" : "FTP");
                printAt(r++, 2, typeInfo, tAttr(TC_WHITE, TC_BLACK, false));
                r++;
                printAt(r, 2, "Hit a key", tAttr(TC_GREEN, TC_BLACK, false));
                g_term->refresh();
                g_term->getKey();
                break;
            }
            case 'q': case 'Q':
            case TK_ESCAPE:
            case TK_CTRL_C:
                return "";
            default:
                break;
        }
    }
}

// ============================================================
// Remote systems list UI
// ============================================================

string showRemoteSystems(const string& dataDir, const string& downloadDir)
{
    auto systems = loadRemoteSystems(dataDir);

    int cols = g_term->getCols();
    int rows = g_term->getRows();
    int selected = 0;
    int scrollOffset = 0;

    TermAttr borderAttr = tAttr(TC_BLUE, TC_BLACK, true);
    TermAttr titleAttr = tAttr(TC_GREEN, TC_BLACK, true);
    TermAttr nameAttr = tAttr(TC_CYAN, TC_BLACK, true);
    TermAttr typeAttr = tAttr(TC_YELLOW, TC_BLACK, false);
    TermAttr dateAttr = tAttr(TC_GREEN, TC_BLACK, false);
    TermAttr selAttr = tAttr(TC_WHITE, TC_BLUE, true);
    TermAttr emptyAttr = tAttr(TC_WHITE, TC_BLACK, false);

    while (true)
    {
        g_term->clear();
        int totalSys = static_cast<int>(systems.size());

        // Title
        printAt(0, 0, string(cols, ' '), tAttr(TC_WHITE, TC_BLUE, true));
        printCentered(0, " Remote Systems ", tAttr(TC_WHITE, TC_BLUE, true));

        // Column headers
        g_term->setAttr(borderAttr);
        g_term->drawHLine(1, 0, cols);
        printAt(1, 2, " Name", titleAttr);
        printAt(1, cols / 2 - 5, "Type", titleAttr);
        printAt(1, cols - 22, "Last Connected", titleAttr);

        int listTop = 2;
        int listHeight = rows - 4;

        if (totalSys == 0)
        {
            printCentered(rows / 2, "No remote systems configured.", emptyAttr);
            printCentered(rows / 2 + 1, "Press A to add a new system.", emptyAttr);
        }
        else
        {
            if (selected < scrollOffset) scrollOffset = selected;
            if (selected >= scrollOffset + listHeight) scrollOffset = selected - listHeight + 1;

            for (int i = 0; i < listHeight && (scrollOffset + i) < totalSys; ++i)
            {
                int idx = scrollOffset + i;
                int y = listTop + i;
                bool isSel = (idx == selected);
                const auto& s = systems[idx];

                if (isSel)
                {
                    fillRow(y, selAttr, 0, cols);
                }
                else
                {
                    fillRow(y, tAttr(TC_BLACK, TC_BLACK, false), 0, cols);
                }

                string displayName = s.name;
                int maxNameLen = cols / 2 - 8;
                if (static_cast<int>(displayName.size()) > maxNameLen)
                {
                    displayName = displayName.substr(0, maxNameLen - 3) + "...";
                }
                printAt(y, 2, displayName, isSel ? selAttr : nameAttr);

                string typeStr = (s.type == RemoteConnType::SSH) ? "SSH" : "FTP";
                printAt(y, cols / 2 - 5, typeStr, isSel ? selAttr : typeAttr);

                string dateStr = s.lastConnected.empty() ? "Never" : s.lastConnected;
                printAt(y, cols - 22, dateStr, isSel ? selAttr : dateAttr);
            }

            // Scrollbar
            if (totalSys > listHeight)
            {
                drawScrollbar(listTop, listHeight, selected, totalSys,
                             tAttr(TC_BLACK, TC_BLACK, true),
                             tAttr(TC_WHITE, TC_BLACK, true));
            }
        }

        // Help bar
        drawDDHelpBar(rows - 1, "",
                      {{'A', "dd"}, {'E', "dit"}, {'D', "elete"},
                       {'Q', "uit"}});

        g_term->refresh();

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:
                if (selected > 0) --selected;
                break;
            case TK_DOWN:
                if (selected < totalSys - 1) ++selected;
                break;
            case TK_PGUP:
                selected -= listHeight;
                if (selected < 0) selected = 0;
                break;
            case TK_PGDN:
                selected += listHeight;
                if (selected >= totalSys) selected = totalSys - 1;
                if (selected < 0) selected = 0;
                break;
            case TK_HOME:
                selected = 0;
                break;
            case TK_END:
                if (totalSys > 0) selected = totalSys - 1;
                break;
            case 'a': case 'A':
            {
                // Add new system
                RemoteSystem newSys;
                if (editRemoteSystem(newSys))
                {
                    systems.push_back(newSys);
                    saveRemoteSystems(dataDir, systems);
                    selected = static_cast<int>(systems.size()) - 1;
                }
                break;
            }
            case 'e': case 'E':
            {
                // Edit selected system
                if (selected >= 0 && selected < totalSys)
                {
                    if (editRemoteSystem(systems[selected]))
                    {
                        saveRemoteSystems(dataDir, systems);
                    }
                }
                break;
            }
            case 'd': case 'D': case TK_DELETE:
            {
                // Delete selected system
                if (selected >= 0 && selected < totalSys)
                {
                    if (confirmDialog("Delete '" + systems[selected].name + "'?"))
                    {
                        systems.erase(systems.begin() + selected);
                        saveRemoteSystems(dataDir, systems);
                        if (selected >= static_cast<int>(systems.size()))
                        {
                            selected = static_cast<int>(systems.size()) - 1;
                        }
                        if (selected < 0) selected = 0;
                    }
                }
                break;
            }
            case TK_ENTER:
            {
                // Connect to selected system
                if (selected >= 0 && selected < totalSys)
                {
                    string result = connectAndBrowse(systems[selected], downloadDir);
                    // Save updated lastConnected timestamp
                    saveRemoteSystems(dataDir, systems);
                    if (!result.empty())
                    {
                        return result;
                    }
                }
                break;
            }
            case 'q': case 'Q':
            case TK_ESCAPE:
            case TK_CTRL_C:
                return "";
            default:
                break;
        }
    }
}

// ============================================================
// Upload a file to a remote system via FTP or SFTP
// Uses curl -T for both protocols (cross-platform: Windows, Linux, macOS, BSD)
// ============================================================

bool uploadFileToRemote(const RemoteSystem& sys, const string& localPath,
                        const string& remotePath, string& errMsg)
{
    if (!fs::exists(localPath))
    {
        errMsg = "Local file not found: " + localPath;
        return false;
    }

    string remoteUrl;
    string cmd;
    string stderrFile = localPath + ".upload_stderr";

    if (sys.type == RemoteConnType::FTP)
    {
        remoteUrl = ftpUrl(sys, remotePath);
        cmd = "curl -sS -v --user \"" + sys.username + ":" + sys.password + "\"";
        if (sys.passiveFTP) cmd += " --ftp-pasv";
        cmd += " --connect-timeout 10 --max-time 300";
        cmd += " -T \"" + localPath + "\" \"" + remoteUrl + "\"";
        cmd += " 2>\"" + stderrFile + "\"";
    }
    else
    {
        remoteUrl = sftpUrl(sys, remotePath);
        cmd = "curl -sS -v --user \"" + sys.username + ":" + sys.password + "\""
            + " --insecure --connect-timeout 15 --max-time 300"
            + " -T \"" + localPath + "\" \"" + remoteUrl + "\""
            + " 2>\"" + stderrFile + "\"";
    }

    int rows = g_term->getRows();
    int cols = g_term->getCols();
    int ret = systemWithSpinner(cmd, rows / 2, cols / 2 + 12);
    string stderrOutput = readAndRemoveStderr(stderrFile);

    bool isFtp = (sys.type == RemoteConnType::FTP);
    int exitCode = curlExitCode(ret);
    if (exitCode == 0)
    {
        return true;
    }

    errMsg = buildDownloadErrorMsg(stderrOutput, ret, sys.host, isFtp);
    return false;
}
