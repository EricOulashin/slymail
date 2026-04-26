> **Nodyn:** Mae'r ffeil hon yn gyfieithiad peirianyddol. Yr original Saesneg yw: [README.md](README.md)

# SlyMail

Darllennydd post all-lein testun-seiliedig aml-lwyfan ar gyfer fformat pecyn [QWK](https://en.wikipedia.org/wiki/QWK_(file_format)) yw hwn. Defnyddiwyd/defnyddir fformat pecyn QWK yn aml i gyfnewid post ar [systemau bwrdd bwletin (bulletin board systems)](https://en.wikipedia.org/wiki/Bulletin_board_system).

Mae SlyMail yn darparu rhyngwyneb llawn nodweddion ar gyfer darllen ac ateb negeseuon o becynnau post QWK BBS (Bulletin Board System). Mae ei ryngwyneb defnyddiwr wedi'i ysbrydoli gan [Digital Distortion Message Reader (DDMsgReader)](https://github.com/SynchronetBBS/sbbs/tree/master/xtrn/DDMsgReader) ar gyfer darllen negeseuon a [SlyEdit](https://github.com/SynchronetBBS/sbbs/tree/master/exec) ar gyfer golygu negeseuon — crëwyd y ddau yn wreiddiol ar gyfer [Synchronet BBS](https://www.synchro.net/).

Crëwyd SlyMail gyda chymorth Claude AI.

## Nodweddion

### Cymorth Pecyn QWK
- Yn agor ac yn darllen pecynnau post QWK safonol (ffeiliau .qwk)
- Yn dosrannu CONTROL.DAT, MESSAGES.DAT, a ffeiliau mynegai NDX
- Cymorth QWKE llawn (QWK estynedig) drwy HEADERS.DAT — paru yn seiliedig ar wrthbwyso ar gyfer meysydd To/From/Subject estynedig cywir, baner UTF-8, ac RFC822 Message-ID
- Dosrannu kludge corff QWKE (`To:`, `From:`, `Subject:` ar ddechrau neges)
- Yn trin rhifio cynhadledd yn arddull Synchronet
- Yn creu pecynnau ateb REP (ffeiliau .rep) i'w lanlwytho yn ôl i'r BBS, gan gynnwys HEADERS.DAT ar gyfer meysydd estynedig a VOTING.DAT ar gyfer pleidleisiau sy'n aros
- Yn cefnogi codio arnofiant Microsoft Binary Format (MBF) mewn ffeiliau NDX
- Yn cofio'r ffeil QWK a agorwyd ddiwethaf a'r cyfeiriadur rhwng sesiynau

### Darllen Negeseuon (arddull DDMsgReader)
- Rhestr gynhadledd gyda chyfrifon negeseuon
- Rhestr negeseuon y gellir ei sgrolio gyda llywio bar golau
- Darllennydd negeseuon llawn gyda dangos pennawd (Oddi wrth, At, Pwnc, Dyddiad)
- Amlygu llinellau dyfyniad (yn cefnogi dyfynnu aml-lefel)
- Dangos llinellau kludge (dewisol)
- Dangosydd bar sgrolio
- Llywio bysellfwrdd: Neges Gyntaf/Olaf/Nesaf/Flaenorol, Tudalen i Fyny/i Lawr
- Sgriniau cymorth y gellir eu cyrchu gyda `?` neu `F1` ym mhob golygfa

### Cymorth Cod Lliw ac Atrib BBS
Mae SlyMail yn dehongli codau lliw/atrib o becynnau meddalwedd BBS lluosog, gan eu rendro fel testun lliw yn y darllennydd negeseuon a'r golygydd negeseuon. Fformatau a gefnogir:
- **Codau ANSI escape** — wedi'u galluogi bob amser; dilyniadau SGR safonol (ESC[...m) ar gyfer blaendir, cefndir, trwm
- **Codau Synchronet Ctrl-A** — `\x01` + nod atrib (e.e., `\x01c` ar gyfer cyan, `\x01h` ar gyfer disglair)
- **Codau calon WWIV** — `\x03` + digid 0–9
- **Codau @X PCBoard/Wildcat** — `@X##` lle mae'r ddau ddigid hex yn amgodio lliw cefndir a blaendir
- **Codau pibell Celerity** — `|` + llythyren (e.e., `|c` ar gyfer cyan, `|W` ar gyfer gwyn llachar)
- **Codau pibell Renegade** — `|` + rhif deuddeg 00–31

Gellir galluogi neu analluogi pob math o god BBS yn unigol drwy'r is-ddeialog **Switshis cod atrib** yn Gosodiadau Darllennydd neu'r cyfleustod `config`. Mae'r switshis hyn yn effeithio ar y darllennydd a'r golygydd. Mae opsiwn ar wahân **Stripio codau ANSI** yn tynnu pob dilyniant ANSI o negeseuon pan gaiff ei alluogi.

### Atodiadau Ffeiliau
- Yn canfod atodiadau ffeiliau y cyfeirir atynt drwy linellau kludge `@ATTACH:` mewn cyrff negeseuon
- Yn dangos dangosydd **[ATT]** ym mhennawd y neges pan fo atodiadau'n bresennol
- Pwyswch **D** neu **Ctrl-D** yn y darllennydd i lawrlwytho atodiadau — yn dangos rhestr ffeiliau gyda meintiau ac yn gofyn am gyfeiriadur cyrchfan

### Pleidleisio ac Arolygon (Synchronet QWKE)
Mae SlyMail yn cefnogi estyniad Synchronet VOTING.DAT ar gyfer arolygon a phleidleisio ar negeseuon:
- **Arolygon**: Mae negeseuon a nodwyd fel arolygon yn dangos eu hopsiynau ateb gyda chyfrifon pleidleisiau a barrau canran. Pwyswch **V** i agor deialog pleidlais lle gallwch newid dewisiadau a bwrw eich pleidlais.
- **Pleidleisiau i fyny/i lawr**: Ar gyfer negeseuon rheolaidd (nad ydynt yn arolygon), pwyswch **V** i bleidleisio i fyny neu i lawr. Dangosir cyfrifon pleidleisiau cyfredol a sgôr ym mhennawd y neges.
- **Cyfrifon pleidleisiau**: Mae pennawd y neges yn dangos cyfrifon pleidleisiau i fyny/i lawr a sgôr net, gyda dangosydd os ydych chi eisoes wedi pleidleisio.
- **Ciwio pleidleisiau**: Mae pleidleisiau'n cael eu ciwio ochr yn ochr ag atebion negeseuon ac yn cael eu hysgrifennu i VOTING.DAT yn y pecyn REP i'w lanlwytho i'r BBS.
- **Porwr arolygon**: Pwyswch **V** o'r rhestr gynhadledd i bori'r holl arolygon yn y pecyn.

### Cymorth UTF-8
- Yn canfod cynnwys UTF-8 mewn negeseuon (drwy faner `Utf8` HEADERS.DAT a chanfyddiad awtomatig o ddilyniadau beit UTF-8)
- Yn arddangos nodau UTF-8 yn gywir ar derfynellau cymwys
- Yn dangos dangosydd **[UTF8]** ym mhennawd y neges ar gyfer negeseuon UTF-8
- Yn arbed negeseuon newydd gyda chodio priodol
- Trosi CP437 i UTF-8 ar gyfer cynnwys BBS etifeddiaeth
- Yn gosod locale ar Linux/macOS/BSD (`setlocale(LC_ALL, "")`) a thudalen cod UTF-8 ar Windows ar gyfer rendro terfynell priodol

### Golygydd Negeseuon (wedi'i ysbrydoli gan SlyEdit)
- **Dau fath gweledol**: Ice a DCT, pob un gyda chynlluniau a chynlluniau lliw gwahanol
- **Modd ar hap**: Yn dewis Ice neu DCT ar hap ar bob sesiwn golygu
- **Lliwiau ffin bob yn ail**: Mae nodau ffin yn newid ar hap rhwng dau liw thema, yn cyfateb i arddull weledol SlyEdit
- **Cymorth thema**: Themâu lliw y gellir eu ffurfweddu wedi'u llwytho o ffeiliau .ini
- Golygydd testun sgrin lawn gyda lapio geiriau
- Ffenestr dyfyniad ar gyfer dewis a mewnosod testun dyfynedig (Ctrl-Q i agor/cau)
- Cyfansoddi ateb a negeseuon newydd
- Dewislen ESC ar gyfer arbed, abortu, toglo mewnosod/gorwneud, a mwy
- **Dewiswr lliw Ctrl-K**: Yn agor deialog i ddewis lliwiau blaendir a chefndir, gan fewnosod cod ANSI escape yng ngosodiad y cyrchwr. Yn cefnogi 16 lliw blaendir (8 arferol + 8 disglair) ac 8 cefndir, gyda rhagolwg byw. Pwyswch **N** i fewnosod cod ailosod.
- **Rendro ymwybodol o liw**: Mae'r ardal olygu yn rendro codau ANSI ac atrib BBS mewn llinell, felly mae testun lliw yn cael ei arddangos wrth i chi deipio
- **Deialog gosodiadau defnyddiwr Ctrl-U** ar gyfer ffurfweddu dewisiadau golygydd ar y pryd
- **Anogaethau ie/na sy'n benodol i arddull**: Mae modd Ice yn defnyddio anogaeth mewn llinell ar waelod y sgrin; mae modd DCT yn defnyddio blwch deialog canolog gyda lliwiau thema

### Gosodiadau Golygydd (drwy Ctrl-U)
- **Dewis modd rhyngwyneb**: Deialog i newid rhwng arddulliau Ice, DCT, a Random (yn dod i rym ar unwaith)
- **Dewis ffeil thema**: Dewiswch o themâu lliw Ice neu DCT sydd ar gael
- **Taglines**: Pan gaiff ei alluogi, yn gofyn am ddewis tagline wrth arbed (o `tagline_files/taglines.txt`)
- **Geiriadur/geiriaduron gwirio sillafu**: Dewiswch pa eiriaduron i'w defnyddio
- **Annog ar gyfer gwirydd sillafu wrth arbed**: Pan gaiff ei alluogi, yn cynnig gwirio sillafu cyn arbed
- **Lapio llinellau dyfyniad i led terfynell**: Lapio llinellau dyfynedig
- **Dyfynnu gyda blaenlythrennau'r awdur**: Rhagddodi llinellau dyfyniad gyda blaenlythrennau'r awdur (e.e., `MP> `)
- **Indentio llinellau dyfyniad sy'n cynnwys blaenlythrennau**: Ychwanegu bwlch arweiniol cyn blaenlythrennau (e.e., ` MP> `)
- **Trimio bylchau o linellau dyfyniad**: Tynnu gofod gwyn arweiniol o destun dyfynedig

### Themâu Lliw
- Mae ffeiliau thema yn ffeiliau ffurfweddu (`.ini`) yn y cyfeiriadur `config_files/`
- Themâu Ice: `EditorIceColors_*.ini` (BlueIce, EmeraldCity, FieryInferno, ayyb)
- Themâu DCT: `EditorDCTColors_*.ini` (Default, Default-Modified, Midnight)
- Mae lliwiau thema yn defnyddio fformat syml: llythyren lliw blaendir (`r`/`g`/`b`/`c`/`y`/`m`/`w`/`k`), `h` dewisol ar gyfer disglair, digid cefndir dewisol (`0`-`7`)
- Mae themâu'n rheoli holl liwiau elfennau rhyngwyneb: ffiniau, labeli, gwerthoedd, ffenestr dyfyniad, bar cymorth, deialogau ie/na

### Gwirydd Sillafu
- Gwirydd sillafu adeiledig yn defnyddio ffeiliau geiriadur testun plaen
- Yn cael ei anfon gyda geiriaduron Saesneg (en, en-US, en-GB, en-AU, en-CA atodlenni)
- Deialog cywiro rhyngweithiol: Amnewid, Hepgor, neu Rhoi'r gorau iddi
- Ffeiliau geiriadur wedi'u storio yn y cyfeiriadur `dictionary_files/`

### Taglines
- Ffeiliau tagline wedi'u storio yn y cyfeiriadur `tagline_files/`
- Y ffeil tagline rhagosodedig yw `tagline_files/taglines.txt`, un tagline fesul llinell
- Trin llinellau sy'n dechrau gyda `#` neu `;` fel sylwadau ac yn eu hanwybyddu
- Dewiswch dagline penodol neu dewiswch un ar hap wrth arbed neges
- Mae taglines yn cael eu hatodi i negeseuon gyda rhagddodiad `...`

### Creu Pecyn REP
- Ar hyn o bryd, dim ond ZIP sy'n cael ei gefnogi (hoffwn ychwanegu cymorth am fwy o fathau cywasgu yn y dyfodol)
- Pan fyddwch chi'n ysgrifennu atebion neu negeseuon newydd, cânt eu ciwio fel rhai sy'n aros
- Mae pleidleisiau (pleidleisiau arolwg, pleidleisiau i fyny/i lawr) hefyd yn cael eu ciwio ochr yn ochr ag atebion
- Wrth ymadael (neu wrth agor ffeil QWK newydd), mae SlyMail yn annog i arbed yr holl eitemau sy'n aros
- Yn creu ffeil `.rep` safonol (archif ZIP) i'w lanlwytho i'r BBS, gan gynnwys:
  - `<BBSID>.MSG` — negeseuon ateb yn fformat QWK safonol
  - `HEADERS.DAT` — penawdau estynedig QWKE ar gyfer meysydd sy'n fwy na 25 nod
  - `VOTING.DAT` — pleidleisiau sy'n aros yn fformat INI sy'n gydnaws â Synchronet
- Mae ffeil REP yn cael ei harbed fel `<BBS-ID>.rep` yn y cyfeiriadur ateb ffurfweddig (neu gyfeiriadur y ffeil QWK)

### Systemau Pell (Ctrl-R)
Gall SlyMail lawrlwytho pecynnau QWK yn uniongyrchol o systemau pell drwy FTP neu SFTP (SSH):
- Pwyswch **Ctrl-R** o'r porwr ffeiliau i agor cyfeiriadur systemau pell
- **Ychwanegu/Golygu/Dileu** cofnodion system bell gyda: enw, gwesteiwr, porthladd, math o gysylltiad (FTP neu SSH), enw defnyddiwr, cyfrinair, toglo FTP goddefol, a llwybr pell cychwynnol
- **Pori cyfeiriaduron pell** gyda phlorwr ffeil/cyfeiriadur tebyg i'r porwr ffeiliau lleol — llywio i mewn i gyfeiriaduron, mynd i fyny gyda `..`, neidio i wreiddyn gyda `/`
- **Lawrlwytho ffeiliau QWK** o'r system bell yn uniongyrchol i is-gyfeiriadur `QWK` cyfeiriadur data SlyMail
- Mae cofnodion system bell yn cael eu cadw yn `remote_systems.json` yn eich cyfeiriadur data SlyMail
- Mae dyddiad/amser y cysylltiad diwethaf yn cael ei olrhain ar gyfer pob system
- Yn defnyddio gorchymyn `curl` y system ar gyfer trosglwyddiadau FTP ac SFTP (dim dibyniaethau llyfrgell amser llunio)

### Gosodiadau Cymhwysiad
- Gosodiadau parhaus wedi'u harbed yn `slymail.ini` yn y cyfeiriadur data SlyMail (`~/.slymail` ar Linux/macOS/BSD, neu gyfeiriadur cartref y defnyddiwr ar Windows)
- Mae'r cyfeiriadur data SlyMail a'i is-gyfeiriadur `QWK` yn cael eu creu'n awtomatig ar y rhediad cyntaf
- Y cyfeiriadur pori ffeil QWK a chadw pecyn REP rhagosodedig yw `~/.slymail/QWK`
- Yn cofio'r cyfeiriadur a phoriwyd ddiwethaf a'r enw ffeil QWK
- Allwedd poeth Ctrl-L i lwytho ffeil QWK wahanol o olygfeydd cynhadledd neu restr negeseuon
- Rhagddodiad dyfyniad, lled llinell dyfyniad, enw defnyddiwr y gellir eu ffurfweddu
- Opsiynau darllennydd: dangos/cuddio llinellau kludge, llinellau tear/origin, bar sgrolio, stripio codau ANSI
- Switshis cod atrib fesul BBS (Synchronet, WWIV, Celerity, Renegade, PCBoard/Wildcat) — yn effeithio ar y darllennydd a'r golygydd
- Cyfeiriadur allbwn pecyn REP

## Sgrinluniau

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

## Gosodiadau Synchronet ar gyfer pecyn QWK
Ar Synchronet BBS, yn gosodiadau pecyn QWK, mae Slymail yn (neu dylai fod yn) gydnaws â chodau lliw Ctrl-A, VOTING.DAT, atodiadau ffeiliau, a fformat pecyn QWKE. Dylai Slymail hefyd fod yn gydnaws ag UTF-8. Er enghraifft:
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

## Adeiladu

### Gofynion

**Linux / macOS / BSD:**
- Cyfieithydd sy'n gydnaws â C++17 (GCC 8+, Clang 7+)
- Llyfrgell datblygu ncurses (`libncurses-dev` ar Debian/Ubuntu, `ncurses-devel` ar Fedora/RHEL)
- Gorchymyn `unzip` (ar gyfer echdynnu pecynnau QWK)
- Gorchymyn `zip` (ar gyfer creu pecynnau REP)
- Gorchymyn `curl` (ar gyfer trosglwyddiadau FTP/SFTP system bell — dewisol, dim ond ei angen ar gyfer nodwedd y systemau pell)

**Windows (Visual Studio 2022):**
- Visual Studio 2022 gyda'r llwyth gwaith "Datblygu bwrdd gwaith gyda C++"
- Windows SDK 10.0 (yn gynwysedig gyda VS)
- Dim llyfrgelloedd ychwanegol yn ofynnol — yn defnyddio'r Win32 Console API adeiledig ar gyfer rhyngwyneb y terfynell, ac un ai `tar.exe` neu PowerShell ar gyfer trin ZIP pecyn QWK/REP (gweler nodyn isod)

**Windows (MinGW/MSYS2):**
- MinGW-w64 neu MSYS2 gyda GCC (cymorth C++17)
- Windows Console API (adeiledig)

> **Nodyn — Trin ZIP QWK/REP ar Windows:** Mae SlyMail yn canfod ar amser rhedeg pa offeryn sydd ar gael ac yn defnyddio'r opsiwn gorau:
>
> - **`tar.exe` (a ffefrir):** Yn cael ei anfon gyda Windows 10 fersiwn 1803 (Diweddariad Ebrill 2018) ac yn ddiweddarach, a gyda phob fersiwn o Windows 11. Mae `tar` yn darllen ffeiliau ZIP yn ôl eu cynnwys yn hytrach na'u estyniad ffeil, felly mae pecynnau `.qwk` yn echdynnu'n uniongyrchol a chrëir pecynnau `.rep` drwy ffeil `.zip` dros dro a gaiff ei hailenwi wedyn. Nid oes angen unrhyw ffurfweddu ychwanegol.
> - **PowerShell (wrth gefn):** Os na cheir `tar.exe` yn y PATH, mae SlyMail yn defnyddio PowerShell fel dewis arall. Ar gyfer echdynnu mae'n defnyddio dosbarth .NET `ZipFile` (`System.IO.Compression`) yn hytrach nag `Expand-Archive`, oherwydd mae `Expand-Archive` yn gwrthod estyniadau ffeil nad ydynt yn `.zip` hyd yn oed pan fo'r ffeil yn archif ZIP ddilys. Ar gyfer creu pecyn REP mae'n defnyddio `Compress-Archive`, gan ysgrifennu i ffeil `.zip` dros dro a gaiff ei hailenwi'n `.rep` wedyn.

### Adeiladu ar Linux/macOS/BSD

```bash
make
```

Mae hyn yn adeiladu dau raglen:
- `slymail` — y prif raglen darllennydd QWK
- `config` — y cyfleustod ffurfweddu annibynnol

### Adeiladu gyda symbolau dadfygio

```bash
make debug
```

### Gosod (dewisol)

```bash
sudo make install    # Installs slymail and config to /usr/local/bin/
sudo make uninstall  # Remove
```

### Adeiladu ar Windows gyda Visual Studio 2022

Agorwch y ffeil ateb yn Visual Studio 2022:

```
vs\SlyMail.sln
```

Neu adeiladwch o'r llinell orchymyn gan ddefnyddio MSBuild:

```powershell
# Release build (output in vs\x64\Release\)
msbuild vs\SlyMail.sln /p:Configuration=Release /p:Platform=x64

# Debug build (output in vs\x64\Debug\)
msbuild vs\SlyMail.sln /p:Configuration=Debug /p:Platform=x64
```

Mae hyn yn adeiladu dwy weithredadwy:
- `x64\Release\slymail.exe` — y prif ddarllennydd QWK
- `x64\Release\config.exe` — y cyfleustod ffurfweddu annibynnol

Mae'r ateb yn cynnwys dau brosiect (`SlyMail.vcxproj` a `Config.vcxproj`) wedi'u targedu at x64, C++17, gyda set offer MSVC v143.

### Adeiladu ar Windows (MinGW/MSYS2)

```bash
make
```

Mae'r Makefile yn canfod y llwyfan yn awtomatig ac yn defnyddio'r gweithrediad terfynell priodol:
- **Linux/macOS/BSD**: ncurses (`terminal_ncurses.cpp`)
- **Windows**: conio + Win32 Console API (`terminal_win32.cpp`)

## Defnydd

```bash
# Launch SlyMail with file browser
./slymail

# Open a specific QWK packet
./slymail MYBBS.qwk

# Run the standalone configuration utility
./config
```

### Rhaglen Ffurfweddu

Mae'r cyfleustod `config` yn darparu rhyngwyneb testun-seiliedig annibynnol ar gyfer ffurfweddu gosodiadau SlyMail heb agor y brif raglen. Mae'n cynnig pedair categori ffurfweddu:

- **Gosodiadau Golygydd** — Yr un gosodiadau i gyd sydd ar gael drwy Ctrl-U yn y golygydd (arddull golygydd, taglines, gwirio sillafu, opsiynau dyfynnu, ayyb)
- **Gosodiadau Darllennydd** — Toglo llinellau kludge, llinellau tear, bar sgrolio, stripio ANSI, modd bar golau, trefn wrthdroi, a switshis cod atrib (galluogi/analluogi fesul BBS)
- **Gosodiadau Thema** — Dewiswch ffeiliau thema lliw Ice a DCT o'r cyfeiriadur `config_files/`
- **Gosodiadau Cyffredinol** — Gosodwch eich enw ar gyfer atebion a chyfeiriadur allbwn pecyn REP

Mae gosodiadau'n cael eu harbed yn awtomatig wrth adael pob categori. Mae SlyMail a'r cyfleustod config ill dau yn darllen ac yn ysgrifennu'r un ffeil gosodiadau.

### Rhwymiadau Bysellau

#### Porwr Ffeiliau
| Bysell | Gweithred |
|--------|-----------|
| Fyny/Lawr | Llywio ffeiliau a chyfeiriaduron |
| Enter | Agor cyfeiriadur / Dewis ffeil QWK |
| Ctrl-R | Agor cyfeiriadur systemau pell |
| Q / ESC | Ymadael |

#### Rhestr Gynhadledd
| Bysell | Gweithred |
|--------|-----------|
| Fyny/Lawr | Llywio cynhadleddau |
| Enter | Agor y gynhadledd a ddewiswyd |
| V | Gweld arolygon/pleidleisiau yn y pecyn |
| O / Ctrl-L | Agor ffeil QWK wahanol |
| S / Ctrl-U | Gosodiadau |
| Q / ESC | Ymadael â SlyMail |
| ? / F1 | Cymorth |

#### Rhestr Negeseuon
| Bysell | Gweithred |
|--------|-----------|
| Fyny/Lawr | Llywio negeseuon |
| Enter / R | Darllen y neges a ddewiswyd |
| N | Ysgrifennu neges newydd |
| G | Mynd i rif neges |
| Ctrl-L | Agor ffeil QWK wahanol |
| S / Ctrl-U | Gosodiadau |
| C / ESC | Yn ôl i'r rhestr gynhadledd |
| Q | Ymadael |
| ? / F1 | Cymorth |

#### Darllennydd Negeseuon
| Bysell | Gweithred |
|--------|-----------|
| Fyny/Lawr | Sgrolio neges |
| Chwith/Dde | Neges Flaenorol / Nesaf |
| F / L | Neges Gyntaf / Olaf |
| R | Ateb y neges |
| V | Pleidleisio (pleidlais i fyny/i lawr neu bleidlais arolwg) |
| D / Ctrl-D | Lawrlwytho atodiadau ffeiliau |
| H | Dangos gwybodaeth pennawd neges |
| S / Ctrl-U | Gosodiadau |
| C / Q / ESC | Yn ôl i'r rhestr negeseuon |
| ? / F1 | Cymorth |

#### Golygydd Negeseuon
| Bysell | Gweithred |
|--------|-----------|
| ESC | Dewislen golygydd (Arbed, Abortu, ayyb) |
| Ctrl-U | Deialog gosodiadau defnyddiwr |
| Ctrl-Q | Agor/cau ffenestr dyfyniad |
| Ctrl-K | Dewiswr lliw (mewnosod cod lliw ANSI yng ngosodiad y cyrchwr) |
| Ctrl-G | Mewnosod nod graffig (CP437) yn ôl cod |
| Ctrl-W | Chwilio am air/testun |
| Ctrl-S | Newid pwnc |
| Ctrl-D | Dileu'r llinell bresennol |
| Ctrl-Z | Arbed neges |
| Ctrl-A | Abortu neges |
| F1 | Sgrin cymorth |
| Insert | Toglo modd Mewnosod/Gorwneud |

#### Ffenestr Dyfyniad
| Bysell | Gweithred |
|--------|-----------|
| Fyny/Lawr | Llywio llinellau dyfyniad |
| Enter | Mewnosod y llinell dyfyniad a ddewiswyd |
| Ctrl-Q / ESC | Cau ffenestr dyfyniad |

## Pensaernïaeth

Mae SlyMail yn defnyddio haen haniaethol llwyfan ar gyfer ei ryngwyneb defnyddiwr testun:

```
ITerminal (abstract base class)
    ├── NCursesTerminal  (Linux/macOS/BSD - ncurses)
    └── Win32Terminal    (Windows - conio + Win32 Console API)
```

Mae nodau lluniadu blwch CP437 a nodau arbennig wedi'u diffinio yn `cp437defs.h` ac yn cael eu rendro drwy'r dull `putCP437()`, sy'n mapio codau CP437 i gyfatebiaid brodorol y llwyfan (nodau ACS ar ncurses, beitiau CP437 uniongyrchol ar Windows).

### Ffeiliau Ffynhonnell

| Ffeil | Disgrifiad |
|-------|------------|
| `terminal.h` | Rhyngwyneb `ITerminal` haniaethol, cysonion allweddi/lliw, ffatri |
| `terminal_ncurses.cpp` | Gweithrediad ncurses gyda mapio CP437-i-ACS |
| `terminal_win32.cpp` | Windows Console API + gweithrediad conio |
| `cp437defs.h` | Diffiniadau nod IBM Code Page 437 |
| `colors.h` | Diffiniadau cynllun lliw (Ice, DCT, darllennydd, rhestr) |
| `theme.h` | Dosranwr ffeil ffurfweddu thema (codau atrib arddull Synchronet) |
| `ui_common.h` | Cymorthyddion rhyngwyneb cyffredin (deialogau, mewnbwn testun, bar sgrolio, ayyb) |
| `qwk.h` / `qwk.cpp` | Dosranwr a chrëwr pecyn QWK/REP (QWKE, atodiadau, pleidleisio) |
| `bbs_colors.h` / `bbs_colors.cpp` | Dosranwr cod lliw/atrib BBS (ANSI, Synchronet, WWIV, PCBoard, Celerity, Renegade) |
| `utf8_util.h` / `utf8_util.cpp` | Cyfleustodau UTF-8 (dilysu, lled arddangos, trosi CP437-i-UTF-8) |
| `voting.h` / `voting.cpp` | Dosranwr VOTING.DAT, cyfrif pleidleisiau, rhyngwyneb dangos arolygon |
| `remote_systems.h` / `remote_systems.cpp` | Cyfeiriadur systemau pell, pori FTP/SFTP, dyfalbarhad JSON, cyfleustodau cyfeiriadur cartref |
| `settings.h` | Dyfalbarhad gosodiadau defnyddiwr |
| `settings_dialog.h` | Deialogau gosodiadau (golygydd, darllennydd, switshis cod atrib) |
| `file_browser.h` | Porwr a dewiswr ffeiliau QWK |
| `msg_list.h` | Golygfeydd rhestr gynhadledd a rhestr negeseuon |
| `msg_reader.h` | Darllennydd negeseuon (arddull DDMsgReader) gyda phleidleisio a rhyngwyneb atodiadau |
| `msg_editor.h` | Golygydd negeseuon (arddull SlyEdit Ice/DCT) gyda dewiswr lliw |
| `main.cpp` | Pwynt mynediad cymhwysiad SlyMail a phrif ddolen |
| `config.cpp` | Cyfleustod ffurfweddu annibynnol |

## Ffurfweddu

### Ffeil Gosodiadau

Mae gosodiadau wedi'u storio mewn ffeil INI o'r enw `slymail.ini` yn yr un cyfeiriadur â gweithredadwy SlyMail. Mae'r ffeil hon yn cael ei rhannu rhwng SlyMail a'r cyfleustod `config`. Mae'r ffeil wedi'i nodi'n dda gyda disgrifiadau o bob gosodiad.

Enghraifft `slymail.ini`:
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

### Ffeiliau Thema

Mae themâu lliw yn ffeiliau `.ini` yn y cyfeiriadur `config_files/`:

**Themâu Ice** (`EditorIceColors_*.ini`):
- BlueIce (rhagosodedig), EmeraldCity, FieryInferno, Fire-N-Ice, GeneralClean, GenericBlue, PurpleHaze, ShadesOfGrey

**Themâu DCT** (`EditorDCTColors_*.ini`):
- Default (rhagosodedig), Default-Modified, Midnight

Mae gwerthoedd lliw thema yn defnyddio fformat cryno sy'n deillio o godau atrib Synchronet:
- `n` = arferol (ailosod)
- Lliwiau blaendir: `k`=du, `r`=coch, `g`=gwyrdd, `y`=melyn, `b`=glas, `m`=magenta, `c`=cyan, `w`=gwyn
- `h` = uchel/disglair
- Digid cefndir: `0`=du, `1`=coch, `2`=gwyrdd, `3`=brown, `4`=glas, `5`=magenta, `6`=cyan, `7`=llwyd golau

Enghraifft: `nbh` = glas llachar, `n4wh` = gwyn llachar ar gefndir glas

### Taglines

Mae taglines yn ddyfyniadau neu ddywediadau byr wedi'u hatodi at ddiwedd negeseuon wrth eu harbed. Gellir galluogi'r nodwedd tagline drwy Ctrl-U yn y golygydd neu'r cyfleustod `config`.

Mae taglines wedi'u storio yn `tagline_files/taglines.txt`, un fesul llinell. Trin llinellau sy'n dechrau gyda `#` neu `;` fel sylwadau ac yn eu hanwybyddu. Wrth arbed neges gyda taglines wedi'u galluogi, anogir y defnyddiwr i ddewis tagline benodol neu ddewis un ar hap. Mae'r tagline a ddewiswyd yn cael ei hatodi at y neges gyda rhagddodiad `...` (e.e., `...To err is human, to really foul things up requires a computer.`).

### Gwirydd Sillafu

Mae SlyMail yn cynnwys gwirydd sillafu adeiledig sy'n defnyddio ffeiliau geiriadur testun plaen. Gellir ffurfweddu'r gwirydd sillafu i annog wrth arbed drwy Ctrl-U yn y golygydd neu'r cyfleustod `config`.

Mae **Ffeiliau geiriadur** yn ffeiliau testun plaen (un gair fesul llinell) wedi'u storio yn `dictionary_files/`. Gellir dewis geiriaduron lluosog ar yr un pryd ar gyfer cyfuniad o eiriau. Mae SlyMail yn cael ei anfon gyda:
- `dictionary_en.txt` — Saesneg (cyffredinol, ~130K o eiriau)
- `dictionary_en-US-supplemental.txt` — atodlen Saesneg yr UD
- `dictionary_en-GB-supplemental.txt` — atodlen Saesneg Prydain
- `dictionary_en-AU-supplemental.txt` — atodlen Saesneg Awstralia
- `dictionary_en-CA-supplemental.txt` — atodlen Saesneg Canada

Pan gaiff gwirio sillafu ei sbarduno, mae'r gwirydd yn sganio'r neges am eiriau camsillafiedig ac yn cyflwyno deialog rhyngweithiol ar gyfer pob un, gan gynnig opsiynau i **A**mnewid y gair, **H**epgor ef, **Y**chwanegu ef (yn y dyfodol), neu **R**hoi'r gorau i wirio.

## Credydau

- Rhyngwyneb wedi'i ysbrydoli gan [DDMsgReader](https://github.com/SynchronetBBS/sbbs) a [SlyEdit](https://github.com/SynchronetBBS/sbbs) gan [Nightfox (Eric Oulashin)](https://github.com/EricOulashin)
- Cydnawsedd fformat QWK wedi'i lywio gan god ffynhonnell [Synchronet BBS](https://www.synchro.net/)
- Diffiniadau nod CP437 o Synchronet

## Trwydded

Mae'r prosiect hwn yn feddalwedd ffynhonnell agored.
