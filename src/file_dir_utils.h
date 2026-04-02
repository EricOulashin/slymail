#ifndef __FILE_DIR_UTILS_H__
#define __FILE_DIR_UTILS_H__

#include <string>

#ifdef _WIN32
    #define PATH_SEP '\\'
    #define PATH_SEP_STR "\\"
#else
    #define PATH_SEP '/'
    #define PATH_SEP_STR "/"
#endif

// Gets the user's home directory in a cross-platform way
std::string getHomeDir();

// Gets the SlyMail data directory (~/.slymail) and ensures it exists
std::string getSlyMailDataDir();

// Get user's config directory for settings
std::string getConfigDir();

// Get a temporary directory for QWK extraction
std::string getTempDir();

// Clean up a temporary directory
void cleanupTempDir(const std::string& dir);

// Extract a QWK file (ZIP archive) to a destination directory
std::string extractQwkPacket(const std::string& qwkPath, const std::string& destDir);

// Create a ZIP archive (for REP packets)
bool createZipArchive(const std::string& zipPath, const std::string& sourceDir);

#endif