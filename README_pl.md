> **Uwaga:** Ten plik jest tłumaczeniem maszynowym. Oryginał w języku angielskim: [README.md](README.md)

# SlyMail

Jest to wieloplatformowy, tekstowy, offline czytnik poczty dla formatu pakietów [QWK](https://en.wikipedia.org/wiki/QWK_(file_format)). Format pakietów QWK był i nadal jest często używany do wymiany poczty w [systemach BBS (bulletin board systems)](https://en.wikipedia.org/wiki/Bulletin_board_system).

SlyMail oferuje w pełni funkcjonalny interfejs do odczytywania i odpowiadania na wiadomości z pakietów pocztowych QWK systemu BBS (Bulletin Board System). Jego interfejs użytkownika jest inspirowany przez [Digital Distortion Message Reader (DDMsgReader)](https://github.com/SynchronetBBS/sbbs/tree/master/xtrn/DDMsgReader) w zakresie odczytywania wiadomości oraz przez [SlyEdit](https://github.com/SynchronetBBS/sbbs/tree/master/exec) w zakresie edycji wiadomości — oba programy zostały pierwotnie stworzone dla systemu [Synchronet BBS](https://www.synchro.net/).

SlyMail powstał przy pomocy Claude AI.

## Funkcje

### Obsługa pakietów QWK
- Otwiera i odczytuje standardowe pakiety pocztowe QWK (pliki .qwk)
- Analizuje pliki CONTROL.DAT, MESSAGES.DAT oraz indeksowe pliki NDX
- Pełna obsługa QWKE (rozszerzone QWK) przez HEADERS.DAT — dopasowywanie na podstawie przesunięcia dla dokładnych rozszerzonych pól To/From/Subject, flaga UTF-8 oraz Message-ID w formacie RFC822
- Parsowanie kludge-ów QWKE w treści wiadomości (`To:`, `From:`, `Subject:` na początku wiadomości)
- Obsługuje numerowanie konferencji w stylu Synchronet
- Tworzy pakiety odpowiedzi REP (pliki .rep) do przesłania z powrotem do systemu BBS, zawierające HEADERS.DAT dla rozszerzonych pól oraz VOTING.DAT dla oczekujących głosów
- Obsługuje kodowanie zmiennoprzecinkowe Microsoft Binary Format (MBF) w plikach NDX
- Zapamiętuje ostatnio otwarty plik QWK i katalog między sesjami

### Odczytywanie wiadomości (w stylu DDMsgReader)
- Lista konferencji z liczbą wiadomości
- Przewijana lista wiadomości z nawigacją podświetleniem
- Pełny czytnik wiadomości z wyświetlaniem nagłówka (Od, Do, Temat, Data)
- Podświetlanie linii cytatów (obsługuje wielopoziomowe cytowanie)
- Wyświetlanie linii kludge (opcjonalne)
- Wskaźnik paska przewijania
- Nawigacja klawiaturowa: Pierwsza/Ostatnia/Następna/Poprzednia wiadomość, Strona w górę/dół
- Ekrany pomocy dostępne po naciśnięciu `?` lub `F1` we wszystkich widokach

### Obsługa kodów kolorów i atrybutów BBS
SlyMail interpretuje kody kolorów/atrybutów z wielu pakietów oprogramowania BBS, renderując je jako kolorowy tekst zarówno w czytniku, jak i edytorze wiadomości. Obsługiwane formaty:
- **Kody ANSI escape** — zawsze włączone; standardowe sekwencje SGR (ESC[...m) dla pierwszego planu, tła i pogrubienia
- **Kody Synchronet Ctrl-A** — `\x01` + znak atrybutu (np. `\x01c` dla cyjan, `\x01h` dla jasnego)
- **Kody serca WWIV** — `\x03` + cyfra 0–9
- **Kody @X PCBoard/Wildcat** — `@X##` gdzie dwie cyfry szesnastkowe kodują kolor tła i pierwszego planu
- **Kody pipe Celerity** — `|` + litera (np. `|c` dla cyjan, `|W` dla jasnej bieli)
- **Kody pipe Renegade** — `|` + dwucyfrowa liczba 00–31

Każdy typ kodu BBS można indywidualnie włączyć lub wyłączyć przez okno dialogowe **Przełączniki kodów atrybutów** w Ustawieniach czytnika lub w narzędziu `config`. Przełączniki te wpływają zarówno na czytnik, jak i na edytor. Odrębna opcja **Usuń kody ANSI** usuwa wszystkie sekwencje ANSI z wiadomości, gdy jest włączona.

### Załączniki plików
- Wykrywa załączniki plików odwoływane przez linie kludge `@ATTACH:` w treściach wiadomości
- Wyświetla wskaźnik **[ATT]** w nagłówku wiadomości, gdy są dostępne załączniki
- Naciśnij **D** lub **Ctrl-D** w czytniku, aby pobrać załączniki — pokazuje listę plików z rozmiarami i prosi o podanie katalogu docelowego

### Głosowanie i ankiety (Synchronet QWKE)
SlyMail obsługuje rozszerzenie Synchronet VOTING.DAT dla ankiet i głosowania na wiadomości:
- **Ankiety**: Wiadomości zidentyfikowane jako ankiety wyświetlają opcje odpowiedzi z liczbą głosów i paskami procentowymi. Naciśnij **V**, aby otworzyć dialog karty do głosowania, w którym możesz przełączać wybory i oddać swój głos.
- **Głosy za/przeciw**: Dla zwykłych wiadomości (niebędących ankietami) naciśnij **V**, aby głosować za lub przeciw. Aktualne wyniki głosowania i wynik są widoczne w nagłówku wiadomości.
- **Podsumowania głosów**: Nagłówek wiadomości wyświetla liczbę głosów za/przeciw oraz wynik netto, z wskaźnikiem informującym, czy już głosowałeś.
- **Kolejkowanie głosów**: Głosy są kolejkowane razem z odpowiedziami na wiadomości i zapisywane do VOTING.DAT w pakiecie REP do przesłania do systemu BBS.
- **Przeglądarka ankiet**: Naciśnij **V** z listy konferencji, aby przeglądać wszystkie ankiety w pakiecie.

### Obsługa UTF-8
- Wykrywa zawartość UTF-8 w wiadomościach (przez flagę `Utf8` w HEADERS.DAT i automatyczne wykrywanie sekwencji bajtów UTF-8)
- Poprawnie wyświetla znaki UTF-8 w kompatybilnych terminalach
- Pokazuje wskaźnik **[UTF8]** w nagłówku wiadomości dla wiadomości UTF-8
- Zapisuje nowe wiadomości z właściwym kodowaniem
- Konwersja CP437 do UTF-8 dla starszych treści BBS
- Ustawia locale na Linux/macOS/BSD (`setlocale(LC_ALL, "")`) i stronę kodową UTF-8 w Windows dla prawidłowego renderowania terminala

### Edytor wiadomości (inspirowany SlyEdit)
- **Dwa tryby wizualne**: Ice i DCT, każdy z odmiennymi schematami kolorów i układami
- **Tryb losowy**: Losowo wybiera Ice lub DCT przy każdej sesji edycji
- **Naprzemiennie zmieniające się kolory obramowania**: Znaki obramowania losowo przełączają się między dwoma kolorami motywu, odzwierciedlając styl wizualny SlyEdit
- **Obsługa motywów**: Konfigurowalne motywy kolorów ładowane z plików .ini
- Pełnoekranowy edytor tekstu z zawijaniem wierszy
- Okno cytatów do zaznaczania i wstawiania cytowanych tekstów (Ctrl-Q aby otworzyć/zamknąć)
- Odpowiadanie i tworzenie nowych wiadomości
- Menu ESC do zapisu, przerwania, przełączania trybu wstawiania/nadpisywania i inne
- **Selektor kolorów Ctrl-K**: Otwiera dialog do wyboru kolorów pierwszego planu i tła, wstawiając kod ANSI escape w pozycji kursora. Obsługuje 16 kolorów pierwszego planu (8 normalnych + 8 jasnych) i 8 teł, z podglądem na żywo. Naciśnij **N**, aby wstawić kod resetujący.
- **Renderowanie uwzględniające kolory**: Obszar edycji renderuje kody ANSI i atrybutów BBS inline, dzięki czemu kolorowy tekst jest wyświetlany podczas pisania
- **Dialog ustawień użytkownika Ctrl-U** do konfiguracji preferencji edytora w locie
- **Prompty tak/nie specyficzne dla stylu**: Tryb Ice używa promptu dolnego ekranu inline; tryb DCT używa wyśrodkowanego okna dialogowego z motywowanymi kolorami

### Ustawienia edytora (przez Ctrl-U)
- **Wybierz tryb interfejsu**: Dialog do przełączania między stylami Ice, DCT i Random (efekt natychmiastowy)
- **Wybierz plik motywu**: Wybierz spośród dostępnych motywów kolorów Ice lub DCT
- **Tagline'y**: Po włączeniu pyta o wybór tagline przy zapisie (z `tagline_files/taglines.txt`)
- **Słownik/słowniki kontroli pisowni**: Wybierz, których słowników używać
- **Pytaj o kontrolę pisowni przy zapisie**: Po włączeniu oferuje sprawdzenie pisowni przed zapisem
- **Zawijaj linie cytatów do szerokości terminala**: Zawijanie cytowanych linii
- **Cytuj z inicjałami autora**: Poprzedzaj linie cytatów inicjałami autora (np. `MP> `)
- **Wcięcie linii cytatów zawierających inicjały**: Dodaj wiodącą spację przed inicjałami (np. ` MP> `)
- **Usuń spacje z linii cytatów**: Usuń wiodące białe znaki z cytowanego tekstu

### Motywy kolorów
- Pliki motywów to pliki konfiguracyjne (`.ini`) w katalogu `config_files/`
- Motywy Ice: `EditorIceColors_*.ini` (BlueIce, EmeraldCity, FieryInferno, itd.)
- Motywy DCT: `EditorDCTColors_*.ini` (Default, Default-Modified, Midnight)
- Kolory motywów używają prostego formatu: litera koloru pierwszego planu (`r`/`g`/`b`/`c`/`y`/`m`/`w`/`k`), opcjonalne `h` dla jasnego, opcjonalna cyfra tła (`0`-`7`)
- Motywy kontrolują kolory wszystkich elementów interfejsu: obramowania, etykiety, wartości, okno cytatów, pasek pomocy, dialogi tak/nie

### Kontroler pisowni
- Wbudowany kontroler pisowni używający zwykłych słowników tekstowych
- Dostarczany ze słownikami angielskimi (en, en-US, en-GB, en-AU, en-CA jako uzupełnienia)
- Interaktywny dialog korekty: Zamień, Pomiń lub Zakończ
- Pliki słowników przechowywane w katalogu `dictionary_files/`

### Tagline'y
- Pliki tagline przechowywane w katalogu `tagline_files/`
- Domyślny plik tagline to `tagline_files/taglines.txt`, jeden tagline na linię
- Linie zaczynające się od `#` lub `;` są traktowane jako komentarze i ignorowane
- Wybierz konkretny tagline lub losuj przy zapisie wiadomości
- Tagline'y są dołączane do wiadomości z prefiksem `...`

### Tworzenie pakietu REP
- Aktualnie obsługiwany jest tylko format ZIP (planuję dodać obsługę więcej typów kompresji w przyszłości)
- Gdy piszesz odpowiedzi lub nowe wiadomości, są one kolejkowane jako oczekujące
- Głosy (karty do głosowania w ankietach, głosy za/przeciw) są również kolejkowane razem z odpowiedziami
- Przy wyjściu (lub po otwarciu nowego pliku QWK) SlyMail pyta o zapisanie wszystkich oczekujących elementów
- Tworzy standardowy plik `.rep` (archiwum ZIP) do przesłania do systemu BBS, zawierający:
  - `<BBSID>.MSG` — wiadomości odpowiedzi w standardowym formacie QWK
  - `HEADERS.DAT` — rozszerzone nagłówki QWKE dla pól przekraczających 25 znaków
  - `VOTING.DAT` — oczekujące głosy w formacie INI kompatybilnym z Synchronet
- Plik REP jest zapisywany jako `<BBS-ID>.rep` w skonfigurowanym katalogu odpowiedzi (lub w katalogu pliku QWK)

### Systemy zdalne (Ctrl-R)
SlyMail może pobierać pakiety QWK bezpośrednio ze zdalnych systemów przez FTP lub SFTP (SSH):
- Naciśnij **Ctrl-R** z przeglądarki plików, aby otworzyć katalog systemów zdalnych
- **Dodaj/Edytuj/Usuń** wpisy zdalnych systemów z: nazwą, hostem, portem, typem połączenia (FTP lub SSH), nazwą użytkownika, hasłem, przełącznikiem pasywnego FTP i początkową ścieżką zdalną
- **Przeglądaj zdalne katalogi** za pomocą przeglądarki plików/katalogów podobnej do lokalnej przeglądarki plików — nawiguj do katalogów, wróć do góry z `..`, przejdź do katalogu głównego z `/`
- **Pobieraj pliki QWK** ze zdalnego systemu bezpośrednio do podkatalogu `QWK` katalogu danych SlyMail
- Wpisy systemów zdalnych są utrwalane w `remote_systems.json` w katalogu danych SlyMail
- Data/godzina ostatniego połączenia jest śledzona dla każdego systemu
- Używa polecenia systemowego `curl` dla transferów FTP i SFTP (brak zależności bibliotek na etapie kompilacji)

### Ustawienia aplikacji
- Trwałe ustawienia zapisywane w `slymail.ini` w katalogu danych SlyMail (`~/.slymail` na Linux/macOS/BSD lub katalogu domowym użytkownika w Windows)
- Katalog danych SlyMail i jego podkatalog `QWK` są tworzone automatycznie przy pierwszym uruchomieniu
- Domyślny katalog do przeglądania plików QWK i zapisywania pakietów REP to `~/.slymail/QWK`
- Zapamiętuje ostatnio przeglądany katalog i nazwę pliku QWK
- Skrót klawiszowy Ctrl-L do załadowania innego pliku QWK z widoku konferencji lub listy wiadomości
- Konfigurowalny prefiks cytatu, szerokość linii cytatu, nazwa użytkownika
- Opcje czytnika: pokaż/ukryj linie kludge, linie tear/origin, pasek przewijania, usuń kody ANSI
- Przełączniki kodów atrybutów dla każdego BBS (Synchronet, WWIV, Celerity, Renegade, PCBoard/Wildcat) — wpływają zarówno na czytnik, jak i edytor
- Katalog wyjściowy pakietu REP

## Zrzuty ekranu

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

## Ustawienia Synchronet dla pakietu QWK
W systemie Synchronet BBS, w ustawieniach pakietu QWK, Slymail jest (lub powinien być) kompatybilny z kodami kolorów Ctrl-A, VOTING.DAT, załącznikami plików i formatem pakietu QWKE. Slymail powinien być również kompatybilny z UTF-8. Na przykład:
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

## Kompilacja

### Wymagania

**Linux / macOS / BSD:**
- Kompilator kompatybilny z C++17 (GCC 8+, Clang 7+)
- Biblioteka deweloperska ncurses (`libncurses-dev` na Debian/Ubuntu, `ncurses-devel` na Fedora/RHEL)
- Polecenie `unzip` (do wypakowywania pakietów QWK)
- Polecenie `zip` (do tworzenia pakietów REP)
- Polecenie `curl` (do transferów FTP/SFTP zdalnych systemów — opcjonalne, potrzebne tylko dla funkcji systemów zdalnych)

**Windows (Visual Studio 2022):**
- Visual Studio 2022 z obciążeniem roboczym „Programowanie aplikacji klasycznych w języku C++"
- Windows SDK 10.0 (dołączony do VS)
- Nie są wymagane żadne dodatkowe biblioteki — używa wbudowanego Win32 Console API dla interfejsu terminalowego oraz `tar.exe` lub PowerShell do obsługi pakietów ZIP QWK/REP (patrz uwaga poniżej)

**Windows (MinGW/MSYS2):**
- MinGW-w64 lub MSYS2 z GCC (obsługa C++17)
- Windows Console API (wbudowany)

> **Uwaga — Obsługa ZIP QWK/REP w Windows:** SlyMail wykrywa w czasie wykonania, które narzędzie jest dostępne i używa najlepszej opcji:
>
> - **`tar.exe` (preferowane):** Dostarczany z systemem Windows 10 w wersji 1803 (aktualizacja z kwietnia 2018) i nowszymi oraz z wszystkimi wersjami systemu Windows 11. `tar` odczytuje pliki ZIP według ich zawartości, a nie rozszerzenia pliku, więc pakiety `.qwk` są wypakowywane bezpośrednio, a pakiety `.rep` są tworzone przez tymczasowy plik `.zip`, który jest następnie zmieniany na właściwą nazwę. Nie jest wymagana żadna dodatkowa konfiguracja.
> - **PowerShell (zastępcze):** Jeśli `tar.exe` nie zostanie znaleziony w PATH, SlyMail korzysta z PowerShell. Do wypakowywania używa klasy .NET `ZipFile` (`System.IO.Compression`) zamiast `Expand-Archive`, ponieważ `Expand-Archive` odrzuca rozszerzenia plików inne niż `.zip`, nawet gdy plik jest prawidłowym archiwum ZIP. Do tworzenia pakietu REP używa `Compress-Archive`, zapisując do tymczasowego pliku `.zip`, który jest następnie zmieniany na `.rep`.

### Kompilacja na Linux/macOS/BSD

```bash
make
```

Kompiluje dwa programy:
- `slymail` — główna aplikacja czytnika QWK
- `config` — samodzielne narzędzie konfiguracyjne

### Kompilacja z symbolami debugowania

```bash
make debug
```

### Instalacja (opcjonalne)

```bash
sudo make install    # Installs slymail and config to /usr/local/bin/
sudo make uninstall  # Remove
```

### Kompilacja w Windows z Visual Studio 2022

Otwórz plik rozwiązania w Visual Studio 2022:

```
vs\SlyMail.sln
```

Lub skompiluj z wiersza poleceń używając MSBuild:

```powershell
# Release build (output in vs\x64\Release\)
msbuild vs\SlyMail.sln /p:Configuration=Release /p:Platform=x64

# Debug build (output in vs\x64\Debug\)
msbuild vs\SlyMail.sln /p:Configuration=Debug /p:Platform=x64
```

Kompiluje dwa pliki wykonywalne:
- `x64\Release\slymail.exe` — główny czytnik QWK
- `x64\Release\config_slymail.exe` — samodzielne narzędzie konfiguracyjne

Rozwiązanie zawiera dwa projekty (`SlyMail.vcxproj` i `Config.vcxproj`) przeznaczone dla x64, C++17, z zestawem narzędzi MSVC v143.

### Kompilacja w Windows (MinGW/MSYS2)

```bash
make
```

Makefile automatycznie wykrywa platformę i używa odpowiedniej implementacji terminala:
- **Linux/macOS/BSD**: ncurses (`terminal_ncurses.cpp`)
- **Windows**: conio + Win32 Console API (`terminal_win32.cpp`)

## Użytkowanie

```bash
# Launch SlyMail with file browser
./slymail

# Open a specific QWK packet
./slymail MYBBS.qwk

# Run the standalone configuration utility
./config_slymail
```

### Program konfiguracyjny

Narzędzie `config` zapewnia samodzielny, tekstowy interfejs do konfigurowania ustawień SlyMail bez otwierania głównej aplikacji. Oferuje cztery kategorie konfiguracji:

- **Ustawienia edytora** — Wszystkie ustawienia dostępne przez Ctrl-U w edytorze (styl edytora, tagline'y, kontrola pisowni, opcje cytowania itp.)
- **Ustawienia czytnika** — Przełącz linie kludge, linie tear, pasek przewijania, usuwanie ANSI, tryb lightbar, odwrócona kolejność i przełączniki kodów atrybutów (włącz/wyłącz dla każdego BBS)
- **Ustawienia motywów** — Wybierz pliki motywów kolorów Ice i DCT z katalogu `config_files/`
- **Ustawienia ogólne** — Ustaw swoje imię dla odpowiedzi i katalog wyjściowy pakietu REP

Ustawienia są zapisywane automatycznie po wyjściu z każdej kategorii. Zarówno SlyMail, jak i narzędzie config czytają i zapisują ten sam plik ustawień.

### Skróty klawiszowe

#### Przeglądarka plików
| Klawisz | Akcja |
|---------|-------|
| Góra/Dół | Nawigacja po plikach i katalogach |
| Enter | Otwórz katalog / Wybierz plik QWK |
| Ctrl-R | Otwórz katalog systemów zdalnych |
| Q / ESC | Zakończ |

#### Lista konferencji
| Klawisz | Akcja |
|---------|-------|
| Góra/Dół | Nawigacja po konferencjach |
| Enter | Otwórz wybraną konferencję |
| V | Wyświetl ankiety/głosy w pakiecie |
| O / Ctrl-L | Otwórz inny plik QWK |
| S / Ctrl-U | Ustawienia |
| Q / ESC | Zakończ SlyMail |
| ? / F1 | Pomoc |

#### Lista wiadomości
| Klawisz | Akcja |
|---------|-------|
| Góra/Dół | Nawigacja po wiadomościach |
| Enter / R | Odczytaj wybraną wiadomość |
| N | Napisz nową wiadomość |
| G | Przejdź do numeru wiadomości |
| Ctrl-L | Otwórz inny plik QWK |
| S / Ctrl-U | Ustawienia |
| C / ESC | Wróć do listy konferencji |
| Q | Zakończ |
| ? / F1 | Pomoc |

#### Czytnik wiadomości
| Klawisz | Akcja |
|---------|-------|
| Góra/Dół | Przewiń wiadomość |
| Lewo/Prawo | Poprzednia / Następna wiadomość |
| F / L | Pierwsza / Ostatnia wiadomość |
| R | Odpowiedz na wiadomość |
| V | Głosuj (głos za/przeciw lub karta do głosowania w ankiecie) |
| D / Ctrl-D | Pobierz załączniki plików |
| H | Pokaż informacje o nagłówku wiadomości |
| S / Ctrl-U | Ustawienia |
| C / Q / ESC | Wróć do listy wiadomości |
| ? / F1 | Pomoc |

#### Edytor wiadomości
| Klawisz | Akcja |
|---------|-------|
| ESC | Menu edytora (Zapisz, Przerwij itp.) |
| Ctrl-U | Dialog ustawień użytkownika |
| Ctrl-Q | Otwórz/zamknij okno cytatów |
| Ctrl-K | Selektor kolorów (wstaw kod koloru ANSI w pozycji kursora) |
| Ctrl-G | Wstaw znak graficzny (CP437) według kodu |
| Ctrl-W | Wyszukaj słowo/tekst |
| Ctrl-S | Zmień temat |
| Ctrl-D | Usuń bieżącą linię |
| Ctrl-Z | Zapisz wiadomość |
| Ctrl-A | Przerwij wiadomość |
| F1 | Ekran pomocy |
| Insert | Przełącz tryb Wstawianie/Nadpisywanie |

#### Okno cytatów
| Klawisz | Akcja |
|---------|-------|
| Góra/Dół | Nawigacja po liniach cytatów |
| Enter | Wstaw wybraną linię cytatu |
| Ctrl-Q / ESC | Zamknij okno cytatów |

## Architektura

SlyMail używa warstwy abstrakcji platformy dla swojego tekstowego interfejsu użytkownika:

```
ITerminal (abstract base class)
    ├── NCursesTerminal  (Linux/macOS/BSD - ncurses)
    └── Win32Terminal    (Windows - conio + Win32 Console API)
```

Znaki ramkowe CP437 i znaki specjalne są zdefiniowane w `cp437defs.h` i renderowane przez metodę `putCP437()`, która mapuje kody CP437 na odpowiedniki natywne dla platformy (znaki ACS w ncurses, bezpośrednie bajty CP437 w Windows).

### Pliki źródłowe

| Plik | Opis |
|------|------|
| `terminal.h` | Abstrakcyjny interfejs `ITerminal`, stałe klawiszy/kolorów, fabryka |
| `terminal_ncurses.cpp` | Implementacja ncurses z mapowaniem CP437-do-ACS |
| `terminal_win32.cpp` | Windows Console API + implementacja conio |
| `cp437defs.h` | Definicje znaków IBM Code Page 437 |
| `colors.h` | Definicje schematów kolorów (Ice, DCT, czytnik, lista) |
| `theme.h` | Parser pliku konfiguracyjnego motywu (kody atrybutów w stylu Synchronet) |
| `ui_common.h` | Wspólne narzędzia interfejsu (dialogi, wprowadzanie tekstu, pasek przewijania itp.) |
| `qwk.h` / `qwk.cpp` | Parser i kreator pakietów QWK/REP (QWKE, załączniki, głosowanie) |
| `bbs_colors.h` / `bbs_colors.cpp` | Parser kodów kolorów/atrybutów BBS (ANSI, Synchronet, WWIV, PCBoard, Celerity, Renegade) |
| `utf8_util.h` / `utf8_util.cpp` | Narzędzia UTF-8 (walidacja, szerokość wyświetlania, konwersja CP437-do-UTF-8) |
| `voting.h` / `voting.cpp` | Parser VOTING.DAT, zliczanie głosów, interfejs wyświetlania ankiet |
| `remote_systems.h` / `remote_systems.cpp` | Katalog systemów zdalnych, przeglądanie FTP/SFTP, trwałość JSON, narzędzia katalogu domowego |
| `settings.h` | Trwałość ustawień użytkownika |
| `settings_dialog.h` | Dialogi ustawień (edytor, czytnik, przełączniki kodów atrybutów) |
| `file_browser.h` | Przeglądarka i selektor plików QWK |
| `msg_list.h` | Widoki listy konferencji i wiadomości |
| `msg_reader.h` | Czytnik wiadomości (w stylu DDMsgReader) z głosowaniem i interfejsem załączników |
| `msg_editor.h` | Edytor wiadomości (w stylu SlyEdit Ice/DCT) z selektorem kolorów |
| `main.cpp` | Punkt wejścia aplikacji SlyMail i główna pętla |
| `config.cpp` | Samodzielne narzędzie konfiguracyjne |

## Konfiguracja

### Plik ustawień

Ustawienia są przechowywane w pliku INI o nazwie `slymail.ini` w tym samym katalogu co plik wykonywalny SlyMail. Ten plik jest współdzielony zarówno przez SlyMail, jak i narzędzie `config`. Plik jest dobrze skomentowany z opisami każdego ustawienia.

Przykładowy `slymail.ini`:
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

### Pliki motywów

Motywy kolorów to pliki `.ini` w katalogu `config_files/`:

**Motywy Ice** (`EditorIceColors_*.ini`):
- BlueIce (domyślny), EmeraldCity, FieryInferno, Fire-N-Ice, GeneralClean, GenericBlue, PurpleHaze, ShadesOfGrey

**Motywy DCT** (`EditorDCTColors_*.ini`):
- Default (domyślny), Default-Modified, Midnight

Wartości kolorów motywów używają zwartego formatu wywodzącego się z kodów atrybutów Synchronet:
- `n` = normalny (reset)
- Kolory pierwszego planu: `k`=czarny, `r`=czerwony, `g`=zielony, `y`=żółty, `b`=niebieski, `m`=magenta, `c`=cyjan, `w`=biały
- `h` = wysoka/jasna intensywność
- Cyfra tła: `0`=czarny, `1`=czerwony, `2`=zielony, `3`=brązowy, `4`=niebieski, `5`=magenta, `6`=cyjan, `7`=jasnoszary

Przykład: `nbh` = jasny niebieski, `n4wh` = jasna biel na niebieskim tle

### Tagline'y

Tagline'y to krótkie cytaty lub powiedzonka dołączane na końcu wiadomości przy zapisie. Funkcja tagline może być włączona przez Ctrl-U w edytorze lub narzędzie `config`.

Tagline'y są przechowywane w `tagline_files/taglines.txt`, po jednym na linię. Linie zaczynające się od `#` lub `;` są traktowane jako komentarze i ignorowane. Przy zapisywaniu wiadomości z włączonymi tagline'ami użytkownik jest proszony o wybranie konkretnego tagline lub wylosowanie jednego. Wybrany tagline jest dołączany do wiadomości z prefiksem `...` (np. `...To err is human, to really foul things up requires a computer.`).

### Kontroler pisowni

SlyMail zawiera wbudowany kontroler pisowni używający zwykłych słowników tekstowych. Kontroler pisowni można skonfigurować tak, aby pytał przy zapisie przez Ctrl-U w edytorze lub narzędzie `config`.

**Pliki słowników** to zwykłe pliki tekstowe (jedno słowo na linię) przechowywane w `dictionary_files/`. Jednocześnie można wybrać wiele słowników dla połączonego pokrycia słów. SlyMail jest dostarczany z:
- `dictionary_en.txt` — angielski (ogólny, ~130 tys. słów)
- `dictionary_en-US-supplemental.txt` — uzupełnienie dla angielskiego amerykańskiego
- `dictionary_en-GB-supplemental.txt` — uzupełnienie dla angielskiego brytyjskiego
- `dictionary_en-AU-supplemental.txt` — uzupełnienie dla angielskiego australijskiego
- `dictionary_en-CA-supplemental.txt` — uzupełnienie dla angielskiego kanadyjskiego

Gdy kontrola pisowni jest uruchamiana, sprawdza wiadomość pod kątem błędnie napisanych słów i wyświetla interaktywny dialog dla każdego z nich, oferując opcje **Z**amień słowo, **P**omiń je, **D**odaj je (w przyszłości) lub **Z**akończ sprawdzanie.

## Podziękowania

- Interfejs inspirowany przez [DDMsgReader](https://github.com/SynchronetBBS/sbbs) i [SlyEdit](https://github.com/SynchronetBBS/sbbs) autorstwa [Nightfox (Eric Oulashin)](https://github.com/EricOulashin)
- Kompatybilność formatu QWK oparta na kodzie źródłowym [Synchronet BBS](https://www.synchro.net/)
- Definicje znaków CP437 pochodzą z Synchronet

## Licencja

Ten projekt jest oprogramowaniem open source.
