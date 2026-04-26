> **注意:** このファイルは機械翻訳です。原文は [README.md](README.md) をご覧ください。

# SlyMail

これは [QWK](https://en.wikipedia.org/wiki/QWK_(file_format)) パケット形式に対応した、クロスプラットフォームのテキストベースオフラインメールリーダーです。QWK パケット形式は [掲示板システム (BBS)](https://en.wikipedia.org/wiki/Bulletin_board_system) でのメール交換に広く使われてきました (現在も使われています)。

SlyMail は、BBS (掲示板システム) の QWK メールパケットに含まれるメッセージの閲覧と返信のための豊富な機能を持つインターフェースを提供します。そのユーザーインターフェースは、メッセージ閲覧機能については [Digital Distortion Message Reader (DDMsgReader)](https://github.com/SynchronetBBS/sbbs/tree/master/xtrn/DDMsgReader)、メッセージ編集機能については [SlyEdit](https://github.com/SynchronetBBS/sbbs/tree/master/exec) から着想を得ており、どちらも元々 [Synchronet BBS](https://www.synchro.net/) 向けに作成されたものです。

SlyMail は Claude AI の助けを借りて作成されました。

## 機能

### QWK パケットのサポート
- 標準的な QWK メールパケット (.qwk ファイル) を開いて読み込む
- CONTROL.DAT、MESSAGES.DAT、および NDX インデックスファイルを解析する
- HEADERS.DAT を介した完全な QWKE (拡張 QWK) サポート — 正確な拡張 To/From/Subject フィールド、UTF-8 フラグ、RFC822 Message-ID のためのオフセットベースマッチング
- QWKE ボディのクラッジ解析 (メッセージ冒頭の `To:`、`From:`、`Subject:`)
- Synchronet スタイルのカンファレンス番号付けに対応
- BBS へのアップロード用 REP 返信パケット (.rep ファイル) を作成。拡張フィールド用の HEADERS.DAT と保留中の投票用の VOTING.DAT を含む
- NDX ファイルの Microsoft Binary Format (MBF) 浮動小数点エンコードをサポート
- セッション間で最後に開いた QWK ファイルとディレクトリを記憶する

### メッセージ閲覧 (DDMsgReader スタイル)
- メッセージ数付きのカンファレンス一覧
- ライトバーナビゲーション付きのスクロール可能なメッセージ一覧
- ヘッダー表示 (From、To、Subject、Date) を備えた完全なメッセージリーダー
- 引用行のハイライト表示 (多段引用対応)
- クラッジ行の表示 (オプション)
- スクロールバーインジケーター
- キーボードナビゲーション: 先頭/末尾/次/前のメッセージ、ページアップ/ダウン
- すべてのビューで `?` または `F1` からアクセス可能なヘルプ画面

### BBS カラー & アトリビュートコードのサポート
SlyMail は複数の BBS ソフトウェアパッケージのカラー/アトリビュートコードを解釈し、メッセージリーダーとメッセージエディタの両方で色付きテキストとしてレンダリングします。対応フォーマット:
- **ANSI エスケープコード** — 常に有効。前景色、背景色、太字のための標準 SGR シーケンス (ESC[...m)
- **Synchronet Ctrl-A コード** — `\x01` + アトリビュート文字 (例: シアンは `\x01c`、明るいは `\x01h`)
- **WWIV ハートコード** — `\x03` + 数字 0～9
- **PCBoard/Wildcat @X コード** — `@X##` (2 桁の 16 進数が背景色と前景色をエンコード)
- **Celerity パイプコード** — `|` + 文字 (例: シアンは `|c`、明るい白は `|W`)
- **Renegade パイプコード** — `|` + 2 桁の数字 00～31

各 BBS コードタイプは、リーダー設定または `config` ユーティリティの **アトリビュートコード切り替え** サブダイアログで個別に有効/無効にできます。これらの切り替えはリーダーとエディタの両方に影響します。別の **ANSI コードを除去** オプションを有効にすると、メッセージからすべての ANSI シーケンスが削除されます。

### ファイル添付
- メッセージ本文内の `@ATTACH:` クラッジ行で参照されたファイル添付を検出する
- 添付ファイルが存在する場合、メッセージヘッダーに **[ATT]** インジケーターを表示する
- リーダーで **D** または **Ctrl-D** を押して添付ファイルをダウンロード — サイズ付きのファイル一覧を表示し、保存先ディレクトリを入力するよう促す

### 投票 & アンケート (Synchronet QWKE)
SlyMail はアンケートとメッセージ投票のために Synchronet VOTING.DAT 拡張をサポートします:
- **アンケート**: アンケートとして識別されたメッセージは、投票数とパーセンテージバーとともに回答選択肢を表示します。**V** を押すと投票ダイアログが開き、回答の選択と投票ができます。
- **賛成/反対投票**: 通常の (アンケートでない) メッセージでは、**V** を押して賛成票または反対票を投じます。現在の投票集計とスコアがメッセージヘッダーに表示されます。
- **投票集計**: メッセージヘッダーに賛成票/反対票の数とネットスコアが表示され、すでに投票済みの場合はインジケーターが表示されます。
- **投票キューイング**: 投票はメッセージ返信とともにキューに入れられ、BBS へのアップロード用の REP パケット内の VOTING.DAT に書き込まれます。
- **アンケートブラウザ**: カンファレンス一覧から **V** を押すと、パケット内のすべてのアンケートを閲覧できます。

### UTF-8 サポート
- メッセージ内の UTF-8 コンテンツを検出する (HEADERS.DAT の `Utf8` フラグと UTF-8 バイトシーケンスの自動検出による)
- 対応端末上で UTF-8 文字を正しく表示する
- UTF-8 メッセージのメッセージヘッダーに **[UTF8]** インジケーターを表示する
- 新しいメッセージを適切なエンコーディングで保存する
- レガシー BBS コンテンツのための CP437 から UTF-8 への変換
- 適切な端末レンダリングのために Linux/macOS/BSD でロケールを設定 (`setlocale(LC_ALL, "")`) し、Windows では UTF-8 コードページを設定する

### メッセージエディタ (SlyEdit からインスパイア)
- **2 つのビジュアルモード**: Ice と DCT。それぞれ異なるカラースキームとレイアウト
- **ランダムモード**: 各編集セッションで Ice または DCT をランダムに選択
- **交互ボーダーカラー**: ボーダー文字が 2 つのテーマカラーの間でランダムに交互に切り替わり、SlyEdit のビジュアルスタイルに合わせる
- **テーマサポート**: .ini ファイルから読み込む設定可能なカラーテーマ
- ワードラップ付きフルスクリーンテキストエディタ
- 引用テキストの選択と挿入のための引用ウィンドウ (Ctrl-Q で開閉)
- 返信と新規メッセージの作成
- 保存、中止、挿入/上書き切り替えなどのための ESC メニュー
- **Ctrl-K カラーピッカー**: 前景色と背景色を選択するダイアログを開き、カーソル位置に ANSI エスケープコードを挿入します。16 色の前景色 (8 通常 + 8 明るい) と 8 色の背景色に対応し、ライブプレビューあり。**N** を押してリセットコードを挿入します。
- **カラー対応レンダリング**: 編集エリアは ANSI および BBS アトリビュートコードをインラインでレンダリングするため、入力中に色付きテキストが表示される
- エディタ設定をその場で変更するための **Ctrl-U ユーザー設定ダイアログ**
- **スタイル固有の yes/no プロンプト**: Ice モードは画面下部のインラインプロンプトを使用。DCT モードはテーマカラーの中央ダイアログボックスを使用

### エディタ設定 (Ctrl-U から)
- **UI モードの選択**: Ice、DCT、Random スタイルを切り替えるダイアログ (即時反映)
- **テーマファイルの選択**: 使用可能な Ice または DCT カラーテーマから選択
- **タグライン**: 有効にすると、保存時にタグライン選択を促す (`tagline_files/taglines.txt` から)
- **スペルチェック辞書**: 使用する辞書を選択
- **保存時にスペルチェックを促す**: 有効にすると、保存前にスペルチェックを提案する
- **引用行を端末幅で折り返す**: 引用行をワードラップする
- **著者のイニシャルで引用する**: 引用行の先頭に著者のイニシャルを付ける (例: `MP> `)
- **イニシャルを含む引用行をインデントする**: イニシャルの前にスペースを追加する (例: ` MP> `)
- **引用行からスペースを削除する**: 引用テキストの先頭の空白を除去する

### カラーテーマ
- テーマファイルは `config_files/` ディレクトリ内の設定ファイル (`.ini`)
- Ice テーマ: `EditorIceColors_*.ini` (BlueIce、EmeraldCity、FieryInferno など)
- DCT テーマ: `EditorDCTColors_*.ini` (Default、Default-Modified、Midnight)
- テーマカラーはシンプルな形式を使用: 前景色の文字 (`r`/`g`/`b`/`c`/`y`/`m`/`w`/`k`)、オプションで `h` (明るい)、オプションで背景数字 (`0`-`7`)
- テーマはすべての UI 要素の色を制御: ボーダー、ラベル、値、引用ウィンドウ、ヘルプバー、yes/no ダイアログ

### スペルチェッカー
- プレーンテキスト辞書ファイルを使用した組み込みスペルチェッカー
- 英語辞書同梱 (en、en-US、en-GB、en-AU、en-CA 補足)
- 対話式修正ダイアログ: 置換、スキップ、または終了
- 辞書ファイルは `dictionary_files/` ディレクトリに保存

### タグライン
- タグラインファイルは `tagline_files/` ディレクトリに保存
- デフォルトのタグラインファイルは `tagline_files/taglines.txt`。1 行に 1 つのタグライン
- `#` または `;` で始まる行はコメントとして扱われ、無視される
- メッセージ保存時に特定のタグラインを選択するか、ランダムに 1 つ選ぶ
- タグラインは `...` プレフィックスとともにメッセージに追加される

### REP パケットの作成
- 現在、ZIP のみサポート (将来的にはより多くの圧縮タイプのサポートを追加予定)
- 返信や新規メッセージを書くと、保留中としてキューに入れられる
- 投票 (アンケートの投票、賛成/反対票) も返信とともにキューに入れられる
- 終了時 (または新しい QWK ファイルを開くとき)、SlyMail はすべての保留中アイテムを保存するよう促す
- BBS へのアップロード用の標準 `.rep` ファイル (ZIP アーカイブ) を作成。含まれるもの:
  - `<BBSID>.MSG` — 標準 QWK 形式の返信メッセージ
  - `HEADERS.DAT` — 25 文字を超えるフィールドの QWKE 拡張ヘッダー
  - `VOTING.DAT` — Synchronet 互換 INI 形式の保留中の投票
- REP ファイルは設定された返信ディレクトリ (または QWK ファイルのディレクトリ) に `<BBS-ID>.rep` として保存される

### リモートシステム (Ctrl-R)
SlyMail は FTP または SFTP (SSH) を経由してリモートシステムから QWK パケットを直接ダウンロードできます:
- ファイルブラウザから **Ctrl-R** を押してリモートシステムディレクトリを開く
- リモートシステムエントリの **追加/編集/削除**: 名前、ホスト、ポート、接続タイプ (FTP または SSH)、ユーザー名、パスワード、パッシブ FTP 切り替え、初期リモートパス
- ローカルファイルブラウザと同様のファイル/ディレクトリブラウザで **リモートディレクトリを閲覧** — ディレクトリへの移動、`..` で上へ、`/` でルートへ
- SlyMail データディレクトリの `QWK` サブディレクトリに QWK ファイルを直接 **ダウンロード**
- リモートシステムエントリは SlyMail データディレクトリの `remote_systems.json` に保存される
- 各システムの最終接続日時が追跡される
- FTP および SFTP 転送にはシステムの `curl` コマンドを使用 (コンパイル時のライブラリ依存なし)

### アプリケーション設定
- SlyMail データディレクトリ (Linux/macOS/BSD では `~/.slymail`、Windows ではユーザーのホームディレクトリ) の `slymail.ini` に永続設定を保存
- SlyMail データディレクトリとその `QWK` サブディレクトリは初回起動時に自動作成される
- デフォルトの QWK ファイル参照と REP パケット保存ディレクトリは `~/.slymail/QWK`
- 最後に閲覧したディレクトリと QWK ファイル名を記憶する
- カンファレンスまたはメッセージ一覧ビューから別の QWK ファイルを読み込むための Ctrl-L ホットキー
- 設定可能な引用プレフィックス、引用行の幅、ユーザー名
- リーダーオプション: クラッジ行、ティア/オリジン行、スクロールバー、ANSI コードの除去の表示/非表示
- BBS ごとのアトリビュートコード切り替え (Synchronet、WWIV、Celerity、Renegade、PCBoard/Wildcat) — リーダーとエディタの両方に影響
- REP パケット出力ディレクトリ

## スクリーンショット

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

## QWK パケットの Synchronet 設定
Synchronet BBS では、QWK パケット設定において、SlyMail は Ctrl-A カラーコード、VOTING.DAT、ファイル添付、および QWKE パケット形式と互換性があります (または互換性があるはずです)。SlyMail は UTF-8 とも互換性があるはずです。例えば:
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

## ビルド

### 必要条件

**Linux / macOS / BSD:**
- C++17 対応コンパイラ (GCC 8+、Clang 7+)
- ncurses 開発ライブラリ (Debian/Ubuntu では `libncurses-dev`、Fedora/RHEL では `ncurses-devel`)
- `unzip` コマンド (QWK パケットの展開用)
- `zip` コマンド (REP パケットの作成用)
- `curl` コマンド (リモートシステムの FTP/SFTP 転送用 — オプション、リモートシステム機能にのみ必要)

**Windows (Visual Studio 2022):**
- "C++ によるデスクトップ開発" ワークロードを含む Visual Studio 2022
- Windows SDK 10.0 (VS に含まれる)
- 追加ライブラリ不要 — 端末 UI には組み込みの Win32 コンソール API を使用し、QWK/REP パケットの ZIP 処理には `tar.exe` または PowerShell を使用 (下記の注意を参照)

**Windows (MinGW/MSYS2):**
- GCC 付きの MinGW-w64 または MSYS2 (C++17 サポート)
- Windows コンソール API (組み込み)

> **注意 — Windows での QWK/REP ZIP 処理:** SlyMail は実行時にどのツールが利用可能かを検出し、最良のオプションを使用します:
>
> - **`tar.exe` (推奨):** Windows 10 バージョン 1803 (2018 年 4 月更新) 以降および Windows 11 のすべてのバージョンに同梱されています。`tar` は拡張子ではなくコンテンツで ZIP ファイルを読み取るため、`.qwk` パケットを直接展開でき、`.rep` パケットは一時的な `.zip` ファイルを経由して作成後にリネームされます。追加設定は不要です。
> - **PowerShell (フォールバック):** `tar.exe` が PATH に見つからない場合、SlyMail は PowerShell にフォールバックします。展開には `Expand-Archive` ではなく .NET の `ZipFile` クラス (`System.IO.Compression`) を使用します。`Expand-Archive` は有効な ZIP アーカイブでも `.zip` 以外の拡張子を拒否するためです。REP パケット作成には `Compress-Archive` を使用し、一時的な `.zip` ファイルに書き込んでから `.rep` にリネームします。

### Linux/macOS/BSD でのビルド

```bash
make
```

これにより 2 つのプログラムがビルドされます:
- `slymail` - メインの QWK リーダーアプリケーション
- `config` - スタンドアロン設定ユーティリティ

### デバッグシンボル付きビルド

```bash
make debug
```

### インストール (オプション)

```bash
sudo make install    # Installs slymail and config to /usr/local/bin/
sudo make uninstall  # Remove
```

### Visual Studio 2022 で Windows 向けにビルド

Visual Studio 2022 でソリューションファイルを開きます:

```
vs\SlyMail.sln
```

または MSBuild を使用してコマンドラインからビルドします:

```powershell
# Release build (output in vs\x64\Release\)
msbuild vs\SlyMail.sln /p:Configuration=Release /p:Platform=x64

# Debug build (output in vs\x64\Debug\)
msbuild vs\SlyMail.sln /p:Configuration=Debug /p:Platform=x64
```

これにより 2 つの実行ファイルがビルドされます:
- `x64\Release\slymail.exe` — メインの QWK リーダー
- `x64\Release\config.exe` — スタンドアロン設定ユーティリティ

ソリューションには 2 つのプロジェクト (`SlyMail.vcxproj` と `Config.vcxproj`) が含まれており、x64、C++17、MSVC v143 ツールセットを対象としています。

### Windows (MinGW/MSYS2) でのビルド

```bash
make
```

Makefile はプラットフォームを自動検出し、適切な端末実装を使用します:
- **Linux/macOS/BSD**: ncurses (`terminal_ncurses.cpp`)
- **Windows**: conio + Win32 コンソール API (`terminal_win32.cpp`)

## 使い方

```bash
# Launch SlyMail with file browser
./slymail

# Open a specific QWK packet
./slymail MYBBS.qwk

# Run the standalone configuration utility
./config
```

### 設定プログラム

`config` ユーティリティは、メインアプリケーションを開かずに SlyMail の設定を行うためのスタンドアロンのテキストベースインターフェースを提供します。4 つの設定カテゴリがあります:

- **エディタ設定** - エディタスタイル、タグライン、スペルチェック、引用オプションなど、エディタで Ctrl-U から利用可能なすべての設定
- **リーダー設定** - クラッジ行、ティア行、スクロールバー、ANSI 除去、ライトバーモード、逆順の切り替え、アトリビュートコード切り替え (BBS ごとの有効/無効)
- **テーマ設定** - `config_files/` ディレクトリから Ice および DCT カラーテーマファイルを選択
- **一般設定** - 返信用の名前と REP パケット出力ディレクトリの設定

各カテゴリを終了すると設定は自動的に保存されます。SlyMail と config ユーティリティは同じ設定ファイルを読み書きします。

### キーバインド

#### ファイルブラウザ
| キー | 操作 |
|-----|--------|
| Up/Down | ファイルとディレクトリのナビゲーション |
| Enter | ディレクトリを開く / QWK ファイルを選択 |
| Ctrl-R | リモートシステムディレクトリを開く |
| Q / ESC | 終了 |

#### カンファレンス一覧
| キー | 操作 |
|-----|--------|
| Up/Down | カンファレンスのナビゲーション |
| Enter | 選択したカンファレンスを開く |
| V | パケット内のアンケート/投票を表示 |
| O / Ctrl-L | 別の QWK ファイルを開く |
| S / Ctrl-U | 設定 |
| Q / ESC | SlyMail を終了 |
| ? / F1 | ヘルプ |

#### メッセージ一覧
| キー | 操作 |
|-----|--------|
| Up/Down | メッセージのナビゲーション |
| Enter / R | 選択したメッセージを読む |
| N | 新規メッセージを書く |
| G | メッセージ番号へ移動 |
| Ctrl-L | 別の QWK ファイルを開く |
| S / Ctrl-U | 設定 |
| C / ESC | カンファレンス一覧に戻る |
| Q | 終了 |
| ? / F1 | ヘルプ |

#### メッセージリーダー
| キー | 操作 |
|-----|--------|
| Up/Down | メッセージをスクロール |
| Left/Right | 前 / 次のメッセージ |
| F / L | 先頭 / 末尾のメッセージ |
| R | メッセージに返信 |
| V | 投票 (賛成/反対票またはアンケート投票) |
| D / Ctrl-D | ファイル添付をダウンロード |
| H | メッセージヘッダー情報を表示 |
| S / Ctrl-U | 設定 |
| C / Q / ESC | メッセージ一覧に戻る |
| ? / F1 | ヘルプ |

#### メッセージエディタ
| キー | 操作 |
|-----|--------|
| ESC | エディタメニュー (保存、中止など) |
| Ctrl-U | ユーザー設定ダイアログ |
| Ctrl-Q | 引用ウィンドウを開く/閉じる |
| Ctrl-K | カラーピッカー (カーソル位置に ANSI カラーコードを挿入) |
| Ctrl-G | コードで CP437 グラフィック文字を挿入 |
| Ctrl-W | 単語/テキスト検索 |
| Ctrl-S | サブジェクトを変更 |
| Ctrl-D | 現在の行を削除 |
| Ctrl-Z | メッセージを保存 |
| Ctrl-A | メッセージを中止 |
| F1 | ヘルプ画面 |
| Insert | 挿入/上書きモードの切り替え |

#### 引用ウィンドウ
| キー | 操作 |
|-----|--------|
| Up/Down | 引用行のナビゲーション |
| Enter | 選択した引用行を挿入 |
| Ctrl-Q / ESC | 引用ウィンドウを閉じる |

## アーキテクチャ

SlyMail はテキストユーザーインターフェースにプラットフォーム抽象化レイヤーを使用しています:

```
ITerminal (abstract base class)
    ├── NCursesTerminal  (Linux/macOS/BSD - ncurses)
    └── Win32Terminal    (Windows - conio + Win32 Console API)
```

CP437 のボックス描画文字と特殊文字は `cp437defs.h` で定義され、`putCP437()` メソッドを通じてレンダリングされます。このメソッドは CP437 コードをプラットフォームネイティブの同等品 (ncurses では ACS 文字、Windows では直接の CP437 バイト) にマッピングします。

### ソースファイル

| ファイル | 説明 |
|------|-------------|
| `terminal.h` | 抽象 `ITerminal` インターフェース、キー/カラー定数、ファクトリ |
| `terminal_ncurses.cpp` | CP437 から ACS へのマッピングを含む ncurses 実装 |
| `terminal_win32.cpp` | Windows コンソール API + conio 実装 |
| `cp437defs.h` | IBM コードページ 437 の文字定義 |
| `colors.h` | カラースキーム定義 (Ice、DCT、リーダー、リスト) |
| `theme.h` | テーマ設定ファイルパーサー (Synchronet スタイルのアトリビュートコード) |
| `ui_common.h` | 共有 UI ヘルパー (ダイアログ、テキスト入力、スクロールバーなど) |
| `qwk.h` / `qwk.cpp` | QWK/REP パケットパーサーとクリエーター (QWKE、添付ファイル、投票) |
| `bbs_colors.h` / `bbs_colors.cpp` | BBS カラー/アトリビュートコードパーサー (ANSI、Synchronet、WWIV、PCBoard、Celerity、Renegade) |
| `utf8_util.h` / `utf8_util.cpp` | UTF-8 ユーティリティ (検証、表示幅、CP437 から UTF-8 への変換) |
| `voting.h` / `voting.cpp` | VOTING.DAT パーサー、投票集計、アンケート表示 UI |
| `remote_systems.h` / `remote_systems.cpp` | リモートシステムディレクトリ、FTP/SFTP ブラウジング、JSON 永続化、ホームディレクトリユーティリティ |
| `settings.h` | ユーザー設定の永続化 |
| `settings_dialog.h` | 設定ダイアログ (エディタ、リーダー、アトリビュートコード切り替え) |
| `file_browser.h` | QWK ファイルブラウザとセレクター |
| `msg_list.h` | カンファレンスとメッセージ一覧ビュー |
| `msg_reader.h` | 投票と添付ファイル UI 付きのメッセージリーダー (DDMsgReader スタイル) |
| `msg_editor.h` | カラーピッカー付きのメッセージエディタ (SlyEdit Ice/DCT スタイル) |
| `main.cpp` | SlyMail アプリケーションのエントリーポイントとメインループ |
| `config.cpp` | スタンドアロン設定ユーティリティ |

## 設定

### 設定ファイル

設定は SlyMail 実行ファイルと同じディレクトリにある `slymail.ini` という名前の INI ファイルに保存されます。このファイルは SlyMail と `config` ユーティリティの両方で共有されます。ファイルには各設定の説明がコメントとして記載されています。

`slymail.ini` の例:
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

### テーマファイル

カラーテーマは `config_files/` ディレクトリ内の `.ini` ファイルです:

**Ice テーマ** (`EditorIceColors_*.ini`):
- BlueIce (デフォルト)、EmeraldCity、FieryInferno、Fire-N-Ice、GeneralClean、GenericBlue、PurpleHaze、ShadesOfGrey

**DCT テーマ** (`EditorDCTColors_*.ini`):
- Default (デフォルト)、Default-Modified、Midnight

テーマカラーの値は Synchronet アトリビュートコードに由来するコンパクトな形式を使用します:
- `n` = ノーマル (リセット)
- 前景色: `k`=黒、`r`=赤、`g`=緑、`y`=黄、`b`=青、`m`=マゼンタ、`c`=シアン、`w`=白
- `h` = 高輝度/明るい
- 背景数字: `0`=黒、`1`=赤、`2`=緑、`3`=茶、`4`=青、`5`=マゼンタ、`6`=シアン、`7`=明るいグレー

例: `nbh` = 明るい青のノーマル、`n4wh` = 青の背景に明るい白

### タグライン

タグラインは、メッセージ保存時にメッセージの末尾に追加される短い引用や格言です。タグライン機能はエディタで Ctrl-U から、または `config` ユーティリティから有効にできます。

タグラインは `tagline_files/taglines.txt` に 1 行に 1 つずつ保存されます。`#` または `;` で始まる行はコメントとして扱われ、無視されます。タグラインを有効にしてメッセージを保存すると、特定のタグラインを選択するか、ランダムに 1 つ選ぶよう促されます。選択したタグラインは `...` プレフィックスとともにメッセージに追加されます (例: `...To err is human, to really foul things up requires a computer.`)。

### スペルチェッカー

SlyMail にはプレーンテキスト辞書ファイルを使用した組み込みスペルチェッカーが含まれています。スペルチェッカーはエディタで Ctrl-U から、または `config` ユーティリティから保存時にプロンプトを表示するよう設定できます。

**辞書ファイル**は `dictionary_files/` に保存されたプレーンテキストファイル (1 行に 1 単語) です。複数の辞書を同時に選択して単語カバレッジを組み合わせることができます。SlyMail には以下が同梱されています:
- `dictionary_en.txt` - 英語 (一般、約 13 万語)
- `dictionary_en-US-supplemental.txt` - 米国英語補足
- `dictionary_en-GB-supplemental.txt` - 英国英語補足
- `dictionary_en-AU-supplemental.txt` - オーストラリア英語補足
- `dictionary_en-CA-supplemental.txt` - カナダ英語補足

スペルチェックが実行されると、チェッカーはメッセージをスキャンしてスペルミスの単語を見つけ、各単語について対話式ダイアログを表示します。**R** で単語を置換、**S** でスキップ、**A** で追加 (将来)、**Q** でチェック終了のオプションがあります。

## クレジット

- UI は [Nightfox (Eric Oulashin)](https://github.com/EricOulashin) による [DDMsgReader](https://github.com/SynchronetBBS/sbbs) と [SlyEdit](https://github.com/SynchronetBBS/sbbs) からインスパイア
- QWK フォーマットの互換性は [Synchronet BBS](https://www.synchro.net/) のソースコードを参考
- CP437 文字定義は Synchronet から

## ライセンス

このプロジェクトはオープンソースソフトウェアです。
