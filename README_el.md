> **Σημείωση:** Αυτό το αρχείο είναι αυτόματη μετάφραση. Το πρωτότυπο αγγλικό κείμενο βρίσκεται στο [README.md](README.md).

# SlyMail

Πρόκειται για έναν διαπλατφορμικό αναγνώστη αλληλογραφίας εκτός σύνδεσης, βασισμένο σε κείμενο, για τη μορφή πακέτων [QWK](https://en.wikipedia.org/wiki/QWK_(file_format)). Η μορφή πακέτων QWK χρησιμοποιούνταν/χρησιμοποιείται συχνά για την ανταλλαγή αλληλογραφίας σε [συστήματα πίνακα ανακοινώσεων (BBS)](https://en.wikipedia.org/wiki/Bulletin_board_system).

Το SlyMail παρέχει μια πλούσια διεπαφή για την ανάγνωση και απάντηση μηνυμάτων από πακέτα αλληλογραφίας QWK του BBS (Bulletin Board System). Η διεπαφή χρήστη εμπνέεται από τον [Digital Distortion Message Reader (DDMsgReader)](https://github.com/SynchronetBBS/sbbs/tree/master/xtrn/DDMsgReader) για την ανάγνωση μηνυμάτων και το [SlyEdit](https://github.com/SynchronetBBS/sbbs/tree/master/exec) για την επεξεργασία μηνυμάτων, αμφότερα δημιουργήθηκαν αρχικά για το [Synchronet BBS](https://www.synchro.net/).

Το SlyMail δημιουργήθηκε με τη βοήθεια του Claude AI.

## Χαρακτηριστικά

### Υποστήριξη πακέτων QWK
- Ανοίγει και διαβάζει τυπικά πακέτα αλληλογραφίας QWK (αρχεία .qwk)
- Αναλύει τα CONTROL.DAT, MESSAGES.DAT και αρχεία ευρετηρίου NDX
- Πλήρης υποστήριξη QWKE (εκτεταμένο QWK) μέσω HEADERS.DAT — αντιστοίχιση βάσει μετατόπισης για ακριβή εκτεταμένα πεδία To/From/Subject, σημαία UTF-8 και RFC822 Message-ID
- Ανάλυση kludge σώματος QWKE (`To:`, `From:`, `Subject:` στην αρχή μηνύματος)
- Χειρισμός αρίθμησης συνεδρίων τύπου Synchronet
- Δημιουργεί πακέτα απαντήσεων REP (αρχεία .rep) για μεταφόρτωση πίσω στο BBS, συμπεριλαμβανομένου του HEADERS.DAT για εκτεταμένα πεδία και του VOTING.DAT για εκκρεμείς ψήφους
- Υποστηρίζει κωδικοποίηση κινητής υποδιαστολής Microsoft Binary Format (MBF) σε αρχεία NDX
- Θυμάται το τελευταίο ανοιχτό αρχείο QWK και κατάλογο μεταξύ συνεδριών

### Ανάγνωση μηνυμάτων (στυλ DDMsgReader)
- Λίστα συνεδρίων με αριθμούς μηνυμάτων
- Κυλιόμενη λίστα μηνυμάτων με πλοήγηση με γραμμή φωτός
- Πλήρης αναγνώστης μηνυμάτων με εμφάνιση κεφαλίδας (From, To, Subject, Date)
- Επισήμανση γραμμών παράθεσης (υποστηρίζει πολλαπλά επίπεδα παράθεσης)
- Εμφάνιση γραμμών kludge (προαιρετική)
- Δείκτης γραμμής κύλισης
- Πλοήγηση με πληκτρολόγιο: Πρώτο/Τελευταίο/Επόμενο/Προηγούμενο μήνυμα, Σελίδα πάνω/κάτω
- Οθόνες βοήθειας προσβάσιμες με `?` ή `F1` σε όλες τις προβολές

### Υποστήριξη κωδικών χρώματος και χαρακτηριστικών BBS
Το SlyMail ερμηνεύει κωδικούς χρώματος/χαρακτηριστικών από πολλά πακέτα λογισμικού BBS, αποδίδοντάς τους ως έγχρωμο κείμενο τόσο στον αναγνώστη όσο και στον επεξεργαστή μηνυμάτων. Υποστηριζόμενες μορφές:
- **Κωδικοί διαφυγής ANSI** — πάντα ενεργοποιημένοι· τυπικές ακολουθίες SGR (ESC[...m) για χρώμα προσκηνίου, φόντου, έντονη γραφή
- **Κωδικοί Ctrl-A του Synchronet** — `\x01` + χαρακτήρας χαρακτηριστικού (π.χ. `\x01c` για κυανό, `\x01h` για φωτεινό)
- **Κωδικοί καρδιάς WWIV** — `\x03` + ψηφίο 0–9
- **Κωδικοί @X του PCBoard/Wildcat** — `@X##` όπου τα δύο δεκαεξαδικά ψηφία κωδικοποιούν το χρώμα φόντου και προσκηνίου
- **Κωδικοί σωλήνα Celerity** — `|` + γράμμα (π.χ. `|c` για κυανό, `|W` για φωτεινό λευκό)
- **Κωδικοί σωλήνα Renegade** — `|` + διψήφιος αριθμός 00–31

Κάθε τύπος κωδικού BBS μπορεί να ενεργοποιηθεί ή να απενεργοποιηθεί μεμονωμένα μέσω του υποδιαλόγου **Εναλλαγές κωδικών χαρακτηριστικών** στις Ρυθμίσεις Αναγνώστη ή στο βοηθητικό πρόγραμμα `config`. Αυτές οι εναλλαγές επηρεάζουν τόσο τον αναγνώστη όσο και τον επεξεργαστή. Μια ξεχωριστή επιλογή **Αφαίρεση κωδικών ANSI** αφαιρεί όλες τις ακολουθίες ANSI από τα μηνύματα όταν είναι ενεργοποιημένη.

### Συνημμένα αρχεία
- Εντοπίζει συνημμένα αρχεία που αναφέρονται μέσω γραμμών kludge `@ATTACH:` στα σώματα μηνυμάτων
- Εμφανίζει έναν δείκτη **[ATT]** στην κεφαλίδα μηνύματος όταν υπάρχουν συνημμένα
- Πατήστε **D** ή **Ctrl-D** στον αναγνώστη για λήψη συνημμένων — εμφανίζει μια λίστα αρχείων με μεγέθη και ζητά έναν κατάλογο προορισμού

### Ψηφοφορία και δημοσκοπήσεις (Synchronet QWKE)
Το SlyMail υποστηρίζει την επέκταση Synchronet VOTING.DAT για δημοσκοπήσεις και ψηφοφορία μηνυμάτων:
- **Δημοσκοπήσεις**: Τα μηνύματα που αναγνωρίζονται ως δημοσκοπήσεις εμφανίζουν τις επιλογές απαντήσεων με αριθμούς ψήφων και γραμμές ποσοστού. Πατήστε **V** για να ανοίξει ένα παράθυρο ψηφοδελτίου όπου μπορείτε να εναλλάξετε τις επιλογές σας και να ψηφίσετε.
- **Ψήφοι υπέρ/κατά**: Για κανονικά (μη δημοσκοπικά) μηνύματα, πατήστε **V** για ψήφο υπέρ ή κατά. Τα τρέχοντα αποτελέσματα ψηφοφορίας και βαθμολογία εμφανίζονται στην κεφαλίδα μηνύματος.
- **Αποτελέσματα ψηφοφορίας**: Η κεφαλίδα μηνύματος εμφανίζει αριθμούς ψήφων υπέρ/κατά και καθαρή βαθμολογία, με δείκτη αν έχετε ήδη ψηφίσει.
- **Ουρά ψηφοφορίας**: Οι ψήφοι τοποθετούνται στην ουρά μαζί με τις απαντήσεις μηνυμάτων και εγγράφονται στο VOTING.DAT στο πακέτο REP για μεταφόρτωση στο BBS.
- **Περιηγητής δημοσκοπήσεων**: Πατήστε **V** από τη λίστα συνεδρίων για περιήγηση σε όλες τις δημοσκοπήσεις στο πακέτο.

### Υποστήριξη UTF-8
- Ανιχνεύει περιεχόμενο UTF-8 σε μηνύματα (μέσω της σημαίας `Utf8` του HEADERS.DAT και αυτόματης ανίχνευσης ακολουθιών byte UTF-8)
- Εμφανίζει σωστά χαρακτήρες UTF-8 σε συμβατά τερματικά
- Εμφανίζει έναν δείκτη **[UTF8]** στην κεφαλίδα μηνύματος για μηνύματα UTF-8
- Αποθηκεύει νέα μηνύματα με κατάλληλη κωδικοποίηση
- Μετατροπή CP437 σε UTF-8 για παλαιό περιεχόμενο BBS
- Ορίζει locale στο Linux/macOS/BSD (`setlocale(LC_ALL, "")`) και κωδικοσελίδα UTF-8 στα Windows για σωστή απόδοση τερματικού

### Επεξεργαστής μηνυμάτων (εμπνευσμένος από το SlyEdit)
- **Δύο οπτικές λειτουργίες**: Ice και DCT, η καθεμία με διαφορετικά χρωματικά σχήματα και διατάξεις
- **Τυχαία λειτουργία**: Επιλέγει τυχαία Ice ή DCT σε κάθε συνεδρία επεξεργασίας
- **Εναλλασσόμενα χρώματα πλαισίου**: Οι χαρακτήρες πλαισίου εναλλάσσονται τυχαία μεταξύ δύο χρωμάτων θέματος, ταιριάζοντας με το οπτικό στυλ του SlyEdit
- **Υποστήριξη θεμάτων**: Διαμορφώσιμα χρωματικά θέματα που φορτώνονται από αρχεία .ini
- Επεξεργαστής κειμένου πλήρους οθόνης με αυτόματη αναδίπλωση
- Παράθυρο παράθεσης για επιλογή και εισαγωγή παρατεθειμένου κειμένου (Ctrl-Q για άνοιγμα/κλείσιμο)
- Σύνταξη απαντήσεων και νέων μηνυμάτων
- Μενού ESC για αποθήκευση, ματαίωση, εναλλαγή εισαγωγής/αντικατάστασης και άλλα
- **Επιλογέας χρώματος Ctrl-K**: Ανοίγει έναν διάλογο για επιλογή χρωμάτων προσκηνίου και φόντου, εισάγοντας έναν κωδικό διαφυγής ANSI στη θέση του δρομέα. Υποστηρίζει 16 χρώματα προσκηνίου (8 κανονικά + 8 φωτεινά) και 8 φόντα, με ζωντανή προεπισκόπηση. Πατήστε **N** για εισαγωγή κωδικού επαναφοράς.
- **Απόδοση με επίγνωση χρώματος**: Η περιοχή επεξεργασίας αποδίδει κωδικούς ANSI και χαρακτηριστικών BBS ενσωματωμένα, ώστε το έγχρωμο κείμενο να εμφανίζεται καθώς πληκτρολογείτε
- **Παράθυρο ρυθμίσεων χρήστη Ctrl-U** για διαμόρφωση προτιμήσεων επεξεργαστή εν κινήσει
- **Προτροπές ναι/όχι ειδικές για στυλ**: Η λειτουργία Ice χρησιμοποιεί ενσωματωμένη προτροπή στο κάτω μέρος της οθόνης· η λειτουργία DCT χρησιμοποιεί κεντρικό παράθυρο διαλόγου με θεματικά χρώματα

### Ρυθμίσεις επεξεργαστή (μέσω Ctrl-U)
- **Επιλογή λειτουργίας UI**: Παράθυρο διαλόγου για εναλλαγή μεταξύ στυλ Ice, DCT και Random (εφαρμόζεται αμέσως)
- **Επιλογή αρχείου θέματος**: Επιλογή από διαθέσιμα θέματα χρωμάτων Ice ή DCT
- **Γραμμές ετικέτας**: Όταν ενεργοποιηθεί, ζητά επιλογή γραμμής ετικέτας κατά την αποθήκευση (από το `tagline_files/taglines.txt`)
- **Λεξικό/λεξικά ορθογραφικού ελέγχου**: Επιλογή λεξικών προς χρήση
- **Προτροπή για ορθογραφικό έλεγχο κατά την αποθήκευση**: Όταν ενεργοποιηθεί, προσφέρει ορθογραφικό έλεγχο πριν την αποθήκευση
- **Αναδίπλωση γραμμών παράθεσης στο πλάτος τερματικού**: Αυτόματη αναδίπλωση παρατεθειμένων γραμμών
- **Παράθεση με αρχικά συγγραφέα**: Πρόθεση αρχικών συγγραφέα στις γραμμές παράθεσης (π.χ. `MP> `)
- **Εσοχή γραμμών παράθεσης που περιέχουν αρχικά**: Προσθήκη κενού διαστήματος πριν τα αρχικά (π.χ. ` MP> `)
- **Αφαίρεση κενών από γραμμές παράθεσης**: Αφαίρεση κενών διαστημάτων στην αρχή παρατεθειμένου κειμένου

### Χρωματικά θέματα
- Τα αρχεία θεμάτων είναι αρχεία ρυθμίσεων (`.ini`) στον κατάλογο `config_files/`
- Θέματα Ice: `EditorIceColors_*.ini` (BlueIce, EmeraldCity, FieryInferno κ.λπ.)
- Θέματα DCT: `EditorDCTColors_*.ini` (Default, Default-Modified, Midnight)
- Τα χρώματα θέματος χρησιμοποιούν απλή μορφή: γράμμα προσκηνίου (`r`/`g`/`b`/`c`/`y`/`m`/`w`/`k`), προαιρετικό `h` για φωτεινό, προαιρετικό ψηφίο φόντου (`0`-`7`)
- Τα θέματα ελέγχουν όλα τα χρώματα στοιχείων UI: πλαίσια, ετικέτες, τιμές, παράθυρο παράθεσης, γραμμή βοήθειας, παράθυρα διαλόγου ναι/όχι

### Ορθογραφικός έλεγχος
- Ενσωματωμένος ορθογραφικός έλεγχος που χρησιμοποιεί αρχεία λεξικού απλού κειμένου
- Παρέχεται με αγγλικά λεξικά (en, en-US, en-GB, en-AU, en-CA συμπληρώματα)
- Διαδραστικό παράθυρο διαλόγου διόρθωσης: Αντικατάσταση, Παράλειψη ή Έξοδος
- Αρχεία λεξικού αποθηκευμένα στον κατάλογο `dictionary_files/`

### Γραμμές ετικέτας
- Τα αρχεία γραμμών ετικέτας αποθηκεύονται στον κατάλογο `tagline_files/`
- Το προεπιλεγμένο αρχείο γραμμών ετικέτας είναι το `tagline_files/taglines.txt`, μία γραμμή ετικέτας ανά γραμμή
- Οι γραμμές που αρχίζουν με `#` ή `;` αντιμετωπίζονται ως σχόλια και αγνοούνται
- Επιλέξτε μια συγκεκριμένη γραμμή ετικέτας ή επιλέξτε μία τυχαία κατά την αποθήκευση μηνύματος
- Οι γραμμές ετικέτας προσαρτώνται στα μηνύματα με πρόθεμα `...`

### Δημιουργία πακέτου REP
- Προς το παρόν, υποστηρίζεται μόνο ZIP (θέλω να προσθέσω υποστήριξη για περισσότερους τύπους συμπίεσης στο μέλλον)
- Όταν γράφετε απαντήσεις ή νέα μηνύματα, τίθενται σε ουρά ως εκκρεμή
- Οι ψήφοι (ψηφοδέλτια δημοσκοπήσεων, ψήφοι υπέρ/κατά) τίθενται επίσης σε ουρά μαζί με τις απαντήσεις
- Κατά την έξοδο (ή κατά το άνοιγμα νέου αρχείου QWK), το SlyMail ζητά να αποθηκευτούν όλα τα εκκρεμή στοιχεία
- Δημιουργεί ένα τυπικό αρχείο `.rep` (αρχείο ZIP) για μεταφόρτωση στο BBS, που περιέχει:
  - `<BBSID>.MSG` — μηνύματα απαντήσεων σε τυπική μορφή QWK
  - `HEADERS.DAT` — εκτεταμένες κεφαλίδες QWKE για πεδία που υπερβαίνουν τους 25 χαρακτήρες
  - `VOTING.DAT` — εκκρεμείς ψήφοι σε μορφή INI συμβατή με Synchronet
- Το αρχείο REP αποθηκεύεται ως `<BBS-ID>.rep` στον διαμορφωμένο κατάλογο απαντήσεων (ή στον κατάλογο του αρχείου QWK)

### Απομακρυσμένα συστήματα (Ctrl-R)
Το SlyMail μπορεί να κατεβάσει πακέτα QWK απευθείας από απομακρυσμένα συστήματα μέσω FTP ή SFTP (SSH):
- Πατήστε **Ctrl-R** από τον περιηγητή αρχείων για να ανοίξετε τον κατάλογο απομακρυσμένων συστημάτων
- **Προσθήκη/Επεξεργασία/Διαγραφή** καταχωρήσεων απομακρυσμένων συστημάτων με: όνομα, κεντρικός υπολογιστής, θύρα, τύπος σύνδεσης (FTP ή SSH), όνομα χρήστη, κωδικός πρόσβασης, εναλλαγή παθητικού FTP και αρχική απομακρυσμένη διαδρομή
- **Περιήγηση σε απομακρυσμένους καταλόγους** με έναν περιηγητή αρχείων/καταλόγων παρόμοιο με τον τοπικό περιηγητή αρχείων — πλοήγηση μέσα σε καταλόγους, άνοδος με `..`, μετάβαση στη ρίζα με `/`
- **Λήψη αρχείων QWK** από το απομακρυσμένο σύστημα απευθείας στον υποκατάλογο `QWK` του καταλόγου δεδομένων SlyMail
- Οι καταχωρήσεις απομακρυσμένων συστημάτων αποθηκεύονται στο `remote_systems.json` στον κατάλογο δεδομένων SlyMail
- Η ημερομηνία/ώρα τελευταίας σύνδεσης παρακολουθείται για κάθε σύστημα
- Χρησιμοποιεί την εντολή `curl` του συστήματος για μεταφορές FTP και SFTP (χωρίς εξαρτήσεις βιβλιοθήκης κατά τη μεταγλώττιση)

### Ρυθμίσεις εφαρμογής
- Μόνιμες ρυθμίσεις αποθηκευμένες στο `slymail.ini` στον κατάλογο δεδομένων SlyMail (`~/.slymail` στο Linux/macOS/BSD, ή τον κατάλογο οικίας του χρήστη στα Windows)
- Ο κατάλογος δεδομένων SlyMail και ο υποκατάλογος `QWK` δημιουργούνται αυτόματα κατά την πρώτη εκτέλεση
- Ο προεπιλεγμένος κατάλογος αναζήτησης αρχείων QWK και αποθήκευσης πακέτων REP είναι το `~/.slymail/QWK`
- Θυμάται τον τελευταίο κατάλογο περιήγησης και το όνομα αρχείου QWK
- Πλήκτρο συντόμευσης Ctrl-L για φόρτωση διαφορετικού αρχείου QWK από τις προβολές συνεδρίου ή λίστας μηνυμάτων
- Διαμορφώσιμο πρόθεμα παράθεσης, πλάτος γραμμής παράθεσης, όνομα χρήστη
- Επιλογές αναγνώστη: εμφάνιση/απόκρυψη γραμμών kludge, γραμμών tear/origin, γραμμής κύλισης, αφαίρεση κωδικών ANSI
- Εναλλαγές κωδικών χαρακτηριστικών ανά BBS (Synchronet, WWIV, Celerity, Renegade, PCBoard/Wildcat) — επηρεάζουν τόσο τον αναγνώστη όσο και τον επεξεργαστή
- Κατάλογος εξόδου πακέτων REP

## Στιγμιότυπα οθόνης

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

## Ρυθμίσεις Synchronet για πακέτο QWK
Σε ένα Synchronet BBS, στις ρυθμίσεις πακέτου QWK, το SlyMail είναι (ή θα πρέπει να είναι) συμβατό με κωδικούς χρώματος Ctrl-A, VOTING.DAT, συνημμένα αρχεία και τη μορφή πακέτου QWKE. Το SlyMail θα πρέπει επίσης να είναι συμβατό με UTF-8. Για παράδειγμα:
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

## Κατασκευή

### Απαιτήσεις

**Linux / macOS / BSD:**
- Μεταγλωττιστής συμβατός με C++17 (GCC 8+, Clang 7+)
- Βιβλιοθήκη ανάπτυξης ncurses (`libncurses-dev` στο Debian/Ubuntu, `ncurses-devel` στο Fedora/RHEL)
- Εντολή `unzip` (για εξαγωγή πακέτων QWK)
- Εντολή `zip` (για δημιουργία πακέτων REP)
- Εντολή `curl` (για μεταφορές FTP/SFTP απομακρυσμένων συστημάτων — προαιρετικό, απαιτείται μόνο για τη λειτουργία απομακρυσμένων συστημάτων)

**Windows (Visual Studio 2022):**
- Visual Studio 2022 με το φόρτο εργασίας "Ανάπτυξη επιτραπέζιας εφαρμογής με C++"
- Windows SDK 10.0 (συμπεριλαμβάνεται στο VS)
- Δεν απαιτούνται επιπλέον βιβλιοθήκες — χρησιμοποιεί το ενσωματωμένο Win32 Console API για το UI τερματικού, και είτε `tar.exe` είτε PowerShell για χειρισμό ZIP πακέτων QWK/REP (δείτε σημείωση παρακάτω)

**Windows (MinGW/MSYS2):**
- MinGW-w64 ή MSYS2 με GCC (υποστήριξη C++17)
- Windows Console API (ενσωματωμένο)

> **Σημείωση — Χειρισμός ZIP QWK/REP στα Windows:** Το SlyMail ανιχνεύει κατά την εκτέλεση ποιο εργαλείο είναι διαθέσιμο και χρησιμοποιεί την καλύτερη επιλογή:
>
> - **`tar.exe` (προτιμώμενο):** Παρέχεται με τα Windows 10 έκδοση 1803 (Ενημέρωση Απριλίου 2018) και μεταγενέστερα, και με όλες τις εκδόσεις των Windows 11. Το `tar` διαβάζει αρχεία ZIP βάσει περιεχομένου και όχι επέκτασης αρχείου, επομένως τα πακέτα `.qwk` εξάγονται απευθείας και τα πακέτα `.rep` δημιουργούνται μέσω προσωρινού αρχείου `.zip` που στη συνέχεια μετονομάζεται. Δεν απαιτείται επιπλέον διαμόρφωση.
> - **PowerShell (εναλλακτικό):** Αν δεν βρεθεί το `tar.exe` στο PATH, το SlyMail χρησιμοποιεί το PowerShell ως εναλλακτικό. Για εξαγωγή χρησιμοποιεί την κλάση .NET `ZipFile` (`System.IO.Compression`) και όχι το `Expand-Archive`, διότι το `Expand-Archive` απορρίπτει επεκτάσεις αρχείων που δεν είναι `.zip` ακόμα και αν το αρχείο είναι έγκυρο αρχείο ZIP. Για δημιουργία πακέτων REP χρησιμοποιεί το `Compress-Archive`, γράφοντας επίσης σε προσωρινό αρχείο `.zip` που στη συνέχεια μετονομάζεται σε `.rep`.

### Κατασκευή σε Linux/macOS/BSD

```bash
make
```

Αυτό κατασκευάζει δύο προγράμματα:
- `slymail` - η κύρια εφαρμογή αναγνώστη QWK
- `config` - το αυτόνομο βοηθητικό πρόγραμμα διαμόρφωσης

### Κατασκευή με σύμβολα εντοπισμού σφαλμάτων

```bash
make debug
```

### Εγκατάσταση (προαιρετικό)

```bash
sudo make install    # Installs slymail and config to /usr/local/bin/
sudo make uninstall  # Remove
```

### Κατασκευή στα Windows με Visual Studio 2022

Ανοίξτε το αρχείο λύσης στο Visual Studio 2022:

```
vs\SlyMail.sln
```

Ή κατασκευάστε από τη γραμμή εντολών χρησιμοποιώντας MSBuild:

```powershell
# Release build (output in vs\x64\Release\)
msbuild vs\SlyMail.sln /p:Configuration=Release /p:Platform=x64

# Debug build (output in vs\x64\Debug\)
msbuild vs\SlyMail.sln /p:Configuration=Debug /p:Platform=x64
```

Αυτό κατασκευάζει δύο εκτελέσιμα αρχεία:
- `x64\Release\slymail.exe` — ο κύριος αναγνώστης QWK
- `x64\Release\config.exe` — το αυτόνομο βοηθητικό πρόγραμμα διαμόρφωσης

Η λύση περιέχει δύο έργα (`SlyMail.vcxproj` και `Config.vcxproj`) που στοχεύουν x64, C++17, με το σύνολο εργαλείων MSVC v143.

### Κατασκευή στα Windows (MinGW/MSYS2)

```bash
make
```

Το Makefile ανιχνεύει αυτόματα την πλατφόρμα και χρησιμοποιεί την κατάλληλη υλοποίηση τερματικού:
- **Linux/macOS/BSD**: ncurses (`terminal_ncurses.cpp`)
- **Windows**: conio + Win32 Console API (`terminal_win32.cpp`)

## Χρήση

```bash
# Launch SlyMail with file browser
./slymail

# Open a specific QWK packet
./slymail MYBBS.qwk

# Run the standalone configuration utility
./config
```

### Πρόγραμμα διαμόρφωσης

Το βοηθητικό πρόγραμμα `config` παρέχει μια αυτόνομη διεπαφή βασισμένη σε κείμενο για τη διαμόρφωση ρυθμίσεων SlyMail χωρίς να ανοίξετε την κύρια εφαρμογή. Προσφέρει τέσσερις κατηγορίες διαμόρφωσης:

- **Ρυθμίσεις επεξεργαστή** - Όλες οι ίδιες ρυθμίσεις που είναι διαθέσιμες μέσω Ctrl-U στον επεξεργαστή (στυλ επεξεργαστή, γραμμές ετικέτας, ορθογραφικός έλεγχος, επιλογές παράθεσης κ.λπ.)
- **Ρυθμίσεις αναγνώστη** - Εναλλαγή γραμμών kludge, γραμμών tear, γραμμής κύλισης, αφαίρεσης ANSI, λειτουργίας γραμμής φωτός, αντίστροφης σειράς και εναλλαγών κωδικών χαρακτηριστικών (ενεργοποίηση/απενεργοποίηση ανά BBS)
- **Ρυθμίσεις θέματος** - Επιλογή αρχείων θεμάτων χρωμάτων Ice και DCT από τον κατάλογο `config_files/`
- **Γενικές ρυθμίσεις** - Ορισμός ονόματος για απαντήσεις και κατάλογος εξόδου πακέτων REP

Οι ρυθμίσεις αποθηκεύονται αυτόματα κατά την έξοδο από κάθε κατηγορία. Τόσο το SlyMail όσο και το βοηθητικό πρόγραμμα config διαβάζουν και γράφουν στο ίδιο αρχείο ρυθμίσεων.

### Συνδέσεις πλήκτρων

#### Περιηγητής αρχείων
| Πλήκτρο | Ενέργεια |
|-----|--------|
| Up/Down | Πλοήγηση σε αρχεία και καταλόγους |
| Enter | Άνοιγμα καταλόγου / Επιλογή αρχείου QWK |
| Ctrl-R | Άνοιγμα καταλόγου απομακρυσμένων συστημάτων |
| Q / ESC | Έξοδος |

#### Λίστα συνεδρίων
| Πλήκτρο | Ενέργεια |
|-----|--------|
| Up/Down | Πλοήγηση σε συνέδρια |
| Enter | Άνοιγμα επιλεγμένου συνεδρίου |
| V | Προβολή δημοσκοπήσεων/ψήφων στο πακέτο |
| O / Ctrl-L | Άνοιγμα διαφορετικού αρχείου QWK |
| S / Ctrl-U | Ρυθμίσεις |
| Q / ESC | Έξοδος από το SlyMail |
| ? / F1 | Βοήθεια |

#### Λίστα μηνυμάτων
| Πλήκτρο | Ενέργεια |
|-----|--------|
| Up/Down | Πλοήγηση σε μηνύματα |
| Enter / R | Ανάγνωση επιλεγμένου μηνύματος |
| N | Σύνταξη νέου μηνύματος |
| G | Μετάβαση σε αριθμό μηνύματος |
| Ctrl-L | Άνοιγμα διαφορετικού αρχείου QWK |
| S / Ctrl-U | Ρυθμίσεις |
| C / ESC | Επιστροφή στη λίστα συνεδρίων |
| Q | Έξοδος |
| ? / F1 | Βοήθεια |

#### Αναγνώστης μηνυμάτων
| Πλήκτρο | Ενέργεια |
|-----|--------|
| Up/Down | Κύλιση μηνύματος |
| Left/Right | Προηγούμενο / Επόμενο μήνυμα |
| F / L | Πρώτο / Τελευταίο μήνυμα |
| R | Απάντηση σε μήνυμα |
| V | Ψηφοφορία (ψήφος υπέρ/κατά ή ψηφοδέλτιο δημοσκόπησης) |
| D / Ctrl-D | Λήψη συνημμένων αρχείων |
| H | Εμφάνιση πληροφοριών κεφαλίδας μηνύματος |
| S / Ctrl-U | Ρυθμίσεις |
| C / Q / ESC | Επιστροφή στη λίστα μηνυμάτων |
| ? / F1 | Βοήθεια |

#### Επεξεργαστής μηνυμάτων
| Πλήκτρο | Ενέργεια |
|-----|--------|
| ESC | Μενού επεξεργαστή (Αποθήκευση, Ματαίωση κ.λπ.) |
| Ctrl-U | Παράθυρο ρυθμίσεων χρήστη |
| Ctrl-Q | Άνοιγμα/κλείσιμο παραθύρου παράθεσης |
| Ctrl-K | Επιλογέας χρώματος (εισαγωγή κωδικού χρώματος ANSI στον δρομέα) |
| Ctrl-G | Εισαγωγή γραφικού χαρακτήρα CP437 με κωδικό |
| Ctrl-W | Αναζήτηση λέξης/κειμένου |
| Ctrl-S | Αλλαγή θέματος |
| Ctrl-D | Διαγραφή τρέχουσας γραμμής |
| Ctrl-Z | Αποθήκευση μηνύματος |
| Ctrl-A | Ματαίωση μηνύματος |
| F1 | Οθόνη βοήθειας |
| Insert | Εναλλαγή λειτουργίας Εισαγωγής/Αντικατάστασης |

#### Παράθυρο παράθεσης
| Πλήκτρο | Ενέργεια |
|-----|--------|
| Up/Down | Πλοήγηση σε γραμμές παράθεσης |
| Enter | Εισαγωγή επιλεγμένης γραμμής παράθεσης |
| Ctrl-Q / ESC | Κλείσιμο παραθύρου παράθεσης |

## Αρχιτεκτονική

Το SlyMail χρησιμοποιεί ένα επίπεδο αφαίρεσης πλατφόρμας για τη διεπαφή χρήστη κειμένου:

```
ITerminal (abstract base class)
    ├── NCursesTerminal  (Linux/macOS/BSD - ncurses)
    └── Win32Terminal    (Windows - conio + Win32 Console API)
```

Οι χαρακτήρες σχεδίασης πλαισίων και ειδικοί χαρακτήρες CP437 ορίζονται στο `cp437defs.h` και αποδίδονται μέσω της μεθόδου `putCP437()`, η οποία αντιστοιχεί κωδικούς CP437 σε ισοδύναμα εγγενή της πλατφόρμας (χαρακτήρες ACS στο ncurses, άμεσα byte CP437 στα Windows).

### Αρχεία πηγαίου κώδικα

| Αρχείο | Περιγραφή |
|------|-------------|
| `terminal.h` | Αφηρημένη διεπαφή `ITerminal`, σταθερές πλήκτρων/χρωμάτων, εργοστάσιο |
| `terminal_ncurses.cpp` | Υλοποίηση ncurses με αντιστοίχιση CP437 σε ACS |
| `terminal_win32.cpp` | Υλοποίηση Windows Console API + conio |
| `cp437defs.h` | Ορισμοί χαρακτήρων IBM Code Page 437 |
| `colors.h` | Ορισμοί χρωματικών σχημάτων (Ice, DCT, αναγνώστης, λίστα) |
| `theme.h` | Αναλυτής αρχείων ρυθμίσεων θέματος (κωδικοί χαρακτηριστικών τύπου Synchronet) |
| `ui_common.h` | Κοινά βοηθητικά UI (παράθυρα διαλόγου, εισαγωγή κειμένου, γραμμή κύλισης κ.λπ.) |
| `qwk.h` / `qwk.cpp` | Αναλυτής και δημιουργός πακέτων QWK/REP (QWKE, συνημμένα, ψηφοφορία) |
| `bbs_colors.h` / `bbs_colors.cpp` | Αναλυτής κωδικών χρώματος/χαρακτηριστικών BBS (ANSI, Synchronet, WWIV, PCBoard, Celerity, Renegade) |
| `utf8_util.h` / `utf8_util.cpp` | Βοηθητικά UTF-8 (επικύρωση, πλάτος εμφάνισης, μετατροπή CP437 σε UTF-8) |
| `voting.h` / `voting.cpp` | Αναλυτής VOTING.DAT, καταμέτρηση ψήφων, UI εμφάνισης δημοσκοπήσεων |
| `remote_systems.h` / `remote_systems.cpp` | Κατάλογος απομακρυσμένων συστημάτων, περιήγηση FTP/SFTP, διατήρηση JSON, βοηθητικά καταλόγου οικίας |
| `settings.h` | Διατήρηση ρυθμίσεων χρήστη |
| `settings_dialog.h` | Παράθυρα διαλόγου ρυθμίσεων (επεξεργαστής, αναγνώστης, εναλλαγές κωδικών χαρακτηριστικών) |
| `file_browser.h` | Περιηγητής και επιλογέας αρχείων QWK |
| `msg_list.h` | Προβολές λίστας συνεδρίων και μηνυμάτων |
| `msg_reader.h` | Αναγνώστης μηνυμάτων (τύπου DDMsgReader) με UI ψηφοφορίας και συνημμένων |
| `msg_editor.h` | Επεξεργαστής μηνυμάτων (τύπου SlyEdit Ice/DCT) με επιλογέα χρωμάτων |
| `main.cpp` | Σημείο εισόδου εφαρμογής SlyMail και κύριος βρόχος |
| `config.cpp` | Αυτόνομο βοηθητικό πρόγραμμα διαμόρφωσης |

## Διαμόρφωση

### Αρχείο ρυθμίσεων

Οι ρυθμίσεις αποθηκεύονται σε αρχείο INI με όνομα `slymail.ini` στον ίδιο κατάλογο με το εκτελέσιμο αρχείο του SlyMail. Αυτό το αρχείο είναι κοινό μεταξύ του SlyMail και του βοηθητικού προγράμματος `config`. Το αρχείο έχει σχόλια με περιγραφές κάθε ρύθμισης.

Παράδειγμα `slymail.ini`:
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

### Αρχεία θεμάτων

Τα χρωματικά θέματα είναι αρχεία `.ini` στον κατάλογο `config_files/`:

**Θέματα Ice** (`EditorIceColors_*.ini`):
- BlueIce (προεπιλογή), EmeraldCity, FieryInferno, Fire-N-Ice, GeneralClean, GenericBlue, PurpleHaze, ShadesOfGrey

**Θέματα DCT** (`EditorDCTColors_*.ini`):
- Default (προεπιλογή), Default-Modified, Midnight

Οι τιμές χρωμάτων θέματος χρησιμοποιούν συμπαγή μορφή που προέρχεται από κωδικούς χαρακτηριστικών Synchronet:
- `n` = κανονικό (επαναφορά)
- Προσκήνιο: `k`=μαύρο, `r`=κόκκινο, `g`=πράσινο, `y`=κίτρινο, `b`=μπλε, `m`=ματζέντα, `c`=κυανό, `w`=λευκό
- `h` = υψηλή/φωτεινή ένταση
- Ψηφίο φόντου: `0`=μαύρο, `1`=κόκκινο, `2`=πράσινο, `3`=καφέ, `4`=μπλε, `5`=ματζέντα, `6`=κυανό, `7`=ανοιχτό γκρι

Παράδειγμα: `nbh` = κανονικό μπλε φωτεινό, `n4wh` = φωτεινό λευκό σε μπλε φόντο

### Γραμμές ετικέτας

Οι γραμμές ετικέτας είναι σύντομες παραθέσεις ή αποφθέγματα που προσαρτώνται στο τέλος μηνυμάτων κατά την αποθήκευση. Η λειτουργία γραμμών ετικέτας μπορεί να ενεργοποιηθεί μέσω Ctrl-U στον επεξεργαστή ή στο βοηθητικό πρόγραμμα `config`.

Οι γραμμές ετικέτας αποθηκεύονται στο `tagline_files/taglines.txt`, μία ανά γραμμή. Οι γραμμές που αρχίζουν με `#` ή `;` αντιμετωπίζονται ως σχόλια και αγνοούνται. Κατά την αποθήκευση μηνύματος με ενεργοποιημένες τις γραμμές ετικέτας, ο χρήστης καλείται να επιλέξει μια συγκεκριμένη γραμμή ετικέτας ή να επιλέξει μία τυχαία. Οι επιλεγμένες γραμμές ετικέτας προσαρτώνται στο μήνυμα με πρόθεμα `...` (π.χ. `...To err is human, to really foul things up requires a computer.`).

### Ορθογραφικός έλεγχος

Το SlyMail περιλαμβάνει ενσωματωμένο ορθογραφικό έλεγχο που χρησιμοποιεί αρχεία λεξικού απλού κειμένου. Ο ορθογραφικός έλεγχος μπορεί να διαμορφωθεί ώστε να εμφανίζει προτροπή κατά την αποθήκευση μέσω Ctrl-U στον επεξεργαστή ή στο βοηθητικό πρόγραμμα `config`.

**Τα αρχεία λεξικού** είναι αρχεία απλού κειμένου (μία λέξη ανά γραμμή) αποθηκευμένα στο `dictionary_files/`. Πολλά λεξικά μπορούν να επιλεγούν ταυτόχρονα για συνδυασμένη κάλυψη λέξεων. Το SlyMail παρέχεται με:
- `dictionary_en.txt` - Αγγλικά (γενικά, ~130Κ λέξεις)
- `dictionary_en-US-supplemental.txt` - Αμερικανικό αγγλικό συμπλήρωμα
- `dictionary_en-GB-supplemental.txt` - Βρετανικό αγγλικό συμπλήρωμα
- `dictionary_en-AU-supplemental.txt` - Αυστραλιανό αγγλικό συμπλήρωμα
- `dictionary_en-CA-supplemental.txt` - Καναδικό αγγλικό συμπλήρωμα

Όταν ενεργοποιείται ο ορθογραφικός έλεγχος, ο ελεγκτής σαρώνει το μήνυμα για ανορθόγραφες λέξεις και εμφανίζει ένα διαδραστικό παράθυρο διαλόγου για κάθε μία, προσφέροντας επιλογές για **Α**ντικατάσταση της λέξης, **Π**αράλειψη, **Π**ροσθήκη (μέλλον) ή **Τ**ερματισμό ελέγχου.

## Αναγνωρίσεις

- Το UI εμπνεύστηκε από τα [DDMsgReader](https://github.com/SynchronetBBS/sbbs) και [SlyEdit](https://github.com/SynchronetBBS/sbbs) του [Nightfox (Eric Oulashin)](https://github.com/EricOulashin)
- Η συμβατότητα μορφής QWK βασίστηκε στον πηγαίο κώδικα του [Synchronet BBS](https://www.synchro.net/)
- Ορισμοί χαρακτήρων CP437 από το Synchronet

## Άδεια χρήσης

Αυτό το έργο είναι λογισμικό ανοιχτού κώδικα.
