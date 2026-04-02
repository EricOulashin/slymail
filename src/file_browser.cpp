#include "file_browser.h"
#include "remote_systems.h"
#include <cctype>

using std::string;
using std::vector;

string formatSize(uintmax_t size)
{
    if (size < 1024)
    {
        return std::to_string(size) + " B";
    }
    if (size < 1024 * 1024)
    {
        return std::to_string(size / 1024) + " KB";
    }
    if (size < 1024ULL * 1024 * 1024)
    {
        return std::to_string(size / (1024 * 1024)) + " MB";
    }
    return std::to_string(size / (1024ULL * 1024 * 1024)) + " GB";
}

vector<FileEntry> listDirectory(const string& dirPath)
{
    vector<FileEntry> entries;
    try
    {
        for (const auto& entry : fs::directory_iterator(dirPath))
        {
            FileEntry fe;
            fe.name = entry.path().filename().string();
            fe.fullPath = entry.path().string();
            fe.isDirectory = entry.is_directory();
            if (!fe.isDirectory)
            {
                try
                {
                    fe.fileSize = entry.file_size();
                }
                catch (...)
                {
                    fe.fileSize = 0;
                }
            }
            else
            {
                fe.fileSize = 0;
            }
            try
            {
                struct stat st;
                if (stat(fe.fullPath.c_str(), &st) == 0)
                {
                    struct tm* tm = localtime(&st.st_mtime);
                    char buf[32];
                    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", tm);
                    fe.dateStr = buf;
                }
            }
            catch (...)
            {
                fe.dateStr = "";
            }

            if (!fe.name.empty() && fe.name[0] == '.' && fe.name != "..")
            {
                continue;
            }
            entries.push_back(fe);
        }
    }
    catch (const std::exception&)
    {
    }

    FileEntry parent;
    parent.name = "..";
    parent.fullPath = fs::path(dirPath).parent_path().string();
    parent.isDirectory = true;
    parent.fileSize = 0;
    entries.push_back(parent);

    std::sort(entries.begin(), entries.end());
    return entries;
}

bool isQwkFile(const string& name)
{
    if (name.size() < 4)
    {
        return false;
    }
    string ext = name.substr(name.size() - 4);
    for (auto& c : ext)
    {
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    }
    return ext == ".qwk";
}

// Check if a filename matches an extension (case-insensitive)
static bool hasExtension(const string& name, const string& ext)
{
    if (ext.empty() || name.size() < ext.size()) return false;
    string fileTail = name.substr(name.size() - ext.size());
    for (auto& c : fileTail) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    string lowerExt = ext;
    for (auto& c : lowerExt) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return fileTail == lowerExt;
}

// File browser dialog styled to match the DDMsgReader look
string showFileBrowser(const string& startDir,
                       const string& preSelectFile,
                       const string& fileExtFilter)
{
    // Determine which extension to accept.
    // "*" means accept any file; empty defaults to ".qwk".
    bool acceptAny = (fileExtFilter == "*");
    string acceptExt = acceptAny ? "" : (fileExtFilter.empty() ? ".qwk" : fileExtFilter);
    string currentDir = startDir;
    if (currentDir.empty())
    {
        char* cwd = getcwd(nullptr, 0);
        if (cwd)
        {
            currentDir = cwd;
            free(cwd);
        }
        else
        {
            currentDir = "/";
        }
    }

    int selected      = 0;
    int scrollOffset  = 0;
    bool needPreSelect  = !preSelectFile.empty();
    bool needFullRedraw = true;
    bool needReloadDir  = true;
    int prevSelected     = -1;
    int prevScrollOffset = -1;
    vector<FileEntry> entries;

    while (true)
    {
        // Reload directory listing when the directory changes
        if (needReloadDir)
        {
            needReloadDir = false;
            entries = listDirectory(currentDir);

            // Pre-select a file if specified
            if (needPreSelect)
            {
                needPreSelect = false;
                string preSelectName = fs::path(preSelectFile).filename().string();
                for (int i = 0; i < static_cast<int>(entries.size()); ++i)
                {
                    if (entries[i].name == preSelectName)
                    {
                        selected = i;
                        break;
                    }
                }
            }

            if (entries.empty())
            {
                currentDir = fs::path(currentDir).parent_path().string();
                entries = listDirectory(currentDir);
                if (entries.empty())
                {
                    break;
                }
            }
            if (selected >= static_cast<int>(entries.size()))
            {
                selected = static_cast<int>(entries.size()) - 1;
            }
            if (selected < 0)
            {
                selected = 0;
            }
            needFullRedraw   = true;
            prevSelected     = -1;
            prevScrollOffset = -1;
        }

        int COLS  = g_term->getCols();
        int LINES = g_term->getRows();

        // List layout constants
        int listTop    = 4;
        int listHeight = LINES - 6;

        // Clamp scroll offset to keep selected in view
        if (selected < scrollOffset)
        {
            scrollOffset = selected;
        }
        if (selected >= scrollOffset + listHeight)
        {
            scrollOffset = selected - listHeight + 1;
        }

        // Lambda: draw a single file-list row
        auto drawRow = [&](int idx)
        {
            if (idx < 0 || idx >= static_cast<int>(entries.size())) return;
            if (idx < scrollOffset || idx >= scrollOffset + listHeight) return;

            int y = listTop + (idx - scrollOffset);
            const auto& entry = entries[idx];
            bool isSel = (idx == selected);

            TermAttr nameAttr, sizeAttr, dateAttr;
            if (isSel)
            {
                fillRow(y, tAttr(TC_BLUE, TC_WHITE, false));
                nameAttr = tAttr(TC_BLUE, TC_WHITE, false);
                sizeAttr = tAttr(TC_BLACK, TC_WHITE, false);
                dateAttr = tAttr(TC_BLACK, TC_WHITE, false);
            }
            else if (entry.isDirectory)
            {
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false));
                nameAttr = tAttr(TC_BLUE, TC_BLACK, true);
                sizeAttr = tAttr(TC_YELLOW, TC_BLACK, false);
                dateAttr = tAttr(TC_GREEN, TC_BLACK, false);
            }
            else if (acceptAny || hasExtension(entry.name, acceptExt))
            {
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false));
                nameAttr = tAttr(TC_GREEN, TC_BLACK, true);
                sizeAttr = tAttr(TC_YELLOW, TC_BLACK, false);
                dateAttr = tAttr(TC_GREEN, TC_BLACK, false);
            }
            else
            {
                fillRow(y, tAttr(TC_BLACK, TC_BLACK, false));
                nameAttr = tAttr(TC_CYAN, TC_BLACK, false);
                sizeAttr = tAttr(TC_YELLOW, TC_BLACK, false);
                dateAttr = tAttr(TC_GREEN, TC_BLACK, false);
            }

            string displayName = entry.name;
            if (entry.isDirectory && entry.name != "..")
            {
                displayName += "/";
            }
            printAt(y, 1, padStr(truncateStr(displayName, COLS - 32), COLS - 30), nameAttr);
            if (!entry.isDirectory)
            {
                printAt(y, COLS - 28, padStr(formatSize(entry.fileSize), 10), sizeAttr);
            }
            else
            {
                printAt(y, COLS - 28, padStr("<DIR>", 10), sizeAttr);
            }
            printAt(y, COLS - 18, padStr(entry.dateStr, 16), dateAttr);
        };

        // Lambda: draw scrollbar
        auto drawSB = [&]()
        {
            if (static_cast<int>(entries.size()) > listHeight)
            {
                drawScrollbar(listTop, listHeight, selected,
                             static_cast<int>(entries.size()),
                             tAttr(TC_BLACK, TC_BLACK, true),
                             tAttr(TC_WHITE, TC_BLACK, true));
            }
        };

        // Lambda: draw status line (changes with every navigation step)
        auto drawStatus = [&]()
        {
            string statusStr = std::to_string(selected + 1) + " of "
                + std::to_string(entries.size()) + " items";
            printAt(LINES - 2, 1, padStr(statusStr, 30), tAttr(TC_WHITE, TC_BLACK, false));
        };

        // --- Draw decision ---
        bool scrollChanged = (scrollOffset != prevScrollOffset);

        if (needFullRedraw || scrollChanged)
        {
            // Full redraw: static chrome + all visible rows
            TermAttr borderAttr = tAttr(TC_BLUE, TC_BLACK, true);
            TermAttr titleAttr  = tAttr(TC_CYAN, TC_BLACK, true);
            TermAttr pathAttr   = tAttr(TC_WHITE, TC_BLACK, true);

            g_term->clear();

            // Title header with border (DDMsgReader style)
            g_term->setAttr(borderAttr);
            g_term->putCP437(0, 0, CP437_BOX_DRAWINGS_UPPER_LEFT_SINGLE);
            g_term->drawHLine(0, 1, COLS - 2);
            g_term->putCP437(0, COLS - 1, CP437_BOX_DRAWINGS_UPPER_RIGHT_SINGLE);
            printAt(0, 4, " SlyMail - Select QWK File ", titleAttr);

            g_term->setAttr(borderAttr);
            g_term->putCP437(1, 0, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
            g_term->putCP437(1, COLS - 1, CP437_BOX_DRAWINGS_LIGHT_VERTICAL);
            printAt(1, 2, "Path: ", tAttr(TC_CYAN, TC_BLACK, false));
            printAt(1, 8, truncateStr(currentDir, COLS - 11), pathAttr);

            g_term->setAttr(borderAttr);
            g_term->putCP437(2, 0, CP437_BOX_DRAWINGS_LOWER_LEFT_SINGLE);
            g_term->drawHLine(2, 1, COLS - 2);
            g_term->putCP437(2, COLS - 1, CP437_BOX_DRAWINGS_LOWER_RIGHT_SINGLE);

            // Column headers
            int headerY = 3;
            TermAttr colHdrAttr = tAttr(TC_CYAN, TC_BLACK, true);
            printAt(headerY, 1, padStr("Name", COLS - 30), colHdrAttr);
            printAt(headerY, COLS - 28, padStr("Size", 10), colHdrAttr);
            printAt(headerY, COLS - 18, padStr("Date", 16), colHdrAttr);

            // All visible rows
            for (int i = 0; i < listHeight && (scrollOffset + i) < static_cast<int>(entries.size()); ++i)
            {
                drawRow(scrollOffset + i);
            }

            drawSB();
            drawStatus();
            drawDDHelpBar(LINES - 1, "Up/Dn/PgUp/PgDn/HOME/END, ",
                {{'Q', "uit"}, {'?', ""}});
            // Show Ctrl-R hint on the far right
            printAt(LINES - 1, COLS - 18, "Ctrl-R=Remote",
                    tAttr(TC_RED, TC_WHITE, false));

            needFullRedraw = false;
        }
        else if (selected != prevSelected)
        {
            // Partial update: only the two changed rows + scrollbar + status
            drawRow(prevSelected);
            drawRow(selected);
            drawSB();
            drawStatus();
        }

        g_term->refresh();
        prevSelected     = selected;
        prevScrollOffset = scrollOffset;

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:
                if (selected > 0)
                {
                    --selected;
                }
                break;
            case TK_DOWN:
                if (selected < static_cast<int>(entries.size()) - 1)
                {
                    ++selected;
                }
                break;
            case TK_PGUP:
                selected -= listHeight;
                if (selected < 0)
                {
                    selected = 0;
                }
                break;
            case TK_PGDN:
                selected += listHeight;
                if (selected >= static_cast<int>(entries.size()))
                {
                    selected = static_cast<int>(entries.size()) - 1;
                }
                break;
            case TK_HOME:
                selected = 0;
                break;
            case TK_END:
                selected = static_cast<int>(entries.size()) - 1;
                break;
            case TK_ENTER:
                if (selected >= 0 && selected < static_cast<int>(entries.size()))
                {
                    const auto& entry = entries[selected];
                    if (entry.isDirectory)
                    {
                        currentDir   = entry.fullPath;
                        selected     = 0;
                        scrollOffset = 0;
                        needReloadDir = true;
                    }
                    else if (acceptAny || hasExtension(entry.name, acceptExt))
                    {
                        return entry.fullPath;
                    }
                    else
                    {
                        messageDialog("Info", "Please select a " + acceptExt + " file");
                        needFullRedraw = true;
                    }
                }
                break;
            case TK_CTRL_R:
            {
                // Remote systems directory
                string dataDir = getSlyMailDataDir();
                string downloadDir = dataDir + PATH_SEP_STR + "QWK";
                string result = showRemoteSystems(dataDir, downloadDir);
                if (!result.empty())
                {
                    return result; // Downloaded QWK file path
                }
                needFullRedraw = true;
                break;
            }
            case '?':
            case TK_F1:
            {
                g_term->clear();
                int r = 1;
                drawProgramInfoLine(r++);
                r++;
                printCentered(r++, "File Browser Help",
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
                helpLine("Enter", "Open directory or select QWK file");
                helpLine("Ctrl-R", "Open remote systems directory");
                helpLine("Q / ESC", "Quit SlyMail");
                helpLine("? / F1", "Show this help screen");
                r += 2;
                printAt(r, 2, "Hit a key", tAttr(TC_GREEN, TC_BLACK, false));
                g_term->refresh();
                g_term->getKey();
                needFullRedraw = true;
                break;
            }
            case 'q':
            case 'Q':
            case TK_ESCAPE:
                return "";
            case TK_RESIZE:
                needFullRedraw = true;
                break;
            default:
                break;
        }
    }
    return "";
}

// ============================================================
// Directory chooser — shows only directories
// ============================================================

string showDirChooser(const string& startDir, const string& title)
{
    string currentDir = startDir;
    if (currentDir.empty())
    {
        currentDir = getSlyMailDataDir();
    }

    int selected = 0;
    int scrollOffset = 0;
    bool needFullRedraw = true;
    vector<FileEntry> entries;
    bool needReloadDir = true;

    while (true)
    {
        if (needReloadDir)
        {
            needReloadDir = false;
            // Only list directories
            entries.clear();
            try
            {
                for (const auto& entry : fs::directory_iterator(currentDir))
                {
                    if (!entry.is_directory()) continue;
                    FileEntry fe;
                    fe.name = entry.path().filename().string();
                    fe.fullPath = entry.path().string();
                    fe.isDirectory = true;
                    fe.fileSize = 0;
                    entries.push_back(fe);
                }
            }
            catch (...) {}

            // Add ".." for parent navigation
            if (currentDir != "/" && !currentDir.empty())
            {
                FileEntry parent;
                parent.name = "..";
                parent.fullPath = fs::path(currentDir).parent_path().string();
                parent.isDirectory = true;
                parent.fileSize = 0;
                entries.insert(entries.begin(), parent);
            }
            std::sort(entries.begin() + (entries.empty() ? 0 : 1), entries.end());
            selected = 0;
            scrollOffset = 0;
            needFullRedraw = true;
        }

        int COLS = g_term->getCols();
        int ROWS = g_term->getRows();
        int listTop = 3;
        int listHeight = ROWS - 5;
        int totalEntries = static_cast<int>(entries.size());

        if (selected < scrollOffset) scrollOffset = selected;
        if (selected >= scrollOffset + listHeight) scrollOffset = selected - listHeight + 1;

        if (needFullRedraw)
        {
            g_term->clear();

            // Title bar
            printAt(0, 0, " " + title + " ", tAttr(TC_WHITE, TC_BLUE, true));
            g_term->setAttr(tAttr(TC_WHITE, TC_BLUE, true));
            g_term->fillRegion(0, static_cast<int>(title.size()) + 2, COLS, ' ');

            // Current directory
            printAt(1, 0, " " + currentDir, tAttr(TC_CYAN, TC_BLACK, true));

            g_term->setAttr(tAttr(TC_BLUE, TC_BLACK, true));
            g_term->drawHLine(2, 0, COLS);

            for (int i = 0; i < listHeight && (scrollOffset + i) < totalEntries; ++i)
            {
                int idx = scrollOffset + i;
                int y = listTop + i;
                bool isSel = (idx == selected);
                const auto& fe = entries[idx];

                if (isSel)
                {
                    fillRow(y, tAttr(TC_WHITE, TC_BLUE, true));
                }
                else
                {
                    fillRow(y, tAttr(TC_BLACK, TC_BLACK, false));
                }

                TermAttr nameAttr = isSel ? tAttr(TC_WHITE, TC_BLUE, true)
                                          : tAttr(TC_YELLOW, TC_BLACK, true);
                printAt(y, 1, "[" + fe.name + "]", nameAttr);
            }

            // Help bar
            drawDDHelpBar(ROWS - 1, "Up/Dn, ",
                          {{'E', "nter=Open"}, {'S', "elect this dir"}, {'Q', "uit"}});

            // Status
            printAt(ROWS - 2, 0, " Press S or Enter+select to choose: " + currentDir,
                    tAttr(TC_GREEN, TC_BLACK, false));

            needFullRedraw = false;
        }

        g_term->refresh();

        int ch = g_term->getKey();
        switch (ch)
        {
            case TK_UP:
                if (selected > 0) --selected;
                needFullRedraw = true;
                break;
            case TK_DOWN:
                if (selected < totalEntries - 1) ++selected;
                needFullRedraw = true;
                break;
            case TK_PGUP:
                selected -= listHeight;
                if (selected < 0) selected = 0;
                needFullRedraw = true;
                break;
            case TK_PGDN:
                selected += listHeight;
                if (selected >= totalEntries) selected = totalEntries - 1;
                needFullRedraw = true;
                break;
            case TK_ENTER:
                if (selected >= 0 && selected < totalEntries)
                {
                    currentDir = entries[selected].fullPath;
                    needReloadDir = true;
                }
                break;
            case 's': case 'S':
            {
                // Select the current directory
                string absPath;
                try { absPath = fs::absolute(currentDir).string(); }
                catch (...) { absPath = currentDir; }
                return absPath;
            }
            case 'q': case 'Q': case TK_ESCAPE:
                return "";
            case TK_RESIZE:
                needFullRedraw = true;
                break;
            default:
                break;
        }
    }
}
