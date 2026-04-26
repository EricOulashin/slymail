> **Nóta:** Is aistriúchán meaisín é an comhad seo. Tá an bunleagan Béarla anseo: [README.md](README.md)

# SlyMail

Is léitheoir ríomhphoist as líne, bunaithe ar théacs, ilardáin é seo don fhormáid paicéad [QWK](https://en.wikipedia.org/wiki/QWK_(file_format)). Úsáidtear/úsáideadh formáid paicéad QWK go minic chun ríomhphost a mhalartú ar [chórais clár fógraí (bulletin board systems)](https://en.wikipedia.org/wiki/Bulletin_board_system).

Soláthraíonn SlyMail comhéadan lán-ghné chun teachtaireachtaí ó phaicéid ríomhphoist QWK BBS (Bulletin Board System) a léamh agus a fhreagairt. Spreagtar a chomhéadan úsáideora ag [Digital Distortion Message Reader (DDMsgReader)](https://github.com/SynchronetBBS/sbbs/tree/master/xtrn/DDMsgReader) le haghaidh léitheoireacht teachtaireachtaí agus [SlyEdit](https://github.com/SynchronetBBS/sbbs/tree/master/exec) le haghaidh eagarthóireacht teachtaireachtaí — cruthaíodh an dá cheann ar dtús do [Synchronet BBS](https://www.synchro.net/).

Cruthaíodh SlyMail le cabhair Claude AI.

## Gnéithe

### Tacaíocht Paicéad QWK
- Osclaíonn agus léann paicéid ríomhphoist QWK caighdeánacha (comhaid .qwk)
- Parsálann CONTROL.DAT, MESSAGES.DAT, agus comhaid innéacs NDX
- Tacaíocht iomlán QWKE (QWK leathnaithe) trí HEADERS.DAT — meaitseáil bunaithe ar fhrithchúiteamh le haghaidh réimsí leathnaithe To/From/Subject cruinne, bratach UTF-8, agus RFC822 Message-ID
- Parsáil kludge coirp QWKE (`To:`, `From:`, `Subject:` ag tús teachtaireachta)
- Láimhseálann uimhriú comhdhála i stíl Synchronet
- Cruthaíonn paicéid freagra REP (comhaid .rep) chun iad a uaslódáil ar ais chuig an BBS, lena n-áirítear HEADERS.DAT le haghaidh réimsí leathnaithe agus VOTING.DAT le haghaidh vótaí ar feitheamh
- Tacaíonn le códú snámhphointe Microsoft Binary Format (MBF) i gcomhaid NDX
- Cuimhníonn an comhad QWK agus an eolaire deireanach a osclaíodh idir seisiúin

### Léitheoireacht Teachtaireachtaí (stíl DDMsgReader)
- Liosta comhdhála le cuntais teachtaireachtaí
- Liosta teachtaireachtaí inscrolaithe le nascleanúint bharra solais
- Léitheoir teachtaireachtaí iomlán le taispeáint ceanntásca (Ó, Chuig, Ábhar, Dáta)
- Aibhsiú línte athluaite (tacaíonn le hathluachan il-leibhéal)
- Taispeáint línte kludge (roghnach)
- Táscaire barra scrollála
- Nascleanúint méarchláir: Teachtaireacht Chéad/Dheiridh/Chéad Eile/Roimhe, Leathanach Suas/Síos
- Scáileáin cabhracha inrochtana le `?` nó `F1` i ngach radharc

### Tacaíocht Cóid Datha agus Tréith BBS
Léirmhíníonn SlyMail cóid datha/tréith ó phacáistí bogearraí BBS iolracha, á gcur ar taispeáint mar théacs dathaithe sa léitheoir teachtaireachtaí agus san eagarthóir teachtaireachtaí. Formáidí tacaithe:
- **Cóid ANSI escape** — cumasaithe i gcónaí; seichimh SGR caighdeánacha (ESC[...m) do réamhraon, cúlraon, trom
- **Cóid Synchronet Ctrl-A** — `\x01` + carachtar tréithe (m.sh., `\x01c` do cian, `\x01h` do gheal)
- **Cóid croí WWIV** — `\x03` + digit 0–9
- **Cóid @X PCBoard/Wildcat** — `@X##` ina gcódaíonn an dá dhigit heicsidheachúlacha dath cúlraon agus réamhraon
- **Cóid píopa Celerity** — `|` + litir (m.sh., `|c` do cian, `|W` do bhán geal)
- **Cóid píopa Renegade** — `|` + uimhir dhá dhigit 00–31

Is féidir gach cineál cód BBS a chumasú nó a dhíchumasú ina aonar trí fho-chomhéadan **Scoránaigh cód tréithe** i Socruithe Léitheora nó i bhfóntas `config`. Cuireann na scoránaigh seo isteach ar an léitheoir agus ar an eagarthóir araon. Baineann rogha ar leith **Stríoc cóid ANSI** gach seicheamh ANSI as teachtaireachtaí nuair a bhíonn sé cumasaithe.

### Ceangaltáin Comhaid
- Braithean ceangaltáin comhaid dá dtagraítear trí línte kludge `@ATTACH:` i gcorp teachtaireachtaí
- Taispeánann táscaire **[ATT]** i gceanntásc na teachtaireachta nuair atá ceangaltáin i láthair
- Brúigh **D** nó **Ctrl-D** sa léitheoir chun ceangaltáin a íoslódáil — taispeánann liosta comhaid le méideanna agus iarrann eolaire ceann scríbe

### Vótáil agus Suirbhéanna (Synchronet QWKE)
Tacaíonn SlyMail leis an síneadh Synchronet VOTING.DAT le haghaidh suirbhéanna agus vótáil ar theachtaireachtaí:
- **Suirbhéanna**: Taispeánann teachtaireachtaí a aithníodh mar shuirbhéanna a gcuid roghanna freagraí le cuntais vótaí agus barraí céatadáin. Brúigh **V** chun comhéadan ballóide a oscailt inar féidir leat roghanna a scoránú agus do vóta a chaitheamh.
- **Vótaí suas/síos**: Le haghaidh teachtaireachtaí rialta (nach suirbhéanna iad), brúigh **V** chun vóta a chaitheamh suas nó síos. Taispeántar cuntais vótaí reatha agus scór i gceanntásc na teachtaireachta.
- **Cuntais vótaí**: Taispeánann ceanntásc na teachtaireachta cuntais vótaí suas/síos agus scór glan, le táscaire má vótáil tú cheana féin.
- **Feitheamh vótaí**: Cuirtear vótaí sa scuaine in éineacht le freagraí teachtaireachtaí agus scríobhtar chuig VOTING.DAT sa phaicéad REP le haghaidh uaslódála chuig an BBS.
- **Brabhsálaí suirbhéanna**: Brúigh **V** ón liosta comhdhála chun gach suirbhé sa phaicéad a bhrabhsáil.

### Tacaíocht UTF-8
- Braithean ábhar UTF-8 i dteachtaireachtaí (trí bhratach `Utf8` HEADERS.DAT agus braite uathoibríoch seicheamh beart UTF-8)
- Taispeánann carachtar UTF-8 i gceart ar chríochfoirt chomhoiriúnacha
- Taispeánann táscaire **[UTF8]** i gceanntásc na teachtaireachta le haghaidh teachtaireachtaí UTF-8
- Sábhálann teachtaireachtaí nua le códú cuí
- Tiontú CP437 go UTF-8 le haghaidh ábhair BBS oidhreachta
- Socraíonn locale ar Linux/macOS/BSD (`setlocale(LC_ALL, "")`) agus leathanach cód UTF-8 ar Windows le haghaidh rindreáil críochfoirt chuí

### Eagarthóir Teachtaireachtaí (inspioráidithe ag SlyEdit)
- **Dhá mhód amhairc**: Ice agus DCT, gach ceann le scéimeanna datha agus leaganacha ar leith
- **Mód randamach**: Roghnaíonn Ice nó DCT go randamach ar gach seisiún eagarthóireachta
- **Dathanna teorann ag malartú**: Athraíonn carachtair teorann go randamach idir dhá dhath téama, ag meaitseáil stíl amhairc SlyEdit
- **Tacaíocht téama**: Téamaí datha in-chumraithe luchtaithe ó chomhaid .ini
- Eagarthóir téacs lánscáileáin le fillíocht focal
- Fuinneog athluaite chun téacs athluaite a roghnú agus a chur isteach (Ctrl-Q chun oscailt/dúnadh)
- Cumadóireacht freagraí agus teachtaireachtaí nua
- Roghchlár ESC le haghaidh sábháil, abortáil, scoránú isteach/forscríobh, agus níos mó
- **Roghnóir datha Ctrl-K**: Osclaíonn comhéadan chun dathanna réamhraoin agus cúlraoin a roghnú, ag cur cód ANSI escape isteach ag suíomh an chúrsóra. Tacaíonn le 16 dath réamhraoin (8 gnáth + 8 geal) agus 8 gcúlraon, le réamhamharc beo. Brúigh **N** chun cód athshocraithe a chur isteach.
- **Rindreáil datha-feasach**: Rindreálann an limistéar eagarthóireachta cóid ANSI agus tréithe BBS go hinlíne, mar sin taispeántar téacs dathaithe de réir mar a chlóscríobhann tú
- **Comhéadan socruithe úsáideora Ctrl-U** chun roghanna eagarthóra a chumrú ar an eitilt
- **Leid tá/níl sonrach go stíl**: Úsáideann mód Ice leid inlíne bun-scáileáin; úsáideann mód DCT bosca comhéadain láraithe le dathanna téama

### Socruithe Eagarthóra (trí Ctrl-U)
- **Roghnaigh mód comhéadain**: Comhéadan chun athrú idir stíleanna Ice, DCT, agus Random (éifeacht láithreach)
- **Roghnaigh comhad téama**: Roghnaigh ó théamaí datha Ice nó DCT atá ar fáil
- **Taglines**: Nuair a bhíonn sé cumasaithe, iarrann roghnú tagline ar shábháil (ó `tagline_files/taglines.txt`)
- **Foclóir/foclóirí seiceálaí litrithe**: Roghnaigh cé na foclóirí le húsáid
- **Iarr ar sheiceálaí litrithe ar shábháil**: Nuair a bhíonn sé cumasaithe, tairgeann seiceáil litrithe roimh shábháil
- **Fillíocht línte athluaite go leithead críochfoirt**: Fillíocht línte athluaite
- **Athluach le tosaigh an údair**: Réamhchuireann línte athluaite le tosaigh an údair (m.sh., `MP> `)
- **Eangú línte athluaite ina bhfuil tosaigh**: Cuir spás tosaigh roimh thosaigh (m.sh., ` MP> `)
- **Bearradh spásanna ó línte athluaite**: Scrios spás bán tosaigh ó théacs athluaite

### Téamaí Datha
- Is comhaid chumraithe (`.ini`) iad comhaid téama sa chomhadlann `config_files/`
- Téamaí Ice: `EditorIceColors_*.ini` (BlueIce, EmeraldCity, FieryInferno, srl.)
- Téamaí DCT: `EditorDCTColors_*.ini` (Default, Default-Modified, Midnight)
- Úsáideann dathanna téama formáid shimplí: litir datha réamhraoin (`r`/`g`/`b`/`c`/`y`/`m`/`w`/`k`), `h` roghnach do gheal, digit cúlraoin roghnach (`0`-`7`)
- Rialaíonn téamaí gach dath eiliminte comhéadain: teorainneacha, lipéid, luachanna, fuinneog athluaite, barra cabhrach, comhéadain tá/níl

### Seiceálaí Litrithe
- Seiceálaí litrithe ionsuite ag úsáid comhaid foclóra gnáth-théacs
- Seoltar le foclóirí Béarla (en, en-US, en-GB, en-AU, forbairtí en-CA)
- Comhéadan ceartúcháin idirghníomhach: Ionadaigh, Scipeáil, nó Éirigh as
- Comhaid foclóra stóráilte sa chomhadlann `dictionary_files/`

### Taglines
- Comhaid tagline stóráilte sa chomhadlann `tagline_files/`
- Is é an comhad tagline réamhshocraithe `tagline_files/taglines.txt`, tagline amháin in aghaidh líne
- Caitear línte ag tosú le `#` nó `;` mar nótaí tráchta agus déantar neamhaird díobh
- Roghnaigh tagline sonrach nó roghnaigh ceann go randamach agus teachtaireacht á sábháil
- Cuirtear taglines le teachtaireachtaí le réimír `...`

### Cruthú Paicéad REP
- Faoi láthair, ní thacaítear ach le ZIP (ba mhaith liom tacaíocht a chur leis do níos mó cineálacha comhbhrú sa todhchaí)
- Nuair a scríobhann tú freagraí nó teachtaireachtaí nua, cuirtear iad sa scuaine mar chinn ar feitheamh
- Cuirtear vótaí (ballóidí suirbhé, vótaí suas/síos) sa scuaine freisin in éineacht le freagraí
- Ar imeacht (nó nuair a osclaítear comhad QWK nua), spreagann SlyMail chun gach mír ar feitheamh a shábháil
- Cruthaíonn comhad `.rep` caighdeánach (cartlann ZIP) chun é a uaslódáil chuig an BBS, ina bhfuil:
  - `<BBSID>.MSG` — teachtaireachtaí freagra i bhformáid QWK caighdeánach
  - `HEADERS.DAT` — ceanntásca leathnaithe QWKE do réimsí a théann thar 25 carachtar
  - `VOTING.DAT` — vótaí ar feitheamh i bhformáid INI comhoiriúnach le Synchronet
- Sábháiltear comhad REP mar `<BBS-ID>.rep` sa chomhadlann freagra cumraithe (nó in eolaire an chomhaid QWK)

### Córais Chianda (Ctrl-R)
Is féidir le SlyMail paicéid QWK a íoslódáil go díreach ó chórais chianda trí FTP nó SFTP (SSH):
- Brúigh **Ctrl-R** ón mbrabhsálaí comhaid chun eolaire na gcóras cianda a oscailt
- **Cuir leis/Cuir in eagar/Scrios** iontrálacha córais chianda le: ainm, óstáil, calafort, cineál ceangail (FTP nó SSH), ainm úsáideora, pasfhocal, scoránú FTP éighníomhach, agus cosán cianda tosaigh
- **Brabhsáil comhadlanna cianda** le brabhsálaí comhaid/comhadlainne cosúil leis an gceann áitiúil — nascleanúin isteach i gcomhadlanna, téigh suas le `..`, léim go fréamh le `/`
- **Íoslódáil comhaid QWK** ón gcóras cianda go díreach isteach i bhfo-eolaire `QWK` de chomhadlann sonraí SlyMail
- Stóráiltear iontrálacha córais chianda i `remote_systems.json` i gcomhadlann sonraí SlyMail
- Déantar dáta/am an naisc dheireanaigh a rianú do gach córas
- Úsáideann an t-ordú `curl` córais le haghaidh aistrithe comhaid FTP agus SFTP (gan spleáchais leabharlainne am-tiomsaithe)

### Socruithe Feidhlimí
- Socruithe leanúnacha sábháilte i `slymail.ini` i gcomhadlann sonraí SlyMail (`~/.slymail` ar Linux/macOS/BSD, nó comhadlann baile an úsáideora ar Windows)
- Cruthaítear comhadlann sonraí SlyMail agus a fo-eolaire `QWK` go huathoibríoch ar an gcéad rith
- Is é an comhadlann réamhshocraithe brabhsála comhaid QWK agus sábhála paicéad REP `~/.slymail/QWK`
- Cuimhníonn an comhadlann deireanach a brabhsáladh agus ainm comhaid QWK
- Eochair te Ctrl-L chun comhad QWK éagsúil a luchtú ó radhairc chomhdhála nó liostaí teachtaireachtaí
- Réimír athluaite in-chumraithe, leithead líne athluaite, ainm úsáideora
- Roghanna léitheora: taispeáin/folaigh línte kludge, línte tear/origin, barra scrollála, stríoc cóid ANSI
- Scoránaigh cód tréithe in aghaidh BBS (Synchronet, WWIV, Celerity, Renegade, PCBoard/Wildcat) — cuireann isteach ar an léitheoir agus ar an eagarthóir araon
- Eolaire aschuir paicéad REP

## Gabhdáin Scáileáin

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

## Socruithe Synchronet le haghaidh paicéad QWK
Ar Synchronet BBS, i socruithe paicéad QWK, tá Slymail (nó ba cheart go mbeadh) comhoiriúnach le cóid datha Ctrl-A, VOTING.DAT, ceangaltáin comhaid, agus formáid paicéad QWKE. Ba cheart go mbeadh Slymail comhoiriúnach freisin le UTF-8. Mar shampla:
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

## Tógáil

### Riachtanais

**Linux / macOS / BSD:**
- Tiomsaitheoir comhoiriúnach C++17 (GCC 8+, Clang 7+)
- Leabharlann forbartha ncurses (`libncurses-dev` ar Debian/Ubuntu, `ncurses-devel` ar Fedora/RHEL)
- Ordú `unzip` (chun paicéid QWK a bhaint as)
- Ordú `zip` (chun paicéid REP a chruthú)
- Ordú `curl` (le haghaidh aistrithe comhaid FTP/SFTP córais chianda — roghnach, ní gá ach don ghné córas cianda)

**Windows (Visual Studio 2022):**
- Visual Studio 2022 leis an ualach oibre "Forbairt deisce le C++"
- Windows SDK 10.0 (san áireamh le VS)
- Ní gá leabharlanna breise — úsáideann an Win32 Console API ionsuite don chomhéadan críochfoirt, agus ceachtar `tar.exe` nó PowerShell le haghaidh láimhseáil ZIP paicéid QWK/REP (féach nóta thíos)

**Windows (MinGW/MSYS2):**
- MinGW-w64 nó MSYS2 le GCC (tacaíocht C++17)
- Windows Console API (ionsuite)

> **Nóta — Láimhseáil ZIP QWK/REP ar Windows:** Braithean SlyMail ag am rith cé acu uirlis atá ar fáil agus úsáideann an rogha is fearr:
>
> - **`tar.exe` (roghnach):** Seoltar le Windows 10 leagan 1803 (Nuashonrú Aibreán 2018) agus níos déanaí, agus le gach leagan de Windows 11. Léann `tar` comhaid ZIP de réir a n-ábhair seachas a síneadh comhaid, mar sin baintear paicéid `.qwk` go díreach agus cruthaítear paicéid `.rep` trí chomhad `.zip` sealadach a athainmnítear ansin. Ní gá aon chumrú breise.
> - **PowerShell (cúltaca):** Mura bhfaightear `tar.exe` sa PATH, úsáideann SlyMail PowerShell mar rogha eile. Le haghaidh eastóscadh úsáideann sé an rang .NET `ZipFile` (`System.IO.Compression`) seachas `Expand-Archive`, toisc go ndiúltaíonn `Expand-Archive` síntí comhaid seachas `.zip` fiú nuair is cartlann ZIP bhailí í an comhad. Le haghaidh cruthú paicéad REP úsáideann sé `Compress-Archive`, ag scríobh chuig comhad `.zip` sealadach a athainmnítear go `.rep` ansin.

### Tógáil ar Linux/macOS/BSD

```bash
make
```

Tógann sé seo dhá chlár:
- `slymail` — príomhfheidhlim léitheoir QWK
- `config` — fóntas cumraithe neamhspleách

### Tógáil le siombailí dífhabhtaithe

```bash
make debug
```

### Suiteáil (roghnach)

```bash
sudo make install    # Installs slymail and config to /usr/local/bin/
sudo make uninstall  # Remove
```

### Tógáil ar Windows le Visual Studio 2022

Oscail an comhad réitigh in Visual Studio 2022:

```
vs\SlyMail.sln
```

Nó tóg ón líne ordaithe ag úsáid MSBuild:

```powershell
# Release build (output in vs\x64\Release\)
msbuild vs\SlyMail.sln /p:Configuration=Release /p:Platform=x64

# Debug build (output in vs\x64\Debug\)
msbuild vs\SlyMail.sln /p:Configuration=Debug /p:Platform=x64
```

Tógann sé seo dhá inrite:
- `x64\Release\slymail.exe` — an príomhléitheoir QWK
- `x64\Release\config.exe` — fóntas cumraithe neamhspleách

Tá dhá thionscadal ag an réiteach (`SlyMail.vcxproj` agus `Config.vcxproj`) dírithe ar x64, C++17, leis an tacar uirlisí MSVC v143.

### Tógáil ar Windows (MinGW/MSYS2)

```bash
make
```

Braithean an Makefile an t-ardán go huathoibríoch agus úsáideann sé an cur chun feidhme críochfoirt cuí:
- **Linux/macOS/BSD**: ncurses (`terminal_ncurses.cpp`)
- **Windows**: conio + Win32 Console API (`terminal_win32.cpp`)

## Úsáid

```bash
# Launch SlyMail with file browser
./slymail

# Open a specific QWK packet
./slymail MYBBS.qwk

# Run the standalone configuration utility
./config
```

### Clár Cumraithe

Soláthraíonn an fóntas `config` comhéadan téacs-bunaithe neamhspleách chun socruithe SlyMail a chumrú gan an príomhfheidhlim a oscailt. Tairgeann sé ceithre chatagóir cumraithe:

- **Socruithe Eagarthóra** — Na socruithe céanna ar fad atá ar fáil trí Ctrl-U san eagarthóir (stíl eagarthóra, taglines, seiceáil litrithe, roghanna athluaite, srl.)
- **Socruithe Léitheora** — Scoránaigh línte kludge, línte tear, barra scrollála, stríocadh ANSI, mód barra solais, ord droim ar ais, agus scoránaigh cód tréithe (cumasaigh/díchumasaigh in aghaidh BBS)
- **Socruithe Téama** — Roghnaigh comhaid téama datha Ice agus DCT ón gcomhadlann `config_files/`
- **Socruithe Ginearálta** — Socraigh d'ainm le haghaidh freagraí agus eolaire aschuir paicéad REP

Sábháiltear socruithe go huathoibríoch agus tú ag fágáil gach catagóire. Léann agus scríobhann SlyMail agus an fóntas config an comhad socruithe céanna araon.

### Ceangaltáin Eochracha

#### Brabhsálaí Comhaid
| Eochair | Gníomh |
|---------|--------|
| Suas/Síos | Nascleanúin comhaid agus comhadlanna |
| Enter | Oscail comhadlann / Roghnaigh comhad QWK |
| Ctrl-R | Oscail eolaire córas cianda |
| Q / ESC | Éirigh as |

#### Liosta Comhdhála
| Eochair | Gníomh |
|---------|--------|
| Suas/Síos | Nascleanúin comhdhála |
| Enter | Oscail an chomhdháil roghnaithe |
| V | Féach ar shuirbhéanna/vótaí sa phaicéad |
| O / Ctrl-L | Oscail comhad QWK éagsúil |
| S / Ctrl-U | Socruithe |
| Q / ESC | Fág SlyMail |
| ? / F1 | Cabhair |

#### Liosta Teachtaireachtaí
| Eochair | Gníomh |
|---------|--------|
| Suas/Síos | Nascleanúin teachtaireachtaí |
| Enter / R | Léigh an teachtaireacht roghnaithe |
| N | Scríobh teachtaireacht nua |
| G | Téigh go huimhir teachtaireachta |
| Ctrl-L | Oscail comhad QWK éagsúil |
| S / Ctrl-U | Socruithe |
| C / ESC | Ar ais chuig an liosta comhdhála |
| Q | Éirigh as |
| ? / F1 | Cabhair |

#### Léitheoir Teachtaireachtaí
| Eochair | Gníomh |
|---------|--------|
| Suas/Síos | Scrolláil teachtaireacht |
| Clé/Deas | Teachtaireacht Roimhe / Chéad Eile |
| F / L | Teachtaireacht Chéad / Dheiridh |
| R | Freagair an teachtaireacht |
| V | Vótáil (vóta suas/síos nó ballóid suirbhé) |
| D / Ctrl-D | Íoslódáil ceangaltáin comhaid |
| H | Taispeáin faisnéis ceanntásca teachtaireachta |
| S / Ctrl-U | Socruithe |
| C / Q / ESC | Ar ais chuig an liosta teachtaireachtaí |
| ? / F1 | Cabhair |

#### Eagarthóir Teachtaireachtaí
| Eochair | Gníomh |
|---------|--------|
| ESC | Roghchlár eagarthóra (Sábháil, Abort, srl.) |
| Ctrl-U | Comhéadan socruithe úsáideora |
| Ctrl-Q | Oscail/dún fuinneog athluaite |
| Ctrl-K | Roghnóir datha (cuir cód datha ANSI isteach ag suíomh an chúrsóra) |
| Ctrl-G | Cuir carachtar grafach (CP437) isteach de réir cóid |
| Ctrl-W | Cuardaigh focal/téacs |
| Ctrl-S | Athraigh ábhar |
| Ctrl-D | Scrios an líne reatha |
| Ctrl-Z | Sábháil teachtaireacht |
| Ctrl-A | Abort teachtaireacht |
| F1 | Scáileán cabhrach |
| Insert | Scoránaigh mód Isteach/Forscríobh |

#### Fuinneog Athluaite
| Eochair | Gníomh |
|---------|--------|
| Suas/Síos | Nascleanúin línte athluaite |
| Enter | Cuir an líne athluaite roghnaithe isteach |
| Ctrl-Q / ESC | Dún fuinneog athluaite |

## Ailtireacht

Úsáideann SlyMail sraith teibíochta ardáin dá chomhéadan úsáideora téacs:

```
ITerminal (abstract base class)
    ├── NCursesTerminal  (Linux/macOS/BSD - ncurses)
    └── Win32Terminal    (Windows - conio + Win32 Console API)
```

Sainmhínítear carachtair líníochta bosca CP437 agus carachtair speisialta i `cp437defs.h` agus rindreáiltear iad tríd an modh `putCP437()`, a mhapálann cóid CP437 chuig coibhéisigh dhúchasacha an ardáin (carachtair ACS ar ncurses, bearta CP437 díreacha ar Windows).

### Comhaid Foinse

| Comhad | Cur síos |
|--------|----------|
| `terminal.h` | Comhéadan `ITerminal` teibí, tairiseach eochracha/datha, monarcha |
| `terminal_ncurses.cpp` | Cur chun feidhme ncurses le mapáil CP437-go-ACS |
| `terminal_win32.cpp` | Windows Console API + cur chun feidhme conio |
| `cp437defs.h` | Sainmhínithe carachtair IBM Code Page 437 |
| `colors.h` | Sainmhínithe scéime datha (Ice, DCT, léitheoir, liosta) |
| `theme.h` | Parsálaí comhaid cumraithe téama (cóid tréithe stíl Synchronet) |
| `ui_common.h` | Cabhraigh leis an gcomhéadan roinnte (comhéadain, ionchuir téacs, barra scrollála, srl.) |
| `qwk.h` / `qwk.cpp` | Parsálaí agus cruthaitheoir paicéad QWK/REP (QWKE, ceangaltáin, vótáil) |
| `bbs_colors.h` / `bbs_colors.cpp` | Parsálaí cóid datha/tréithe BBS (ANSI, Synchronet, WWIV, PCBoard, Celerity, Renegade) |
| `utf8_util.h` / `utf8_util.cpp` | Fóntais UTF-8 (bailíochtú, leithead taispeána, tiontú CP437-go-UTF-8) |
| `voting.h` / `voting.cpp` | Parsálaí VOTING.DAT, comhaireamh vótaí, comhéadan taispeána suirbhé |
| `remote_systems.h` / `remote_systems.cpp` | Eolaire córas cianda, brabhsáil FTP/SFTP, marthanacht JSON, fóntas eolaire baile |
| `settings.h` | Marthanacht socruithe úsáideora |
| `settings_dialog.h` | Comhéadain socruithe (eagarthóir, léitheoir, scoránaigh cód tréithe) |
| `file_browser.h` | Brabhsálaí agus roghnóir comhaid QWK |
| `msg_list.h` | Radhairc liosta comhdhála agus liosta teachtaireachtaí |
| `msg_reader.h` | Léitheoir teachtaireachtaí (stíl DDMsgReader) le vótáil agus comhéadan ceangaltán |
| `msg_editor.h` | Eagarthóir teachtaireachtaí (stíl SlyEdit Ice/DCT) le roghnóir datha |
| `main.cpp` | Pointe iontrála feidhlimí SlyMail agus príomhlúb |
| `config.cpp` | Fóntas cumraithe neamhspleách |

## Cumrú

### Comhad Socruithe

Stóráiltear socruithe i gcomhad INI darb ainm `slymail.ini` san eolaire céanna leis an inrite SlyMail. Roinntear an comhad seo idir SlyMail agus fóntas `config` araon. Tá an comhad maith-nótáilte le tuairiscí gach socraithe.

Sampla `slymail.ini`:
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

### Comhaid Téama

Is comhaid `.ini` iad téamaí datha sa chomhadlann `config_files/`:

**Téamaí Ice** (`EditorIceColors_*.ini`):
- BlueIce (réamhshocraithe), EmeraldCity, FieryInferno, Fire-N-Ice, GeneralClean, GenericBlue, PurpleHaze, ShadesOfGrey

**Téamaí DCT** (`EditorDCTColors_*.ini`):
- Default (réamhshocraithe), Default-Modified, Midnight

Úsáideann luachanna datha téama formáid chompacht díorthaithe ó chóid tréithe Synchronet:
- `n` = gnáth (athshocrú)
- Dathanna réamhraoin: `k`=dubh, `r`=dearg, `g`=glas, `y`=buí, `b`=gorm, `m`=magenta, `c`=cian, `w`=bán
- `h` = ard/geal
- Digit cúlraoin: `0`=dubh, `1`=dearg, `2`=glas, `3`=donn, `4`=gorm, `5`=magenta, `6`=cian, `7`=liath éadrom

Sampla: `nbh` = gorm geal, `n4wh` = bán geal ar chúlra gorm

### Taglines

Is dráchtanna nó seanfhocail ghearra iad taglines a ghreamaítear de dheireadh teachtaireachtaí agus iad á sábháil. Is féidir an ghné tagline a chumasú trí Ctrl-U san eagarthóir nó trí fhóntas `config`.

Stóráiltear taglines i `tagline_files/taglines.txt`, ceann in aghaidh líne. Caitear línte ag tosú le `#` nó `;` mar nótaí tráchta agus déantar neamhaird díobh. Agus teachtaireacht á sábháil le taglines cumasaithe, spreagtar an t-úsáideoir chun tagline ar leith a roghnú nó ceann a roghnú go randamach. Greamítear an tagline roghnaithe den teachtaireacht le réimír `...` (m.sh., `...To err is human, to really foul things up requires a computer.`).

### Seiceálaí Litrithe

Áiríonn SlyMail seiceálaí litrithe ionsuite ag úsáid comhaid foclóra gnáth-théacs. Is féidir an seiceálaí litrithe a chumrú chun spreagadh ar shábháil trí Ctrl-U san eagarthóir nó trí fhóntas `config`.

Is comhaid gnáth-théacs iad **Comhaid foclóra** (focal amháin in aghaidh líne) stóráilte i `dictionary_files/`. Is féidir foclóirí iolracha a roghnú ag an am céanna le haghaidh cumhdach focal comhcheangailte. Seoltar SlyMail le:
- `dictionary_en.txt` — Béarla (ginearálta, ~130K focal)
- `dictionary_en-US-supplemental.txt` — forbairt Béarla SAM
- `dictionary_en-GB-supplemental.txt` — forbairt Béarla na Breataine
- `dictionary_en-AU-supplemental.txt` — forbairt Béarla na hAstráile
- `dictionary_en-CA-supplemental.txt` — forbairt Béarla Cheanada

Nuair a spreagtar seiceáil litrithe, scannaíonn an seiceálaí an teachtaireacht le haghaidh focal mílitrithe agus cuireann comhéadan idirghníomhach i láthair do gach ceann, ag tairiscint roghanna chun an focal a **I**onadú, é a **S**cipeáil, é a **C**ur leis (amach anseo), nó **É**irigh as seiceáil.

## Creidmheasanna

- Comhéadan spreagtha ag [DDMsgReader](https://github.com/SynchronetBBS/sbbs) agus [SlyEdit](https://github.com/SynchronetBBS/sbbs) le [Nightfox (Eric Oulashin)](https://github.com/EricOulashin)
- Comhoiriúnacht formáide QWK bunaithe ar chód foinse [Synchronet BBS](https://www.synchro.net/)
- Sainmhínithe carachtair CP437 ó Synchronet

## Ceadúnas

Is bogearraí foinse oscailte é an tionscadal seo.
