# Changelog

All notable changes to SlyMail are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/), and this project adheres to [Semantic Versioning](https://semver.org/).

## [0.52] - 2026-03-25

### Fixed
- **Editor ANSI color integrity**: Overwrite mode no longer corrupts ANSI sequences when the cursor is on an ESC byte (inserts before the sequence instead). Cursor auto-skips past ANSI sequences after typing. Enter key and word wrap no longer split lines inside ANSI sequences. Up/Down/PageUp/PageDown no longer land the cursor inside an ANSI sequence on the target line.
- **Editor arrow key movement over color codes**: Left/Right arrow keys now skip over all consecutive ANSI escape sequences between visible characters in a single keypress. Previously each ANSI sequence required a separate keypress to traverse. Backspace and Delete also now remove one visible character plus all adjacent ANSI sequences in one keystroke, matching the cursor movement behavior.

## [0.51] - 2026-03-25

### Added
- **QWKE (Extended QWK) support**: Offset-based HEADERS.DAT matching for accurate extended To/From/Subject fields, UTF-8 flag, and RFC822 Message-ID parsing. QWKE body kludge parsing (`To:`, `From:`, `Subject:` at message start). REP packets now include HEADERS.DAT for extended fields.
- **BBS color/attribute code support**: Interprets color codes from multiple BBS software packages in both the message reader and editor:
  - ANSI escape codes (always enabled)
  - Synchronet Ctrl-A codes
  - WWIV heart codes
  - PCBoard/Wildcat @X codes
  - Celerity pipe codes
  - Renegade pipe codes
- **Per-system attribute code toggles**: Each BBS code type can be individually enabled/disabled via the Attribute Code Toggles sub-dialog in Reader Settings or the `config` utility. These settings affect both the reader and editor.
- **Strip ANSI codes option**: When enabled, strips all ANSI escape codes from message text for plain display.
- **Synchronet voting support**: Full support for the VOTING.DAT extension:
  - Poll display with answer options, vote counts, and percentage bars
  - Poll ballot UI (press V) to toggle answer selections and cast votes
  - Up/down voting on regular messages (press V for Up, Down, or Quit prompt)
  - Vote tallies shown in message header (upvote/downvote counts, net score, user vote indicator)
  - Votes queued alongside replies and written to VOTING.DAT in REP packets
  - Poll browser accessible via V from the conference list
- **File attachment support**: Detects `@ATTACH:` kludge lines in messages, shows an [ATT] indicator in the header, and allows downloading attachments with D or Ctrl-D.
- **UTF-8 support**: Detects UTF-8 content via HEADERS.DAT flag and automatic byte sequence detection. Displays UTF-8 characters correctly on compatible terminals. Shows a [UTF8] indicator in the message header. Includes CP437-to-UTF-8 conversion for legacy BBS content.
- **Editor color picker (Ctrl-K)**: Opens a dialog to select foreground (8 normal + 8 bright) and background (8) colors, with a live preview. Inserts ANSI escape codes at the cursor position. Includes an Attributes section with Normal (reset all, `ESC[0m`) and High (bright/bold, `ESC[1m`). Hotkeys: N for Normal, H for High.
- **ANSI-aware cursor movement in editor**: Left/Right arrow keys skip over ANSI sequences as atomic units. Backspace and Delete remove entire ANSI sequences in one keystroke. Cursor position on screen correctly accounts for zero-width ANSI codes. Word wrap uses display width rather than byte length.
- **SlyMail data directory**: Settings and data are now stored in `~/.slymail` (Linux/macOS/BSD) or the user's home directory (Windows). The data directory and a `QWK` subdirectory are created automatically on first run. Default QWK browse and REP save directory is now `~/.slymail/QWK`.
- **Remote systems directory (Ctrl-R)**: Download QWK packets from remote BBSes via FTP or SFTP directly from the file browser. Manage a directory of remote system connections with name, host, port, credentials, and connection type. Browse remote directories, navigate folders, and download files. Remote system entries are persisted to `remote_systems.json`. Uses the system's `curl` command for transfers (no compile-time library dependencies).
- **Search and filtering** (`/` key): Search conferences by name in the conference list, and search messages by subject, body, from, or to in the message list. Includes an advanced search dialog (Ctrl-A) with date range filtering via a visual calendar date picker. Search results filter the list in place; press Q to clear the filter.
- **Advanced search date picker**: Calendar-style date selection dialog with `<`/`>` month navigation, editable year field with digit validation, day-of-week headers, and Tab-based focus cycling.
- **Regex search option**: Toggle in Reader Settings to treat search text as a regular expression (case-insensitive ECMAScript) instead of a plain substring.
- **Remote systems directory (Ctrl-R)**: FTP and SFTP support for downloading QWK packets from remote BBSes. Scrollable system list with add/edit/delete. Remote file browser with directory navigation. Password input masked with `*` characters. Passwords encrypted (XOR + base64) in `remote_systems.json`. Port input validated to digits only. Spinning progress indicator during downloads. Comprehensive curl error code mapping (~45 codes) with user-friendly messages.
- **Splash screen toggle**: Setting in Reader Settings and `config` General Settings to enable/disable the startup splash screen.
- **Command-line interface**: `-qwk_file=<path>` to open a QWK packet directly (bypasses splash screen and file browser), `-v`/`--version` for version info, `-?`/`-help`/`--help`/`/?`/`/help` for help screen. Also supports bare positional argument for backward compatibility.
- **File browser help screen**: Press `?` or `F1` for a help screen listing available hotkeys.
- **Remote file browser help screen**: Press `?` or `F1` when connected to a remote system for a help screen with connection info.
- **M key in message reader**: Returns to the message list, same as Q/C/ESC.
- New source module: `search.cpp/h`
- **Cross-platform `getHomeDir()`**: Reliable home directory detection for Windows (`USERPROFILE`/`HOMEDRIVE+HOMEPATH`), Linux, macOS, and BSD (`HOME`).
- New source modules: `bbs_colors.cpp/h`, `utf8_util.cpp/h`, `voting.cpp/h`, `remote_systems.cpp/h`
- GitHub Actions CI workflow for Linux and macOS builds

### Changed
- **Conference and message list position memory**: Returning from reading a message or from a conference preserves the previously selected position.
- **Color picker remembers last selection**: Re-opening Ctrl-K in the editor starts with the last selected foreground, background, bright, and section.
- **Date picker improvements**: Focus starts on calendar; tab order is Calendar → `<` → `>` → Year → OK → Cancel. `<` and `>` buttons shown without brackets. Year field opens a digit-validated text input on Tab or Enter; focus returns to calendar after editing.
- **Remote system editor**: Selected field background color extends full width across the dialog. DEL key deletes a remote system in addition to D.
- Download progress spinner shown during FTP/SFTP file transfers.
- Settings file (`slymail.ini`) is now stored in the SlyMail data directory (`~/.slymail`) instead of alongside the executable.
- Default directory for QWK file browsing and REP packet saving is now `~/.slymail/QWK`.
- Help bar at the bottom of the screen uses new color scheme: grey background, red hotkeys, blue regular text, magenta parentheses. Text is horizontally centered across the full terminal width.
- REP packets now include HEADERS.DAT (for fields > 25 chars) and VOTING.DAT (for queued votes) alongside the message file.
- Message reader body rendering now parses BBS color codes segment-by-segment for colored output.
- Q key in conference/message list now clears the active search filter before quitting, so filtered results can be dismissed without leaving the screen.
- Editor `drawEditArea()` renders ANSI and BBS attribute codes inline for live colored text preview.

### Fixed
- **Vote response filtering**: Messages with QWK status `V` (vote/ballot responses) are hidden from the message list. Their vote data is still tallied into the target message.
- **Synchronet poll display**: Polls from VOTING.DAT that have no corresponding message in MESSAGES.DAT are now shown as synthetic messages inserted at their chronological position in the conference. Poll results display with Synchronet-style backfill percentage bars (bright white on blue for the filled portion, dim cyan for the remainder).
- **Vote tallying**: `tallyVotes()` and up/down vote counting now match by `vote.inReplyTo` (the ID of the poll/message being voted on) instead of `vote.msgId` (the vote's own ID). Previously all vote counts were zero.
- **Synthetic poll messages**: Polls defined only in VOTING.DAT (common with Synchronet) now appear in the message list at their correct chronological position instead of being invisible or appended at the end.
- **Ctrl-S subject cursor position**: Fixed incorrect cursor placement for subject editing in both Ice mode (was column 12, now column 10) and DCT mode (was column 11, now column 8). Subject input width now dynamically matches the header layout. Also fixed in the ESC menu's subject change handler.
- HEADERS.DAT parsing now uses byte-offset-based matching instead of fragile string prefix matching, greatly improving accuracy for QWKE packets.
- QWK message body parsing now allows Ctrl-A (0x01), Ctrl-C (0x03), and ESC (0x1B) bytes through for BBS color code support.
- Smart 0xE3 handling: distinguishes between QWK newline markers and UTF-8 3-byte lead bytes by checking for valid continuation bytes.
- `toupper()`/`tolower()`/`isalpha()` calls now cast to `unsigned char` to prevent undefined behavior with extended ASCII characters (portability fix for macOS/BSD).
- Used `<clocale>` instead of `<locale.h>` for proper C++ header usage.
- Added missing `<cctype>` includes where character classification functions are used.

## [0.50] - 2026-03-24

### Added
- Initial release
- Cross-platform text-based QWK offline mail reader (Linux, macOS, BSD, Windows)
- QWK packet parsing: CONTROL.DAT, MESSAGES.DAT, NDX index files, HEADERS.DAT
- DDMsgReader-inspired message reading UI with scrollable lightbar navigation
- SlyEdit-inspired message editor with Ice and DCT visual modes
- Theme support with configurable `.ini` color theme files
- Quote window for selecting and inserting quoted text
- Built-in spell checker with multiple English dictionaries
- Tagline support with random or manual selection
- REP reply packet creation (ZIP format)
- File browser for selecting QWK packets
- Standalone `config` utility for configuring settings outside the main app
- Persistent settings via `slymail.ini`
- CP437 box-drawing character support via platform abstraction layer
- Windows support via Win32 Console API (Visual Studio 2022 and MinGW/MSYS2)
