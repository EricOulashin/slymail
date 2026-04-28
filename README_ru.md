> **Примечание:** Этот файл является машинным переводом. Оригинал на английском языке: [README.md](README.md)

# SlyMail

Это кроссплатформенный текстовый офлайн-ридер почты для формата пакетов [QWK](https://en.wikipedia.org/wiki/QWK_(file_format)). Формат пакетов QWK используется/использовался для обмена почтой в [системах BBS (bulletin board systems)](https://en.wikipedia.org/wiki/Bulletin_board_system).

SlyMail предоставляет полнофункциональный интерфейс для чтения и ответа на сообщения из QWK-пакетов BBS. Его пользовательский интерфейс вдохновлён [Digital Distortion Message Reader (DDMsgReader)](https://github.com/SynchronetBBS/sbbs/tree/master/xtrn/DDMsgReader) для чтения сообщений и [SlyEdit](https://github.com/SynchronetBBS/sbbs/tree/master/exec) для редактирования сообщений — оба были изначально созданы для [Synchronet BBS](https://www.synchro.net/).

SlyMail создан с помощью Claude AI.

## Возможности

### Поддержка пакетов QWK
- Открывает и читает стандартные почтовые пакеты QWK (файлы .qwk)
- Разбирает файлы CONTROL.DAT, MESSAGES.DAT и индексные файлы NDX
- Полная поддержка QWKE (расширенный QWK) через HEADERS.DAT — сопоставление на основе смещений для точных расширенных полей To/From/Subject, флаг UTF-8 и RFC822 Message-ID
- Разбор кладж-строк QWKE в теле сообщения (`To:`, `From:`, `Subject:` в начале сообщения)
- Поддержка нумерации конференций в стиле Synchronet
- Создаёт пакеты ответов REP (файлы .rep) для загрузки обратно на BBS, включая HEADERS.DAT для расширенных полей и VOTING.DAT для ожидающих голосов
- Поддерживает кодирование чисел с плавающей запятой Microsoft Binary Format (MBF) в файлах NDX
- Запоминает последний открытый файл QWK и каталог между сессиями

### Чтение сообщений (в стиле DDMsgReader)
- Список конференций с количеством сообщений
- Прокручиваемый список сообщений с навигацией по подсветке
- Полный ридер сообщений с отображением заголовка (Откого, Кому, Тема, Дата)
- Подсветка строк цитирования (поддерживает многоуровневое цитирование)
- Отображение кладж-строк (опционально)
- Индикатор полосы прокрутки
- Навигация с клавиатуры: Первое/Последнее/Следующее/Предыдущее сообщение, Страница вверх/вниз
- Экраны справки доступны по нажатию `?` или `F1` во всех видах

### Поддержка цветовых кодов и атрибутов BBS
SlyMail интерпретирует коды цвета/атрибутов из нескольких программных пакетов BBS, отображая их как цветной текст в ридере и редакторе сообщений. Поддерживаемые форматы:
- **Коды ANSI escape** — всегда включены; стандартные последовательности SGR (ESC[...m) для цвета переднего плана, фона и жирного шрифта
- **Коды Synchronet Ctrl-A** — `\x01` + символ атрибута (например, `\x01c` для голубого, `\x01h` для яркого)
- **Коды сердечек WWIV** — `\x03` + цифра 0–9
- **Коды @X PCBoard/Wildcat** — `@X##` где две шестнадцатеричные цифры кодируют цвет фона и переднего плана
- **Коды pipe Celerity** — `|` + буква (например, `|c` для голубого, `|W` для яркого белого)
- **Коды pipe Renegade** — `|` + двузначное число 00–31

Каждый тип кода BBS можно индивидуально включить или отключить через подменю **Переключатели кодов атрибутов** в настройках ридера или утилите `config`. Эти переключатели влияют как на ридер, так и на редактор. Отдельная опция **Удалять коды ANSI** убирает все последовательности ANSI из сообщений при включении.

### Вложения файлов
- Обнаруживает вложения файлов, указанные через кладж-строки `@ATTACH:` в теле сообщения
- Показывает индикатор **[ATT]** в заголовке сообщения при наличии вложений
- Нажмите **D** или **Ctrl-D** в ридере для загрузки вложений — отображает список файлов с размерами и запрашивает каталог назначения

### Голосование и опросы (Synchronet QWKE)
SlyMail поддерживает расширение Synchronet VOTING.DAT для опросов и голосования за сообщения:
- **Опросы**: Сообщения, определённые как опросы, отображают варианты ответов с количеством голосов и процентными полосками. Нажмите **V**, чтобы открыть диалог голосования, где можно выбрать ответы и проголосовать.
- **Голоса за/против**: Для обычных сообщений (не опросов) нажмите **V**, чтобы проголосовать за или против. Текущий подсчёт голосов и счёт отображаются в заголовке сообщения.
- **Подсчёт голосов**: Заголовок сообщения отображает количество голосов за/против и итоговый счёт, с индикатором, если вы уже проголосовали.
- **Очередь голосов**: Голоса ставятся в очередь вместе с ответами на сообщения и записываются в VOTING.DAT в пакете REP для загрузки на BBS.
- **Браузер опросов**: Нажмите **V** из списка конференций, чтобы просмотреть все опросы в пакете.

### Поддержка UTF-8
- Обнаруживает содержимое UTF-8 в сообщениях (через флаг `Utf8` в HEADERS.DAT и автоматическое обнаружение байтовых последовательностей UTF-8)
- Корректно отображает символы UTF-8 в совместимых терминалах
- Показывает индикатор **[UTF8]** в заголовке сообщения для сообщений UTF-8
- Сохраняет новые сообщения с правильной кодировкой
- Конвертация CP437 в UTF-8 для устаревшего контента BBS
- Устанавливает locale на Linux/macOS/BSD (`setlocale(LC_ALL, "")`) и кодовую страницу UTF-8 в Windows для правильного рендеринга терминала

### Редактор сообщений (вдохновлён SlyEdit)
- **Два визуальных режима**: Ice и DCT, каждый со своей цветовой схемой и компоновкой
- **Случайный режим**: Случайно выбирает Ice или DCT при каждой сессии редактирования
- **Чередующиеся цвета рамки**: Символы рамки случайно чередуются между двумя цветами темы, соответствуя визуальному стилю SlyEdit
- **Поддержка тем**: Настраиваемые цветовые темы, загружаемые из файлов .ini
- Полноэкранный текстовый редактор с переносом строк
- Окно цитат для выбора и вставки цитируемого текста (Ctrl-Q для открытия/закрытия)
- Создание ответов и новых сообщений
- Меню ESC для сохранения, отмены, переключения режима вставки/замены и другого
- **Выбор цвета Ctrl-K**: Открывает диалог для выбора цветов переднего плана и фона, вставляя код ANSI escape в позиции курсора. Поддерживает 16 цветов переднего плана (8 обычных + 8 ярких) и 8 фонов, с предварительным просмотром в реальном времени. Нажмите **N** для вставки кода сброса.
- **Рендеринг с учётом цвета**: Область редактирования отображает коды ANSI и атрибутов BBS встроенно, поэтому цветной текст виден по мере ввода
- **Диалог настроек пользователя Ctrl-U** для настройки параметров редактора на лету
- **Подсказки да/нет, специфичные для стиля**: Режим Ice использует встроенную подсказку внизу экрана; режим DCT использует центрированное диалоговое окно с тематическими цветами

### Настройки редактора (через Ctrl-U)
- **Выбор режима интерфейса**: Диалог для переключения между стилями Ice, DCT и Random (эффект немедленный)
- **Выбор файла темы**: Выбор из доступных цветовых тем Ice или DCT
- **Подписи (Taglines)**: При включении запрашивает выбор подписи при сохранении (из `tagline_files/taglines.txt`)
- **Словарь/словари проверки орфографии**: Выбор используемых словарей
- **Предлагать проверку орфографии при сохранении**: При включении предлагает проверку орфографии перед сохранением
- **Перенос строк цитат по ширине терминала**: Перенос цитируемых строк
- **Цитирование с инициалами автора**: Добавлять перед строками цитат инициалы автора (например, `MP> `)
- **Отступ строк цитат с инициалами**: Добавлять ведущий пробел перед инициалами (например, ` MP> `)
- **Удалять пробелы из строк цитат**: Убирать ведущие пробелы из цитируемого текста

### Цветовые темы
- Файлы тем — это файлы конфигурации (`.ini`) в каталоге `config_files/`
- Темы Ice: `EditorIceColors_*.ini` (BlueIce, EmeraldCity, FieryInferno и др.)
- Темы DCT: `EditorDCTColors_*.ini` (Default, Default-Modified, Midnight)
- Цвета темы используют простой формат: буква цвета переднего плана (`r`/`g`/`b`/`c`/`y`/`m`/`w`/`k`), опциональное `h` для яркого, опциональная цифра фона (`0`-`7`)
- Темы управляют всеми цветами элементов интерфейса: рамки, метки, значения, окно цитат, строка справки, диалоги да/нет

### Проверка орфографии
- Встроенная проверка орфографии, использующая словари в виде обычных текстовых файлов
- Поставляется с английскими словарями (en, en-US, en-GB, en-AU, en-CA — дополнения)
- Интерактивный диалог исправления: Заменить, Пропустить или Выйти
- Файлы словарей хранятся в каталоге `dictionary_files/`

### Подписи (Taglines)
- Файлы подписей хранятся в каталоге `tagline_files/`
- Файл подписей по умолчанию: `tagline_files/taglines.txt`, одна подпись на строку
- Строки, начинающиеся с `#` или `;`, считаются комментариями и игнорируются
- Выберите конкретную подпись или случайную при сохранении сообщения
- Подписи добавляются к сообщениям с префиксом `...`

### Создание пакета REP
- В настоящее время поддерживается только ZIP (в будущем планируется добавить поддержку других типов сжатия)
- Когда вы пишете ответы или новые сообщения, они ставятся в очередь как ожидающие
- Голоса (бюллетени опросов, голоса за/против) также ставятся в очередь вместе с ответами
- При выходе (или при открытии нового файла QWK) SlyMail предлагает сохранить все ожидающие элементы
- Создаёт стандартный файл `.rep` (ZIP-архив) для загрузки на BBS, содержащий:
  - `<BBSID>.MSG` — сообщения-ответы в стандартном формате QWK
  - `HEADERS.DAT` — расширенные заголовки QWKE для полей, превышающих 25 символов
  - `VOTING.DAT` — ожидающие голоса в формате INI, совместимом с Synchronet
- Файл REP сохраняется как `<BBS-ID>.rep` в настроенном каталоге ответов (или в каталоге файла QWK)

### Удалённые системы (Ctrl-R)
SlyMail может загружать пакеты QWK непосредственно с удалённых систем через FTP или SFTP (SSH):
- Нажмите **Ctrl-R** в браузере файлов, чтобы открыть каталог удалённых систем
- **Добавляйте/редактируйте/удаляйте** записи удалённых систем с: именем, хостом, портом, типом соединения (FTP или SSH), именем пользователя, паролем, переключателем пассивного FTP и начальным удалённым путём
- **Просматривайте удалённые каталоги** с помощью браузера файлов/каталогов, аналогичного локальному — перемещайтесь в каталоги, возвращайтесь вверх с `..`, переходите в корень с `/`
- **Загружайте файлы QWK** с удалённой системы непосредственно в подкаталог `QWK` каталога данных SlyMail
- Записи удалённых систем сохраняются в `remote_systems.json` в каталоге данных SlyMail
- Дата/время последнего подключения отслеживается для каждой системы
- Использует системную команду `curl` для передачи файлов по FTP и SFTP (нет зависимостей от библиотек на этапе компиляции)

### Настройки приложения
- Постоянные настройки сохраняются в `slymail.ini` в каталоге данных SlyMail (`~/.slymail` на Linux/macOS/BSD или в домашнем каталоге пользователя в Windows)
- Каталог данных SlyMail и его подкаталог `QWK` создаются автоматически при первом запуске
- Каталог по умолчанию для просмотра файлов QWK и сохранения пакетов REP: `~/.slymail/QWK`
- Запоминает последний просматриваемый каталог и имя файла QWK
- Горячая клавиша Ctrl-L для загрузки другого файла QWK из видов конференции или списка сообщений
- Настраиваемый префикс цитаты, ширина строки цитаты, имя пользователя
- Параметры ридера: показывать/скрывать кладж-строки, строки tear/origin, полосу прокрутки, удалять коды ANSI
- Переключатели кодов атрибутов для каждого BBS (Synchronet, WWIV, Celerity, Renegade, PCBoard/Wildcat) — влияют как на ридер, так и на редактор
- Каталог вывода пакета REP

## Снимки экрана

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

## Настройки Synchronet для пакета QWK
На Synchronet BBS в настройках пакета QWK Slymail совместим (или должен быть совместим) с кодами цвета Ctrl-A, VOTING.DAT, вложениями файлов и форматом пакета QWKE. Slymail также должен быть совместим с UTF-8. Например:
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

## Сборка

### Требования

**Linux / macOS / BSD:**
- Компилятор, совместимый с C++17 (GCC 8+, Clang 7+)
- Библиотека разработки ncurses (`libncurses-dev` на Debian/Ubuntu, `ncurses-devel` на Fedora/RHEL)
- Команда `unzip` (для распаковки пакетов QWK)
- Команда `zip` (для создания пакетов REP)
- Команда `curl` (для передачи файлов FTP/SFTP через удалённые системы — необязательно, нужна только для функции удалённых систем)

**Windows (Visual Studio 2022):**
- Visual Studio 2022 с нагрузкой «Разработка настольных приложений на C++»
- Windows SDK 10.0 (входит в VS)
- Дополнительных библиотек не требуется — используется встроенный Win32 Console API для терминального интерфейса, а также `tar.exe` или PowerShell для обработки ZIP-пакетов QWK/REP (см. примечание ниже)

**Windows (MinGW/MSYS2):**
- MinGW-w64 или MSYS2 с GCC (поддержка C++17)
- Windows Console API (встроен)

> **Примечание — Обработка ZIP QWK/REP в Windows:** SlyMail определяет во время выполнения, какой инструмент доступен, и использует наилучший вариант:
>
> - **`tar.exe` (предпочтительно):** Поставляется с Windows 10 версии 1803 (обновление апреля 2018) и более поздними, а также со всеми версиями Windows 11. `tar` читает ZIP-файлы по их содержимому, а не по расширению файла, поэтому пакеты `.qwk` распаковываются напрямую, а пакеты `.rep` создаются через временный файл `.zip`, который затем переименовывается. Дополнительная настройка не требуется.
> - **PowerShell (резервный вариант):** Если `tar.exe` не найден в PATH, SlyMail переключается на PowerShell. Для распаковки используется класс .NET `ZipFile` (`System.IO.Compression`), а не `Expand-Archive`, поскольку `Expand-Archive` отклоняет расширения файлов, отличные от `.zip`, даже когда файл является действительным ZIP-архивом. Для создания пакета REP используется `Compress-Archive`, записывая во временный файл `.zip`, который затем переименовывается в `.rep`.

### Сборка на Linux/macOS/BSD

```bash
make
```

Собирает две программы:
- `slymail` — основное приложение ридера QWK
- `config` — автономная утилита конфигурации

### Сборка с отладочными символами

```bash
make debug
```

### Установка (необязательно)

```bash
sudo make install    # Installs slymail and config to /usr/local/bin/
sudo make uninstall  # Remove
```

### Сборка в Windows с Visual Studio 2022

Откройте файл решения в Visual Studio 2022:

```
vs\SlyMail.sln
```

Или выполните сборку из командной строки с помощью MSBuild:

```powershell
# Release build (output in vs\x64\Release\)
msbuild vs\SlyMail.sln /p:Configuration=Release /p:Platform=x64

# Debug build (output in vs\x64\Debug\)
msbuild vs\SlyMail.sln /p:Configuration=Debug /p:Platform=x64
```

Собирает два исполняемых файла:
- `x64\Release\slymail.exe` — основной ридер QWK
- `x64\Release\config_slymail.exe` — автономная утилита конфигурации

Решение содержит два проекта (`SlyMail.vcxproj` и `Config.vcxproj`), нацеленных на x64, C++17, с набором инструментов MSVC v143.

### Сборка в Windows (MinGW/MSYS2)

```bash
make
```

Makefile автоматически определяет платформу и использует подходящую реализацию терминала:
- **Linux/macOS/BSD**: ncurses (`terminal_ncurses.cpp`)
- **Windows**: conio + Win32 Console API (`terminal_win32.cpp`)

## Использование

```bash
# Launch SlyMail with file browser
./slymail

# Open a specific QWK packet
./slymail MYBBS.qwk

# Run the standalone configuration utility
./config_slymail
```

### Программа конфигурации

Утилита `config` предоставляет автономный текстовый интерфейс для настройки параметров SlyMail без открытия основного приложения. Она предлагает четыре категории конфигурации:

- **Настройки редактора** — Все те же настройки, доступные через Ctrl-U в редакторе (стиль редактора, подписи, проверка орфографии, параметры цитирования и т.д.)
- **Настройки ридера** — Переключение кладж-строк, строк tear, полосы прокрутки, удаления ANSI, режима подсветки, обратного порядка и переключателей кодов атрибутов (включить/отключить для каждого BBS)
- **Настройки тем** — Выбор файлов цветовых тем Ice и DCT из каталога `config_files/`
- **Общие настройки** — Задайте своё имя для ответов и каталог вывода пакета REP

Настройки сохраняются автоматически при выходе из каждой категории. Как SlyMail, так и утилита config читают и записывают один и тот же файл настроек.

### Привязки клавиш

#### Браузер файлов
| Клавиша | Действие |
|---------|---------|
| Вверх/Вниз | Навигация по файлам и каталогам |
| Enter | Открыть каталог / Выбрать файл QWK |
| Ctrl-R | Открыть каталог удалённых систем |
| Q / ESC | Выйти |

#### Список конференций
| Клавиша | Действие |
|---------|---------|
| Вверх/Вниз | Навигация по конференциям |
| Enter | Открыть выбранную конференцию |
| V | Просмотр опросов/голосований в пакете |
| O / Ctrl-L | Открыть другой файл QWK |
| S / Ctrl-U | Настройки |
| Q / ESC | Выйти из SlyMail |
| ? / F1 | Справка |

#### Список сообщений
| Клавиша | Действие |
|---------|---------|
| Вверх/Вниз | Навигация по сообщениям |
| Enter / R | Прочитать выбранное сообщение |
| N | Написать новое сообщение |
| G | Перейти к номеру сообщения |
| Ctrl-L | Открыть другой файл QWK |
| S / Ctrl-U | Настройки |
| C / ESC | Назад к списку конференций |
| Q | Выйти |
| ? / F1 | Справка |

#### Ридер сообщений
| Клавиша | Действие |
|---------|---------|
| Вверх/Вниз | Прокрутка сообщения |
| Влево/Вправо | Предыдущее / Следующее сообщение |
| F / L | Первое / Последнее сообщение |
| R | Ответить на сообщение |
| V | Голосовать (голос за/против или бюллетень опроса) |
| D / Ctrl-D | Загрузить вложения файлов |
| H | Показать информацию о заголовке сообщения |
| S / Ctrl-U | Настройки |
| C / Q / ESC | Назад к списку сообщений |
| ? / F1 | Справка |

#### Редактор сообщений
| Клавиша | Действие |
|---------|---------|
| ESC | Меню редактора (Сохранить, Отменить и т.д.) |
| Ctrl-U | Диалог настроек пользователя |
| Ctrl-Q | Открыть/закрыть окно цитат |
| Ctrl-K | Выбор цвета (вставить код цвета ANSI в позиции курсора) |
| Ctrl-G | Вставить графический символ (CP437) по коду |
| Ctrl-W | Поиск слова/текста |
| Ctrl-S | Изменить тему |
| Ctrl-D | Удалить текущую строку |
| Ctrl-Z | Сохранить сообщение |
| Ctrl-A | Отменить сообщение |
| F1 | Экран справки |
| Insert | Переключить режим Вставка/Замена |

#### Окно цитат
| Клавиша | Действие |
|---------|---------|
| Вверх/Вниз | Навигация по строкам цитат |
| Enter | Вставить выбранную строку цитаты |
| Ctrl-Q / ESC | Закрыть окно цитат |

## Архитектура

SlyMail использует уровень абстракции платформы для своего текстового пользовательского интерфейса:

```
ITerminal (abstract base class)
    ├── NCursesTerminal  (Linux/macOS/BSD - ncurses)
    └── Win32Terminal    (Windows - conio + Win32 Console API)
```

Символы рамок CP437 и специальные символы определены в `cp437defs.h` и отображаются через метод `putCP437()`, который отображает коды CP437 на нативные эквиваленты платформы (символы ACS в ncurses, прямые байты CP437 в Windows).

### Исходные файлы

| Файл | Описание |
|------|----------|
| `terminal.h` | Абстрактный интерфейс `ITerminal`, константы клавиш/цветов, фабрика |
| `terminal_ncurses.cpp` | Реализация ncurses с отображением CP437-в-ACS |
| `terminal_win32.cpp` | Windows Console API + реализация conio |
| `cp437defs.h` | Определения символов IBM Code Page 437 |
| `colors.h` | Определения цветовых схем (Ice, DCT, ридер, список) |
| `theme.h` | Парсер файла конфигурации темы (коды атрибутов в стиле Synchronet) |
| `ui_common.h` | Общие вспомогательные функции интерфейса (диалоги, ввод текста, полоса прокрутки и т.д.) |
| `qwk.h` / `qwk.cpp` | Парсер и создатель пакетов QWK/REP (QWKE, вложения, голосование) |
| `bbs_colors.h` / `bbs_colors.cpp` | Парсер кодов цвета/атрибутов BBS (ANSI, Synchronet, WWIV, PCBoard, Celerity, Renegade) |
| `utf8_util.h` / `utf8_util.cpp` | Утилиты UTF-8 (проверка, ширина отображения, конвертация CP437-в-UTF-8) |
| `voting.h` / `voting.cpp` | Парсер VOTING.DAT, подсчёт голосов, интерфейс отображения опросов |
| `remote_systems.h` / `remote_systems.cpp` | Каталог удалённых систем, просмотр FTP/SFTP, сохранение JSON, утилиты домашнего каталога |
| `settings.h` | Сохранение настроек пользователя |
| `settings_dialog.h` | Диалоги настроек (редактор, ридер, переключатели кодов атрибутов) |
| `file_browser.h` | Браузер и выбор файлов QWK |
| `msg_list.h` | Виды списка конференций и сообщений |
| `msg_reader.h` | Ридер сообщений (в стиле DDMsgReader) с голосованием и интерфейсом вложений |
| `msg_editor.h` | Редактор сообщений (в стиле SlyEdit Ice/DCT) с выбором цвета |
| `main.cpp` | Точка входа приложения SlyMail и основной цикл |
| `config.cpp` | Автономная утилита конфигурации |

## Конфигурация

### Файл настроек

Настройки хранятся в INI-файле с именем `slymail.ini` в том же каталоге, что и исполняемый файл SlyMail. Этот файл является общим для SlyMail и утилиты `config`. Файл хорошо прокомментирован с описаниями каждого параметра.

Пример `slymail.ini`:
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

### Файлы тем

Цветовые темы — это файлы `.ini` в каталоге `config_files/`:

**Темы Ice** (`EditorIceColors_*.ini`):
- BlueIce (по умолчанию), EmeraldCity, FieryInferno, Fire-N-Ice, GeneralClean, GenericBlue, PurpleHaze, ShadesOfGrey

**Темы DCT** (`EditorDCTColors_*.ini`):
- Default (по умолчанию), Default-Modified, Midnight

Значения цветов тем используют компактный формат, основанный на кодах атрибутов Synchronet:
- `n` = нормальный (сброс)
- Цвета переднего плана: `k`=чёрный, `r`=красный, `g`=зелёный, `y`=жёлтый, `b`=синий, `m`=пурпурный, `c`=голубой, `w`=белый
- `h` = высокая/яркая интенсивность
- Цифра фона: `0`=чёрный, `1`=красный, `2`=зелёный, `3`=коричневый, `4`=синий, `5`=пурпурный, `6`=голубой, `7`=светло-серый

Пример: `nbh` = яркий синий, `n4wh` = яркий белый на синем фоне

### Подписи (Taglines)

Подписи — это короткие цитаты или высказывания, добавляемые в конец сообщений при сохранении. Функция подписей может быть включена через Ctrl-U в редакторе или утилиту `config`.

Подписи хранятся в `tagline_files/taglines.txt`, по одной на строку. Строки, начинающиеся с `#` или `;`, считаются комментариями и игнорируются. При сохранении сообщения с включёнными подписями пользователю предлагается выбрать конкретную подпись или случайную. Выбранная подпись добавляется к сообщению с префиксом `...` (например, `...To err is human, to really foul things up requires a computer.`).

### Проверка орфографии

SlyMail включает встроенную проверку орфографии, использующую словари в виде обычных текстовых файлов. Проверку орфографии можно настроить для предложения при сохранении через Ctrl-U в редакторе или утилиту `config`.

**Файлы словарей** — это обычные текстовые файлы (по одному слову на строку), хранящиеся в `dictionary_files/`. Для расширенного покрытия слов одновременно можно выбрать несколько словарей. SlyMail поставляется с:
- `dictionary_en.txt` — английский (общий, ~130 тыс. слов)
- `dictionary_en-US-supplemental.txt` — дополнение для американского английского
- `dictionary_en-GB-supplemental.txt` — дополнение для британского английского
- `dictionary_en-AU-supplemental.txt` — дополнение для австралийского английского
- `dictionary_en-CA-supplemental.txt` — дополнение для канадского английского

При запуске проверки орфографии программа сканирует сообщение на наличие ошибок и предоставляет интерактивный диалог для каждого слова, предлагая варианты **З**аменить слово, **П**ропустить его, **Д**обавить его (в будущем) или **В**ыйти из проверки.

## Благодарности

- Интерфейс вдохновлён [DDMsgReader](https://github.com/SynchronetBBS/sbbs) и [SlyEdit](https://github.com/SynchronetBBS/sbbs) от [Nightfox (Eric Oulashin)](https://github.com/EricOulashin)
- Совместимость с форматом QWK на основе исходного кода [Synchronet BBS](https://www.synchro.net/)
- Определения символов CP437 из Synchronet

## Лицензия

Этот проект является программным обеспечением с открытым исходным кодом.
