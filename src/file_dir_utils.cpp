#include "file_dir_utils.h"

#include <filesystem>

// Platform-specific headers for getpid(), getcwd(), etc.
#ifdef _WIN32
    #include <direct.h>
    #include <windows.h>
    #include <process.h>
    #define getcwd _getcwd
    #define getpid _getpid
#else
    #include <unistd.h>
    #include <sys/types.h>
#endif

using std::string;

namespace fs = std::filesystem;

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
    namespace fs = std::filesystem;

    string home = getHomeDir();
    string dataDir = home + PATH_SEP_STR + ".slymail";
    try
    {
        fs::create_directories(dataDir);
        // Also create standard subdirectories
        fs::create_directories(dataDir + PATH_SEP_STR + "QWK");
        fs::create_directories(dataDir + PATH_SEP_STR + "REP");
        fs::create_directories(dataDir + PATH_SEP_STR + "config_files");
        fs::create_directories(dataDir + PATH_SEP_STR + "dictionary_files");
        fs::create_directories(dataDir + PATH_SEP_STR + "tagline_files");
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

string getConfigDir()
{
#if SLYMAIL_WINDOWS
    const char* appdata = getenv("APPDATA");
    if (appdata)
    {
        string dir = string(appdata) + "\\SlyMail";
        fs::create_directories(dir);
        return dir;
    }
    return ".";
#else
    const char* home = getenv("HOME");
    if (home)
    {
        string dir = string(home) + "/.config/slymail";
        fs::create_directories(dir);
        return dir;
    }
    return ".";
#endif
}

string getTempDir()
{
#if SLYMAIL_WINDOWS
    char buf[260];
    GetTempPathA(260, buf);
    string dir = string(buf) + "slymail_tmp";
#else
    string dir = "/tmp/slymail_tmp_" + std::to_string(getpid());
#endif
    fs::create_directories(dir);
    return dir;
}

void cleanupTempDir(const string& dir)
{
    if (!dir.empty())
    {
        fs::remove_all(dir);
    }
}

string extractQwkPacket(const string& qwkPath, const string& destDir)
{
#if SLYMAIL_WINDOWS
    string cmd;
    if (isTarAvailable())
    {
        // tar reads ZIP magic bytes and ignores the file extension, so .qwk
        // files extract fine without any rename.
        try { fs::remove_all(destDir); } catch (...) {}
        fs::create_directories(destDir);
        cmd = "tar -xf \"" + qwkPath + "\" -C \"" + destDir + "\" >NUL 2>&1";
    }
    else
    {
        // Fallback: PowerShell's Expand-Archive rejects non-.zip extensions,
        // so use the .NET ZipFile class which works with any file extension.
        cmd = "powershell -Command \""
            "if (Test-Path '" + destDir + "') { Remove-Item '" + destDir + "' -Recurse -Force }; "
            "Add-Type -AssemblyName System.IO.Compression.FileSystem; "
            "[System.IO.Compression.ZipFile]::ExtractToDirectory('"
            + qwkPath + "', '" + destDir + "')\" 2>NUL";
    }
#else
    fs::create_directories(destDir);
    string cmd = "unzip -o -qq \"" + qwkPath + "\" -d \"" + destDir + "\" 2>/dev/null";
#endif
    int ret = system(cmd.c_str());
    if (ret != 0)
    {
        return "";
    }
    return destDir;
}

bool createZipArchive(const string& zipPath, const string& sourceDir)
{
    // Convert zipPath to absolute so it works after cd to sourceDir
    string absZipPath = zipPath;
    try
    {
        absZipPath = fs::absolute(zipPath).string();
    }
    catch (...)
    {
    }

    // Remove existing file first to avoid appending to old archive
    try
    {
        fs::remove(absZipPath);
    }
    catch (...)
    {
    }

#if SLYMAIL_WINDOWS
    // Both tar and Compress-Archive need a .zip output extension to produce a
    // valid ZIP archive.  Create to a temporary .zip path then rename.
    string tmpZip = absZipPath + ".tmp.zip";
    try { fs::remove(tmpZip); } catch (...) {}

    string cmd;
    if (isTarAvailable())
    {
        // tar -a auto-detects ZIP format from the .zip output extension.
        cmd = "tar -a -c -f \"" + tmpZip + "\" -C \"" + sourceDir + "\" . >NUL 2>&1";
    }
    else
    {
        // Fallback: PowerShell Compress-Archive.
        cmd = "powershell -Command \"Compress-Archive -Path '"
            + sourceDir + PATH_SEP_STR + "*' -DestinationPath '"
            + tmpZip + "'\" 2>NUL";
    }

    if (system(cmd.c_str()) != 0)
    {
        return false;
    }
    try
    {
        fs::rename(tmpZip, absZipPath);
    }
    catch (...)
    {
        try { fs::remove(tmpZip); } catch (...) {}
        return false;
    }
    return true;
#else
    string cmd = "cd \"" + sourceDir + "\" && zip -j -q \"" + absZipPath + "\" * 2>/dev/null";
    return system(cmd.c_str()) == 0;
#endif
}