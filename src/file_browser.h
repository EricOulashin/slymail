#ifndef SLYMAIL_FILE_BROWSER_H
#define SLYMAIL_FILE_BROWSER_H

#include "terminal.h"
#include "colors.h"
#include "ui_common.h"
#include <algorithm>

#if !SLYMAIL_WINDOWS
    #include <sys/stat.h>
#else
    #include <sys/stat.h>   // _stat on MSVC
#endif

struct FileEntry
{
    std::string name;
    std::string fullPath;
    bool        isDirectory;
    uintmax_t   fileSize;
    std::string dateStr;

    bool operator<(const FileEntry& other) const
    {
        if (isDirectory != other.isDirectory)
        {
            return isDirectory > other.isDirectory;
        }
        return name < other.name;
    }
};

std::string formatSize(uintmax_t size);

std::vector<FileEntry> listDirectory(const std::string& dirPath);

bool isQwkFile(const std::string& name);

// File browser dialog styled to match the DDMsgReader look
std::string showFileBrowser(const std::string& startDir = "",
                            const std::string& preSelectFile = "");

#endif // SLYMAIL_FILE_BROWSER_H
