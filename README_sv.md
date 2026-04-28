*(Maskinöversättning — för den engelska originalversionen, se [README.md](README.md))*

# SlyMail

Det här är en plattformsoberoende textbaserad offline-postläsare för [QWK](https://en.wikipedia.org/wiki/QWK_(file_format))-paketformatet. QWK-paketformatet användes/används ofta för att utbyta post på [bulletin board-system](https://en.wikipedia.org/wiki/Bulletin_board_system).

SlyMail erbjuder ett komplett gränssnitt för att läsa och svara på meddelanden från BBS (Bulletin Board System) QWK-postpaket. Användargränssnittet är inspirerat av [Digital Distortion Message Reader (DDMsgReader)](https://github.com/SynchronetBBS/sbbs/tree/master/xtrn/DDMsgReader) för meddelandeläsning och [SlyEdit](https://github.com/SynchronetBBS/sbbs/tree/master/exec) för meddelanderedigering, båda ursprungligen skapade för [Synchronet BBS](https://www.synchro.net/).

SlyMail skapades med hjälp av Claude AI.

## Funktioner

### Stöd för QWK-paket
- Öppnar och läser standard-QWK-postpaket (.qwk-filer)
- Tolkar CONTROL.DAT, MESSAGES.DAT och NDX-indexfiler
- Fullt QWKE-stöd (utökat QWK) via HEADERS.DAT — offsetbaserad matchning för korrekta utökade Till/Från/Ämne-fält, UTF-8-flagga och RFC822 Message-ID
- QWKE-brödtexts-kludge-tolkning (`To:`, `From:`, `Subject:` i början av meddelanden)
- Hanterar Synchronet-stil konferensnumrering
- Skapar REP-svarspaket (.rep-filer) för uppladdning tillbaka till BBS, inklusive HEADERS.DAT för utökade fält och VOTING.DAT för köade röster
- Stöder Microsoft Binary Format (MBF) flyttalskodning i NDX-filer
- Kommer ihåg senast öppnad QWK-fil och katalog mellan sessioner

### Meddelandeläsning (DDMsgReader-stil)
- Konferenslista med meddelandeantal
- Rullbar meddelandelista med ljusstångsnavigering
- Fullständig meddelandeläsare med rubrikvisning (Från, Till, Ämne, Datum)
- Citatradsmarkering (stöder flernivåcitering)
- Kludge-radsvisning (valfri)
- Rullningslistindikator
- Tangentbordsnavigering: Första/Sista/Nästa/Föregående meddelande, Sida Upp/Ned
- Hjälpskärmar tillgängliga med `?` eller `F1` i alla vyer

### Stöd för BBS-färg- och attributkoder
SlyMail tolkar färg- och attributkoder från flera BBS-programvarupaket och renderar dem som färgad text i både meddelandeläsaren och meddelanderedigeraren. Format som stöds:
- **ANSI escape-koder** — alltid aktiverade; standard SGR-sekvenser (ESC[...m) för förgrund, bakgrund, fetstil
- **Synchronet Ctrl-A-koder** — `\x01` + attributtecken (t.ex. `\x01c` för cyan, `\x01h` för ljus)
- **WWIV hjärtkoder** — `\x03` + siffra 0–9
- **PCBoard/Wildcat @X-koder** — `@X##` där de två hexsiffrorna kodar bakgrunds- och förgrundsfärg
- **Celerity pipe-koder** — `|` + bokstav (t.ex. `|c` för cyan, `|W` för ljusvitt)
- **Renegade pipe-koder** — `|` + tvåsiffrigt tal 00–31

Varje BBS-kodstyp kan aktiveras eller inaktiveras individuellt via underdialogrutan **Attributkodsomkopplare** i läsarinställningar eller `config`-verktyget. Dessa omkopplare påverkar både läsaren och redigeraren. Ett separat alternativ **Ta bort ANSI-koder** tar bort alla ANSI-sekvenser från meddelanden när det är aktiverat.

### Bifogade filer
- Identifierar bifogade filer refererade via `@ATTACH:`-kludge-rader i meddelandekroppar
- Visar en **[ATT]**-indikator i meddelanderubriken när bilagor finns
- Tryck **D** eller **Ctrl-D** i läsaren för att ladda ned bilagor — visar en fillista med storlekar och frågar efter en målkatalog

### Röstning och omröstningar (Synchronet QWKE)
SlyMail stöder Synchronet VOTING.DAT-tillägget för omröstningar och meddelanderöstning:
- **Omröstningar**: Meddelanden identifierade som omröstningar visar svarsalternativ med rösträkningar och procentstaplar. Tryck **V** för att öppna en röstsedeldialog där du kan växla svarsval och avge din röst.
- **Upp-/Nedröstning**: För vanliga (icke-omröstnings-)meddelanden, tryck **V** för att rösta upp eller ned. Aktuella röstresultat och poäng visas i meddelanderubriken.
- **Röstresultat**: Meddelanderubriken visar upp-/nedröstningsantal och nettopoäng, med en indikator om du redan har röstat.
- **Röstköning**: Röster köas tillsammans med meddelandesvar och skrivs till VOTING.DAT i REP-paketet för uppladdning till BBS.
- **Omröstningsbläddrare**: Tryck **V** från konferenslistan för att bläddra bland alla omröstningar i paketet.

### UTF-8-stöd
- Identifierar UTF-8-innehåll i meddelanden (via HEADERS.DAT `Utf8`-flagga och automatisk identifiering av UTF-8-bytesekvenser)
- Visar UTF-8-tecken korrekt på kompatibla terminaler
- Visar en **[UTF8]**-indikator i meddelanderubriken för UTF-8-meddelanden
- Sparar nya meddelanden med korrekt kodning
- CP437-till-UTF-8-konvertering för äldre BBS-innehåll
- Ställer in locale på Linux/macOS/BSD (`setlocale(LC_ALL, "")`) och UTF-8-teckentabell på Windows för korrekt terminalrendering

### Meddelanderedigerare (inspirerad av SlyEdit)
- **Två visuella lägen**: Ice och DCT, vart och ett med distinkta färgscheman och layouter
- **Slumpmässigt läge**: Väljer slumpmässigt Ice eller DCT vid varje redigeringssession
- **Växlande kantfärger**: Kanttecken växlar slumpmässigt mellan två temafärger, i likhet med SlyEdits visuella stil
- **Temastöd**: Konfigurerbara färgteman laddade från .ini-filer
- Helskärmstexteditor med radbrytning
- Citatfönster för att välja och infoga citerad text (Ctrl-Q för att öppna/stänga)
- Svar och nytt meddelandekomponering
- ESC-meny för spara, avbryt, infoga/överskrivningsväxel och mer
- **Ctrl-K färgväljare**: Öppnar en dialog för att välja förgrunds- och bakgrundsfärger, och infogar en ANSI escape-kod vid markörpositionen. Stöder 16 förgrundsfärger (8 normala + 8 ljusa) och 8 bakgrunder, med förhandsgranskning. Tryck **N** för att infoga en återställningskod.
- **Färgmedveten rendering**: Redigeringsområdet renderar ANSI- och BBS-attributkoder inline, så att färgad text visas när du skriver
- **Ctrl-U användarinställningsdialog** för att konfigurera redigerarinställningar i farten
- **Stilspecifika ja/nej-frågor**: Ice-läget använder en prompt längst ned på skärmen; DCT-läget använder en centrerad dialogruta med temafärger

### Redigerarinställningar (via Ctrl-U)
- **Välj UI-läge**: Dialog för att växla mellan Ice-, DCT- och Slumpmässiga stilar (träder i kraft omedelbart)
- **Välj temafil**: Välj bland tillgängliga Ice- eller DCT-färgteman
- **Taglines**: När aktiverat, frågar om val av tagrader vid spara (från `tagline_files/taglines.txt`)
- **Stavningskontrollsordbok/ordböcker**: Välj vilka ordböcker som ska användas
- **Fråga om stavningskontroll vid spara**: När aktiverat, erbjuder stavningskontroll innan spara
- **Radbryt citerade rader till terminalens bredd**: Radbryt citerade rader med ordbrytning
- **Citera med författarens initialer**: Lägg till citatrader med författarens initialer (t.ex. `MP> `)
- **Indrag för citerade rader med initialer**: Lägg till inledande mellanrum före initialer (t.ex. ` MP> `)
- **Ta bort mellanslag från citerade rader**: Ta bort ledande blanksteg från citerad text

### Färgteman
- Temafiler är konfigurationsfiler (`.ini`) i katalogen `config_files/`
- Ice-teman: `EditorIceColors_*.ini` (BlueIce, EmeraldCity, FieryInferno, osv.)
- DCT-teman: `EditorDCTColors_*.ini` (Default, Default-Modified, Midnight)
- Temafärger använder ett enkelt format: förgrundsbokstav (`r`/`g`/`b`/`c`/`y`/`m`/`w`/`k`), valfritt `h` för ljus, valfri bakgrundssiffra (`0`-`7`)
- Teman styr alla UI-elementfärger: kanter, etiketter, värden, citatfönster, hjälprad, ja/nej-dialoger

### Stavningskontroll
- Inbyggd stavningskontroll med klartextordböcker
- Levereras med engelska ordböcker (en, en-US, en-GB, en-AU, en-CA-tillägg)
- Interaktiv korrigeringsdialog: Ersätt, Hoppa över eller Avsluta
- Ordboksfiler lagrade i katalogen `dictionary_files/`

### Taglines
- Tagradsfilerna lagras i katalogen `tagline_files/`
- Standardtagradfilen är `tagline_files/taglines.txt`, en tagline per rad
- Rader som börjar med `#` eller `;` behandlas som kommentarer och ignoreras
- Välj en specifik tagline eller välj en slumpmässigt när du sparar ett meddelande
- Taglines läggs till i meddelanden med prefixet `...`

### Skapande av REP-paket
- För närvarande stöds bara ZIP (stöd för fler komprimeringstyper planeras i framtiden)
- När du skriver svar eller nya meddelanden köas de som väntande
- Röster (omröstningsröstsedlar, upp-/nedröstningar) köas också tillsammans med svar
- Vid avslutning (eller när en ny QWK-fil öppnas) frågar SlyMail om alla väntande objekt ska sparas
- Skapar en standard `.rep`-fil (ZIP-arkiv) för uppladdning till BBS, innehållande:
  - `<BBSID>.MSG` — svarsmedelanden i standard QWK-format
  - `HEADERS.DAT` — QWKE utökade rubriker för fält som överstiger 25 tecken
  - `VOTING.DAT` — väntande röster i Synchronet-kompatibelt INI-format
- REP-filen sparas som `<BBS-ID>.rep` i den konfigurerade svarskatalogen (eller QWK-filens katalog)

### Fjärrsystem (Ctrl-R)
SlyMail kan ladda ned QWK-paket direkt från fjärrsystem via FTP eller SFTP (SSH):
- Tryck **Ctrl-R** från filbläddraren för att öppna fjärrsystemskatalogen
- **Lägg till/Redigera/Ta bort** fjärrsystemposter med: namn, värd, port, anslutningstyp (FTP eller SSH), användarnamn, lösenord, passiv FTP-växel och initial fjärrsökväg
- **Bläddra i fjärrkataloger** med en fil-/katalogbläddrare liknande den lokala filbläddraren — navigera in i kataloger, gå upp med `..`, hoppa till roten med `/`
- **Ladda ned QWK-filer** från fjärrsystemet direkt till underkatalogen `QWK` i SlyMail-datakatalogen
- Fjärrsystemposter sparas till `remote_systems.json` i SlyMail-datakatalogen
- Senaste anslutningsdatum/-tid spåras för varje system
- Använder systemets `curl`-kommando för FTP- och SFTP-överföringar (inga kompileringstidsbibliotekarsberoenden)

### Programinställningar
- Beständiga inställningar sparade till `slymail.ini` i SlyMail-datakatalogen (`~/.slymail` på Linux/macOS/BSD, eller användarens hemkatalog på Windows)
- SlyMail-datakatalogen och dess `QWK`-underkatalog skapas automatiskt vid första körning
- Standardkatalog för att bläddra efter QWK-filer och spara REP-paket är `~/.slymail/QWK`
- Kommer ihåg senast bläddrade katalog och QWK-filnamn
- Ctrl-L-snabbtangent för att ladda en annan QWK-fil från konferens- eller meddelandelistevyerna
- Konfigurerbar citatprefix, citatradbredd, användarnamn
- Läsaralternativ: visa/dölj kludge-rader, bortrivnings-/ursprungsrader, rullningslist, ta bort ANSI-koder
- Per-BBS-attributkodsomkopplare (Synchronet, WWIV, Celerity, Renegade, PCBoard/Wildcat) — påverkar både läsaren och redigeraren
- REP-paketets utdatakatalog

## Skärmdumpar

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

## Synchronet-inställningar för ett QWK-paket
På ett Synchronet BBS, i QWK-paketinställningarna, är SlyMail (eller bör vara) kompatibel med Ctrl-A-färgkoder, VOTING.DAT, bifogade filer och QWKE-paketformatet. SlyMail bör också vara kompatibel med UTF-8. Till exempel:
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

## Bygga

### Krav

**Linux / macOS / BSD:**
- C++17-kompatibel kompilator (GCC 8+, Clang 7+)
- ncurses-utvecklingsbibliotek (`libncurses-dev` på Debian/Ubuntu, `ncurses-devel` på Fedora/RHEL)
- Kommandot `unzip` (för att extrahera QWK-paket)
- Kommandot `zip` (för att skapa REP-paket)
- Kommandot `curl` (för fjärrsystem FTP/SFTP-överföringar — valfritt, behövs bara för fjärrsystemsfunktionen)

**Windows (Visual Studio 2022):**
- Visual Studio 2022 med arbetsbelastningen "Skrivbordsutveckling med C++"
- Windows SDK 10.0 (ingår i VS)
- Inga ytterligare bibliotek krävs — använder den inbyggda Win32 Console API för terminalens gränssnitt, och antingen `tar.exe` eller PowerShell för QWK/REP-paket ZIP-hantering (se anmärkning nedan)

**Windows (MinGW/MSYS2):**
- MinGW-w64 eller MSYS2 med GCC (C++17-stöd)
- Windows Console API (inbyggt)

> **Anmärkning — QWK/REP ZIP-hantering på Windows:** SlyMail identifierar vid körning vilket verktyg som är tillgängligt och använder det bästa alternativet:
>
> - **`tar.exe` (föredraget):** Levereras med Windows 10 version 1803 (April 2018 Update) och senare, och med alla versioner av Windows 11. `tar` läser ZIP-filer utifrån deras innehåll snarare än filnamnstillägg, så `.qwk`-paket extraheras direkt och `.rep`-paket skapas via en tillfällig `.zip`-fil som sedan byter namn. Ingen extra konfiguration behövs.
> - **PowerShell (reserv):** Om `tar.exe` inte hittas i PATH, faller SlyMail tillbaka till PowerShell. För extrahering används .NET-klassen `ZipFile` (`System.IO.Compression`) snarare än `Expand-Archive`, eftersom `Expand-Archive` avvisar filnamnstillägg som inte är `.zip` även när filen är ett giltigt ZIP-arkiv. För REP-paketskapande används `Compress-Archive`, som också skriver till en tillfällig `.zip`-fil som sedan byter namn till `.rep`.

### Bygga på Linux/macOS/BSD

```bash
make
```

Detta bygger två program:
- `slymail` - huvudprogrammet för QWK-läsning
- `config_slymail` - det fristående konfigurationsverktyget

### Bygga med felsökningssymboler

```bash
make debug
```

### Installera (valfritt)

```bash
sudo make install    # Installs slymail and config to /usr/local/bin/
sudo make uninstall  # Remove
```

### Bygga på Windows med Visual Studio 2022

Öppna lösningsfilen i Visual Studio 2022:

```
vs\SlyMail.sln
```

Eller bygg från kommandoraden med MSBuild:

```powershell
# Release build (output in vs\x64\Release\)
msbuild vs\SlyMail.sln /p:Configuration=Release /p:Platform=x64

# Debug build (output in vs\x64\Debug\)
msbuild vs\SlyMail.sln /p:Configuration=Debug /p:Platform=x64
```

Detta bygger två körbara filer:
- `x64\Release\slymail.exe` — huvudprogrammet för QWK-läsning
- `x64\Release\config_slymail.exe` — det fristående konfigurationsverktyget

Lösningen innehåller två projekt (`SlyMail.vcxproj` och `Config.vcxproj`) som riktar sig mot x64, C++17, med MSVC v143-verktygsuppsättningen.

### Bygga på Windows (MinGW/MSYS2)

```bash
make
```

Makefilen identifierar automatiskt plattformen och använder lämplig terminalimplementering:
- **Linux/macOS/BSD**: ncurses (`terminal_ncurses.cpp`)
- **Windows**: conio + Win32 Console API (`terminal_win32.cpp`)

## Användning

```bash
# Launch SlyMail with file browser
./slymail

# Open a specific QWK packet
./slymail MYBBS.qwk

# Run the standalone configuration utility
./config_slymail
```

### Konfigurationsprogram

Verktyget `config` erbjuder ett fristående textbaserat gränssnitt för att konfigurera SlyMail-inställningar utan att öppna huvudprogrammet. Det erbjuder fyra konfigurationskategorier:

- **Redigerarinställningar** - Alla samma inställningar som är tillgängliga via Ctrl-U i redigeraren (redigerarstil, taglines, stavningskontroll, citeringsalternativ, osv.)
- **Läsarinställningar** - Växla kludge-rader, bortrivningsrader, rullningslist, ANSI-borttagning, ljusstångsläge, omvänd ordning och attributkodsomkopplare (per-BBS aktivera/inaktivera)
- **Temainställningar** - Välj Ice- och DCT-färgtemafiler från katalogen `config_files/`
- **Allmänna inställningar** - Ange ditt namn för svar och REP-paketets utdatakatalog

Inställningarna sparas automatiskt när du avslutar varje kategori. Både SlyMail och config-verktyget läser och skriver samma inställningsfil.

### Tangentbindningar

#### Filbläddrare
| Tangent | Åtgärd |
|---------|--------|
| Upp/Ned | Navigera filer och kataloger |
| Enter | Öppna katalog / Välj QWK-fil |
| Ctrl-R | Öppna fjärrsystemskatalog |
| Q / ESC | Avsluta |

#### Konferenslista
| Tangent | Åtgärd |
|---------|--------|
| Upp/Ned | Navigera konferenser |
| Enter | Öppna vald konferens |
| V | Visa omröstningar/röster i paketet |
| O / Ctrl-L | Öppna en annan QWK-fil |
| S / Ctrl-U | Inställningar |
| Q / ESC | Avsluta SlyMail |
| ? / F1 | Hjälp |

#### Meddelandelista
| Tangent | Åtgärd |
|---------|--------|
| Upp/Ned | Navigera meddelanden |
| Enter / R | Läs valt meddelande |
| N | Skriv ett nytt meddelande |
| G | Gå till meddelandenummer |
| Ctrl-L | Öppna en annan QWK-fil |
| S / Ctrl-U | Inställningar |
| C / ESC | Tillbaka till konferenslistan |
| Q | Avsluta |
| ? / F1 | Hjälp |

#### Meddelandeläsare
| Tangent | Åtgärd |
|---------|--------|
| Upp/Ned | Rulla meddelande |
| Vänster/Höger | Föregående / Nästa meddelande |
| F / L | Första / Sista meddelandet |
| R | Svara på meddelande |
| V | Rösta (upp-/nedröstning eller omröstningsröstsedel) |
| D / Ctrl-D | Ladda ned bifogade filer |
| H | Visa information om meddelanderubriken |
| S / Ctrl-U | Inställningar |
| C / Q / ESC | Tillbaka till meddelandelistan |
| ? / F1 | Hjälp |

#### Meddelanderedigerare
| Tangent | Åtgärd |
|---------|--------|
| ESC | Redigerarmeny (Spara, Avbryt, osv.) |
| Ctrl-U | Användarinställningsdialog |
| Ctrl-Q | Öppna/stäng citatfönster |
| Ctrl-K | Färgväljare (infoga ANSI-färgkod vid markören) |
| Ctrl-G | Infoga grafiskt (CP437) tecken efter kod |
| Ctrl-W | Ord-/textsökning |
| Ctrl-S | Ändra ämne |
| Ctrl-D | Ta bort aktuell rad |
| Ctrl-Z | Spara meddelande |
| Ctrl-A | Avbryt meddelande |
| F1 | Hjälpskärm |
| Insert | Växla Infoga/Överskrivningsläge |

#### Citatfönster
| Tangent | Åtgärd |
|---------|--------|
| Upp/Ned | Navigera citerade rader |
| Enter | Infoga vald citerad rad |
| Ctrl-Q / ESC | Stäng citatfönster |

## Arkitektur

SlyMail använder ett plattformsabstraktionslager för sitt textbaserade användargränssnitt:

```
ITerminal (abstract base class)
    ├── NCursesTerminal  (Linux/macOS/BSD - ncurses)
    └── Win32Terminal    (Windows - conio + Win32 Console API)
```

CP437-boxritning och specialtecken definieras i `cp437defs.h` och renderas via metoden `putCP437()`, som mappar CP437-koder till plattformsnativa motsvarigheter (ACS-tecken på ncurses, direkta CP437-bytes på Windows).

### Källfiler

| Fil | Beskrivning |
|-----|-------------|
| `terminal.h` | Abstrakt `ITerminal`-gränssnitt, tangent-/färgkonstanter, fabrik |
| `terminal_ncurses.cpp` | ncurses-implementering med CP437-till-ACS-mappning |
| `terminal_win32.cpp` | Windows Console API + conio-implementering |
| `cp437defs.h` | IBM Code Page 437 teckendefinitioner |
| `colors.h` | Färgschemadefinitioner (Ice, DCT, läsare, lista) |
| `theme.h` | Temakonfigurationsfilstolkare (Synchronet-stil attributkoder) |
| `ui_common.h` | Delade UI-hjälpare (dialoger, textinmatning, rullningslist, osv.) |
| `qwk.h` / `qwk.cpp` | QWK/REP-pakettolkare och -skapare (QWKE, bilagor, röstning) |
| `bbs_colors.h` / `bbs_colors.cpp` | BBS-färg-/attributkodstolkare (ANSI, Synchronet, WWIV, PCBoard, Celerity, Renegade) |
| `utf8_util.h` / `utf8_util.cpp` | UTF-8-verktyg (validering, visningsbredd, CP437-till-UTF-8-konvertering) |
| `voting.h` / `voting.cpp` | VOTING.DAT-tolkare, rösträkning, omröstningsvisnings-UI |
| `remote_systems.h` / `remote_systems.cpp` | Fjärrsystemskatalog, FTP/SFTP-bläddring, JSON-persistens, hemkatalogverktyg |
| `settings.h` | Persistens av användarinställningar |
| `settings_dialog.h` | Inställningsdialoger (redigerare, läsare, attributkodsomkopplare) |
| `file_browser.h` | QWK-filbläddrare och -väljare |
| `msg_list.h` | Konferens- och meddelandelistevyer |
| `msg_reader.h` | Meddelandeläsare (DDMsgReader-stil) med röstnings- och bilagshanteringsgränssnitt |
| `msg_editor.h` | Meddelanderedigerare (SlyEdit Ice/DCT-stil) med färgväljare |
| `main.cpp` | SlyMail-programmets ingångspunkt och huvudloop |
| `config.cpp` | Fristående konfigurationsverktyg |

## Konfiguration

### Inställningsfil

Inställningarna lagras i en INI-fil med namnet `slymail.ini` i samma katalog som SlyMail-programmet. Den här filen delas mellan både SlyMail och `config`-verktyget. Filen är välkommenterad med beskrivningar av varje inställning.

Exempel på `slymail.ini`:
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

Färgteman är `.ini`-filer i katalogen `config_files/`:

**Ice-teman** (`EditorIceColors_*.ini`):
- BlueIce (standard), EmeraldCity, FieryInferno, Fire-N-Ice, GeneralClean, GenericBlue, PurpleHaze, ShadesOfGrey

**DCT-teman** (`EditorDCTColors_*.ini`):
- Default (standard), Default-Modified, Midnight

Temafärgvärden använder ett kompakt format härlett från Synchronet-attributkoder:
- `n` = normal (återställ)
- Förgrund: `k`=svart, `r`=röd, `g`=grön, `y`=gul, `b`=blå, `m`=magenta, `c`=cyan, `w`=vit
- `h` = hög/ljus intensitet
- Bakgrundssiffra: `0`=svart, `1`=röd, `2`=grön, `3`=brun, `4`=blå, `5`=magenta, `6`=cyan, `7`=ljusgrå

Exempel: `nbh` = normal blå ljus, `n4wh` = ljusvit på blå bakgrund

### Taglines

Taglines är korta citat eller talesätt som läggs till i slutet av meddelanden när de sparas. Tagline-funktionen kan aktiveras via Ctrl-U i redigeraren eller `config`-verktyget.

Taglines lagras i `tagline_files/taglines.txt`, en per rad. Rader som börjar med `#` eller `;` behandlas som kommentarer och ignoreras. När ett meddelande sparas med taglines aktiverade uppmanas användaren att antingen välja en specifik tagline eller välja en slumpmässigt. Valda taglines läggs till i meddelandet med prefixet `...` (t.ex. `...To err is human, to really foul things up requires a computer.`).

### Stavningskontroll

SlyMail inkluderar en inbyggd stavningskontroll som använder klartextordböcker. Stavningskontrollen kan konfigureras att fråga vid spara via Ctrl-U i redigeraren eller `config`-verktyget.

**Ordboksfiler** är klartextfiler (ett ord per rad) lagrade i `dictionary_files/`. Flera ordböcker kan väljas samtidigt för kombinerad ordtäckning. SlyMail levereras med:
- `dictionary_en.txt` - Engelska (allmän, ~130 000 ord)
- `dictionary_en-US-supplemental.txt` - Tillägg för amerikansk engelska
- `dictionary_en-GB-supplemental.txt` - Tillägg för brittisk engelska
- `dictionary_en-AU-supplemental.txt` - Tillägg för australisk engelska
- `dictionary_en-CA-supplemental.txt` - Tillägg för kanadensisk engelska

När stavningskontroll utlöses söker kontrollen igenom meddelandet efter felstavade ord och presenterar en interaktiv dialog för varje ord, med alternativen att **E**rsätta ordet, **H**oppa över det, **L**ägga till det (framtida funktion), eller **A**vsluta kontrollen.

## Erkännanden

- Gränssnittet är inspirerat av [DDMsgReader](https://github.com/SynchronetBBS/sbbs) och [SlyEdit](https://github.com/SynchronetBBS/sbbs) av [Nightfox (Eric Oulashin)](https://github.com/EricOulashin)
- QWK-formatkompatibilitet informerad av källkoden för [Synchronet BBS](https://www.synchro.net/)
- CP437-teckendefinitioner från Synchronet

## Licens

Det här projektet är öppen källkodsprogramvara.
