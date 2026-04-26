*(Maschinelle Übersetzung — für die englische Originalversion siehe [README.md](README.md))*

# SlyMail

Dies ist ein plattformübergreifender, textbasierter Offline-Mail-Reader für das [QWK](https://en.wikipedia.org/wiki/QWK_(file_format))-Paketformat. Das QWK-Paketformat wurde/wird häufig zum Austausch von Nachrichten auf [Bulletin-Board-Systemen](https://en.wikipedia.org/wiki/Bulletin_board_system) verwendet.

SlyMail bietet eine voll ausgestattete Oberfläche zum Lesen und Beantworten von Nachrichten aus BBS-QWK-Mail-Paketen (Bulletin Board System). Die Benutzeroberfläche ist inspiriert von [Digital Distortion Message Reader (DDMsgReader)](https://github.com/SynchronetBBS/sbbs/tree/master/xtrn/DDMsgReader) für das Lesen von Nachrichten und [SlyEdit](https://github.com/SynchronetBBS/sbbs/tree/master/exec) für das Bearbeiten von Nachrichten, beide ursprünglich erstellt für [Synchronet BBS](https://www.synchro.net/).

SlyMail wurde mit Hilfe von Claude AI erstellt.

## Funktionen

### QWK-Paket-Unterstützung
- Öffnet und liest Standard-QWK-Mail-Pakete (.qwk-Dateien)
- Analysiert CONTROL.DAT, MESSAGES.DAT und NDX-Indexdateien
- Vollständige QWKE-Unterstützung (erweitertes QWK) über HEADERS.DAT — offset-basiertes Matching für genaue erweiterte To/From/Subject-Felder, UTF-8-Flag und RFC822 Message-ID
- QWKE-Body-Kludge-Analyse (`To:`, `From:`, `Subject:` am Nachrichtenanfang)
- Verarbeitet Synchronet-style Konferenznummerierung
- Erstellt REP-Antwortpakete (.rep-Dateien) zum Hochladen zurück auf das BBS, einschließlich HEADERS.DAT für erweiterte Felder und VOTING.DAT für ausstehende Abstimmungen
- Unterstützt Microsoft Binary Format (MBF) Float-Kodierung in NDX-Dateien
- Merkt sich die zuletzt geöffnete QWK-Datei und das Verzeichnis zwischen Sitzungen

### Nachrichtenlesen (DDMsgReader-Stil)
- Konferenzliste mit Nachrichtenanzahl
- Scrollbare Nachrichtenliste mit Lightbar-Navigation
- Vollständiger Nachrichten-Reader mit Kopfzeilenanzeige (Von, An, Betreff, Datum)
- Zitat-Zeilenhervorhebung (unterstützt mehrstufiges Zitieren)
- Kludge-Zeilenanzeige (optional)
- Scrollbalken-Indikator
- Tastaturnavigation: Erste/Letzte/Nächste/Vorherige Nachricht, Seite auf/ab
- Hilfebildschirme zugänglich mit `?` oder `F1` in allen Ansichten

### BBS Farb- und Attributcode-Unterstützung
SlyMail interpretiert Farb-/Attributcodes aus mehreren BBS-Softwarepaketen und stellt sie als farbigen Text sowohl im Nachrichten-Reader als auch im Nachrichten-Editor dar. Unterstützte Formate:
- **ANSI-Escape-Codes** — immer aktiviert; Standard-SGR-Sequenzen (ESC[...m) für Vordergrund, Hintergrund, Fettschrift
- **Synchronet Ctrl-A-Codes** — `\x01` + Attributzeichen (z. B. `\x01c` für Cyan, `\x01h` für hell)
- **WWIV-Herz-Codes** — `\x03` + Ziffer 0–9
- **PCBoard/Wildcat @X-Codes** — `@X##` wobei die zwei Hexziffern Hintergrund- und Vordergrundfarbe kodieren
- **Celerity-Pipe-Codes** — `|` + Buchstabe (z. B. `|c` für Cyan, `|W` für hellweiß)
- **Renegade-Pipe-Codes** — `|` + zweistellige Zahl 00–31

Jeder BBS-Code-Typ kann einzeln über das Untermenü **Attributcode-Umschalter** in den Reader-Einstellungen oder das Dienstprogramm `config` aktiviert oder deaktiviert werden. Diese Umschalter wirken sich sowohl auf den Reader als auch auf den Editor aus. Eine separate Option **ANSI-Codes entfernen** entfernt alle ANSI-Sequenzen aus Nachrichten, wenn aktiviert.

### Dateianhänge
- Erkennt Dateianhänge, die über `@ATTACH:`-Kludge-Zeilen in Nachrichtentexten referenziert werden
- Zeigt einen **[ATT]**-Indikator in der Nachrichtenkopfzeile an, wenn Anhänge vorhanden sind
- Drücken Sie **D** oder **Ctrl-D** im Reader, um Anhänge herunterzuladen — zeigt eine Dateiliste mit Größen und fragt nach einem Zielverzeichnis

### Abstimmungen & Umfragen (Synchronet QWKE)
SlyMail unterstützt die Synchronet VOTING.DAT-Erweiterung für Umfragen und Nachrichtenabstimmungen:
- **Umfragen**: Als Umfragen identifizierte Nachrichten zeigen ihre Antwortoptionen mit Stimmenzählungen und Prozentbalken an. Drücken Sie **V**, um einen Stimmzettel-Dialog zu öffnen, in dem Sie Antwortauswahlen umschalten und Ihre Stimme abgeben können.
- **Auf-/Abstimmen**: Für reguläre (Nicht-Umfrage-)Nachrichten drücken Sie **V**, um hoch- oder runterzustimmen. Aktuelle Stimmauszählungen und Punktestand werden in der Nachrichtenkopfzeile angezeigt.
- **Stimmauszählungen**: Die Nachrichtenkopfzeile zeigt Hoch-/Runter-Stimmen und Nettopunktestand an, mit einem Indikator, ob Sie bereits abgestimmt haben.
- **Abstimmungs-Warteschlange**: Abstimmungen werden zusammen mit Nachrichtenantworten in die Warteschlange gestellt und in VOTING.DAT im REP-Paket für den Upload auf das BBS geschrieben.
- **Umfragen-Browser**: Drücken Sie **V** aus der Konferenzliste, um alle Umfragen im Paket zu durchsuchen.

### UTF-8-Unterstützung
- Erkennt UTF-8-Inhalt in Nachrichten (über HEADERS.DAT `Utf8`-Flag und automatische Erkennung von UTF-8-Byte-Sequenzen)
- Zeigt UTF-8-Zeichen korrekt auf kompatiblen Terminals an
- Zeigt einen **[UTF8]**-Indikator in der Nachrichtenkopfzeile für UTF-8-Nachrichten
- Speichert neue Nachrichten mit korrekter Kodierung
- CP437-zu-UTF-8-Konvertierung für veraltete BBS-Inhalte
- Setzt Locale auf Linux/macOS/BSD (`setlocale(LC_ALL, "")`) und UTF-8-Codepage auf Windows für korrekte Terminal-Darstellung

### Nachrichten-Editor (inspiriert von SlyEdit)
- **Zwei visuelle Modi**: Ice und DCT, jeweils mit eigenen Farbschemata und Layouts
- **Zufallsmodus**: Wählt bei jeder Bearbeitungssitzung zufällig Ice oder DCT aus
- **Wechselnde Rahmenfarben**: Rahmenzeichen wechseln zufällig zwischen zwei Themenfarben, passend zum visuellen Stil von SlyEdit
- **Themen-Unterstützung**: Konfigurierbare Farbthemen werden aus .ini-Dateien geladen
- Vollbild-Texteditor mit Zeilenumbruch
- Zitatfenster zum Auswählen und Einfügen von zitiertem Text (Ctrl-Q zum Öffnen/Schließen)
- Antworten und neue Nachrichtenerstellung
- ESC-Menü zum Speichern, Abbrechen, Einfügen/Überschreiben umschalten und mehr
- **Ctrl-K Farbauswahl**: Öffnet einen Dialog zur Auswahl von Vorder- und Hintergrundfarben und fügt einen ANSI-Escape-Code an der Cursorposition ein. Unterstützt 16 Vordergrundfarben (8 normal + 8 hell) und 8 Hintergründe mit Live-Vorschau. Drücken Sie **N**, um einen Reset-Code einzufügen.
- **Farbbewusstes Rendering**: Der Bearbeitungsbereich rendert ANSI- und BBS-Attributcodes inline, sodass farbiger Text beim Tippen angezeigt wird
- **Ctrl-U Benutzereinstellungen-Dialog** zum direkten Konfigurieren von Editor-Einstellungen
- **Stilspezifische Ja/Nein-Eingabeaufforderungen**: Der Ice-Modus verwendet eine Inline-Eingabeaufforderung am unteren Bildschirmrand; DCT-Modus verwendet ein zentriertes Dialogfeld mit thematischen Farben

### Editor-Einstellungen (über Ctrl-U)
- **UI-Modus wählen**: Dialog zum Wechseln zwischen Ice-, DCT- und Random-Stilen (wirkt sofort)
- **Thema-Datei auswählen**: Wählen Sie aus verfügbaren Ice- oder DCT-Farbthemen
- **Taglines**: Wenn aktiviert, wird beim Speichern eine Tagline-Auswahl angezeigt (aus `tagline_files/taglines.txt`)
- **Rechtschreibprüfungs-Wörterbuch/Wörterbücher**: Wählen Sie aus, welche Wörterbücher verwendet werden sollen
- **Beim Speichern zur Rechtschreibprüfung auffordern**: Wenn aktiviert, wird vor dem Speichern eine Rechtschreibprüfung angeboten
- **Zitat-Zeilen auf Terminalbreite umbrechen**: Zitierte Zeilen mit Zeilenumbruch versehen
- **Mit Initialen des Autors zitieren**: Zitat-Zeilen mit den Initialen des Autors voranstellen (z. B. `MP> `)
- **Zitat-Zeilen mit Initialen einrücken**: Leerzeichen vor den Initialen hinzufügen (z. B. ` MP> `)
- **Leerzeichen aus Zitat-Zeilen entfernen**: Führende Leerzeichen aus zitiertem Text entfernen

### Farbthemen
- Thema-Dateien sind Konfigurationsdateien (`.ini`) im Verzeichnis `config_files/`
- Ice-Themen: `EditorIceColors_*.ini` (BlueIce, EmeraldCity, FieryInferno, usw.)
- DCT-Themen: `EditorDCTColors_*.ini` (Default, Default-Modified, Midnight)
- Thema-Farben verwenden ein einfaches Format: Vordergrundsbuchstabe (`r`/`g`/`b`/`c`/`y`/`m`/`w`/`k`), optionales `h` für hell, optionale Hintergrundziffer (`0`-`7`)
- Themen steuern alle UI-Elementfarben: Rahmen, Beschriftungen, Werte, Zitatfenster, Hilfsleiste, Ja/Nein-Dialoge

### Rechtschreibprüfung
- Eingebaute Rechtschreibprüfung mit einfachen Textdateien als Wörterbücher
- Wird mit englischen Wörterbüchern geliefert (en, en-US, en-GB, en-AU, en-CA-Ergänzungen)
- Interaktiver Korrektur-Dialog: Ersetzen, Überspringen oder Beenden
- Wörterbuchdateien werden im Verzeichnis `dictionary_files/` gespeichert

### Taglines
- Tagline-Dateien werden im Verzeichnis `tagline_files/` gespeichert
- Die Standard-Tagline-Datei ist `tagline_files/taglines.txt`, eine Tagline pro Zeile
- Zeilen, die mit `#` oder `;` beginnen, werden als Kommentare behandelt und ignoriert
- Wählen Sie eine bestimmte Tagline oder eine zufällige, wenn Sie eine Nachricht speichern
- Taglines werden mit einem `...`-Präfix an Nachrichten angehängt

### REP-Paketerstellung
- Derzeit wird nur ZIP unterstützt (Ich möchte in Zukunft mehr Komprimierungstypen hinzufügen)
- Wenn Sie Antworten oder neue Nachrichten schreiben, werden sie als ausstehend in die Warteschlange gestellt
- Abstimmungen (Umfrage-Stimmzettel, Hoch-/Runter-Abstimmungen) werden ebenfalls zusammen mit Antworten in die Warteschlange gestellt
- Beim Beenden (oder beim Öffnen einer neuen QWK-Datei) fordert SlyMail auf, alle ausstehenden Elemente zu speichern
- Erstellt eine Standard-`.rep`-Datei (ZIP-Archiv) zum Hochladen auf das BBS, die enthält:
  - `<BBSID>.MSG` — Antwortnachrichten im Standard-QWK-Format
  - `HEADERS.DAT` — QWKE-erweiterte Kopfzeilen für Felder, die 25 Zeichen überschreiten
  - `VOTING.DAT` — Ausstehende Abstimmungen im Synchronet-kompatiblen INI-Format
- REP-Datei wird als `<BBS-ID>.rep` im konfigurierten Antwortverzeichnis (oder dem Verzeichnis der QWK-Datei) gespeichert

### Remote-Systeme (Ctrl-R)
SlyMail kann QWK-Pakete direkt von Remote-Systemen über FTP oder SFTP (SSH) herunterladen:
- Drücken Sie **Ctrl-R** im Dateibrowser, um das Remote-System-Verzeichnis zu öffnen
- **Hinzufügen/Bearbeiten/Löschen** von Remote-System-Einträgen mit: Name, Host, Port, Verbindungstyp (FTP oder SSH), Benutzername, Passwort, passiver FTP-Umschalter und anfänglicher Remote-Pfad
- **Remote-Verzeichnisse durchsuchen** mit einem Datei-/Verzeichnis-Browser ähnlich dem lokalen Dateibrowser — in Verzeichnisse navigieren, mit `..` aufwärts gehen, mit `/` zur Wurzel springen
- **QWK-Dateien herunterladen** vom Remote-System direkt in das `QWK`-Unterverzeichnis des SlyMail-Datenverzeichnisses
- Remote-System-Einträge werden in `remote_systems.json` im SlyMail-Datenverzeichnis gespeichert
- Das letzte Verbindungsdatum/-uhrzeit wird für jedes System nachverfolgt
- Verwendet den `curl`-Befehl des Systems für FTP- und SFTP-Übertragungen (keine Compiler-Bibliotheksabhängigkeiten)

### Anwendungseinstellungen
- Persistente Einstellungen werden in `slymail.ini` im SlyMail-Datenverzeichnis gespeichert (`~/.slymail` unter Linux/macOS/BSD oder dem Heimatverzeichnis des Benutzers unter Windows)
- Das SlyMail-Datenverzeichnis und sein `QWK`-Unterverzeichnis werden beim ersten Start automatisch erstellt
- Standard-QWK-Datei-Durchsuch- und REP-Paketspeicherverzeichnis ist `~/.slymail/QWK`
- Merkt sich zuletzt durchsuchtes Verzeichnis und QWK-Dateiname
- Ctrl-L-Hotkey zum Laden einer anderen QWK-Datei aus Konferenz- oder Nachrichtenlistenansichten
- Konfigurierbares Zitat-Präfix, Zeilenbreite für Zitate, Benutzername
- Reader-Optionen: Kludge-Zeilen anzeigen/ausblenden, Abriss-/Ursprungszeilen, Scrollbalken, ANSI-Codes entfernen
- Pro-BBS-Attributcode-Umschalter (Synchronet, WWIV, Celerity, Renegade, PCBoard/Wildcat) — wirken sich auf Reader und Editor aus
- REP-Paket-Ausgabeverzeichnis

## Screenshots

<p align="center">
	<a href="screenshots/SlyMail_01_OpeningScreen.png" target='_blank'><img src="screenshots/SlyMail_01_OpeningScreen.png" alt="Startbildschirm" width="800"></a>
	<a href="screenshots/SlyMail_02_File_Chooser.png" target='_blank'><img src="screenshots/SlyMail_02_File_Chooser.png" alt="Dateiauswahl: Suche nach QWK-Datei" width="800"></a>
	<a href="screenshots/SlyMail_03_remote_system_list.png" target='_blank'><img src="screenshots/SlyMail_03_remote_system_list.png" alt="Remote-Systemliste" width="800"></a>
	<a href="screenshots/SlyMail_04_Remote_System_Edit.png" target='_blank'><img src="screenshots/SlyMail_04_Remote_System_Edit.png" alt="Remote-System bearbeiten" width="800"></a>
	<a href="screenshots/SlyMail_05_Remote_System_Browsing.png" target='_blank'><img src="screenshots/SlyMail_05_Remote_System_Browsing.png" alt="Remote-System-Durchsuchen" width="800"></a>
	<a href="screenshots/SlyMail_06_msg_area_list.png" target='_blank'><img src="screenshots/SlyMail_06_msg_area_list.png" alt="Nachrichtenbereichsliste" width="800"></a>
	<a href="screenshots/SlyMail_07_msg_list.png" target='_blank'><img src="screenshots/SlyMail_07_msg_list.png" alt="Nachrichtenliste" width="800"></a>
	<a href="screenshots/SlyMail_08_reading_msg.png" target='_blank'><img src="screenshots/SlyMail_08_reading_msg.png" alt="Eine Nachricht lesen" width="800"></a>
	<a href="screenshots/SlyMail_09_msg_edit_start.png" target='_blank'><img src="screenshots/SlyMail_09_msg_edit_start.png" alt="Beginn der Nachrichtenbearbeitung" width="800"></a>
	<a href="screenshots/SlyMail_10_quote_line_selection.png" target='_blank'><img src="screenshots/SlyMail_10_quote_line_selection.png" alt="Editor: Zitat-Zeilenauswahl" width="800"></a>
	<a href="screenshots/SlyMail_11_writing_reply_msg.png" target='_blank'><img src="screenshots/SlyMail_11_writing_reply_msg.png" alt="Editor: Eine Nachricht bearbeiten" width="800"></a>
	<a href="screenshots/SlyMail_12_editor_color_picker.png" target='_blank'><img src="screenshots/SlyMail_12_editor_color_picker.png" alt="Editor: Farbauswahl" width="800"></a>
	<a href="screenshots/SlyMail_13_Sync_poll_msg.png" target='_blank'><img src="screenshots/SlyMail_13_Sync_poll_msg.png" alt="Synchronet-Umfragenachricht" width="800"></a>
	<a href="screenshots/SlyMail_14_reader_settings.png" target='_blank'><img src="screenshots/SlyMail_14_reader_settings.png" alt="Reader-Einstellungen" width="800"></a>
	<a href="screenshots/SlyMail_15_editor_settings.png" target='_blank'><img src="screenshots/SlyMail_15_editor_settings.png" alt="Editor-Einstellungen" width="800"></a>
	<a href="screenshots/SlyMail_16_msg_search.png" target='_blank'><img src="screenshots/SlyMail_16_msg_search.png" alt="Nachrichtensuche" width="800"></a>
	<a href="screenshots/SlyMail_17_Advanced_msg_search.png" target='_blank'><img src="screenshots/SlyMail_17_Advanced_msg_search.png" alt="Erweiterte Nachrichtensuche" width="800"></a>
	<a href="screenshots/SlyMail_18_advanced_msg_search_date_picker" target='_blank'><img src="screenshots/SlyMail_18_advanced_msg_search_date_picker" alt="Datumsauswahl in erweiterter Nachrichtensuche" width="800"></a>
	<a href="screenshots/SlyMail_19_config_program.png" target='_blank'><img src="screenshots/SlyMail_19_config_program.png" alt="Konfigurationsprogramm" width="800"></a>
	<a href="screenshots/SlyMail_20_reading_ANSI_art.png" target='_blank'><img src="screenshots/SlyMail_20_reading_ANSI_art.png" alt="Eine Nachricht mit ANSI-Kunst lesen" width="800"></a>
	<a href="screenshots/SlyMail_21_reading_ANSI_art.png" target='_blank'><img src="screenshots/SlyMail_21_reading_ANSI_art.png" alt="Eine Nachricht mit ANSI-Kunst lesen" width="800"></a>
	<a href="screenshots/SlyMail_22_reading_ANSI_art.png" target='_blank'><img src="screenshots/SlyMail_22_reading_ANSI_art.png" alt="Eine Nachricht mit ANSI-Kunst lesen" width="800"></a>
	<a href="screenshots/SlyMail_23_reading_ANSI_art.png" target='_blank'><img src="screenshots/SlyMail_23_reading_ANSI_art.png" alt="Eine Nachricht mit ANSI-Kunst lesen" width="800"></a>
</p>

## Synchronet-Einstellungen für ein QWK-Paket
Auf einem Synchronet-BBS ist SlyMail in den QWK-Paketeinstellungen (oder sollte sein) mit Ctrl-A-Farbcodes, VOTING.DAT, Dateianhängen und dem QWKE-Paketformat kompatibel. SlyMail sollte auch mit UTF-8 kompatibel sein. Zum Beispiel:
<table>
<tr><<td aligh='right'>Ctrl-A Color Codes</td><td>Leave in</td></tr>
<tr><<td aligh='right'>Archive Type</td><td>ZIP</td></tr>
<tr><<td aligh='right'>Include E-Mail Messages</td><td>Un-Read Only</td></tr>
<tr><<td aligh='right'>Include File Attachments</td><td>Yes</td></tr>
<tr><<td aligh='right'>Delete E-mail Automatically</td><td>No</td></tr>
<tr><<td aligh='right'>Include New Files List</td><td>Yes</td></tr>
<tr><<td aligh='right'>Include Index Files</td><td>Yes</td></tr>
<tr><<td aligh='right'>Include Control Files</td><td>Yes</td></tr>
<tr><<td aligh='right'>Include VOTING.DAT File</td><td>Yes</td></tr>
<tr><<td aligh='right'>Include HEADERS.DAT File</td><td>Yes</td></tr>
<tr><<td aligh='right'>Include Messages from You</td><td>No</td></tr>
<tr><<td aligh='right'>Include Time Zone (@TZ)</td><td>No</td></tr>
<tr><<td aligh='right'>Include Message Path (@VIA)</td><td>No</td></tr>
<tr><<td aligh='right'>Include Message/Reply IDs</td><td>No</td></tr>
<tr><<td aligh='right'>Include UTF-8 Characters</td><td>Yes</td></tr>
<tr><<td aligh='right'>MIME-encoded Message Text</td><td>No</td></tr>
<tr><<td aligh='right'>Extended (QWKE) Packet Format</td><td>Yes</td></tr>
</table>

## Erstellen

### Anforderungen

**Linux / macOS / BSD:**
- C++17-kompatibler Compiler (GCC 8+, Clang 7+)
- ncurses-Entwicklungsbibliothek (`libncurses-dev` auf Debian/Ubuntu, `ncurses-devel` auf Fedora/RHEL)
- `unzip`-Befehl (zum Extrahieren von QWK-Paketen)
- `zip`-Befehl (zum Erstellen von REP-Paketen)
- `curl`-Befehl (für Remote-System-FTP/SFTP-Übertragungen — optional, nur für die Remote-Systeme-Funktion benötigt)

**Windows (Visual Studio 2022):**
- Visual Studio 2022 mit der Arbeitslast "Desktopentwicklung mit C++"
- Windows SDK 10.0 (in VS enthalten)
- Keine zusätzlichen Bibliotheken erforderlich — verwendet die integrierte Win32-Konsolen-API für die Terminal-UI sowie entweder `tar.exe` oder PowerShell für die QWK/REP-Paket-ZIP-Verarbeitung (siehe Hinweis unten)

**Windows (MinGW/MSYS2):**
- MinGW-w64 oder MSYS2 mit GCC (C++17-Unterstützung)
- Windows Console API (integriert)

> **Hinweis — QWK/REP-ZIP-Verarbeitung unter Windows:** SlyMail erkennt zur Laufzeit, welches Tool verfügbar ist, und verwendet die beste Option:
>
> - **`tar.exe` (bevorzugt):** Wird mit Windows 10 Version 1803 (April 2018-Update) und später sowie mit allen Versionen von Windows 11 geliefert. `tar` liest ZIP-Dateien anhand ihres Inhalts statt der Dateiendung, sodass `.qwk`-Pakete direkt extrahiert und `.rep`-Pakete über eine temporäre `.zip`-Datei erstellt werden, die dann umbenannt wird. Keine zusätzliche Konfiguration erforderlich.
> - **PowerShell (Ausweichlösung):** Wenn `tar.exe` nicht im PATH gefunden wird, greift SlyMail auf PowerShell zurück. Zum Extrahieren verwendet es die .NET `ZipFile`-Klasse (`System.IO.Compression`) statt `Expand-Archive`, da `Expand-Archive` Nicht-`.zip`-Dateiendungen ablehnt, auch wenn die Datei ein gültiges ZIP-Archiv ist. Für die REP-Paketerstellung verwendet es `Compress-Archive`, wobei ebenfalls eine temporäre `.zip`-Datei geschrieben wird, die dann in `.rep` umbenannt wird.

### Erstellen unter Linux/macOS/BSD

```bash
make
```

Dies erstellt zwei Programme:
- `slymail` - die QWK-Reader-Hauptanwendung
- `config` - das eigenständige Konfigurationsdienstprogramm

### Mit Debug-Symbolen erstellen

```bash
make debug
```

### Installieren (optional)

```bash
sudo make install    # Installs slymail and config to /usr/local/bin/
sudo make uninstall  # Remove
```

### Erstellen unter Windows mit Visual Studio 2022

Öffnen Sie die Lösungsdatei in Visual Studio 2022:

```
vs\SlyMail.sln
```

Oder über die Befehlszeile mit MSBuild erstellen:

```powershell
# Release build (output in vs\x64\Release\)
msbuild vs\SlyMail.sln /p:Configuration=Release /p:Platform=x64

# Debug build (output in vs\x64\Debug\)
msbuild vs\SlyMail.sln /p:Configuration=Debug /p:Platform=x64
```

Dies erstellt zwei ausführbare Dateien:
- `x64\Release\slymail.exe` — der Haupt-QWK-Reader
- `x64\Release\config.exe` — das eigenständige Konfigurationsdienstprogramm

Die Lösung enthält zwei Projekte (`SlyMail.vcxproj` und `Config.vcxproj`), die auf x64, C++17 mit dem MSVC v143-Toolset abzielen.

### Erstellen unter Windows (MinGW/MSYS2)

```bash
make
```

Das Makefile erkennt automatisch die Plattform und verwendet die entsprechende Terminal-Implementierung:
- **Linux/macOS/BSD**: ncurses (`terminal_ncurses.cpp`)
- **Windows**: conio + Win32 Console API (`terminal_win32.cpp`)

## Verwendung

```bash
# Launch SlyMail with file browser
./slymail

# Open a specific QWK packet
./slymail MYBBS.qwk

# Run the standalone configuration utility
./config
```

### Konfigurationsprogramm

Das Dienstprogramm `config` bietet eine eigenständige textbasierte Oberfläche zur Konfiguration der SlyMail-Einstellungen ohne Öffnen der Hauptanwendung. Es bietet vier Konfigurationskategorien:

- **Editor-Einstellungen** - Alle Einstellungen, die auch über Ctrl-U im Editor verfügbar sind (Editor-Stil, Taglines, Rechtschreibprüfung, Zitatoptionen usw.)
- **Reader-Einstellungen** - Kludge-Zeilen, Abrisszeilen, Scrollbalken, ANSI-Entfernung, Lightbar-Modus, umgekehrte Reihenfolge und Attributcode-Umschalter (pro BBS aktivieren/deaktivieren) umschalten
- **Themen-Einstellungen** - Ice- und DCT-Farbthema-Dateien aus dem Verzeichnis `config_files/` auswählen
- **Allgemeine Einstellungen** - Ihren Namen für Antworten und das REP-Paket-Ausgabeverzeichnis festlegen

Einstellungen werden beim Verlassen jeder Kategorie automatisch gespeichert. Sowohl SlyMail als auch das Konfigurationsdienstprogramm lesen und schreiben dieselbe Einstellungsdatei.

### Tastenbelegungen

#### Dateibrowser
| Taste | Aktion |
|-------|--------|
| Auf/Ab | Dateien und Verzeichnisse navigieren |
| Enter | Verzeichnis öffnen / QWK-Datei auswählen |
| Ctrl-R | Remote-System-Verzeichnis öffnen |
| Q / ESC | Beenden |

#### Konferenzliste
| Taste | Aktion |
|-------|--------|
| Auf/Ab | Konferenzen navigieren |
| Enter | Ausgewählte Konferenz öffnen |
| V | Umfragen/Abstimmungen im Paket anzeigen |
| O / Ctrl-L | Eine andere QWK-Datei öffnen |
| S / Ctrl-U | Einstellungen |
| Q / ESC | SlyMail beenden |
| ? / F1 | Hilfe |

#### Nachrichtenliste
| Taste | Aktion |
|-------|--------|
| Auf/Ab | Nachrichten navigieren |
| Enter / R | Ausgewählte Nachricht lesen |
| N | Neue Nachricht schreiben |
| G | Zu Nachrichtennummer springen |
| Ctrl-L | Eine andere QWK-Datei öffnen |
| S / Ctrl-U | Einstellungen |
| C / ESC | Zurück zur Konferenzliste |
| Q | Beenden |
| ? / F1 | Hilfe |

#### Nachrichten-Reader
| Taste | Aktion |
|-------|--------|
| Auf/Ab | Nachricht scrollen |
| Links/Rechts | Vorherige / Nächste Nachricht |
| F / L | Erste / Letzte Nachricht |
| R | Auf Nachricht antworten |
| V | Abstimmen (Hoch-/Runter-Abstimmung oder Umfrage-Stimmzettel) |
| D / Ctrl-D | Dateianhänge herunterladen |
| H | Nachrichtenkopfzeilen-Informationen anzeigen |
| S / Ctrl-U | Einstellungen |
| C / Q / ESC | Zurück zur Nachrichtenliste |
| ? / F1 | Hilfe |

#### Nachrichten-Editor
| Taste | Aktion |
|-------|--------|
| ESC | Editor-Menü (Speichern, Abbrechen usw.) |
| Ctrl-U | Benutzereinstellungen-Dialog |
| Ctrl-Q | Zitatfenster öffnen/schließen |
| Ctrl-K | Farbauswahl (ANSI-Farbcode an Cursorposition einfügen) |
| Ctrl-G | Grafikzeichen (CP437) nach Code einfügen |
| Ctrl-W | Wort-/Textsuche |
| Ctrl-S | Betreff ändern |
| Ctrl-D | Aktuelle Zeile löschen |
| Ctrl-Z | Nachricht speichern |
| Ctrl-A | Nachricht abbrechen |
| F1 | Hilfebildschirm |
| Insert | Einfügen/Überschreiben-Modus umschalten |

#### Zitatfenster
| Taste | Aktion |
|-------|--------|
| Auf/Ab | Zitat-Zeilen navigieren |
| Enter | Ausgewählte Zitat-Zeile einfügen |
| Ctrl-Q / ESC | Zitatfenster schließen |

## Architektur

SlyMail verwendet eine Plattformabstraktionsschicht für seine Text-Benutzeroberfläche:

```
ITerminal (abstract base class)
    ├── NCursesTerminal  (Linux/macOS/BSD - ncurses)
    └── Win32Terminal    (Windows - conio + Win32 Console API)
```

CP437-Rahmenzeichen und Sonderzeichen sind in `cp437defs.h` definiert und werden durch die `putCP437()`-Methode gerendert, die CP437-Codes auf plattformspezifische Äquivalente abbildet (ACS-Zeichen bei ncurses, direkte CP437-Bytes unter Windows).

### Quelldateien

| Datei | Beschreibung |
|-------|--------------|
| `terminal.h` | Abstrakte `ITerminal`-Schnittstelle, Tasten-/Farbkonstanten, Factory |
| `terminal_ncurses.cpp` | ncurses-Implementierung mit CP437-zu-ACS-Mapping |
| `terminal_win32.cpp` | Windows Console API + conio-Implementierung |
| `cp437defs.h` | IBM Code Page 437-Zeichendefinitionen |
| `colors.h` | Farbschemadefinitionen (Ice, DCT, Reader, Liste) |
| `theme.h` | Thema-Konfigurationsdatei-Parser (Synchronet-Stil-Attributcodes) |
| `ui_common.h` | Gemeinsame UI-Hilfsfunktionen (Dialoge, Texteingabe, Scrollbalken usw.) |
| `qwk.h` / `qwk.cpp` | QWK/REP-Paket-Parser und -Ersteller (QWKE, Anhänge, Abstimmungen) |
| `bbs_colors.h` / `bbs_colors.cpp` | BBS-Farb-/Attributcode-Parser (ANSI, Synchronet, WWIV, PCBoard, Celerity, Renegade) |
| `utf8_util.h` / `utf8_util.cpp` | UTF-8-Dienstprogramme (Validierung, Anzeigebreite, CP437-zu-UTF-8-Konvertierung) |
| `voting.h` / `voting.cpp` | VOTING.DAT-Parser, Stimmenauszählung, Umfrage-Anzeige-UI |
| `remote_systems.h` / `remote_systems.cpp` | Remote-System-Verzeichnis, FTP/SFTP-Durchsuchen, JSON-Persistenz, Home-Verzeichnis-Dienstprogramme |
| `settings.h` | Benutzereinstellungs-Persistenz |
| `settings_dialog.h` | Einstellungs-Dialoge (Editor, Reader, Attributcode-Umschalter) |
| `file_browser.h` | QWK-Dateibrowser und -Auswahl |
| `msg_list.h` | Konferenz- und Nachrichtenlistenansichten |
| `msg_reader.h` | Nachrichten-Reader (DDMsgReader-Stil) mit Abstimmungs- und Anhang-UI |
| `msg_editor.h` | Nachrichten-Editor (SlyEdit Ice/DCT-Stil) mit Farbauswahl |
| `main.cpp` | SlyMail-Anwendungseinstiegspunkt und Hauptschleife |
| `config.cpp` | Eigenständiges Konfigurationsdienstprogramm |

## Konfiguration

### Einstellungsdatei

Einstellungen werden in einer INI-Datei namens `slymail.ini` im selben Verzeichnis wie die SlyMail-ausführbare Datei gespeichert. Diese Datei wird sowohl von SlyMail als auch vom Dienstprogramm `config` gemeinsam genutzt. Die Datei ist gut kommentiert mit Beschreibungen jeder Einstellung.

Beispiel `slymail.ini`:
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

### Thema-Dateien

Farbthemen sind `.ini`-Dateien im Verzeichnis `config_files/`:

**Ice-Themen** (`EditorIceColors_*.ini`):
- BlueIce (Standard), EmeraldCity, FieryInferno, Fire-N-Ice, GeneralClean, GenericBlue, PurpleHaze, ShadesOfGrey

**DCT-Themen** (`EditorDCTColors_*.ini`):
- Default (Standard), Default-Modified, Midnight

Thema-Farbwerte verwenden ein kompaktes Format, das von Synchronet-Attributcodes abgeleitet ist:
- `n` = normal (zurücksetzen)
- Vordergrund: `k`=schwarz, `r`=rot, `g`=grün, `y`=gelb, `b`=blau, `m`=magenta, `c`=cyan, `w`=weiß
- `h` = hell/helle Intensität
- Hintergrundziffer: `0`=schwarz, `1`=rot, `2`=grün, `3`=braun, `4`=blau, `5`=magenta, `6`=cyan, `7`=hellgrau

Beispiel: `nbh` = normales helles Blau, `n4wh` = helles Weiß auf blauem Hintergrund

### Taglines

Taglines sind kurze Zitate oder Sprüche, die am Ende von Nachrichten beim Speichern angehängt werden. Die Tagline-Funktion kann über Ctrl-U im Editor oder das Dienstprogramm `config` aktiviert werden.

Taglines werden in `tagline_files/taglines.txt` gespeichert, eine pro Zeile. Zeilen, die mit `#` oder `;` beginnen, werden als Kommentare behandelt und ignoriert. Beim Speichern einer Nachricht mit aktivierten Taglines wird der Benutzer aufgefordert, entweder eine bestimmte Tagline auszuwählen oder eine zufällige zu wählen. Ausgewählte Taglines werden mit einem `...`-Präfix an die Nachricht angehängt (z. B. `...To err is human, to really foul things up requires a computer.`).

### Rechtschreibprüfung

SlyMail enthält eine eingebaute Rechtschreibprüfung, die einfache Textdateien als Wörterbücher verwendet. Die Rechtschreibprüfung kann so konfiguriert werden, dass sie beim Speichern über Ctrl-U im Editor oder das Dienstprogramm `config` aufgefordert wird.

**Wörterbuchdateien** sind einfache Textdateien (ein Wort pro Zeile), die in `dictionary_files/` gespeichert sind. Mehrere Wörterbücher können gleichzeitig ausgewählt werden, um eine kombinierte Wortabdeckung zu erhalten. SlyMail wird geliefert mit:
- `dictionary_en.txt` - Englisch (allgemein, ~130K Wörter)
- `dictionary_en-US-supplemental.txt` - US-Englisch-Ergänzung
- `dictionary_en-GB-supplemental.txt` - Britisch-Englisch-Ergänzung
- `dictionary_en-AU-supplemental.txt` - Australisch-Englisch-Ergänzung
- `dictionary_en-CA-supplemental.txt` - Kanadisch-Englisch-Ergänzung

Wenn die Rechtschreibprüfung ausgelöst wird, durchsucht der Prüfer die Nachricht nach falsch geschriebenen Wörtern und präsentiert für jedes einen interaktiven Dialog mit den Optionen, das Wort zu **E**rsetzen, zu **Ü**berspringen, **H**inzuzufügen (zukünftig) oder die Prüfung zu **B**eenden.

## Danksagungen

- UI inspiriert von [DDMsgReader](https://github.com/SynchronetBBS/sbbs) und [SlyEdit](https://github.com/SynchronetBBS/sbbs) von [Nightfox (Eric Oulashin)](https://github.com/EricOulashin)
- QWK-Format-Kompatibilität basiert auf [Synchronet BBS](https://www.synchro.net/)-Quellcode
- CP437-Zeichendefinitionen von Synchronet

## Lizenz

Dieses Projekt ist Open-Source-Software.
