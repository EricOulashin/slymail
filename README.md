# SlyMail

This is a cross-platform text-based offline mail reader for the [QWK](https://en.wikipedia.org/wiki/QWK_(file_format)) packet format.  The QWK packet format was/is often used to exchange mail on [bulletin board systems](https://en.wikipedia.org/wiki/Bulletin_board_system).

SlyMail provides a full-featured interface for reading and replying to messages from BBS (Bulletin Board System) QWK mail packets. Its user interface is inspired by [Digital Distortion Message Reader (DDMsgReader)](https://github.com/SynchronetBBS/sbbs/tree/master/xtrn/DDMsgReader) for message reading and [SlyEdit](https://github.com/SynchronetBBS/sbbs/tree/master/exec) for message editing, both originally created for [Synchronet BBS](https://www.synchro.net/).

SlyMail was created with the help of Claude AI.

## Features

### QWK Packet Support
- Opens and reads standard QWK mail packets (.qwk files)
- Parses CONTROL.DAT, MESSAGES.DAT, and NDX index files
- Full QWKE (extended QWK) support via HEADERS.DAT — offset-based matching for accurate extended To/From/Subject fields, UTF-8 flag, and RFC822 Message-ID
- QWKE body kludge parsing (`To:`, `From:`, `Subject:` at message start)
- Handles Synchronet-style conference numbering
- Creates REP reply packets (.rep files) for uploading back to the BBS, including HEADERS.DAT for extended fields and VOTING.DAT for pending votes
- Supports Microsoft Binary Format (MBF) float encoding in NDX files
- Remembers last opened QWK file and directory between sessions

### Message Reading (DDMsgReader-style)
- Conference list with message counts
- Scrollable message list with lightbar navigation
- Full message reader with header display (From, To, Subject, Date)
- Quote line highlighting (supports multi-level quoting)
- Kludge line display (optional)
- Scrollbar indicator
- Keyboard navigation: First/Last/Next/Previous message, Page Up/Down
- Help screens accessible with `?` or `F1` in all views

### BBS Color & Attribute Code Support
SlyMail interprets color/attribute codes from multiple BBS software packages, rendering them as colored text in both the message reader and the message editor. Supported formats:
- **ANSI escape codes** — always enabled; standard SGR sequences (ESC[...m) for foreground, background, bold
- **Synchronet Ctrl-A codes** — `\x01` + attribute character (e.g., `\x01c` for cyan, `\x01h` for bright)
- **WWIV heart codes** — `\x03` + digit 0–9
- **PCBoard/Wildcat @X codes** — `@X##` where the two hex digits encode background and foreground color
- **Celerity pipe codes** — `|` + letter (e.g., `|c` for cyan, `|W` for bright white)
- **Renegade pipe codes** — `|` + two-digit number 00–31

Each BBS code type can be individually enabled or disabled via the **Attribute code toggles** sub-dialog in Reader Settings or the `config` utility. These toggles affect both the reader and the editor. A separate **Strip ANSI codes** option removes all ANSI sequences from messages when enabled.

### File Attachments
- Detects file attachments referenced via `@ATTACH:` kludge lines in message bodies
- Shows an **[ATT]** indicator in the message header when attachments are present
- Press **D** or **Ctrl-D** in the reader to download attachments — shows a file list with sizes and prompts for a destination directory

### Voting & Polls (Synchronet QWKE)
SlyMail supports the Synchronet VOTING.DAT extension for polls and message voting:
- **Polls**: Messages identified as polls display their answer options with vote counts and percentage bars. Press **V** to open a ballot dialog where you can toggle answer selections and cast your vote.
- **Up/Down votes**: For regular (non-poll) messages, press **V** to up-vote or down-vote. Current vote tallies and score are shown in the message header.
- **Vote tallies**: The message header displays up-vote/down-vote counts and net score, with an indicator if you have already voted.
- **Vote queueing**: Votes are queued alongside message replies and written to VOTING.DAT in the REP packet for upload to the BBS.
- **Poll browser**: Press **V** from the conference list to browse all polls in the packet.

### UTF-8 Support
- Detects UTF-8 content in messages (via HEADERS.DAT `Utf8` flag and automatic detection of UTF-8 byte sequences)
- Displays UTF-8 characters correctly on compatible terminals
- Shows a **[UTF8]** indicator in the message header for UTF-8 messages
- Saves new messages with proper encoding
- CP437 to UTF-8 conversion for legacy BBS content
- Sets locale on Linux/macOS/BSD (`setlocale(LC_ALL, "")`) and UTF-8 code page on Windows for proper terminal rendering

### Message Editor (inspired by SlyEdit)
- **Two visual modes**: Ice and DCT, each with distinct color schemes and layouts
- **Random mode**: Randomly selects Ice or DCT on each edit session
- **Alternating border colors**: Border characters randomly alternate between two theme colors, matching SlyEdit's visual style
- **Theme support**: Configurable color themes loaded from .ini files
- Full-screen text editor with word wrap
- Quote window for selecting and inserting quoted text (Ctrl-Q to open/close)
- Reply and new message composition
- ESC menu for save, abort, insert/overwrite toggle, and more
- **Ctrl-K color picker**: Opens a dialog to select foreground and background colors, inserting an ANSI escape code at the cursor position. Supports 16 foreground colors (8 normal + 8 bright) and 8 backgrounds, with a live preview. Press **N** to insert a reset code.
- **Color-aware rendering**: The edit area renders ANSI and BBS attribute codes inline, so colored text is displayed as you type
- **Ctrl-U user settings dialog** for configuring editor preferences on the fly
- **Style-specific yes/no prompts**: Ice mode uses a bottom-of-screen inline prompt; DCT mode uses a centered dialog box with themed colors

### Editor Settings (via Ctrl-U)
- **Choose UI mode**: Dialog to switch between Ice, DCT, and Random styles (takes effect immediately)
- **Select theme file**: Choose from available Ice or DCT color themes
- **Taglines**: When enabled, prompts for tagline selection on save (from `tagline_files/taglines.txt`)
- **Spell-check dictionary/dictionaries**: Select which dictionaries to use
- **Prompt for spell checker on save**: When enabled, offers to spell-check before saving
- **Wrap quote lines to terminal width**: Word-wrap quoted lines
- **Quote with author's initials**: Prepend quote lines with the author's initials (e.g., `MP> `)
- **Indent quote lines containing initials**: Add leading space before initials (e.g., ` MP> `)
- **Trim spaces from quote lines**: Strip leading whitespace from quoted text

### Color Themes
- Theme files are configuration files (`.ini`) in the `config_files/` directory
- Ice themes: `EditorIceColors_*.ini` (BlueIce, EmeraldCity, FieryInferno, etc.)
- DCT themes: `EditorDCTColors_*.ini` (Default, Default-Modified, Midnight)
- Theme colors use a simple format: foreground letter (`r`/`g`/`b`/`c`/`y`/`m`/`w`/`k`), optional `h` for bright, optional background digit (`0`-`7`)
- Themes control all UI element colors: borders, labels, values, quote window, help bar, yes/no dialogs

### Spell Checker
- Built-in spell checker using plain-text dictionary files
- Ships with English dictionaries (en, en-US, en-GB, en-AU, en-CA supplements)
- Interactive correction dialog: Replace, Skip, or Quit
- Dictionary files stored in the `dictionary_files/` directory

### Taglines
- Tagline files are stored in the `tagline_files/` directory
- The default tagline file is `tagline_files/taglines.txt`, one tagline per line
- Lines starting with `#` or `;` are treated as comments and ignored
- Select a specific tagline or choose one at random when saving a message
- Taglines are appended to messages with a `...` prefix

### REP Packet Creation
- When you write replies or new messages, they are queued as pending
- Votes (poll ballots, up/down votes) are also queued alongside replies
- On exit (or when opening a new QWK file), SlyMail prompts to save all pending items
- Creates a standard `.rep` file (ZIP archive) for uploading to the BBS, containing:
  - `<BBSID>.MSG` — reply messages in standard QWK format
  - `HEADERS.DAT` — QWKE extended headers for fields exceeding 25 characters
  - `VOTING.DAT` — pending votes in Synchronet-compatible INI format
- REP file is saved as `<BBS-ID>.rep` in the configured reply directory (or the QWK file's directory)

### Remote Systems (Ctrl-R)
SlyMail can download QWK packets directly from remote systems via FTP or SFTP (SSH):
- Press **Ctrl-R** from the file browser to open the remote systems directory
- **Add/Edit/Delete** remote system entries with: name, host, port, connection type (FTP or SSH), username, password, passive FTP toggle, and initial remote path
- **Browse remote directories** with a file/directory browser similar to the local file browser — navigate into directories, go up with `..`, jump to root with `/`
- **Download QWK files** from the remote system directly into the `QWK` subdirectory of the SlyMail data directory
- Remote system entries are persisted to `remote_systems.json` in the SlyMail data directory
- Last connection date/time is tracked for each system
- Uses the system's `curl` command for FTP and SFTP transfers (no compile-time library dependencies)

### Application Settings
- Persistent settings saved to `slymail.ini` in the SlyMail data directory (`~/.slymail` on Linux/macOS/BSD, or the user's home directory on Windows)
- The SlyMail data directory and its `QWK` subdirectory are created automatically on first run
- Default QWK file browse and REP packet save directory is `~/.slymail/QWK`
- Remembers last browsed directory and QWK filename
- Ctrl-L hotkey to load a different QWK file from conference or message list views
- Configurable quote prefix, quote line width, user name
- Reader options: show/hide kludge lines, tear/origin lines, scrollbar, strip ANSI codes
- Per-BBS attribute code toggles (Synchronet, WWIV, Celerity, Renegade, PCBoard/Wildcat) — affect both reader and editor
- REP packet output directory

## Screenshots

<p align="center">
	<a href="screenshots/SlyMail_01_OpeningScreen.png" target='_blank'><img src="screenshots/SlyMail_01_OpeningScreen.png" alt="Opening screen" width="800"></a>
	<a href="screenshots/SlyMail_02_File_Chooser.png" target='_blank'><img src="screenshots/SlyMail_02_File_Chooser.png" alt="File chooser: Browsing for QWK file" width="800"></a>
	<a href="screenshots/SlyMail_03_remote_system_list.png" target='_blank'><img src="SlyMail_03_remote_system_list.png" alt="Remote system list" width="800"></a>
	<a href="screenshots/SlyMail_04_Remote_System_Edit.png" target='_blank'><img src="screenshots/SlyMail_04_Remote_System_Edit.png" alt="Editing a remote system" width="800"></a>
	<a href="screenshots/SlyMail_05_Remote_System_Browsing.png" target='_blank'><img src="screenshots/SlyMail_05_Remote_System_Browsing" alt="Remote system browsing" width="800"></a>
	<a href="screenshots/SlyMail_06_msg_area_list.png" target='_blank'><img src="screenshots/SlyMail_06_msg_area_list.png" alt="Message area list" width="800"></a>
	<a href="screenshots/SlyMail_07_msg_list.png" target='_blank'><img src="screenshots/SlyMail_07_msg_list.png" alt="Message list" width="800"></a>
	<a href="screenshots/SlyMail_08_reading_msg.png" target='_blank'><img src="screenshots/SlyMail_08_reading_msg.png" alt="Reading a message" width="800"></a>
	<a href="screenshots/SlyMail_09_msg_edit_start.png" target='_blank'><img src="screenshots/SlyMail_09_msg_edit_start.png" alt="Start of editing a message" width="800"></a>
	<a href="screenshots/SlyMail_10_quote_line_selection.png" target='_blank'><img src="screenshots/SlyMail_10_quote_line_selection.png" alt="Editor: Quote line selection" width="800"></a>
	<a href="screenshots/SlyMail_11_writing_reply_msg.png" target='_blank'><img src="screenshots/SlyMail_11_writing_reply_msg.png" alt="Editor: Editing a message" width="800"></a>
	<a href="screenshots/SlyMail_12_editor_color_picker.png" target='_blank'><img src="screenshots/SlyMail_12_editor_color_picker.png" alt="Editor: Color picker" width="800"></a>
	<a href="screenshots/SlyMail_13_Sync_poll_msg.png" target='_blank'><img src="screenshots/SlyMail_13_Sync_poll_msg.png" alt="Synchronet poll message" width="800"></a>
	<a href="screenshots/SlyMail_14_reader_settings.png" target='_blank'><img src="screenshots/SlyMail_14_reader_settings.png" alt="Reader settings" width="800"></a>
	<a href="screenshots/SlyMail_15_editor_settings.png" target='_blank'><img src="screenshots/SlyMail_15_editor_settings.png" alt="Editor settings" width="800"></a>
	<a href="screenshots/SlyMail_16_msg_search.png" target='_blank'><img src="screenshots/SlyMail_16_msg_search.png" alt="Message Search" width="800"></a>
	<a href="screenshots/SlyMail_17_Advanced_msg_search.png" target='_blank'><img src="screenshots/SlyMail_17_Advanced_msg_search.png" alt="Advanced Message Search" width="800"></a>
	<a href="screenshots/SlyMail_18_advanced_msg_search_date_picker" target='_blank'><img src="screenshots/SlyMail_18_advanced_msg_search_date_picker" alt="Date Picker in Advanced Message Search" width="800"></a>
	<a href="screenshots/SlyMail_19_config_program.png" target='_blank'><img src="screenshots/SlyMail_19_config_program.png" alt="Configuration Program" width="800"></a>
</p>

## Building

### Requirements

**Linux / macOS / BSD:**
- C++17 compatible compiler (GCC 8+, Clang 7+)
- ncurses development library (`libncurses-dev` on Debian/Ubuntu, `ncurses-devel` on Fedora/RHEL)
- `unzip` command (for extracting QWK packets)
- `zip` command (for creating REP packets)
- `curl` command (for remote system FTP/SFTP transfers — optional, only needed for the remote systems feature)

**Windows (Visual Studio 2022):**
- Visual Studio 2022 with the "Desktop development with C++" workload
- Windows SDK 10.0 (included with VS)
- No additional libraries required — uses the built-in Win32 Console API for the terminal UI, and either `tar.exe` or PowerShell for QWK/REP packet ZIP handling (see note below)

**Windows (MinGW/MSYS2):**
- MinGW-w64 or MSYS2 with GCC (C++17 support)
- Windows Console API (built-in)

> **Note — QWK/REP ZIP handling on Windows:** SlyMail detects at runtime which tool is available and uses the best option:
>
> - **`tar.exe` (preferred):** Ships with Windows 10 version 1803 (April 2018 Update) and later, and with all versions of Windows 11. `tar` reads ZIP files by their content rather than their file extension, so `.qwk` packets extract directly and `.rep` packets are created via a temporary `.zip` file that is then renamed. No extra configuration is needed.
> - **PowerShell (fallback):** If `tar.exe` is not found in the PATH, SlyMail falls back to PowerShell. For extraction it uses the .NET `ZipFile` class (`System.IO.Compression`) rather than `Expand-Archive`, because `Expand-Archive` rejects non-`.zip` file extensions even when the file is a valid ZIP archive. For REP packet creation it uses `Compress-Archive`, again writing to a temporary `.zip` file that is then renamed to `.rep`.

### Build on Linux/macOS/BSD

```bash
make
```

This builds two programs:
- `slymail` - the main QWK reader application
- `config` - the standalone configuration utility

### Build with debug symbols

```bash
make debug
```

### Install (optional)

```bash
sudo make install    # Installs slymail and config to /usr/local/bin/
sudo make uninstall  # Remove
```

### Build on Windows with Visual Studio 2022

Open the solution file in Visual Studio 2022:

```
vs\SlyMail.sln
```

Or build from the command line using MSBuild:

```powershell
# Release build (output in vs\x64\Release\)
msbuild vs\SlyMail.sln /p:Configuration=Release /p:Platform=x64

# Debug build (output in vs\x64\Debug\)
msbuild vs\SlyMail.sln /p:Configuration=Debug /p:Platform=x64
```

This builds two executables:
- `x64\Release\slymail.exe` — the main QWK reader
- `x64\Release\config.exe` — the standalone configuration utility

The solution contains two projects (`SlyMail.vcxproj` and `Config.vcxproj`) targeting x64, C++17, with the MSVC v143 toolset.

### Build on Windows (MinGW/MSYS2)

```bash
make
```

The Makefile automatically detects the platform and uses the appropriate terminal implementation:
- **Linux/macOS/BSD**: ncurses (`terminal_ncurses.cpp`)
- **Windows**: conio + Win32 Console API (`terminal_win32.cpp`)

## Usage

```bash
# Launch SlyMail with file browser
./slymail

# Open a specific QWK packet
./slymail MYBBS.qwk

# Run the standalone configuration utility
./config
```

### Configuration Program

The `config` utility provides a standalone text-based interface for configuring SlyMail settings without opening the main application. It offers four configuration categories:

- **Editor Settings** - All the same settings available via Ctrl-U in the editor (editor style, taglines, spell-check, quoting options, etc.)
- **Reader Settings** - Toggle kludge lines, tear lines, scrollbar, ANSI stripping, lightbar mode, reverse order, and attribute code toggles (per-BBS enable/disable)
- **Theme Settings** - Select Ice and DCT color theme files from the `config_files/` directory
- **General Settings** - Set your name for replies and the REP packet output directory

Settings are saved automatically when exiting each category. Both SlyMail and the config utility read and write the same settings file.

### Key Bindings

#### File Browser
| Key | Action |
|-----|--------|
| Up/Down | Navigate files and directories |
| Enter | Open directory / Select QWK file |
| Ctrl-R | Open remote systems directory |
| Q / ESC | Quit |

#### Conference List
| Key | Action |
|-----|--------|
| Up/Down | Navigate conferences |
| Enter | Open selected conference |
| V | View polls/votes in packet |
| O / Ctrl-L | Open a different QWK file |
| S / Ctrl-U | Settings |
| Q / ESC | Quit SlyMail |
| ? / F1 | Help |

#### Message List
| Key | Action |
|-----|--------|
| Up/Down | Navigate messages |
| Enter / R | Read selected message |
| N | Write a new message |
| G | Go to message number |
| Ctrl-L | Open a different QWK file |
| S / Ctrl-U | Settings |
| C / ESC | Back to conference list |
| Q | Quit |
| ? / F1 | Help |

#### Message Reader
| Key | Action |
|-----|--------|
| Up/Down | Scroll message |
| Left/Right | Previous / Next message |
| F / L | First / Last message |
| R | Reply to message |
| V | Vote (up/down vote or poll ballot) |
| D / Ctrl-D | Download file attachments |
| H | Show message header information |
| S / Ctrl-U | Settings |
| C / Q / ESC | Back to message list |
| ? / F1 | Help |

#### Message Editor
| Key | Action |
|-----|--------|
| ESC | Editor menu (Save, Abort, etc.) |
| Ctrl-U | User settings dialog |
| Ctrl-Q | Open/close quote window |
| Ctrl-K | Color picker (insert ANSI color code at cursor) |
| Ctrl-G | Insert graphic (CP437) character by code |
| Ctrl-W | Word/text search |
| Ctrl-S | Change subject |
| Ctrl-D | Delete current line |
| Ctrl-Z | Save message |
| Ctrl-A | Abort message |
| F1 | Help screen |
| Insert | Toggle Insert/Overwrite mode |

#### Quote Window
| Key | Action |
|-----|--------|
| Up/Down | Navigate quote lines |
| Enter | Insert selected quote line |
| Ctrl-Q / ESC | Close quote window |

## Architecture

SlyMail uses a platform abstraction layer for its text user interface:

```
ITerminal (abstract base class)
    ├── NCursesTerminal  (Linux/macOS/BSD - ncurses)
    └── Win32Terminal    (Windows - conio + Win32 Console API)
```

CP437 box-drawing and special characters are defined in `cp437defs.h` and rendered through the `putCP437()` method, which maps CP437 codes to platform-native equivalents (ACS characters on ncurses, direct CP437 bytes on Windows).

### Source Files

| File | Description |
|------|-------------|
| `terminal.h` | Abstract `ITerminal` interface, key/color constants, factory |
| `terminal_ncurses.cpp` | ncurses implementation with CP437-to-ACS mapping |
| `terminal_win32.cpp` | Windows Console API + conio implementation |
| `cp437defs.h` | IBM Code Page 437 character definitions |
| `colors.h` | Color scheme definitions (Ice, DCT, reader, list) |
| `theme.h` | Theme config file parser (Synchronet-style attribute codes) |
| `ui_common.h` | Shared UI helpers (dialogs, text input, scrollbar, etc.) |
| `qwk.h` / `qwk.cpp` | QWK/REP packet parser and creator (QWKE, attachments, voting) |
| `bbs_colors.h` / `bbs_colors.cpp` | BBS color/attribute code parser (ANSI, Synchronet, WWIV, PCBoard, Celerity, Renegade) |
| `utf8_util.h` / `utf8_util.cpp` | UTF-8 utilities (validation, display width, CP437-to-UTF-8 conversion) |
| `voting.h` / `voting.cpp` | VOTING.DAT parser, vote tallying, poll display UI |
| `remote_systems.h` / `remote_systems.cpp` | Remote systems directory, FTP/SFTP browsing, JSON persistence, home dir utilities |
| `settings.h` | User settings persistence |
| `settings_dialog.h` | Settings dialogs (editor, reader, attribute code toggles) |
| `file_browser.h` | QWK file browser and selector |
| `msg_list.h` | Conference and message list views |
| `msg_reader.h` | Message reader (DDMsgReader-style) with voting and attachment UI |
| `msg_editor.h` | Message editor (SlyEdit Ice/DCT-style) with color picker |
| `main.cpp` | SlyMail application entry point and main loop |
| `config.cpp` | Standalone configuration utility |

## Configuration

### Settings File

Settings are stored in an INI file named `slymail.ini` in the same directory as the SlyMail executable. This file is shared between both SlyMail and the `config` utility. The file is well-commented with descriptions of each setting.

Example `slymail.ini`:
```ini
[Editor]

; Editor style for writing messages: Ice, Dct, or Random
editorStyle=Ice

; Enable tagline insertion when saving a message
taglines=false

; Prompt the user to run the spell checker when saving a message
promptSpellCheck=false

[Reader]

; Show kludge/control lines (@MSGID, @REPLY, etc.) in the message reader
showKludgeLines=false

; Strip ANSI escape codes from message text
stripAnsi=false

; Attribute code toggles (affect both reader and editor)
attrSynchronet=true
attrWWIV=true
attrCelerity=true
attrRenegade=true
attrPCBoard=true

[Themes]

; Color theme file for the editor in Ice mode
iceThemeFile=EditorIceColors_BlueIce.ini

; Color theme file for the editor in DCT mode
dctThemeFile=EditorDCTColors_Default.ini
```

### Theme Files

Color themes are `.ini` files in the `config_files/` directory:

**Ice themes** (`EditorIceColors_*.ini`):
- BlueIce (default), EmeraldCity, FieryInferno, Fire-N-Ice, GeneralClean, GenericBlue, PurpleHaze, ShadesOfGrey

**DCT themes** (`EditorDCTColors_*.ini`):
- Default (default), Default-Modified, Midnight

Theme color values use a compact format derived from Synchronet attribute codes:
- `n` = normal (reset)
- Foreground: `k`=black, `r`=red, `g`=green, `y`=yellow, `b`=blue, `m`=magenta, `c`=cyan, `w`=white
- `h` = high/bright intensity
- Background digit: `0`=black, `1`=red, `2`=green, `3`=brown, `4`=blue, `5`=magenta, `6`=cyan, `7`=light gray

Example: `nbh` = normal blue bright, `n4wh` = bright white on blue background

### Taglines

Taglines are short quotes or sayings appended to the end of messages when saved. The tagline feature can be enabled via Ctrl-U in the editor or the `config` utility.

Taglines are stored in `tagline_files/taglines.txt`, one per line. Lines starting with `#` or `;` are treated as comments and ignored. When saving a message with taglines enabled, the user is prompted to either select a specific tagline or choose one at random. Selected taglines are appended to the message with a `...` prefix (e.g., `...To err is human, to really foul things up requires a computer.`).

### Spell Checker

SlyMail includes a built-in spell checker that uses plain-text dictionary files. The spell checker can be configured to prompt on save via Ctrl-U in the editor or the `config` utility.

**Dictionary files** are plain text files (one word per line) stored in `dictionary_files/`. Multiple dictionaries can be selected simultaneously for combined word coverage. SlyMail ships with:
- `dictionary_en.txt` - English (general, ~130K words)
- `dictionary_en-US-supplemental.txt` - US English supplement
- `dictionary_en-GB-supplemental.txt` - British English supplement
- `dictionary_en-AU-supplemental.txt` - Australian English supplement
- `dictionary_en-CA-supplemental.txt` - Canadian English supplement

When spell-checking is triggered, the checker scans the message for misspelled words and presents an interactive dialog for each one, offering options to **R**eplace the word, **S**kip it, **A**dd it (future), or **Q**uit checking.

## Credits

- UI inspired by [DDMsgReader](https://github.com/SynchronetBBS/sbbs) and [SlyEdit](https://github.com/SynchronetBBS/sbbs) by [Nightfox (Eric Oulashin)](https://github.com/nightfox)
- QWK format compatibility informed by [Synchronet BBS](https://www.synchro.net/) source code
- CP437 character definitions from Synchronet

## License

This project is open source software.
