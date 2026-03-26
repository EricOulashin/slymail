# Changelog

All notable changes to SlyMail are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/), and this project adheres to [Semantic Versioning](https://semver.org/).

## [0.52] - 2026-03-26

### Added
- **REP directory**: A dedicated REP directory is created automatically within the SlyMail data directory for storing reply packets.
- **Save REP packet hotkey (Ctrl-P)**: Save the reply packet at any time from the conference list or message list, without needing to exit SlyMail.
- **Save REP prompt after composing a message**: After saving a reply or new message, SlyMail prompts whether to save the REP packet immediately, so it can be uploaded right away.
- **Remote systems from conference/message lists (Ctrl-R)**: The remote systems browser is now accessible via Ctrl-R from the conference list and message list, not just the file browser.
- **Upload reply packets to remote systems**: Press 'U' when connected to a remote system to upload a .rep file via FTP or SFTP. Browse for the file locally, upload with a progress spinner, and optionally delete the local file after a successful upload. This addresses [issue 7](https://github.com/EricOulashin/SlyMail/issues/7).
- **Quote line re-wrapping**: When replying to a message, quote lines are re-wrapped to fit within the configured quote line width (default 79 characters). Consecutive lines with the same quote prefix are grouped into paragraphs, joined, and re-wrapped. Blank lines, tear lines, and origin lines are preserved. Supports all common BBS quote prefix formats including nested quotes, initials-based quotes, and multi-level quoting. This addresses [issue 10](https://github.com/EricOulashin/SlyMail/issues/10).
- **Updated help screens**: Conference list and message list help screens (? / F1) now list all applicable hotkeys including / (search), V (polls), Ctrl-R (remote systems), Ctrl-P (save REP), and ? / F1 (help).
- **Directory chooser**: A visual directory browser is now used when selecting the reply packet directory in settings, instead of a plain text input field.

### Changed
- **Faster editor screen updates**: Cursor-only movements (arrow keys, Home, End, PageUp, PageDown) no longer redraw the edit area, eliminating visible flicker when holding down arrow keys. Only actual content changes trigger a redraw.
- **Paragraph-aware message saving**: When saving a message, word-wrapped display lines within the same paragraph are joined back into a single line. Only explicit Enter presses create line breaks in the saved message. This produces cleaner message formatting for recipients.
- **Reply packet default directory**: Reply packets are now saved to the REP directory within the SlyMail data directory by default, instead of alongside the QWK source file.
- **Reply directory setting**: The reply packet directory setting in all settings dialogs now opens a directory browser for easier selection.
- **Theme setting location**: The "Select theme file" option was removed from the Editor Settings (Ctrl-U) dialog since it already exists in the Theme Settings of the config program.
- **Improved error messages**: Error dialogs now provide more specific and helpful messages when operations fail. Recoverable situations (such as invalid input or missing optional files) are handled gracefully without showing unnecessary error dialogs.
- **Improved ANSI rendering**: Greatly expanded support for ANSI color and attribute codes when displaying messages. Newly supported: normal/dim intensity, reverse video, conceal/reveal, default colors, bright foreground and background colors, 256-color mode, and truecolor (24-bit RGB). Messages with ANSI art and color formatting now display much more accurately.
- **Improved quote prefix detection**: The quote prefix detector now correctly identifies all common BBS quote formats including spaced nested quotes (> > ), triple nesting (> > > ), and initials combined with nesting (EO> > ).

### Fixed
- **Editor color code handling**: Color codes embedded in message text are now handled correctly during all editing operations. Overwrite mode, Enter, word wrap, cursor movement, backspace, and delete all preserve the integrity of color codes. Arrow keys skip over color codes in a single keypress rather than requiring multiple presses.
- **Editor flicker eliminated**: Holding down arrow keys no longer causes the screen to flicker. This addresses [issue 14](https://github.com/EricOulashin/SlyMail/issues/14).
- **Editor word wrap behavior**: Text now wraps correctly when inserting in the middle of a line. Wrapped words flow to the next line's existing content instead of creating a new line for each wrapped word. Deleting text causes words to flow back up from the next line to fill available space. Lines are re-wrapped after being merged by backspace or delete.
- **Editor subject field cursor**: Pressing Ctrl-S to change the subject now places the cursor at the correct position in both Ice and DCT editor modes.
- **Quote prefix trailing space**: The trailing space in the configured quote prefix (e.g., " > ") is no longer stripped when loading settings, so quotes display correctly.
- **Message reader screen clear**: The screen is now fully cleared before displaying a message, preventing remnants of the previous screen from being briefly visible. This addresses [issue 8](https://github.com/EricOulashin/SlyMail/issues/8).
- **REP save tracking**: On exit, SlyMail only prompts to save the REP packet if there are messages that haven't been saved since the last REP save.
- **Upload file filter**: When uploading a reply packet to a remote system, the file browser now correctly shows .rep files instead of only .qwk files.
- **Synchronet poll display**: Polls defined only in VOTING.DAT (common with Synchronet) now appear in the message list at their correct chronological position. Poll vote counts are now tallied correctly.
- **Vote response filtering**: Vote/ballot response messages are hidden from the message list to reduce clutter. Their vote data is still counted in the tallies shown on the voted-on message.

## [0.51] - 2026-03-25

### Added
- **QWKE (Extended QWK) support**: Improved handling of the extended QWK format, including accurate matching of extended To/From/Subject fields, UTF-8 detection, and message ID tracking. REP packets now include HEADERS.DAT for extended fields.
- **BBS color/attribute code support**: Interprets color codes from multiple BBS software packages in both the message reader and editor: ANSI escape codes (always enabled), Synchronet Ctrl-A codes, WWIV heart codes, PCBoard/Wildcat @X codes, Celerity pipe codes, and Renegade pipe codes.
- **Per-system attribute code toggles**: Each BBS color code type can be individually enabled or disabled via the Attribute Code Toggles sub-dialog in Reader Settings or the config utility. These settings affect both the reader and editor.
- **Strip ANSI codes option**: When enabled, strips all ANSI escape codes from message text for plain display.
- **Synchronet voting support**: Full support for polls and message voting via the VOTING.DAT extension. Includes poll display with vote counts and percentage bars, ballot selection UI, up/down voting, vote tallies in the message header, and vote queueing in REP packets. Poll browser accessible via V from the conference list.
- **File attachment support**: Detects file attachments in messages, shows an [ATT] indicator in the message header, and allows downloading attachments with D or Ctrl-D.
- **UTF-8 support**: Detects and displays UTF-8 characters correctly on compatible terminals. Shows a [UTF8] indicator in the message header. Includes CP437-to-UTF-8 conversion for legacy BBS content.
- **Editor color picker (Ctrl-K)**: Opens a dialog to select foreground and background colors with a live preview. Inserts ANSI escape codes at the cursor position. Includes Normal (reset) and High (bright/bold) attribute options.
- **SlyMail data directory**: Settings and data are now stored in `~/.slymail` (Linux/macOS/BSD) or the user's home directory (Windows). The data directory is created automatically on first run.
- **Remote systems directory (Ctrl-R)**: Download QWK packets from remote BBSes via FTP or SFTP directly from the file browser. Manage remote system connections with name, host, port, credentials, and connection type. Passwords are masked during input and encrypted in the configuration file.
- **Search and filtering (/ key)**: Search conferences by name or search messages by subject, body, from, or to. Includes an advanced search dialog (Ctrl-A) with date range filtering via a visual calendar date picker. Press Q to clear the filter.
- **Regex search option**: Toggle in Reader Settings to treat search text as a regular expression instead of a plain substring.
- **Splash screen toggle**: Setting to enable or disable the startup splash screen.
- **Command-line interface**: `-qwk_file=<path>` to open a QWK packet directly (bypasses splash screen and file browser), `-v`/`--version` for version info, `-?`/`--help` for help screen.
- **File browser help screen**: Press ? or F1 in the file browser for a list of available hotkeys.
- **Remote file browser help screen**: Press ? or F1 when connected to a remote system for a help screen showing available hotkeys and connection info.
- **M key in message reader**: Returns to the message list, same as Q/C/ESC.

### Changed
- **Conference and message list position memory**: Returning from reading a message or from a conference preserves the previously selected position.
- **Color picker remembers last selection**: Re-opening Ctrl-K starts with the previously selected colors.
- **Date picker improvements**: Focus starts on the calendar; Tab cycles through the month navigation buttons, year field, and OK/Cancel buttons. The year field accepts digit-only input.
- **Remote system editor**: Selected field highlighting extends the full width of the dialog. DEL key deletes a remote system entry in addition to D.
- **Download progress indicator**: A spinning character is shown during FTP/SFTP file transfers.
- **Settings storage**: The settings file is now stored in the SlyMail data directory instead of alongside the executable.
- **Help bar appearance**: The help bar at the bottom of the screen uses a new color scheme with grey background, red hotkeys, blue text, and magenta parentheses. Text is horizontally centered.
- **REP packets include HEADERS.DAT and VOTING.DAT**: Reply packets now include extended headers for long fields and any pending votes.
- **Q key clears search filter**: In the conference and message lists, pressing Q while a search filter is active clears the filter instead of quitting.

### Fixed
- **Synchronet poll display**: Polls from Synchronet BBSes now appear correctly in the message list at their chronological position, with accurate vote counts and percentage bars.
- **Subject field cursor**: Fixed incorrect cursor placement when changing the subject with Ctrl-S in both Ice and DCT editor modes.
- **QWKE header matching**: Extended header fields are now matched accurately by message offset, improving reliability with various QWK packet formats.
- **UTF-8 detection in QWK bodies**: The QWK parser now correctly distinguishes between QWK newline markers and UTF-8 multi-byte sequences.
- **macOS/BSD compatibility**: Fixed potential issues with character classification functions on non-Linux platforms.

## [0.50] - 2026-03-24

### Added
- Initial release
- Cross-platform text-based QWK offline mail reader (Linux, macOS, BSD, Windows)
- QWK packet parsing with support for CONTROL.DAT, MESSAGES.DAT, NDX index files, and HEADERS.DAT
- DDMsgReader-inspired message reading UI with scrollable lightbar navigation
- SlyEdit-inspired message editor with Ice and DCT visual modes
- Theme support with configurable color theme files
- Quote window for selecting and inserting quoted text
- Built-in spell checker with multiple English dictionaries
- Tagline support with random or manual selection
- REP reply packet creation
- File browser for selecting QWK packets
- Standalone config utility for configuring settings outside the main application
- Persistent settings
- CP437 box-drawing character support
- Windows support via Win32 Console API (Visual Studio 2022 and MinGW/MSYS2)
