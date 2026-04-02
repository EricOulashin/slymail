#ifndef SLYMAIL_REMOTE_SYSTEMS_H
#define SLYMAIL_REMOTE_SYSTEMS_H

// Remote Systems - FTP/SSH connection directory for downloading QWK packets
//
// Provides a UI for managing remote BBS/system connections and
// browsing/downloading QWK files from them via FTP or SFTP (SSH).
// Uses the system's `curl` command for FTP and `sftp` for SSH transfers.
// Remote system entries are persisted to a JSON file.

#include <string>
#include <vector>

// Connection type
enum class RemoteConnType
{
    FTP,
    SSH
};

// A remote system entry
struct RemoteSystem
{
    std::string name;           // Display name for the connection
    RemoteConnType type;        // FTP or SSH
    std::string host;           // Hostname or IP address
    int         port;           // Port number (0 = default: 21 for FTP, 22 for SSH)
    std::string username;       // Login username
    std::string password;       // Login password
    bool        passiveFTP;     // Use passive mode for FTP
    std::string remotePath;     // Initial remote directory path
    std::string lastConnected;  // Date/time of last connection (ISO format)

    RemoteSystem()
        : type(RemoteConnType::FTP), port(0), passiveFTP(true)
    {
    }
};

// Load remote systems from JSON file
std::vector<RemoteSystem> loadRemoteSystems(const std::string& dataDir);

// Save remote systems to JSON file
bool saveRemoteSystems(const std::string& dataDir,
                       const std::vector<RemoteSystem>& systems);

// Show the remote systems directory UI.
// Returns the local path of a downloaded QWK file, or empty string if cancelled.
// downloadDir is the directory to save downloaded QWK files into.
std::string showRemoteSystems(const std::string& dataDir,
                              const std::string& downloadDir);

// Show the editor dialog for a single remote system entry.
// Returns true if the user saved changes.
bool editRemoteSystem(RemoteSystem& sys);

// Connect to a remote system and browse/download a QWK file.
// Returns local path of downloaded file, or empty string if cancelled/failed.
std::string connectAndBrowse(RemoteSystem& sys, const std::string& downloadDir);

// Upload a local file to a remote system's current directory.
// Returns true on success, sets errMsg on failure.
bool uploadFileToRemote(const RemoteSystem& sys, const std::string& localPath,
                        const std::string& remotePath, std::string& errMsg);

#endif // SLYMAIL_REMOTE_SYSTEMS_H
