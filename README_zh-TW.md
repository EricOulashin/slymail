> **注意：** 本文件為機器翻譯。原文請參閱 [README.md](README.md)。

# SlyMail

這是一個跨平台的文字介面離線郵件閱讀程式，支援 [QWK](https://en.wikipedia.org/wiki/QWK_(file_format)) 封包格式。QWK 封包格式曾被（現在也被）廣泛用於 [電子布告欄系統 (BBS)](https://en.wikipedia.org/wiki/Bulletin_board_system) 上的郵件交換。

SlyMail 提供功能完整的介面，用於閱讀和回覆來自 BBS（電子布告欄系統）QWK 郵件封包中的訊息。其使用者介面的靈感來源於訊息閱讀方面的 [Digital Distortion Message Reader (DDMsgReader)](https://github.com/SynchronetBBS/sbbs/tree/master/xtrn/DDMsgReader) 和訊息編輯方面的 [SlyEdit](https://github.com/SynchronetBBS/sbbs/tree/master/exec)，兩者均最初為 [Synchronet BBS](https://www.synchro.net/) 所建立。

SlyMail 在 Claude AI 的協助下建立。

## 功能

### QWK 封包支援
- 開啟並讀取標準 QWK 郵件封包（.qwk 檔案）
- 解析 CONTROL.DAT、MESSAGES.DAT 及 NDX 索引檔案
- 透過 HEADERS.DAT 提供完整的 QWKE（擴充 QWK）支援——基於位移的比對，用於精確的擴充 To/From/Subject 欄位、UTF-8 旗標和 RFC822 Message-ID
- QWKE 訊息內容 kludge 解析（訊息開頭的 `To:`、`From:`、`Subject:`）
- 處理 Synchronet 風格的討論區編號
- 建立用於上傳回 BBS 的 REP 回覆封包（.rep 檔案），包含擴充欄位的 HEADERS.DAT 和待處理投票的 VOTING.DAT
- 支援 NDX 檔案中的 Microsoft Binary Format (MBF) 浮點編碼
- 在工作階段之間記住最後開啟的 QWK 檔案和目錄

### 訊息閱讀（DDMsgReader 風格）
- 帶訊息計數的討論區清單
- 帶光棒導覽的可捲動訊息清單
- 帶標題顯示（From、To、Subject、Date）的完整訊息閱讀器
- 引用行高亮顯示（支援多層引用）
- Kludge 行顯示（選用）
- 捲動條指示器
- 鍵盤導覽：第一則/最後一則/下一則/上一則訊息，向上/向下翻頁
- 在所有視圖中透過 `?` 或 `F1` 存取說明畫面

### BBS 色彩和屬性代碼支援
SlyMail 解譯來自多個 BBS 軟體套件的色彩/屬性代碼，在訊息閱讀器和訊息編輯器中將其渲染為彩色文字。支援的格式：
- **ANSI 跳脫碼** — 一律啟用；用於前景色、背景色、粗體的標準 SGR 序列（ESC[...m）
- **Synchronet Ctrl-A 代碼** — `\x01` + 屬性字元（例如，`\x01c` 表示青色，`\x01h` 表示亮色）
- **WWIV 愛心代碼** — `\x03` + 數字 0–9
- **PCBoard/Wildcat @X 代碼** — `@X##`，其中兩個十六進位數字編碼背景色和前景色
- **Celerity 管道代碼** — `|` + 字母（例如，`|c` 表示青色，`|W` 表示亮白色）
- **Renegade 管道代碼** — `|` + 兩位數字 00–31

每種 BBS 代碼類型都可透過閱讀器設定或 `config` 工具中的 **屬性代碼切換** 子對話框個別啟用或停用。這些切換同時影響閱讀器和編輯器。單獨的 **去除 ANSI 代碼** 選項在啟用時會從訊息中移除所有 ANSI 序列。

### 檔案附件
- 偵測訊息內容中透過 `@ATTACH:` kludge 行所參照的檔案附件
- 當存在附件時，在訊息標題中顯示 **[ATT]** 指示器
- 在閱讀器中按 **D** 或 **Ctrl-D** 下載附件——顯示帶大小的檔案清單並提示選擇目的地目錄

### 投票和調查（Synchronet QWKE）
SlyMail 支援用於調查和訊息投票的 Synchronet VOTING.DAT 擴充功能：
- **調查**：被識別為調查的訊息顯示其回答選項以及投票數和百分比條。按 **V** 開啟投票對話框，您可以切換選擇並投票。
- **贊成/反對投票**：對於一般（非調查）訊息，按 **V** 進行贊成或反對投票。目前的投票統計和分數顯示在訊息標題中。
- **投票統計**：訊息標題顯示贊成/反對票數和淨分數，如果您已經投票，則顯示一個指示器。
- **投票佇列**：投票與訊息回覆一起排入佇列，並寫入 REP 封包中的 VOTING.DAT 以上傳到 BBS。
- **調查瀏覽器**：從討論區清單按 **V** 瀏覽封包中的所有調查。

### UTF-8 支援
- 偵測訊息中的 UTF-8 內容（透過 HEADERS.DAT 的 `Utf8` 旗標和 UTF-8 位元組序列的自動偵測）
- 在相容的終端機上正確顯示 UTF-8 字元
- 在 UTF-8 訊息的訊息標題中顯示 **[UTF8]** 指示器
- 使用適當的編碼儲存新訊息
- 舊版 BBS 內容的 CP437 到 UTF-8 轉換
- 在 Linux/macOS/BSD 上設定地區設定（`setlocale(LC_ALL, "")`）並在 Windows 上設定 UTF-8 字碼頁，以實現正確的終端機渲染

### 訊息編輯器（受 SlyEdit 啟發）
- **兩種視覺模式**：Ice 和 DCT，各有不同的色彩方案和版面
- **隨機模式**：在每次編輯工作階段中隨機選擇 Ice 或 DCT
- **交替邊框色彩**：邊框字元在兩種主題色彩之間隨機交替，與 SlyEdit 的視覺風格保持一致
- **主題支援**：從 .ini 檔案載入的可設定色彩主題
- 帶自動換行的全螢幕文字編輯器
- 用於選擇和插入引用文字的引用視窗（Ctrl-Q 開啟/關閉）
- 回覆和新訊息撰寫
- ESC 選單，用於儲存、中止、插入/覆寫切換等
- **Ctrl-K 色彩選擇器**：開啟一個對話框以選擇前景色和背景色，並在游標位置插入 ANSI 跳脫碼。支援 16 種前景色（8 種一般 + 8 種亮色）和 8 種背景色，帶即時預覽。按 **N** 插入重設代碼。
- **色彩感知渲染**：編輯區域內嵌渲染 ANSI 和 BBS 屬性代碼，因此在您輸入時顯示彩色文字
- **Ctrl-U 使用者設定對話框**，用於即時設定編輯器偏好
- **風格專屬的是/否提示**：Ice 模式使用螢幕底部的內嵌提示；DCT 模式使用帶主題色彩的置中對話框

### 編輯器設定（透過 Ctrl-U）
- **選擇 UI 模式**：在 Ice、DCT 和 Random 風格之間切換的對話框（立即生效）
- **選擇主題檔案**：從可用的 Ice 或 DCT 色彩主題中選擇
- **標籤行**：啟用時，儲存時提示選擇標籤行（來自 `tagline_files/taglines.txt`）
- **拼字檢查字典**：選擇要使用的字典
- **儲存時提示拼字檢查**：啟用時，在儲存前提供拼字檢查
- **將引用行換行到終端機寬度**：對引用行進行自動換行
- **使用作者縮寫引用**：在引用行前加上作者的縮寫（例如，`MP> `）
- **縮排包含縮寫的引用行**：在縮寫前新增前置空格（例如，` MP> `）
- **修剪引用行中的空格**：去除引用文字中的前置空白

### 色彩主題
- 主題檔案是 `config_files/` 目錄中的設定檔（`.ini`）
- Ice 主題：`EditorIceColors_*.ini`（BlueIce、EmeraldCity、FieryInferno 等）
- DCT 主題：`EditorDCTColors_*.ini`（Default、Default-Modified、Midnight）
- 主題色彩使用簡單格式：前景色字母（`r`/`g`/`b`/`c`/`y`/`m`/`w`/`k`），可選的 `h` 表示亮色，可選的背景數字（`0`-`7`）
- 主題控制所有 UI 元素色彩：邊框、標籤、值、引用視窗、說明列、是/否對話框

### 拼字檢查器
- 使用純文字字典檔案的內建拼字檢查器
- 隨附英語字典（en、en-US、en-GB、en-AU、en-CA 補充）
- 互動式更正對話框：取代、略過或結束
- 字典檔案儲存在 `dictionary_files/` 目錄中

### 標籤行
- 標籤行檔案儲存在 `tagline_files/` 目錄中
- 預設標籤行檔案為 `tagline_files/taglines.txt`，每行一個標籤行
- 以 `#` 或 `;` 開頭的行被視為註解並被忽略
- 儲存訊息時選擇特定的標籤行或隨機選擇一個
- 標籤行以 `...` 前綴附加到訊息

### REP 封包建立
- 目前僅支援 ZIP（將來希望新增對更多壓縮類型的支援）
- 當您撰寫回覆或新訊息時，它們會被排入佇列為待處理
- 投票（調查投票、贊成/反對票）也與回覆一起排入佇列
- 結束時（或開啟新 QWK 檔案時），SlyMail 提示儲存所有待處理項目
- 建立用於上傳到 BBS 的標準 `.rep` 檔案（ZIP 封存），包含：
  - `<BBSID>.MSG` — 標準 QWK 格式的回覆訊息
  - `HEADERS.DAT` — 超過 25 個字元的欄位的 QWKE 擴充標題
  - `VOTING.DAT` — Synchronet 相容 INI 格式的待處理投票
- REP 檔案儲存為設定的回覆目錄（或 QWK 檔案所在目錄）中的 `<BBS-ID>.rep`

### 遠端系統（Ctrl-R）
SlyMail 可以透過 FTP 或 SFTP (SSH) 直接從遠端系統下載 QWK 封包：
- 從檔案瀏覽器按 **Ctrl-R** 開啟遠端系統目錄
- **新增/編輯/刪除** 遠端系統項目：名稱、主機、連接埠、連線類型（FTP 或 SSH）、使用者名稱、密碼、被動 FTP 切換和初始遠端路徑
- 使用類似本機檔案瀏覽器的檔案/目錄瀏覽器 **瀏覽遠端目錄** — 導覽進入目錄，用 `..` 返回上層，用 `/` 跳轉到根目錄
- 將 QWK 檔案直接 **下載** 到 SlyMail 資料目錄的 `QWK` 子目錄
- 遠端系統項目持久儲存到 SlyMail 資料目錄中的 `remote_systems.json`
- 追蹤每個系統的最後連線日期/時間
- 使用系統的 `curl` 命令進行 FTP 和 SFTP 傳輸（無編譯時函式庫相依性）

### 應用程式設定
- 將持久設定儲存到 SlyMail 資料目錄（Linux/macOS/BSD 上為 `~/.slymail`，Windows 上為使用者主目錄）中的 `slymail.ini`
- SlyMail 資料目錄及其 `QWK` 子目錄在首次執行時自動建立
- 預設 QWK 檔案瀏覽和 REP 封包儲存目錄為 `~/.slymail/QWK`
- 記住最後瀏覽的目錄和 QWK 檔案名稱
- 在討論區或訊息清單視圖中使用 Ctrl-L 快速鍵載入不同的 QWK 檔案
- 可設定的引用前綴、引用行寬度、使用者名稱
- 閱讀器選項：顯示/隱藏 kludge 行、tear/origin 行、捲動條、去除 ANSI 代碼
- 每個 BBS 的屬性代碼切換（Synchronet、WWIV、Celerity、Renegade、PCBoard/Wildcat）— 同時影響閱讀器和編輯器
- REP 封包輸出目錄

## 截圖

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

## QWK 封包的 Synchronet 設定
在 Synchronet BBS 上，在 QWK 封包設定中，SlyMail 與 Ctrl-A 色彩代碼、VOTING.DAT、檔案附件以及 QWKE 封包格式相容（或應該相容）。SlyMail 也應與 UTF-8 相容。例如：
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

## 建置

### 需求

**Linux / macOS / BSD：**
- C++17 相容編譯器（GCC 8+、Clang 7+）
- ncurses 開發函式庫（Debian/Ubuntu 上為 `libncurses-dev`，Fedora/RHEL 上為 `ncurses-devel`）
- `unzip` 命令（用於解壓縮 QWK 封包）
- `zip` 命令（用於建立 REP 封包）
- `curl` 命令（用於遠端系統 FTP/SFTP 傳輸——選用，僅遠端系統功能需要）

**Windows（Visual Studio 2022）：**
- 帶有「使用 C++ 的桌面開發」工作負載的 Visual Studio 2022
- Windows SDK 10.0（包含在 VS 中）
- 無需額外函式庫——使用內建 Win32 主控台 API 作為終端機 UI，使用 `tar.exe` 或 PowerShell 處理 QWK/REP 封包 ZIP（見下方說明）

**Windows（MinGW/MSYS2）：**
- 帶有 GCC 的 MinGW-w64 或 MSYS2（C++17 支援）
- Windows 主控台 API（內建）

> **說明——Windows 上的 QWK/REP ZIP 處理：** SlyMail 在執行時偵測哪個工具可用並使用最佳選項：
>
> - **`tar.exe`（首選）：** 隨 Windows 10 版本 1803（2018 年 4 月更新）及更高版本以及所有版本的 Windows 11 一起提供。`tar` 根據內容而不是副檔名讀取 ZIP 檔案，因此 `.qwk` 封包可以直接解壓縮，`.rep` 封包透過暫存的 `.zip` 檔案建立，然後重新命名。無需額外設定。
> - **PowerShell（備用）：** 如果在 PATH 中找不到 `tar.exe`，SlyMail 會退回到 PowerShell。對於解壓縮，它使用 .NET 的 `ZipFile` 類別（`System.IO.Compression`）而不是 `Expand-Archive`，因為 `Expand-Archive` 即使檔案是有效的 ZIP 封存，也會拒絕非 `.zip` 副檔名。對於 REP 封包建立，它使用 `Compress-Archive`，同樣寫入暫存 `.zip` 檔案，然後重新命名為 `.rep`。

### 在 Linux/macOS/BSD 上建置

```bash
make
```

這將建置兩個程式：
- `slymail` - 主 QWK 閱讀器應用程式
- `config` - 獨立設定工具

### 使用除錯符號建置

```bash
make debug
```

### 安裝（選用）

```bash
sudo make install    # Installs slymail and config to /usr/local/bin/
sudo make uninstall  # Remove
```

### 在 Windows 上使用 Visual Studio 2022 建置

在 Visual Studio 2022 中開啟方案檔案：

```
vs\SlyMail.sln
```

或使用 MSBuild 從命令列建置：

```powershell
# Release build (output in vs\x64\Release\)
msbuild vs\SlyMail.sln /p:Configuration=Release /p:Platform=x64

# Debug build (output in vs\x64\Debug\)
msbuild vs\SlyMail.sln /p:Configuration=Debug /p:Platform=x64
```

這將建置兩個執行檔：
- `x64\Release\slymail.exe` — 主 QWK 閱讀器
- `x64\Release\config.exe` — 獨立設定工具

方案包含兩個專案（`SlyMail.vcxproj` 和 `Config.vcxproj`），以 x64、C++17、MSVC v143 工具集為目標。

### 在 Windows 上建置（MinGW/MSYS2）

```bash
make
```

Makefile 自動偵測平台並使用適當的終端機實作：
- **Linux/macOS/BSD**：ncurses（`terminal_ncurses.cpp`）
- **Windows**：conio + Win32 主控台 API（`terminal_win32.cpp`）

## 使用方式

```bash
# Launch SlyMail with file browser
./slymail

# Open a specific QWK packet
./slymail MYBBS.qwk

# Run the standalone configuration utility
./config
```

### 設定程式

`config` 工具提供了一個獨立的文字介面，用於在不開啟主應用程式的情況下設定 SlyMail。它提供四個設定類別：

- **編輯器設定** - 編輯器中透過 Ctrl-U 可用的所有相同設定（編輯器風格、標籤行、拼字檢查、引用選項等）
- **閱讀器設定** - 切換 kludge 行、tear 行、捲動條、ANSI 去除、光棒模式、逆序以及屬性代碼切換（每個 BBS 啟用/停用）
- **主題設定** - 從 `config_files/` 目錄選擇 Ice 和 DCT 色彩主題檔案
- **一般設定** - 設定回覆的名稱和 REP 封包輸出目錄

結束每個類別時自動儲存設定。SlyMail 和 config 工具讀寫同一個設定檔。

### 按鍵綁定

#### 檔案瀏覽器
| 鍵 | 操作 |
|-----|--------|
| Up/Down | 瀏覽檔案和目錄 |
| Enter | 開啟目錄 / 選擇 QWK 檔案 |
| Ctrl-R | 開啟遠端系統目錄 |
| Q / ESC | 結束 |

#### 討論區清單
| 鍵 | 操作 |
|-----|--------|
| Up/Down | 瀏覽討論區 |
| Enter | 開啟選定的討論區 |
| V | 檢視封包中的調查/投票 |
| O / Ctrl-L | 開啟不同的 QWK 檔案 |
| S / Ctrl-U | 設定 |
| Q / ESC | 結束 SlyMail |
| ? / F1 | 說明 |

#### 訊息清單
| 鍵 | 操作 |
|-----|--------|
| Up/Down | 瀏覽訊息 |
| Enter / R | 閱讀選定的訊息 |
| N | 撰寫新訊息 |
| G | 前往訊息編號 |
| Ctrl-L | 開啟不同的 QWK 檔案 |
| S / Ctrl-U | 設定 |
| C / ESC | 返回討論區清單 |
| Q | 結束 |
| ? / F1 | 說明 |

#### 訊息閱讀器
| 鍵 | 操作 |
|-----|--------|
| Up/Down | 捲動訊息 |
| Left/Right | 上一則 / 下一則訊息 |
| F / L | 第一則 / 最後一則訊息 |
| R | 回覆訊息 |
| V | 投票（贊成/反對票或調查投票） |
| D / Ctrl-D | 下載檔案附件 |
| H | 顯示訊息標題資訊 |
| S / Ctrl-U | 設定 |
| C / Q / ESC | 返回訊息清單 |
| ? / F1 | 說明 |

#### 訊息編輯器
| 鍵 | 操作 |
|-----|--------|
| ESC | 編輯器選單（儲存、中止等） |
| Ctrl-U | 使用者設定對話框 |
| Ctrl-Q | 開啟/關閉引用視窗 |
| Ctrl-K | 色彩選擇器（在游標位置插入 ANSI 色彩代碼） |
| Ctrl-G | 按代碼插入 CP437 圖形字元 |
| Ctrl-W | 文字搜尋 |
| Ctrl-S | 變更主題 |
| Ctrl-D | 刪除目前行 |
| Ctrl-Z | 儲存訊息 |
| Ctrl-A | 中止訊息 |
| F1 | 說明畫面 |
| Insert | 切換插入/覆寫模式 |

#### 引用視窗
| 鍵 | 操作 |
|-----|--------|
| Up/Down | 瀏覽引用行 |
| Enter | 插入選定的引用行 |
| Ctrl-Q / ESC | 關閉引用視窗 |

## 架構

SlyMail 為其文字使用者介面使用平台抽象層：

```
ITerminal (abstract base class)
    ├── NCursesTerminal  (Linux/macOS/BSD - ncurses)
    └── Win32Terminal    (Windows - conio + Win32 Console API)
```

CP437 方框繪圖和特殊字元在 `cp437defs.h` 中定義，並透過 `putCP437()` 方法渲染，該方法將 CP437 代碼對應到平台原生等效項（ncurses 上的 ACS 字元，Windows 上的直接 CP437 位元組）。

### 原始碼檔案

| 檔案 | 說明 |
|------|-------------|
| `terminal.h` | 抽象 `ITerminal` 介面、鍵/色彩常數、工廠 |
| `terminal_ncurses.cpp` | 帶 CP437 到 ACS 對應的 ncurses 實作 |
| `terminal_win32.cpp` | Windows 主控台 API + conio 實作 |
| `cp437defs.h` | IBM 字碼頁 437 字元定義 |
| `colors.h` | 色彩方案定義（Ice、DCT、閱讀器、清單） |
| `theme.h` | 主題設定檔解析器（Synchronet 風格屬性代碼） |
| `ui_common.h` | 共用 UI 助手（對話框、文字輸入、捲動條等） |
| `qwk.h` / `qwk.cpp` | QWK/REP 封包解析器和建立器（QWKE、附件、投票） |
| `bbs_colors.h` / `bbs_colors.cpp` | BBS 色彩/屬性代碼解析器（ANSI、Synchronet、WWIV、PCBoard、Celerity、Renegade） |
| `utf8_util.h` / `utf8_util.cpp` | UTF-8 工具（驗證、顯示寬度、CP437 到 UTF-8 轉換） |
| `voting.h` / `voting.cpp` | VOTING.DAT 解析器、投票統計、調查顯示 UI |
| `remote_systems.h` / `remote_systems.cpp` | 遠端系統目錄、FTP/SFTP 瀏覽、JSON 持久化、主目錄工具 |
| `settings.h` | 使用者設定持久化 |
| `settings_dialog.h` | 設定對話框（編輯器、閱讀器、屬性代碼切換） |
| `file_browser.h` | QWK 檔案瀏覽器和選擇器 |
| `msg_list.h` | 討論區和訊息清單視圖 |
| `msg_reader.h` | 帶投票和附件 UI 的訊息閱讀器（DDMsgReader 風格） |
| `msg_editor.h` | 帶色彩選擇器的訊息編輯器（SlyEdit Ice/DCT 風格） |
| `main.cpp` | SlyMail 應用程式進入點和主迴圈 |
| `config.cpp` | 獨立設定工具 |

## 設定

### 設定檔

設定儲存在與 SlyMail 執行檔同目錄下名為 `slymail.ini` 的 INI 檔案中。此檔案由 SlyMail 和 `config` 工具共用。檔案中有各設定說明的註解。

`slymail.ini` 範例：
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

### 主題檔案

色彩主題是 `config_files/` 目錄中的 `.ini` 檔案：

**Ice 主題**（`EditorIceColors_*.ini`）：
- BlueIce（預設）、EmeraldCity、FieryInferno、Fire-N-Ice、GeneralClean、GenericBlue、PurpleHaze、ShadesOfGrey

**DCT 主題**（`EditorDCTColors_*.ini`）：
- Default（預設）、Default-Modified、Midnight

主題色彩值使用源自 Synchronet 屬性代碼的緊湊格式：
- `n` = 一般（重設）
- 前景色：`k`=黑色、`r`=紅色、`g`=綠色、`y`=黃色、`b`=藍色、`m`=品紅、`c`=青色、`w`=白色
- `h` = 高亮/亮色
- 背景數字：`0`=黑色、`1`=紅色、`2`=綠色、`3`=棕色、`4`=藍色、`5`=品紅、`6`=青色、`7`=淺灰

範例：`nbh` = 一般藍色亮色，`n4wh` = 藍色背景上的亮白色

### 標籤行

標籤行是儲存訊息時附加到訊息末尾的簡短引用或格言。標籤行功能可以透過編輯器中的 Ctrl-U 或 `config` 工具啟用。

標籤行儲存在 `tagline_files/taglines.txt` 中，每行一個。以 `#` 或 `;` 開頭的行被視為註解並被忽略。啟用標籤行儲存訊息時，系統會提示您選擇特定的標籤行或隨機選擇一個。所選標籤行以 `...` 前綴附加到訊息（例如，`...To err is human, to really foul things up requires a computer.`）。

### 拼字檢查器

SlyMail 包含一個使用純文字字典檔案的內建拼字檢查器。拼字檢查器可以透過編輯器中的 Ctrl-U 或 `config` 工具設定為在儲存時提示。

**字典檔案**是儲存在 `dictionary_files/` 中的純文字檔案（每行一個單字）。可以同時選擇多個字典以合併單字覆蓋範圍。SlyMail 隨附：
- `dictionary_en.txt` - 英語（通用，約 13 萬個單字）
- `dictionary_en-US-supplemental.txt` - 美國英語補充
- `dictionary_en-GB-supplemental.txt` - 英國英語補充
- `dictionary_en-AU-supplemental.txt` - 澳洲英語補充
- `dictionary_en-CA-supplemental.txt` - 加拿大英語補充

觸發拼字檢查時，檢查器會掃描訊息中的拼字錯誤單字，並為每個單字顯示一個互動式對話框，提供選項：**R** 取代單字、**S** 略過、**A** 新增（將來）或 **Q** 結束檢查。

## 致謝

- UI 受 [Nightfox (Eric Oulashin)](https://github.com/EricOulashin) 的 [DDMsgReader](https://github.com/SynchronetBBS/sbbs) 和 [SlyEdit](https://github.com/SynchronetBBS/sbbs) 啟發
- QWK 格式相容性參考了 [Synchronet BBS](https://www.synchro.net/) 原始碼
- CP437 字元定義來自 Synchronet

## 授權條款

此專案為開放原始碼軟體。
