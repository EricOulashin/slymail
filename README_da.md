*(Maskinoversættelse — for den engelske originalversion, se [README.md](README.md))*

# SlyMail

Dette er en platformsuafhængig tekstbaseret offline-postlæser til [QWK](https://en.wikipedia.org/wiki/QWK_(file_format))-pakkeformatet. QWK-pakkeformatet blev/bruges ofte til at udveksle post på [bulletin board-systemer](https://en.wikipedia.org/wiki/Bulletin_board_system).

SlyMail giver en komplet brugerflade til at læse og besvare meddelelser fra BBS (Bulletin Board System) QWK-postpakker. Brugerfladen er inspireret af [Digital Distortion Message Reader (DDMsgReader)](https://github.com/SynchronetBBS/sbbs/tree/master/xtrn/DDMsgReader) til meddelelselæsning og [SlyEdit](https://github.com/SynchronetBBS/sbbs/tree/master/exec) til meddelelseredigering, begge oprindeligt skabt til [Synchronet BBS](https://www.synchro.net/).

SlyMail er skabt med hjælp fra Claude AI.

## Funktioner

### QWK-pakkeunderstøttelse
- Åbner og læser standard-QWK-postpakker (.qwk-filer)
- Fortolker CONTROL.DAT, MESSAGES.DAT og NDX-indeksfiler
- Fuld QWKE-understøttelse (udvidet QWK) via HEADERS.DAT — offsetbaseret matchning for nøjagtige udvidede Til/Fra/Emne-felter, UTF-8-flag og RFC822 Message-ID
- QWKE-brødtekst-kludge-fortolkning (`To:`, `From:`, `Subject:` ved begyndelsen af meddelelser)
- Håndterer Synchronet-stil konferencenummerering
- Opretter REP-svarspakker (.rep-filer) til upload tilbage til BBS, herunder HEADERS.DAT for udvidede felter og VOTING.DAT for køede stemmer
- Understøtter Microsoft Binary Format (MBF) flydende-komma-kodning i NDX-filer
- Husker senest åbnede QWK-fil og katalog mellem sessioner

### Meddelelselæsning (DDMsgReader-stil)
- Konferenceliste med meddelelsesantal
- Rulbar meddelelseliste med lysstangsnavigation
- Komplet meddelelselæser med rubrikvisning (Fra, Til, Emne, Dato)
- Citatlinjemarkering (understøtter flerniveaucitater)
- Kludge-linjevisning (valgfri)
- Rullelisteindikator
- Tastaturnavigation: Første/Sidste/Næste/Forrige meddelelse, Side Op/Ned
- Hjælpeskærme tilgængelige med `?` eller `F1` i alle visninger

### BBS-farve- og attributkodesupport
SlyMail fortolker farve-/attributkoder fra flere BBS-softwarepakker og gengiver dem som farvet tekst i både meddelelselæseren og meddelelseredigereren. Understøttede formater:
- **ANSI escape-koder** — altid aktiverede; standard SGR-sekvenser (ESC[...m) for forgrund, baggrund, fed
- **Synchronet Ctrl-A-koder** — `\x01` + attributtegn (f.eks. `\x01c` for cyan, `\x01h` for lys)
- **WWIV hjertkoder** — `\x03` + ciffer 0–9
- **PCBoard/Wildcat @X-koder** — `@X##` hvor de to hexcifre koder baggrunds- og forgrundsfærve
- **Celerity pipe-koder** — `|` + bogstav (f.eks. `|c` for cyan, `|W` for lys hvid)
- **Renegade pipe-koder** — `|` + tocifret tal 00–31

Hver BBS-kodetype kan aktiveres eller deaktiveres individuelt via underdialogruden **Attributkodeomskiftere** i Læserindstillinger eller `config`-værktøjet. Disse omskiftere påvirker både læseren og redigereren. En separat **Fjern ANSI-koder**-mulighed fjerner alle ANSI-sekvenser fra meddelelser, når den er aktiveret.

### Vedhæftede filer
- Registrerer vedhæftede filer refereret via `@ATTACH:`-kludge-linjer i meddelelseskroppe
- Viser en **[ATT]**-indikator i meddelelsesrubriken, når vedhæftede filer er til stede
- Tryk **D** eller **Ctrl-D** i læseren for at downloade vedhæftede filer — viser en filliste med størrelser og beder om en destinationsmappe

### Afstemning og meningsmålinger (Synchronet QWKE)
SlyMail understøtter Synchronet VOTING.DAT-udvidelsen til meningsmålinger og meddelelsesafstemning:
- **Meningsmålinger**: Meddelelser identificeret som meningsmålinger viser svarmuligheder med stemmeantal og procentstænger. Tryk **V** for at åbne en stemmeseddeldialog, hvor du kan skifte svarvalgene og afgive din stemme.
- **Op-/Nedafstemning**: For almindelige (ikke-meningsmålings-)meddelelser, tryk **V** for at stemme op eller ned. Aktuelle stemmeresultater og score vises i meddelelsesrubriken.
- **Stemmeresultater**: Meddelelsesrubriken viser op-/nedafstemningsantal og nettoscore, med en indikator, hvis du allerede har stemt.
- **Stemmekøning**: Stemmer sættes i kø ved siden af meddelelsebesvarelser og skrives til VOTING.DAT i REP-pakken til upload til BBS.
- **Meningsmålingsbrowser**: Tryk **V** fra konferencelisten for at gennemse alle meningsmålinger i pakken.

### UTF-8-understøttelse
- Registrerer UTF-8-indhold i meddelelser (via HEADERS.DAT `Utf8`-flag og automatisk registrering af UTF-8-bytesekvenser)
- Viser UTF-8-tegn korrekt på kompatible terminaler
- Viser en **[UTF8]**-indikator i meddelelsesrubriken for UTF-8-meddelelser
- Gemmer nye meddelelser med korrekt kodning
- CP437-til-UTF-8-konvertering for ældre BBS-indhold
- Indstiller locale på Linux/macOS/BSD (`setlocale(LC_ALL, "")`) og UTF-8-tegnside på Windows for korrekt terminalgengivelse

### Meddelelseredigerer (inspireret af SlyEdit)
- **To visuelle tilstande**: Ice og DCT, hver med distinkte farveskemaer og layouts
- **Tilfældig tilstand**: Vælger tilfældigt Ice eller DCT ved hver redigeringssession
- **Skiftende kantfarver**: Kanttegn skifter tilfældigt mellem to temafarver, i lighed med SlyEdits visuelle stil
- **Temaunderstøttelse**: Konfigurerbare farvetemaer indlæst fra .ini-filer
- Fuldskærmstekstredigerer med linjeskift
- Citatvindue til valg og indsætning af citeret tekst (Ctrl-Q for at åbne/lukke)
- Besvarelse og ny meddelelseskomponering
- ESC-menu til gem, afbryd, indsæt-/overskrivningsskifter og mere
- **Ctrl-K farvevælger**: Åbner en dialog til valg af forgrunds- og baggrundsfarver, og indsætter en ANSI escape-kode ved markørpositionen. Understøtter 16 forgrundfarver (8 normale + 8 lyse) og 8 baggrunde, med forhåndsvisning. Tryk **N** for at indsætte en nulstillingskode.
- **Farvebevidst gengivelse**: Redigeringsområdet gengiver ANSI- og BBS-attributkoder inline, så farvet tekst vises, mens du skriver
- **Ctrl-U brugerindstillingsdialog** til konfigurering af redigererpræferencer on-the-fly
- **Stilspecifikke ja/nej-prompter**: Ice-tilstand bruger en prompt nederst på skærmen; DCT-tilstand bruger en centreret dialogboks med temafarver

### Redigeringsindstillinger (via Ctrl-U)
- **Vælg UI-tilstand**: Dialog til at skifte mellem Ice-, DCT- og Tilfældig-stilarter (træder i kraft med det samme)
- **Vælg temafil**: Vælg mellem tilgængelige Ice- eller DCT-farvetemaer
- **Taglines**: Når aktiveret, beder om tagline-valg ved gem (fra `tagline_files/taglines.txt`)
- **Stavekontrolsordbog/ordbøger**: Vælg hvilke ordbøger der skal bruges
- **Spørg om stavekontrol ved gem**: Når aktiveret, tilbyder stavekontrol inden gem
- **Ombryd citatlinjer til terminalbredde**: Ordbryds citatlinjer
- **Citér med forfatterens initialer**: Tilføj citatlinjer med forfatterens initialer (f.eks. `MP> `)
- **Indryk citatlinjer med initialer**: Tilføj foranstående mellemrum før initialer (f.eks. ` MP> `)
- **Fjern mellemrum fra citatlinjer**: Fjern foranstående blanktegn fra citeret tekst

### Farvetemaer
- Temafiler er konfigurationsfiler (`.ini`) i mappen `config_files/`
- Ice-temaer: `EditorIceColors_*.ini` (BlueIce, EmeraldCity, FieryInferno, osv.)
- DCT-temaer: `EditorDCTColors_*.ini` (Default, Default-Modified, Midnight)
- Temafarver bruger et simpelt format: forgrundsbogstav (`r`/`g`/`b`/`c`/`y`/`m`/`w`/`k`), valgfrit `h` for lys, valgfrit baggrundsciffer (`0`-`7`)
- Temaer styrer alle UI-elementfarver: kanter, etiketter, værdier, citatvindue, hjælpelinje, ja/nej-dialoger

### Stavekontrol
- Indbygget stavekontrol ved hjælp af klartekstordbøger
- Leveres med engelske ordbøger (en, en-US, en-GB, en-AU, en-CA-tillæg)
- Interaktiv korrektionsdialog: Erstat, Spring over eller Afslut
- Ordbogsfiler gemt i mappen `dictionary_files/`

### Taglines
- Tagline-filerne gemmes i mappen `tagline_files/`
- Standardtagline-filen er `tagline_files/taglines.txt`, én tagline per linje
- Linjer der starter med `#` eller `;` behandles som kommentarer og ignoreres
- Vælg en specifik tagline eller vælg én tilfældigt, når du gemmer en meddelelse
- Taglines tilføjes meddelelser med et `...`-præfiks

### REP-pakkeoprettelse
- Kun ZIP understøttes i øjeblikket (understøttelse af flere komprimeringstyper ønskes tilføjet i fremtiden)
- Når du skriver svar eller nye meddelelser, sættes de i kø som afventende
- Stemmer (meningsmålingsrøster, op-/nedafstemninger) sættes også i kø ved siden af svar
- Ved afslutning (eller når en ny QWK-fil åbnes) spørger SlyMail om at gemme alle afventende elementer
- Opretter en standard `.rep`-fil (ZIP-arkiv) til upload til BBS, der indeholder:
  - `<BBSID>.MSG` — svarmeddelelser i standard QWK-format
  - `HEADERS.DAT` — QWKE udvidede overskrifter for felter der overstiger 25 tegn
  - `VOTING.DAT` — afventende stemmer i Synchronet-kompatibelt INI-format
- REP-filen gemmes som `<BBS-ID>.rep` i den konfigurerede svarmappe (eller QWK-filens mappe)

### Fjernfjernede systemer (Ctrl-R)
SlyMail kan downloade QWK-pakker direkte fra fjernsystemer via FTP eller SFTP (SSH):
- Tryk **Ctrl-R** fra filbrowseren for at åbne fjernsystemskataloget
- **Tilføj/Rediger/Slet** fjernsystemposter med: navn, vært, port, forbindelsestype (FTP eller SSH), brugernavn, adgangskode, passiv FTP-omskifter og indledende fjernsti
- **Gennemse fjernmapper** med en fil-/mappebrowser svarende til den lokale filbrowser — naviger ind i mapper, gå op med `..`, spring til roden med `/`
- **Download QWK-filer** fra fjernsystemet direkte til undermappen `QWK` i SlyMail-datamappen
- Fjernsystemposter gemmes i `remote_systems.json` i SlyMail-datamappen
- Seneste forbindelsesdato/-tid spores for hvert system
- Bruger systemets `curl`-kommando til FTP- og SFTP-overførsler (ingen kompileringstidsbiblioteksafhængigheder)

### Programindstillinger
- Vedvarende indstillinger gemt i `slymail.ini` i SlyMail-datamappen (`~/.slymail` på Linux/macOS/BSD, eller brugerens hjemmemappe på Windows)
- SlyMail-datamappen og dens `QWK`-undermappe oprettes automatisk ved første kørsel
- Standard-QWK-filgennemse- og REP-pakkegemmemappe er `~/.slymail/QWK`
- Husker senest gennemset mappe og QWK-filnavn
- Ctrl-L-genvejstast til at indlæse en anden QWK-fil fra konference- eller meddelelselistevisningerne
- Konfigurerbart citatpræfiks, citatlinjebredde, brugernavn
- Læsermuligheder: vis/skjul kludge-linjer, rive-/oprindelseslinjer, rulleliste, fjern ANSI-koder
- Per-BBS-attributkodeomskiftere (Synchronet, WWIV, Celerity, Renegade, PCBoard/Wildcat) — påvirker både læseren og redigereren
- REP-pakkens outputmappe

## Skærmbilleder

<p align="center">
	<a href="screenshots/SlyMail_01_OpeningScreen.png" target='_blank'><img src="screenshots/SlyMail_01_OpeningScreen.png" alt="Opening screen" width="800"></a>
	<a href="screenshots/SlyMail_02_File_Chooser.png" target='_blank'><img src="screenshots/SlyMail_02_File_Chooser.png" alt="File chooser: Browsing for QWK file" width="800"></a>
	<a href="screenshots/SlyMail_03_remote_system_list.png" target='_blank'><img src="screenshots/SlyMail_03_remote_system_list.png" alt="Remote system list" width="800"></a>
	<a href="screenshots/SlyMail_04_Remote_System_Edit.png" target='_blank'><img src="screenshots/SlyMail_04_Remote_System_Edit.png" alt="Editing a remote system" width="800"></a>
	<a href="screenshots/SlyMail_05_Remote_System_Browsing.png" target='_blank'><img src="screenshots/SlyMail_05_Remote_System_Browsing.png" alt="Remote system browsing" width="800"></a>
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
	<a href="screenshots/SlyMail_20_reading_ANSI_art.png" target='_blank'><img src="screenshots/SlyMail_20_reading_ANSI_art.png" alt="Reading a message with ANSI art" width="800"></a>
	<a href="screenshots/SlyMail_21_reading_ANSI_art.png" target='_blank'><img src="screenshots/SlyMail_21_reading_ANSI_art.png" alt="Reading a message with ANSI art" width="800"></a>
	<a href="screenshots/SlyMail_22_reading_ANSI_art.png" target='_blank'><img src="screenshots/SlyMail_22_reading_ANSI_art.png" alt="Reading a message with ANSI art" width="800"></a>
	<a href="screenshots/SlyMail_23_reading_ANSI_art.png" target='_blank'><img src="screenshots/SlyMail_23_reading_ANSI_art.png" alt="Reading a message with ANSI art" width="800"></a>
</p>

## Synchronet-indstillinger for en QWK-pakke
På et Synchronet BBS, i QWK-pakkeindstillingerne, er SlyMail (eller bør være) kompatibel med Ctrl-A-farvekoder, VOTING.DAT, vedhæftede filer og QWKE-pakkeformatet. SlyMail bør også være kompatibel med UTF-8. For eksempel:
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

## Bygning

### Krav

**Linux / macOS / BSD:**
- C++17-kompatibel kompiler (GCC 8+, Clang 7+)
- ncurses-udviklingsbibliotek (`libncurses-dev` på Debian/Ubuntu, `ncurses-devel` på Fedora/RHEL)
- Kommandoen `unzip` (til udpakning af QWK-pakker)
- Kommandoen `zip` (til oprettelse af REP-pakker)
- Kommandoen `curl` (til fjernfjernsystems-FTP/SFTP-overførsler — valgfri, kun nødvendig til fjernfjernede systemers funktion)

**Windows (Visual Studio 2022):**
- Visual Studio 2022 med arbejdsbyrden "Desktop development with C++"
- Windows SDK 10.0 (inkluderet med VS)
- Ingen yderligere biblioteker kræves — bruger den indbyggede Win32 Console API til terminal-UI og enten `tar.exe` eller PowerShell til QWK/REP-pakke ZIP-håndtering (se note nedenfor)

**Windows (MinGW/MSYS2):**
- MinGW-w64 eller MSYS2 med GCC (C++17-understøttelse)
- Windows Console API (indbygget)

> **Note — QWK/REP ZIP-håndtering på Windows:** SlyMail registrerer ved kørselstid, hvilket værktøj der er tilgængeligt og bruger den bedste mulighed:
>
> - **`tar.exe` (foretrukket):** Leveres med Windows 10 version 1803 (April 2018 Update) og nyere, og med alle versioner af Windows 11. `tar` læser ZIP-filer ud fra deres indhold snarere end deres filendelse, så `.qwk`-pakker udpakkes direkte og `.rep`-pakker oprettes via en midlertidig `.zip`-fil, der derefter omdøbes. Ingen ekstra konfiguration er nødvendig.
> - **PowerShell (reserve):** Hvis `tar.exe` ikke findes i PATH, falder SlyMail tilbage til PowerShell. Til udpakning bruger den .NET-klassen `ZipFile` (`System.IO.Compression`) snarere end `Expand-Archive`, fordi `Expand-Archive` afviser filendelser der ikke er `.zip`, selv når filen er et gyldigt ZIP-arkiv. Til REP-pakkeoprettelse bruges `Compress-Archive`, som også skriver til en midlertidig `.zip`-fil, der derefter omdøbes til `.rep`.

### Byg på Linux/macOS/BSD

```bash
make
```

Dette bygger to programmer:
- `slymail` - hoved-QWK-læserprogrammet
- `config` - det selvstændige konfigurationsværktøj

### Byg med fejlsøgningssymboler

```bash
make debug
```

### Installer (valgfrit)

```bash
sudo make install    # Installs slymail and config to /usr/local/bin/
sudo make uninstall  # Remove
```

### Byg på Windows med Visual Studio 2022

Åbn løsningsfilen i Visual Studio 2022:

```
vs\SlyMail.sln
```

Eller byg fra kommandolinjen ved hjælp af MSBuild:

```powershell
# Release build (output in vs\x64\Release\)
msbuild vs\SlyMail.sln /p:Configuration=Release /p:Platform=x64

# Debug build (output in vs\x64\Debug\)
msbuild vs\SlyMail.sln /p:Configuration=Debug /p:Platform=x64
```

Dette bygger to eksekverbare filer:
- `x64\Release\slymail.exe` — hoved-QWK-læseren
- `x64\Release\config.exe` — det selvstændige konfigurationsværktøj

Løsningen indeholder to projekter (`SlyMail.vcxproj` og `Config.vcxproj`) der målretter x64, C++17, med MSVC v143-værktøjssættet.

### Byg på Windows (MinGW/MSYS2)

```bash
make
```

Makefilen registrerer automatisk platformen og bruger den passende terminalimplementering:
- **Linux/macOS/BSD**: ncurses (`terminal_ncurses.cpp`)
- **Windows**: conio + Win32 Console API (`terminal_win32.cpp`)

## Brug

```bash
# Launch SlyMail with file browser
./slymail

# Open a specific QWK packet
./slymail MYBBS.qwk

# Run the standalone configuration utility
./config
```

### Konfigurationsprogram

`config`-værktøjet giver en selvstændig tekstbaseret brugerflade til konfigurering af SlyMail-indstillinger uden at åbne hovedprogrammet. Det tilbyder fire konfigurationskategorier:

- **Redigeringsindstillinger** - Alle de samme indstillinger tilgængelige via Ctrl-U i redigereren (redigeringsstil, taglines, stavekontrol, citeringsmuligheder, osv.)
- **Læserindstillinger** - Skift kludge-linjer, rivningslinjer, rulleliste, ANSI-fjernelse, lysstangsindstilling, omvendt rækkefølge og attributkodeomskiftere (per-BBS aktivér/deaktivér)
- **Temaindstillinger** - Vælg Ice- og DCT-farvetemaefiler fra mappen `config_files/`
- **Generelle indstillinger** - Indstil dit navn til svar og REP-pakkens outputmappe

Indstillinger gemmes automatisk, når du afslutter hver kategori. Både SlyMail og config-værktøjet læser og skriver den samme indstillingsfil.

### Tastebindinger

#### Filbrowser
| Tast | Handling |
|------|----------|
| Op/Ned | Naviger filer og mapper |
| Enter | Åbn mappe / Vælg QWK-fil |
| Ctrl-R | Åbn fjernsystemsmappe |
| Q / ESC | Afslut |

#### Konferenceliste
| Tast | Handling |
|------|----------|
| Op/Ned | Naviger konferencer |
| Enter | Åbn valgt konference |
| V | Vis meningsmålinger/stemmer i pakken |
| O / Ctrl-L | Åbn en anden QWK-fil |
| S / Ctrl-U | Indstillinger |
| Q / ESC | Afslut SlyMail |
| ? / F1 | Hjælp |

#### Meddelelseliste
| Tast | Handling |
|------|----------|
| Op/Ned | Naviger meddelelser |
| Enter / R | Læs valgt meddelelse |
| N | Skriv en ny meddelelse |
| G | Gå til meddelelsenummer |
| Ctrl-L | Åbn en anden QWK-fil |
| S / Ctrl-U | Indstillinger |
| C / ESC | Tilbage til konferencelisten |
| Q | Afslut |
| ? / F1 | Hjælp |

#### Meddelelselæser
| Tast | Handling |
|------|----------|
| Op/Ned | Rul meddelelse |
| Venstre/Højre | Forrige / Næste meddelelse |
| F / L | Første / Sidste meddelelse |
| R | Besvar meddelelse |
| V | Stem (op-/nedafstemning eller meningsmålingsrøst) |
| D / Ctrl-D | Download vedhæftede filer |
| H | Vis information om meddelelsesrubrik |
| S / Ctrl-U | Indstillinger |
| C / Q / ESC | Tilbage til meddelelselisten |
| ? / F1 | Hjælp |

#### Meddelelseredigerer
| Tast | Handling |
|------|----------|
| ESC | Redigerermenu (Gem, Afbryd, osv.) |
| Ctrl-U | Brugerindstillingsdialog |
| Ctrl-Q | Åbn/luk citatvindue |
| Ctrl-K | Farvevælger (indsæt ANSI-farvekode ved markøren) |
| Ctrl-G | Indsæt grafisk (CP437) tegn efter kode |
| Ctrl-W | Ord-/tekstsøgning |
| Ctrl-S | Skift emne |
| Ctrl-D | Slet aktuel linje |
| Ctrl-Z | Gem meddelelse |
| Ctrl-A | Afbryd meddelelse |
| F1 | Hjælpeskærm |
| Insert | Skift Indsæt-/Overskrivningstilstand |

#### Citatvindue
| Tast | Handling |
|------|----------|
| Op/Ned | Naviger citatlinjer |
| Enter | Indsæt valgt citatlinje |
| Ctrl-Q / ESC | Luk citatvindue |

## Arkitektur

SlyMail bruger et platformsabstraktionslag til sin tekstbaserede brugerflade:

```
ITerminal (abstract base class)
    ├── NCursesTerminal  (Linux/macOS/BSD - ncurses)
    └── Win32Terminal    (Windows - conio + Win32 Console API)
```

CP437-boxtegning og specialtegn er defineret i `cp437defs.h` og gengives via metoden `putCP437()`, som mapper CP437-koder til platformsnative ækvivalenter (ACS-tegn på ncurses, direkte CP437-bytes på Windows).

### Kildefiler

| Fil | Beskrivelse |
|-----|-------------|
| `terminal.h` | Abstrakt `ITerminal`-interface, tast-/farvekonstanter, fabrik |
| `terminal_ncurses.cpp` | ncurses-implementering med CP437-til-ACS-mapping |
| `terminal_win32.cpp` | Windows Console API + conio-implementering |
| `cp437defs.h` | IBM Code Page 437 tegnedefinitioner |
| `colors.h` | Farveschemadefinitioner (Ice, DCT, læser, liste) |
| `theme.h` | Temakonfigurationsfil-fortolker (Synchronet-stil attributkoder) |
| `ui_common.h` | Delte UI-hjælpere (dialoger, tekstinput, rulleliste, osv.) |
| `qwk.h` / `qwk.cpp` | QWK/REP-pakkefortolker og -opretter (QWKE, vedhæftede filer, afstemning) |
| `bbs_colors.h` / `bbs_colors.cpp` | BBS-farve-/attributkodefortolker (ANSI, Synchronet, WWIV, PCBoard, Celerity, Renegade) |
| `utf8_util.h` / `utf8_util.cpp` | UTF-8-værktøjer (validering, visningsbredde, CP437-til-UTF-8-konvertering) |
| `voting.h` / `voting.cpp` | VOTING.DAT-fortolker, stemmeoptælling, meningsmålingsvisnings-UI |
| `remote_systems.h` / `remote_systems.cpp` | Fjernsystemsmappe, FTP/SFTP-browsing, JSON-persistens, hjemmemappeværktøjer |
| `settings.h` | Vedvarende brugerindstillinger |
| `settings_dialog.h` | Indstillingsdialoger (redigerer, læser, attributkodeomskiftere) |
| `file_browser.h` | QWK-filbrowser og -vælger |
| `msg_list.h` | Konference- og meddelelselistevisninger |
| `msg_reader.h` | Meddelelselæser (DDMsgReader-stil) med afstemnings- og vedhæftnings-UI |
| `msg_editor.h` | Meddelelseredigerer (SlyEdit Ice/DCT-stil) med farvevælger |
| `main.cpp` | SlyMail-programmets indgangspunkt og hovedløkke |
| `config.cpp` | Selvstændigt konfigurationsværktøj |

## Konfiguration

### Indstillingsfil

Indstillinger gemmes i en INI-fil med navnet `slymail.ini` i samme mappe som SlyMail-programmet. Denne fil deles mellem både SlyMail og `config`-værktøjet. Filen er velkommenteret med beskrivelser af hver indstilling.

Eksempel på `slymail.ini`:
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

### Temafiler

Farvetemaer er `.ini`-filer i mappen `config_files/`:

**Ice-temaer** (`EditorIceColors_*.ini`):
- BlueIce (standard), EmeraldCity, FieryInferno, Fire-N-Ice, GeneralClean, GenericBlue, PurpleHaze, ShadesOfGrey

**DCT-temaer** (`EditorDCTColors_*.ini`):
- Default (standard), Default-Modified, Midnight

Temafarveværdier bruger et kompakt format afledt af Synchronet-attributkoder:
- `n` = normal (nulstil)
- Forgrund: `k`=sort, `r`=rød, `g`=grøn, `y`=gul, `b`=blå, `m`=magenta, `c`=cyan, `w`=hvid
- `h` = høj/lys intensitet
- Baggrundsciffer: `0`=sort, `1`=rød, `2`=grøn, `3`=brun, `4`=blå, `5`=magenta, `6`=cyan, `7`=lysegrå

Eksempel: `nbh` = normal blå lys, `n4wh` = lys hvid på blå baggrund

### Taglines

Taglines er korte citater eller ordsprog der tilføjes i slutningen af meddelelser, når de gemmes. Tagline-funktionen kan aktiveres via Ctrl-U i redigereren eller `config`-værktøjet.

Taglines gemmes i `tagline_files/taglines.txt`, én per linje. Linjer der starter med `#` eller `;` behandles som kommentarer og ignoreres. Når en meddelelse gemmes med taglines aktiveret, opfordres brugeren til enten at vælge en specifik tagline eller vælge én tilfældigt. Valgte taglines tilføjes meddelelsen med præfikset `...` (f.eks. `...To err is human, to really foul things up requires a computer.`).

### Stavekontrol

SlyMail inkluderer en indbygget stavekontrol der bruger klartekstordbøger. Stavekontrollen kan konfigureres til at spørge ved gem via Ctrl-U i redigereren eller `config`-værktøjet.

**Ordbogsfiler** er klartekstfiler (ét ord per linje) gemt i `dictionary_files/`. Flere ordbøger kan vælges samtidigt for kombineret orddekning. SlyMail leveres med:
- `dictionary_en.txt` - Engelsk (generel, ~130.000 ord)
- `dictionary_en-US-supplemental.txt` - Amerikansk engelsk tillæg
- `dictionary_en-GB-supplemental.txt` - Britisk engelsk tillæg
- `dictionary_en-AU-supplemental.txt` - Australsk engelsk tillæg
- `dictionary_en-CA-supplemental.txt` - Canadisk engelsk tillæg

Når stavekontrol udløses, scanner kontrollen meddelelsen for forkert stavede ord og præsenterer en interaktiv dialog for hvert, med muligheder for at **E**rstate ordet, **S**pringe over det, **T**ilføje det (fremtidigt), eller **A**fslutte kontrollen.

## Krediteringer

- UI inspireret af [DDMsgReader](https://github.com/SynchronetBBS/sbbs) og [SlyEdit](https://github.com/SynchronetBBS/sbbs) af [Nightfox (Eric Oulashin)](https://github.com/EricOulashin)
- QWK-formatkompatibilitet informeret af [Synchronet BBS](https://www.synchro.net/) kildekode
- CP437-tegnsdefinitioner fra Synchronet

## Licens

Dette projekt er open source-software.
