---
title: "SlyMail User Manual (Pirate)"
subtitle: "QWK Offline Mail Reader"
version: "0.55"
date: "2026-04-09"
author: "Eric Oulashin"
---

# SlyMail User Manual (Pirate)

## Introduction

SlyMail be a cross‑platform, text‑mode, offline mail reader fer the QWK packet format. QWK packets be used t’ trade mail on Bulletin Board Systems (BBSes). SlyMail gives ye a full‑featured interface fer readin’, searchin’, and replyin’ t’ messages from QWK mail packets.

Its user interface be inspired by Digital Distortion Message Reader (DDMsgReader) fer readin’ messages, and SlyEdit fer editin’ messages, both o’ which were forged fer Synchronet BBS back in the day.

SlyMail sails on Linux, macOS, BSD, and Windows.

## Getting Started

### Launching SlyMail

![SlyMail Opening Screen](../screenshots/SlyMail_01_OpeningScreen.png)

Launch SlyMail from the command line:

    slymail

That shows the splash screen (if enabled) and opens the file browser t’ pick a QWK packet.

T’ open a specific QWK packet directly:

    slymail MYBBS.qwk

Or use the named parameter:

    slymail -qwk_file=/path/to/MYBBS.qwk

Other command‑line options:

- `-v` or `--version` - Show version information
- `-?` or `--help` - Show help

### The SlyMail Data Directory

On first run, SlyMail builds a data hold at `~/.slymail` (on Linux, macOS, and BSD) or in yer home directory on Windows. This hold contains:

- `slymail.ini` - Yer settings
- `QWK/` - Default berth fer QWK mail packets
- `REP/` - Default berth fer reply packets
- `config_files/` - Color theme files
- `dictionary_files/` - Spell‑check dictionaries
- `tagline_files/` - Tagline files
- `remote_systems.json` - Saved remote system connections
- `lastread_<BBSID>.json` - Last‑read message pointers per conference

### The Configuration Program

SlyMail comes with a standalone configuration program called `config` (or `slymail-config` if installed via `make install`). This program gives a text‑based way t’ set all SlyMail options without openin’ the main app.

Run it from the command line:

    config

The configuration program offers these categories:

- **Editor Settings** - Editor style, taglines, spell‑check, quoting options, theme selection
- **Reader Settings** - Kludge lines, tear lines, scrollbar, ANSI stripping, reverse order, attribute code toggles
- **Theme Settings** - Select Ice and DCT color theme files
- **General Settings** - Yer name fer replies, reply packet directory, external editor configuration, lightbar mode, splash screen

Settings be saved automatically when ye leave each category. Both SlyMail and the config program read and write the same `slymail.ini` file.

## File Browser

When SlyMail starts, a file browser be shown fer selectin’ a QWK packet t’ open.

![File Browser](../screenshots/SlyMail_02_File_Chooser.png)

### File Browser Keys

| Key | Action |
|-----|--------|
| Up / Down | Navigate files and directories |
| Enter | Open directory or select a QWK file |
| PgUp / PgDn | Scroll the file list |
| Home / End | Jump t’ first or last entry |
| Ctrl-R | Open the remote systems directory |
| Q / ESC | Quit SlyMail |
| ? / F1 | Help |

Only `.qwk` files can be selected. Directories be shown in blue and can be entered by pressin’ Enter.

## Remote Systems (Ctrl-R)

![Remote Systems List](../screenshots/SlyMail_03_remote_system_list.png)

SlyMail can haul down QWK packets straight from remote BBSes via FTP or SFTP (SSH).

Press **Ctrl-R** from the file browser, conference list, or message list t’ open the remote systems manager. From there ye can:

- **Add** a new remote system with its hostname, port, connection type (FTP or SSH), username, password, passive FTP toggle, and initial remote path
- **Edit** or **Delete** existing entries
- **Connect** t’ a remote system and browse its directories
- **Download** QWK files straight into yer `QWK/` directory
- **Upload** REP reply packets t’ the remote system

Remote system entries be stored in `remote_systems.json` in yer SlyMail data directory. The last connection date/time be tracked fer each system.

SlyMail uses the ship’s `curl` command fer FTP and SFTP file transfers.

## Conference List

![Conference List](../screenshots/SlyMail_06_msg_area_list.png)

After openin’ a QWK packet, the conference list shows all message areas (conferences) with their message counts. A “New” column shows a checkmark next t’ conferences that have new messages. If the “Only show areas with new mail” setting be enabled, conferences with zero messages be hidden.

### Conference List Keys

| Key | Action |
|-----|--------|
| Up / Down | Navigate conferences |
| PgUp / PgDn | Scroll the list |
| Home / End | Jump t’ first or last conference |
| Enter (E) | Open the selected conference |
| G | Go t’ a specific conference number |
| / | Search conferences by name |
| V | View polls and votes in the packet |
| O / Ctrl-L | Open a different QWK file |
| Ctrl-R | Open remote systems |
| Ctrl-P | Save the REP reply packet |
| S / Ctrl-U | Open settings |
| Q / ESC | Quit SlyMail |
| ? / F1 | Help |

When a search filter be active, pressin’ **Q** clears the filter instead o’ quittin’.

## Message List

![Message List](../screenshots/SlyMail_07_msg_list.png)

The message list shows all messages in the selected conference with columns fer message number, from, to, subject, date, and time.

### Last-Read Message Tracking

SlyMail remembers the last message ye read in each conference. When ye enter a conference, the cursor be placed on the first unread message (the one after the last one ye read). This be saved per‑BBS in the SlyMail data directory.

### Message List Keys

| Key | Action |
|-----|--------|
| Up / Down | Navigate messages |
| PgUp / PgDn | Scroll the list |
| Home / End | Jump t’ first or last message |
| Enter / R | Read the selected message |
| N | Write a new message |
| G | Go t’ a specific message number |
| / | Search messages |
| Ctrl-A | Advanced search (with date range) |
| Ctrl-L | Open a different QWK file |
| Ctrl-R | Open remote systems |
| Ctrl-P | Save the REP reply packet |
| S / Ctrl-U | Open settings |
| C / ESC | Back t’ the conference list |
| Q | Quit |
| ? / F1 | Help |

### Searching Messages

![Message Search](../screenshots/SlyMail_16_msg_search.png)

Press **/** t’ search messages by subject, body text, sender, or recipient. Press **Ctrl-A** fer advanced search, which includes date‑range filterin’ with a calendar picker. When a search filter be active, pressin’ **Q** clears the filter.

The search can use plain substring matchin’ or regular expressions, dependin’ on the “Search using regular expression” setting.

## Message Reader

![Reading a Message](../screenshots/SlyMail_08_reading_msg.png)

The message reader shows message content with a header showin’ From, To, Subject, and Date.

### Message Reader Keys

| Key | Action |
|-----|--------|
| Up / Down | Scroll message content |
| PgUp / PgDn | Page up or down |
| Home / End | Jump t’ top or bottom |
| Left / Right | Previous or next message |
| F / L | First or last message |
| R | Reply t’ the message |
| V | Vote (up/down vote or poll ballot) |
| D / Ctrl-D | Download file attachments |
| H | Show detailed message header |
| S / Ctrl-U | Open settings |
| C / Q / ESC / M | Back t’ the message list |
| ? / F1 | Help |

### ANSI Art

![ANSI Art](../screenshots/SlyMail_20_reading_ANSI_art.png)

SlyMail renders ANSI art in messages, supportin’ cursor positionin’, colors, and CP437 block characters. Messages containin’ ANSI cursor control sequences be detected and rendered through a virtual screen buffer fer proper display.

### File Attachments

When a message has file attachments (marked by **[ATT]** in the header), press **D** or **Ctrl-D** t’ view and download them. Ye’ll see a list o’ attached files and be asked t’ choose a destination directory.

### Voting and Polls

SlyMail supports Synchronet‑style votin’:

- **Polls**: Messages flagged as polls show answer options with vote counts and percentage bars. Press **V** t’ open a ballot where ye can toggle selections and cast yer vote.
- **Up/Down Votes**: Fer regular messages, press **V** t’ up‑vote or down‑vote. Current tallies and net score be shown in the header.
- **Poll Browser**: Press **V** from the conference list t’ browse all polls in the packet.

Votes be queued alongside replies and written to the REP packet.

## Message Editor

![Message Editor](../screenshots/SlyMail_09_msg_edit_start.png)

The message editor be fer composin’ replies and new messages. It offers two visual modes inspired by SlyEdit: **Ice** mode and **DCT** mode.

### Editor Keys

| Key | Action |
|-----|--------|
| Arrow keys | Move the cursor |
| Home / End | Beginning or end o’ line |
| PgUp / PgDn | Page up or down |
| Insert | Toggle Insert/Overwrite mode |
| Delete | Delete character at cursor |
| Backspace | Delete character before cursor |
| ESC | Open the editor menu |
| Ctrl-Z | Save the message |
| Ctrl-A | Abort the message |
| Ctrl-Q | Open or close the quote window |
| Ctrl-K | Open the color picker |
| Ctrl-G | Insert a CP437 graphic character |
| Ctrl-O | Import a text file at the cursor position |
| Ctrl-W | Search fer text |
| Ctrl-S | Change the subject |
| Ctrl-D | Delete the current line |
| Ctrl-T | List text replacements |
| Ctrl-U | Open editor settings |
| F1 | Help screen |

### Slash Commands

Ye can also type slash commands on an empty line and press Enter:

| Command | Action |
|---------|--------|
| /S | Save the message |
| /A | Abort the message |
| /Q | Open the quote window |
| /U | Open editor settings |
| /? | Show help |

### Quote Window

![Quote Window](../screenshots/SlyMail_10_quote_line_selection.png)

When replyin’, press **Ctrl-Q** or type `/Q` on an empty line t’ open the quote window. It shows the original message text with quote prefixes.

| Key | Action |
|-----|--------|
| Up / Down | Navigate quote lines |
| PgUp / PgDn | Page up or down |
| Home / End | Jump t’ first or last line |
| Enter | Insert the selected quote line into yer message |
| Ctrl-Q / ESC | Close the quote window |

If the number o’ quote lines be more than the window’s height, a scrollbar appears on the right.

### Color Picker (Ctrl-K)

Press **Ctrl-K** t’ open an interactive color picker. Pick a foreground color (16 options: 8 normal + 8 bright) and a background color (8 options). A live preview shows the chosen mix. Press **Enter** t’ insert the ANSI color code at the cursor. Press **N** t’ insert a reset (normal) code.

### File Import (Ctrl-O)

Press **Ctrl-O** t’ import the contents o’ a text file at the cursor. A file browser opens so ye can pick any file. The contents be inserted line by line, keepin’ line breaks.

![Editing a message](../screenshots/SlyMail_11_writing_reply_msg.png)

### Quote Line Handling

When replyin’, lines inserted from the quote window be tracked as quote lines. If ye edit a quote line and it wraps, the overflow goes onto a new line (also marked as quote) instead o’ mergin’ with nearby text. Deletin’ text from a quote line won’t pull text up from the next line if that line be also a quote line. This keeps yer quoted text shipshape.

### Paragraph-Based Saving

When savin’ a message, SlyMail joins soft‑wrapped lines o’ newly typed text into single long paragraphs. That lets other mail readers re‑wrap the text t’ fit their own terminal width. Quote lines be always preserved exactly and never joined.

### Empty Messages

If ye save a message with no text (empty or only whitespace), SlyMail aborts the message and shows a notification instead o’ postin’ it.

### External Editors

SlyMail supports usin’ external text editors instead o’ the built‑in editor. T’ configure one:

1. Open Settings (Ctrl-U or the `config` program)
2. Go to **External Editors...** t’ add and configure editors
3. Set **External Editor** to choose which editor to use
4. Enable **Use external editor**

Each external editor can be set up with:

- **Name** - A descriptive name
- **Startup Directory** - The directory containin’ the editor executable
- **Command Line** - The command to run (use `%f` fer the temp filename)
- **Word-wrap Quoted Text** - Whether t’ wrap quoted text and the column width
- **Auto Quoted Text** - Always include quotes, prompt each time, or never
- **Editor Information Files** - Drop file type (None, MSGINF, EDITOR.INF, DOOR.SYS, DOOR32.SYS)
- **Strip FidoNet Kludges** - Remove FidoNet control lines from quoted text

When usin’ an external editor, SlyMail creates a temporary file, launches the editor, and reads the result when the editor quits. If the editor produces `RESULT.ED` (Synchronet‑compatible), SlyMail reads the new subject and editor details from it.

## Settings

### Reader Settings (Ctrl-U)

![Reader Settings](../screenshots/SlyMail_14_reader_settings.png)

Open reader settings by pressin’ **Ctrl-U** or **S** from the conference list, message list, or reader.

Available settings:

- **Show kludge lines** - Show or hide kludge/control lines
- **Show tear/origin lines** - Show or hide tear and origin lines
- **Scrollbar in reader** - Show or hide the scrollbar
- **Only show areas with new mail** - Hide conferences with zero messages
- **Strip ANSI codes from messages** - Remove ANSI escape sequences
- **Attribute code toggles** - Enable/disable BBS color code interpretation per type
- **Search using regular expression** - Use regex fer searches
- **List messages in reversed** - Show newest messages first
- **Show splash screen on startup** - Toggle the splash screen
- **Reply packet directory** - Set the directory fer savin’ REP packets
- **Use external editor** - Enable external editor mode
- **External Editors...** - Configure external editors
- **External Editor** - Select which configured editor to use
- **Language** - Choose the UI language (or “OS Default”)

Press **S** t’ save, or **ESC/Q** t’ close (with a prompt t’ save if changes were made).

### Editor Settings (Ctrl-U in Editor)

Open editor settings by pressin’ **Ctrl-U** while in the message editor.

Available settings:

- **Choose UI mode** - Switch between Ice, DCT, and Random styles
- **Taglines** - Enable tagline insertion when savin’
- **Spell-check dictionary/dictionaries** - Select which dictionaries to use
- **Prompt for spell checker on save** - Enable spell‑check promptin’
- **Wrap quote lines to terminal width** - Word‑wrap quoted lines
- **Quote with author's initials** - Use initials‑based quote prefixes
- **Indent quote lines containing initials** - Add leadin’ space before initials
- **Trim spaces from quote lines** - Strip leadin’ whitespace from quotes
- **Language** - Choose the UI language (or “OS Default”)

## BBS Color Code Support

SlyMail interprets color and attribute codes from multiple BBS software packages:

- **ANSI escape codes** - Always enabled; standard SGR sequences
- **Synchronet Ctrl-A codes** - Ctrl-A + attribute character
- **WWIV heart codes** - Ctrl-C + digit 0-9
- **PCBoard/Wildcat @X codes** - @X followed by two hex digits
- **Celerity pipe codes** - Pipe + letter
- **Renegade pipe codes** - Pipe + two‑digit number

Each code type can be enabled or disabled via the Attribute Code Toggles sub‑dialog in settings.

## Color Themes

SlyMail’s editor uses configurable color themes loaded from `.ini` files in `config_files/`.

### Ice Themes

Ice themes control the look o’ Ice mode:

- BlueIce (default)
- EmeraldCity
- FieryInferno
- Fire-N-Ice
- GeneralClean
- GenericBlue
- PurpleHaze
- ShadesOfGrey

### DCT Themes

DCT themes control the look o’ DCT mode:

- Default
- Default-Modified
- Midnight

### Theme Color Format

Theme colors use a compact format based on Synchronet attribute codes:

- `n` - Normal (reset all attributes)
- Foreground letters: `k`=black, `r`=red, `g`=green, `y`=yellow, `b`=blue, `m`=magenta, `c`=cyan, `w`=white
- `h` - High/bright intensity
- Background digits: `0`=black, `1`=red, `2`=green, `3`=brown, `4`=blue, `5`=magenta, `6`=cyan, `7`=light gray

Examples: `nbh` = bright blue, `n4wh` = bright white on blue background, `nk7` = black on light gray

## Spell Checker

SlyMail includes a built‑in spell checker usin’ plain‑text dictionary files.

### Dictionaries

Dictionary files be stored in `dictionary_files/`:

- `dictionary_en.txt` - English (general)
- `dictionary_en-US-supplemental.txt` - US English supplement
- `dictionary_en-GB-supplemental.txt` - British English supplement
- `dictionary_en-AU-supplemental.txt` - Australian English supplement
- `dictionary_en-CA-supplemental.txt` - Canadian English supplement

Ye can pick multiple dictionaries at once.

### Using the Spell Checker

Enable “Prompt for spell checker on save” in settings. When ye save a message, the spell checker hunts misspelled words and gives ye choices:

- **R**eplace - Type a replacement word
- **S**kip - Skip this word
- **Q**uit - Stop spell checkin’

## Taglines

Taglines be short quotes or sayings appended t’ messages when saved.

### Setup

1. Enable “Taglines” in editor settings
2. Put taglines in `tagline_files/taglines.txt`, one per line
3. Lines startin’ with `#` or `;` be treated as comments

### Usage

When savin’ a message with taglines enabled, ye be prompted t’ pick a tagline or choose one at random. The chosen tagline be appended with a `...` prefix.

## REP Reply Packets

When ye write replies or new messages, they be queued as pendin’. Votes (poll ballots, up/down votes) be queued as well.

### Saving REP Packets

- Press **Ctrl-P** from the conference or message list t’ save the REP packet anytime
- SlyMail prompts ye t’ save after composin’ a message
- On exit, SlyMail prompts ye t’ save any unsaved pendin’ items

The REP packet be saved as `<BBS-ID>.rep` in yer configured reply directory (default `~/.slymail/REP/`). Upload it back t’ the BBS t’ post yer replies.

### REP Packet Contents

- Message replies in standard QWK format
- `HEADERS.DAT` fer extended fields exceedin’ 25 characters
- `VOTING.DAT` fer pendin’ votes (Synchronet‑compatible)

## UTF-8 Support

SlyMail detects and displays UTF‑8 content proper:

- UTF‑8 messages be detected via `HEADERS.DAT` flags and automatic byte‑sequence analysis
- A **[UTF8]** marker appears in the header fer UTF‑8 messages
- CP437 characters from old BBS content be converted t’ Unicode equivalents

## Settings File Reference

Settings be stored in `slymail.ini` in the SlyMail data directory. The file uses INI format with sections:

### [General]

| Setting | Default | Description |
|---------|---------|-------------|
| showSplashScreen | true | Show splash screen on startup |
| userName | (empty) | Yer name fer the From field in replies |
| replyDir | (empty) | Directory fer savin’ REP packets |
| useExternalEditor | false | Use external editor instead o’ built‑in |
| selectedEditor | (empty) | Name o’ the selected external editor |
| language | (empty) | UI language code (empty = OS Default) |

### [Reader]

| Setting | Default | Description |
|---------|---------|-------------|
| showKludgeLines | false | Show kludge/control lines |
| showTearLine | true | Show tear and origin lines |
| useScrollbar | true | Show scrollbar in message reader |
| onlyShowAreasWithNewMail | false | Hide empty conferences |
| stripAnsi | false | Strip ANSI codes |
| attrSynchronet | true | Interpret Synchronet Ctrl-A codes |
| attrWWIV | true | Interpret WWIV heart codes |
| attrCelerity | true | Interpret Celerity pipe codes |
| attrRenegade | true | Interpret Renegade pipe codes |
| attrPCBoard | true | Interpret PCBoard/Wildcat @X codes |
| useRegexSearch | false | Use regex fer searches |

### [MessageList]

| Setting | Default | Description |
|---------|---------|-------------|
| lightbarMode | true | Lightbar navigation in message list |
| reverseOrder | false | Show newest messages first |

### [Editor]

| Setting | Default | Description |
|---------|---------|-------------|
| editorStyle | Ice | Editor mode: Ice, Dct, or Random |
| insertMode | true | Default insert mode |
| wrapQuoteLines | true | Word‑wrap quoted lines |
| quoteLineWidth | 76 | Max width fer quote lines |
| quotePrefix | > (space) | Quote line prefix |
| taglines | false | Enable tagline insertion |
| promptSpellCheck | false | Prompt fer spell check on save |
| quoteWithInitials | false | Use author initials in quote prefix |
| indentQuoteInitials | true | Add space before initials |
| trimQuoteSpaces | false | Trim whitespace from quote lines |

### [Themes]

| Setting | Default | Description |
|---------|---------|-------------|
| iceThemeFile | EditorIceColors_BlueIce.ini | Ice mode theme file |
| dctThemeFile | EditorDCTColors_Default.ini | DCT mode theme file |

## Synchronet BBS QWK Settings

If ye be usin’ a Synchronet BBS, these QWK packet settings be recommended fer best compatibility with SlyMail:

| Setting | Recommended Value |
|---------|-------------------|
| Ctrl-A Color Codes | Leave in |
| Archive Type | ZIP |
| Include File Attachments | Yes |
| Include Index Files | Yes |
| Include Control Files | Yes |
| Include VOTING.DAT File | Yes |
| Include HEADERS.DAT File | Yes |
| Include UTF-8 Characters | Yes |
| Extended (QWKE) Packet Format | Yes |

## Changelog

### Version 0.54 (2026-03-31)

#### Added

- Last‑read message tracking per conference with automatic cursor positioning
- New message indicator (checkmark column) in the conference list
- File import in message editor (Ctrl-O) t’ insert text files at cursor position
- Quote line tracking t’ preserve quote line integrity during editing
- Paragraph‑based message saving fer better text reflow by recipients
- Terminal resize detection: Allows SlyMail t’ resize its window(s) when the terminal be resized

#### Changed

- Quote line wrapping now respects terminal width when enabled
- Configuration on first start: If the user ain’t got settings yet (no `.slymail` dir), `config` runs on startup and asks fer yer name first

#### Fixed

- Quote line editing no longer merges overflow with adjacent lines

### Version 0.53 (2026-03-29)

#### Added

- Setting t’ only show conferences with new messages
- External editor support with multiple editor configurations
- Drop file type support (MSGINF, EDITOR.INF, DOOR.SYS, DOOR32.SYS)
- RESULT.ED parsing after external editor exits
- Go‑to conference number (G key) in conference list
- Digit‑only input validation fer Go‑to prompts

#### Fixed

- Slash commands (/Q, /S, etc.) now work on the first line o’ the editor
- DCT quote window colors now use the loaded theme properly
- Quote window scrollbar restored fer long quote lists
- Improved screen refresh in settings dialogs

### Version 0.52 (2026-03-26)

#### Added

- REP directory created automatically
- Save REP packet hotkey (Ctrl-P)
- Upload reply packets t’ remote systems
- Quote line re‑wrappin’
- Directory chooser fer reply packet directory

#### Changed

- Faster editor screen updates with reduced flicker
- Paragraph‑aware message saving
- Improved ANSI renderin’ with 256‑color and truecolor support

#### Fixed

- Editor color code handling during all editing operations
- Editor word wrap behavior
- Quote prefix trailin’ space preservation
- REP save tracking

### Version 0.51 (2026-03-25)

#### Added

- QWKE (Extended QWK) support
- BBS color/attribute code support fer multiple BBS types
- Synchronet votin’ support (polls and up/down votes)
- File attachment support
- UTF‑8 support
- Color picker (Ctrl-K)
- Remote systems directory (Ctrl-R)
- Search and filtering with advanced search
- Splash screen toggle

### Version 0.50 (2026-03-24)

- Initial release
- Cross‑platform QWK offline mail reader
- DDMsgReader‑inspired readin’ interface
- SlyEdit‑inspired message editor with Ice and DCT modes
- Theme support, spell checker, and taglines
- REP reply packet creation
- File browser and standalone config utility

