*(Traduction automatique — pour la version originale en anglais, voir [README.md](README.md))*

# SlyMail

Il s'agit d'un lecteur de courrier hors ligne multiplateforme en mode texte pour le format de paquets [QWK](https://en.wikipedia.org/wiki/QWK_(file_format)). Le format de paquets QWK était/est souvent utilisé pour échanger du courrier sur les [systèmes de bulletin électronique](https://en.wikipedia.org/wiki/Bulletin_board_system).

SlyMail fournit une interface complète pour lire et répondre aux messages provenant de paquets de courrier QWK de BBS (Bulletin Board System). Son interface utilisateur est inspirée de [Digital Distortion Message Reader (DDMsgReader)](https://github.com/SynchronetBBS/sbbs/tree/master/xtrn/DDMsgReader) pour la lecture des messages et de [SlyEdit](https://github.com/SynchronetBBS/sbbs/tree/master/exec) pour l'édition des messages, tous deux créés à l'origine pour [Synchronet BBS](https://www.synchro.net/).

SlyMail a été créé avec l'aide de Claude AI.

## Fonctionnalités

### Prise en charge des paquets QWK
- Ouvre et lit les paquets de courrier QWK standard (fichiers .qwk)
- Analyse CONTROL.DAT, MESSAGES.DAT et les fichiers d'index NDX
- Prise en charge complète de QWKE (QWK étendu) via HEADERS.DAT — correspondance basée sur le décalage pour des champs étendus précis To/From/Subject, indicateur UTF-8 et RFC822 Message-ID
- Analyse de kludge de corps QWKE (`To:`, `From:`, `Subject:` au début du message)
- Gère la numérotation des conférences de style Synchronet
- Crée des paquets de réponse REP (fichiers .rep) pour les téléverser sur le BBS, incluant HEADERS.DAT pour les champs étendus et VOTING.DAT pour les votes en attente
- Prend en charge l'encodage flottant au format binaire Microsoft (MBF) dans les fichiers NDX
- Mémorise le dernier fichier QWK ouvert et le répertoire entre les sessions

### Lecture des messages (style DDMsgReader)
- Liste des conférences avec le nombre de messages
- Liste de messages défilable avec navigation par barre lumineuse
- Lecteur de messages complet avec affichage d'en-tête (De, À, Sujet, Date)
- Mise en évidence des lignes de citation (prend en charge les citations multi-niveaux)
- Affichage des lignes kludge (optionnel)
- Indicateur de barre de défilement
- Navigation au clavier : Premier/Dernier/Message suivant/précédent, Page haut/bas
- Écrans d'aide accessibles avec `?` ou `F1` dans toutes les vues

### Prise en charge des codes couleur et d'attribut BBS
SlyMail interprète les codes couleur/attribut de plusieurs logiciels BBS, les rendant sous forme de texte coloré à la fois dans le lecteur de messages et dans l'éditeur de messages. Formats pris en charge :
- **Codes d'échappement ANSI** — toujours activés ; séquences SGR standard (ESC[...m) pour le premier plan, l'arrière-plan, le gras
- **Codes Ctrl-A de Synchronet** — `\x01` + caractère d'attribut (par ex., `\x01c` pour cyan, `\x01h` pour brillant)
- **Codes de cœur WWIV** — `\x03` + chiffre de 0 à 9
- **Codes @X PCBoard/Wildcat** — `@X##` où les deux chiffres hexadécimaux encodent les couleurs d'arrière-plan et de premier plan
- **Codes de tube Celerity** — `|` + lettre (par ex., `|c` pour cyan, `|W` pour blanc brillant)
- **Codes de tube Renegade** — `|` + nombre à deux chiffres de 00 à 31

Chaque type de code BBS peut être activé ou désactivé individuellement via la sous-boîte de dialogue **Bascules de code d'attribut** dans les paramètres du lecteur ou l'utilitaire `config`. Ces bascules affectent à la fois le lecteur et l'éditeur. Une option séparée **Supprimer les codes ANSI** supprime toutes les séquences ANSI des messages lorsqu'elle est activée.

### Pièces jointes
- Détecte les pièces jointes référencées via des lignes kludge `@ATTACH:` dans les corps de messages
- Affiche un indicateur **[ATT]** dans l'en-tête du message lorsque des pièces jointes sont présentes
- Appuyez sur **D** ou **Ctrl-D** dans le lecteur pour télécharger les pièces jointes — affiche une liste de fichiers avec les tailles et demande un répertoire de destination

### Votes et sondages (Synchronet QWKE)
SlyMail prend en charge l'extension Synchronet VOTING.DAT pour les sondages et les votes sur les messages :
- **Sondages** : Les messages identifiés comme sondages affichent leurs options de réponse avec les décomptes de votes et les barres de pourcentage. Appuyez sur **V** pour ouvrir une boîte de dialogue de bulletin de vote où vous pouvez basculer les sélections de réponse et soumettre votre vote.
- **Votes haut/bas** : Pour les messages réguliers (non-sondages), appuyez sur **V** pour voter positivement ou négativement. Les décomptes de votes actuels et le score sont affichés dans l'en-tête du message.
- **Décomptes de votes** : L'en-tête du message affiche les décomptes de votes positifs/négatifs et le score net, avec un indicateur si vous avez déjà voté.
- **File d'attente des votes** : Les votes sont mis en file d'attente avec les réponses aux messages et écrits dans VOTING.DAT dans le paquet REP pour le téléversement sur le BBS.
- **Navigateur de sondages** : Appuyez sur **V** depuis la liste des conférences pour parcourir tous les sondages du paquet.

### Prise en charge UTF-8
- Détecte le contenu UTF-8 dans les messages (via l'indicateur `Utf8` de HEADERS.DAT et la détection automatique des séquences d'octets UTF-8)
- Affiche correctement les caractères UTF-8 sur les terminaux compatibles
- Affiche un indicateur **[UTF8]** dans l'en-tête du message pour les messages UTF-8
- Enregistre les nouveaux messages avec un encodage approprié
- Conversion CP437 vers UTF-8 pour les contenus BBS hérités
- Définit la locale sur Linux/macOS/BSD (`setlocale(LC_ALL, "")`) et la page de code UTF-8 sur Windows pour un rendu correct du terminal

### Éditeur de messages (inspiré de SlyEdit)
- **Deux modes visuels** : Ice et DCT, chacun avec des schémas de couleurs et des mises en page distinctes
- **Mode aléatoire** : Sélectionne aléatoirement Ice ou DCT à chaque session d'édition
- **Couleurs de bordure alternantes** : Les caractères de bordure alternent aléatoirement entre deux couleurs de thème, correspondant au style visuel de SlyEdit
- **Prise en charge des thèmes** : Thèmes de couleurs configurables chargés depuis des fichiers .ini
- Éditeur de texte plein écran avec retour à la ligne automatique
- Fenêtre de citation pour sélectionner et insérer du texte cité (Ctrl-Q pour ouvrir/fermer)
- Composition de réponses et de nouveaux messages
- Menu ESC pour enregistrer, annuler, basculer l'insertion/le remplacement et plus encore
- **Sélecteur de couleur Ctrl-K** : Ouvre une boîte de dialogue pour sélectionner les couleurs de premier plan et d'arrière-plan, en insérant un code d'échappement ANSI à la position du curseur. Prend en charge 16 couleurs de premier plan (8 normales + 8 brillantes) et 8 arrière-plans, avec un aperçu en direct. Appuyez sur **N** pour insérer un code de réinitialisation.
- **Rendu avec reconnaissance des couleurs** : La zone d'édition rend les codes ANSI et d'attribut BBS en ligne, de sorte que le texte coloré s'affiche pendant que vous tapez
- **Boîte de dialogue des paramètres utilisateur Ctrl-U** pour configurer les préférences de l'éditeur à la volée
- **Invites oui/non spécifiques au style** : Le mode Ice utilise une invite en ligne en bas de l'écran ; le mode DCT utilise une boîte de dialogue centrée avec des couleurs thématiques

### Paramètres de l'éditeur (via Ctrl-U)
- **Choisir le mode d'interface** : Boîte de dialogue pour basculer entre les styles Ice, DCT et Aléatoire (prend effet immédiatement)
- **Sélectionner le fichier de thème** : Choisissez parmi les thèmes de couleurs Ice ou DCT disponibles
- **Taglines** : Lorsqu'il est activé, invite à sélectionner une tagline lors de l'enregistrement (depuis `tagline_files/taglines.txt`)
- **Dictionnaire/dictionnaires de vérification orthographique** : Sélectionnez les dictionnaires à utiliser
- **Demander le correcteur orthographique lors de l'enregistrement** : Lorsqu'il est activé, propose de vérifier l'orthographe avant d'enregistrer
- **Adapter les lignes de citation à la largeur du terminal** : Effectuer un retour à la ligne automatique sur les lignes citées
- **Citer avec les initiales de l'auteur** : Faire précéder les lignes de citation des initiales de l'auteur (par ex., `MP> `)
- **Indenter les lignes de citation contenant des initiales** : Ajouter un espace avant les initiales (par ex., ` MP> `)
- **Supprimer les espaces des lignes de citation** : Retirer les espaces blancs en tête du texte cité

### Thèmes de couleurs
- Les fichiers de thème sont des fichiers de configuration (`.ini`) dans le répertoire `config_files/`
- Thèmes Ice : `EditorIceColors_*.ini` (BlueIce, EmeraldCity, FieryInferno, etc.)
- Thèmes DCT : `EditorDCTColors_*.ini` (Default, Default-Modified, Midnight)
- Les couleurs du thème utilisent un format simple : lettre de premier plan (`r`/`g`/`b`/`c`/`y`/`m`/`w`/`k`), `h` optionnel pour brillant, chiffre d'arrière-plan optionnel (`0`-`7`)
- Les thèmes contrôlent toutes les couleurs des éléments de l'interface : bordures, étiquettes, valeurs, fenêtre de citation, barre d'aide, boîtes de dialogue oui/non

### Vérificateur orthographique
- Vérificateur orthographique intégré utilisant des fichiers de dictionnaire en texte brut
- Livré avec des dictionnaires anglais (suppléments en, en-US, en-GB, en-AU, en-CA)
- Boîte de dialogue de correction interactive : Remplacer, Passer ou Quitter
- Fichiers de dictionnaire stockés dans le répertoire `dictionary_files/`

### Taglines
- Les fichiers de taglines sont stockés dans le répertoire `tagline_files/`
- Le fichier de taglines par défaut est `tagline_files/taglines.txt`, une tagline par ligne
- Les lignes commençant par `#` ou `;` sont traitées comme des commentaires et ignorées
- Sélectionnez une tagline spécifique ou choisissez-en une au hasard lors de l'enregistrement d'un message
- Les taglines sont ajoutées aux messages avec le préfixe `...`

### Création de paquets REP
- Actuellement, seul ZIP est pris en charge (je souhaite ajouter la prise en charge d'autres types de compression à l'avenir)
- Lorsque vous rédigez des réponses ou de nouveaux messages, ils sont mis en file d'attente comme en attente
- Les votes (bulletins de sondage, votes haut/bas) sont également mis en file d'attente avec les réponses
- À la fermeture (ou lors de l'ouverture d'un nouveau fichier QWK), SlyMail vous invite à enregistrer tous les éléments en attente
- Crée un fichier `.rep` standard (archive ZIP) pour le téléversement sur le BBS, contenant :
  - `<BBSID>.MSG` — messages de réponse au format QWK standard
  - `HEADERS.DAT` — en-têtes étendus QWKE pour les champs dépassant 25 caractères
  - `VOTING.DAT` — votes en attente au format INI compatible Synchronet
- Le fichier REP est enregistré sous `<BBS-ID>.rep` dans le répertoire de réponses configuré (ou le répertoire du fichier QWK)

### Systèmes distants (Ctrl-R)
SlyMail peut télécharger des paquets QWK directement depuis des systèmes distants via FTP ou SFTP (SSH) :
- Appuyez sur **Ctrl-R** depuis le navigateur de fichiers pour ouvrir le répertoire des systèmes distants
- **Ajouter/Modifier/Supprimer** des entrées de systèmes distants avec : nom, hôte, port, type de connexion (FTP ou SSH), nom d'utilisateur, mot de passe, bascule FTP passif et chemin distant initial
- **Parcourir les répertoires distants** avec un navigateur de fichiers/répertoires similaire au navigateur de fichiers local — naviguer dans les répertoires, remonter avec `..`, aller à la racine avec `/`
- **Télécharger des fichiers QWK** depuis le système distant directement dans le sous-répertoire `QWK` du répertoire de données SlyMail
- Les entrées des systèmes distants sont sauvegardées dans `remote_systems.json` dans le répertoire de données SlyMail
- La dernière date/heure de connexion est suivie pour chaque système
- Utilise la commande `curl` du système pour les transferts FTP et SFTP (aucune dépendance de bibliothèque à la compilation)

### Paramètres de l'application
- Les paramètres persistants sont enregistrés dans `slymail.ini` dans le répertoire de données SlyMail (`~/.slymail` sous Linux/macOS/BSD, ou le répertoire personnel de l'utilisateur sous Windows)
- Le répertoire de données SlyMail et son sous-répertoire `QWK` sont créés automatiquement au premier démarrage
- Le répertoire de navigation des fichiers QWK et d'enregistrement des paquets REP par défaut est `~/.slymail/QWK`
- Mémorise le dernier répertoire parcouru et le nom du fichier QWK
- Raccourci clavier Ctrl-L pour charger un autre fichier QWK depuis les vues de liste de conférences ou de messages
- Préfixe de citation, largeur de ligne de citation, nom d'utilisateur configurables
- Options du lecteur : afficher/masquer les lignes kludge, les lignes de déchirement/origine, la barre de défilement, supprimer les codes ANSI
- Bascules de code d'attribut par BBS (Synchronet, WWIV, Celerity, Renegade, PCBoard/Wildcat) — affectent à la fois le lecteur et l'éditeur
- Répertoire de sortie des paquets REP

## Captures d'écran

<p align="center">
	<a href="screenshots/SlyMail_01_OpeningScreen.png" target='_blank'><img src="screenshots/SlyMail_01_OpeningScreen.png" alt="Écran d'ouverture" width="800"></a>
	<a href="screenshots/SlyMail_02_File_Chooser.png" target='_blank'><img src="screenshots/SlyMail_02_File_Chooser.png" alt="Sélecteur de fichiers : Recherche d'un fichier QWK" width="800"></a>
	<a href="screenshots/SlyMail_03_remote_system_list.png" target='_blank'><img src="screenshots/SlyMail_03_remote_system_list.png" alt="Liste des systèmes distants" width="800"></a>
	<a href="screenshots/SlyMail_04_Remote_System_Edit.png" target='_blank'><img src="screenshots/SlyMail_04_Remote_System_Edit.png" alt="Modification d'un système distant" width="800"></a>
	<a href="screenshots/SlyMail_05_Remote_System_Browsing.png" target='_blank'><img src="screenshots/SlyMail_05_Remote_System_Browsing.png" alt="Navigation sur un système distant" width="800"></a>
	<a href="screenshots/SlyMail_06_msg_area_list.png" target='_blank'><img src="screenshots/SlyMail_06_msg_area_list.png" alt="Liste des zones de messages" width="800"></a>
	<a href="screenshots/SlyMail_07_msg_list.png" target='_blank'><img src="screenshots/SlyMail_07_msg_list.png" alt="Liste des messages" width="800"></a>
	<a href="screenshots/SlyMail_08_reading_msg.png" target='_blank'><img src="screenshots/SlyMail_08_reading_msg.png" alt="Lecture d'un message" width="800"></a>
	<a href="screenshots/SlyMail_09_msg_edit_start.png" target='_blank'><img src="screenshots/SlyMail_09_msg_edit_start.png" alt="Début de la modification d'un message" width="800"></a>
	<a href="screenshots/SlyMail_10_quote_line_selection.png" target='_blank'><img src="screenshots/SlyMail_10_quote_line_selection.png" alt="Éditeur : Sélection de ligne de citation" width="800"></a>
	<a href="screenshots/SlyMail_11_writing_reply_msg.png" target='_blank'><img src="screenshots/SlyMail_11_writing_reply_msg.png" alt="Éditeur : Modification d'un message" width="800"></a>
	<a href="screenshots/SlyMail_12_editor_color_picker.png" target='_blank'><img src="screenshots/SlyMail_12_editor_color_picker.png" alt="Éditeur : Sélecteur de couleur" width="800"></a>
	<a href="screenshots/SlyMail_13_Sync_poll_msg.png" target='_blank'><img src="screenshots/SlyMail_13_Sync_poll_msg.png" alt="Message de sondage Synchronet" width="800"></a>
	<a href="screenshots/SlyMail_14_reader_settings.png" target='_blank'><img src="screenshots/SlyMail_14_reader_settings.png" alt="Paramètres du lecteur" width="800"></a>
	<a href="screenshots/SlyMail_15_editor_settings.png" target='_blank'><img src="screenshots/SlyMail_15_editor_settings.png" alt="Paramètres de l'éditeur" width="800"></a>
	<a href="screenshots/SlyMail_16_msg_search.png" target='_blank'><img src="screenshots/SlyMail_16_msg_search.png" alt="Recherche de messages" width="800"></a>
	<a href="screenshots/SlyMail_17_Advanced_msg_search.png" target='_blank'><img src="screenshots/SlyMail_17_Advanced_msg_search.png" alt="Recherche avancée de messages" width="800"></a>
	<a href="screenshots/SlyMail_18_advanced_msg_search_date_picker" target='_blank'><img src="screenshots/SlyMail_18_advanced_msg_search_date_picker" alt="Sélecteur de date dans la recherche avancée de messages" width="800"></a>
	<a href="screenshots/SlyMail_19_config_program.png" target='_blank'><img src="screenshots/SlyMail_19_config_program.png" alt="Programme de configuration" width="800"></a>
	<a href="screenshots/SlyMail_20_reading_ANSI_art.png" target='_blank'><img src="screenshots/SlyMail_20_reading_ANSI_art.png" alt="Lecture d'un message avec de l'art ANSI" width="800"></a>
	<a href="screenshots/SlyMail_21_reading_ANSI_art.png" target='_blank'><img src="screenshots/SlyMail_21_reading_ANSI_art.png" alt="Lecture d'un message avec de l'art ANSI" width="800"></a>
	<a href="screenshots/SlyMail_22_reading_ANSI_art.png" target='_blank'><img src="screenshots/SlyMail_22_reading_ANSI_art.png" alt="Lecture d'un message avec de l'art ANSI" width="800"></a>
	<a href="screenshots/SlyMail_23_reading_ANSI_art.png" target='_blank'><img src="screenshots/SlyMail_23_reading_ANSI_art.png" alt="Lecture d'un message avec de l'art ANSI" width="800"></a>
</p>

## Paramètres Synchronet pour un paquet QWK
Sur un BBS Synchronet, dans les paramètres du paquet QWK, SlyMail est (ou devrait être) compatible avec les codes couleur Ctrl-A, VOTING.DAT, les pièces jointes et le format de paquet QWKE. SlyMail devrait également être compatible avec UTF-8. Par exemple :
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

## Compilation

### Prérequis

**Linux / macOS / BSD :**
- Compilateur compatible C++17 (GCC 8+, Clang 7+)
- Bibliothèque de développement ncurses (`libncurses-dev` sur Debian/Ubuntu, `ncurses-devel` sur Fedora/RHEL)
- Commande `unzip` (pour extraire les paquets QWK)
- Commande `zip` (pour créer les paquets REP)
- Commande `curl` (pour les transferts FTP/SFTP de systèmes distants — optionnelle, uniquement nécessaire pour la fonctionnalité des systèmes distants)

**Windows (Visual Studio 2022) :**
- Visual Studio 2022 avec la charge de travail « Développement Desktop en C++ »
- Windows SDK 10.0 (inclus avec VS)
- Aucune bibliothèque supplémentaire requise — utilise l'API Console Win32 intégrée pour l'interface terminal, et soit `tar.exe` soit PowerShell pour la gestion des ZIP des paquets QWK/REP (voir la note ci-dessous)

**Windows (MinGW/MSYS2) :**
- MinGW-w64 ou MSYS2 avec GCC (prise en charge de C++17)
- API Console Windows (intégrée)

> **Note — Gestion des ZIP QWK/REP sous Windows :** SlyMail détecte à l'exécution quel outil est disponible et utilise la meilleure option :
>
> - **`tar.exe` (préféré) :** Livré avec Windows 10 version 1803 (mise à jour d'avril 2018) et versions ultérieures, ainsi qu'avec toutes les versions de Windows 11. `tar` lit les fichiers ZIP par leur contenu plutôt que par leur extension, donc les paquets `.qwk` s'extraient directement et les paquets `.rep` sont créés via un fichier `.zip` temporaire qui est ensuite renommé. Aucune configuration supplémentaire n'est nécessaire.
> - **PowerShell (solution de repli) :** Si `tar.exe` n'est pas trouvé dans le PATH, SlyMail se rabat sur PowerShell. Pour l'extraction, il utilise la classe .NET `ZipFile` (`System.IO.Compression`) plutôt que `Expand-Archive`, car `Expand-Archive` rejette les extensions de fichier non `.zip` même quand le fichier est une archive ZIP valide. Pour la création des paquets REP, il utilise `Compress-Archive`, en écrivant également dans un fichier `.zip` temporaire qui est ensuite renommé en `.rep`.

### Compiler sur Linux/macOS/BSD

```bash
make
```

Ceci compile deux programmes :
- `slymail` - l'application principale du lecteur QWK
- `config` - l'utilitaire de configuration autonome

### Compiler avec les symboles de débogage

```bash
make debug
```

### Installer (optionnel)

```bash
sudo make install    # Installs slymail and config to /usr/local/bin/
sudo make uninstall  # Remove
```

### Compiler sur Windows avec Visual Studio 2022

Ouvrez le fichier de solution dans Visual Studio 2022 :

```
vs\SlyMail.sln
```

Ou compilez depuis la ligne de commande avec MSBuild :

```powershell
# Release build (output in vs\x64\Release\)
msbuild vs\SlyMail.sln /p:Configuration=Release /p:Platform=x64

# Debug build (output in vs\x64\Debug\)
msbuild vs\SlyMail.sln /p:Configuration=Debug /p:Platform=x64
```

Ceci compile deux exécutables :
- `x64\Release\slymail.exe` — le lecteur QWK principal
- `x64\Release\config.exe` — l'utilitaire de configuration autonome

La solution contient deux projets (`SlyMail.vcxproj` et `Config.vcxproj`) ciblant x64, C++17, avec l'ensemble d'outils MSVC v143.

### Compiler sur Windows (MinGW/MSYS2)

```bash
make
```

Le Makefile détecte automatiquement la plateforme et utilise l'implémentation de terminal appropriée :
- **Linux/macOS/BSD** : ncurses (`terminal_ncurses.cpp`)
- **Windows** : conio + Win32 Console API (`terminal_win32.cpp`)

## Utilisation

```bash
# Launch SlyMail with file browser
./slymail

# Open a specific QWK packet
./slymail MYBBS.qwk

# Run the standalone configuration utility
./config
```

### Programme de configuration

L'utilitaire `config` fournit une interface textuelle autonome pour configurer les paramètres de SlyMail sans ouvrir l'application principale. Il propose quatre catégories de configuration :

- **Paramètres de l'éditeur** - Tous les paramètres disponibles via Ctrl-U dans l'éditeur (style d'éditeur, taglines, vérification orthographique, options de citation, etc.)
- **Paramètres du lecteur** - Basculer les lignes kludge, les lignes de déchirement, la barre de défilement, la suppression ANSI, le mode lightbar, l'ordre inversé et les bascules de code d'attribut (activer/désactiver par BBS)
- **Paramètres des thèmes** - Sélectionner des fichiers de thème de couleur Ice et DCT depuis le répertoire `config_files/`
- **Paramètres généraux** - Définir votre nom pour les réponses et le répertoire de sortie des paquets REP

Les paramètres sont enregistrés automatiquement en quittant chaque catégorie. SlyMail et l'utilitaire de configuration lisent et écrivent tous les deux dans le même fichier de paramètres.

### Raccourcis clavier

#### Navigateur de fichiers
| Touche | Action |
|--------|--------|
| Haut/Bas | Naviguer dans les fichiers et répertoires |
| Entrée | Ouvrir le répertoire / Sélectionner le fichier QWK |
| Ctrl-R | Ouvrir le répertoire des systèmes distants |
| Q / ESC | Quitter |

#### Liste des conférences
| Touche | Action |
|--------|--------|
| Haut/Bas | Naviguer dans les conférences |
| Entrée | Ouvrir la conférence sélectionnée |
| V | Voir les sondages/votes dans le paquet |
| O / Ctrl-L | Ouvrir un autre fichier QWK |
| S / Ctrl-U | Paramètres |
| Q / ESC | Quitter SlyMail |
| ? / F1 | Aide |

#### Liste des messages
| Touche | Action |
|--------|--------|
| Haut/Bas | Naviguer dans les messages |
| Entrée / R | Lire le message sélectionné |
| N | Écrire un nouveau message |
| G | Aller au numéro de message |
| Ctrl-L | Ouvrir un autre fichier QWK |
| S / Ctrl-U | Paramètres |
| C / ESC | Retour à la liste des conférences |
| Q | Quitter |
| ? / F1 | Aide |

#### Lecteur de messages
| Touche | Action |
|--------|--------|
| Haut/Bas | Faire défiler le message |
| Gauche/Droite | Message précédent / suivant |
| F / L | Premier / dernier message |
| R | Répondre au message |
| V | Voter (vote haut/bas ou bulletin de sondage) |
| D / Ctrl-D | Télécharger les pièces jointes |
| H | Afficher les informations d'en-tête du message |
| S / Ctrl-U | Paramètres |
| C / Q / ESC | Retour à la liste des messages |
| ? / F1 | Aide |

#### Éditeur de messages
| Touche | Action |
|--------|--------|
| ESC | Menu de l'éditeur (Enregistrer, Annuler, etc.) |
| Ctrl-U | Boîte de dialogue des paramètres utilisateur |
| Ctrl-Q | Ouvrir/fermer la fenêtre de citation |
| Ctrl-K | Sélecteur de couleur (insérer un code couleur ANSI à la position du curseur) |
| Ctrl-G | Insérer un caractère graphique (CP437) par code |
| Ctrl-W | Recherche de mot/texte |
| Ctrl-S | Modifier le sujet |
| Ctrl-D | Supprimer la ligne actuelle |
| Ctrl-Z | Enregistrer le message |
| Ctrl-A | Annuler le message |
| F1 | Écran d'aide |
| Insert | Basculer le mode Insertion/Remplacement |

#### Fenêtre de citation
| Touche | Action |
|--------|--------|
| Haut/Bas | Naviguer dans les lignes de citation |
| Entrée | Insérer la ligne de citation sélectionnée |
| Ctrl-Q / ESC | Fermer la fenêtre de citation |

## Architecture

SlyMail utilise une couche d'abstraction de plateforme pour son interface utilisateur textuelle :

```
ITerminal (abstract base class)
    ├── NCursesTerminal  (Linux/macOS/BSD - ncurses)
    └── Win32Terminal    (Windows - conio + Win32 Console API)
```

Les caractères de dessin de boîte et les caractères spéciaux CP437 sont définis dans `cp437defs.h` et rendus via la méthode `putCP437()`, qui mappe les codes CP437 vers les équivalents natifs de la plateforme (caractères ACS sur ncurses, octets CP437 directs sur Windows).

### Fichiers sources

| Fichier | Description |
|---------|-------------|
| `terminal.h` | Interface abstraite `ITerminal`, constantes de touches/couleurs, fabrique |
| `terminal_ncurses.cpp` | Implémentation ncurses avec mappage CP437-vers-ACS |
| `terminal_win32.cpp` | API Console Windows + implémentation conio |
| `cp437defs.h` | Définitions des caractères de la page de codes IBM 437 |
| `colors.h` | Définitions des schémas de couleurs (Ice, DCT, lecteur, liste) |
| `theme.h` | Analyseur de fichiers de configuration de thème (codes d'attribut style Synchronet) |
| `ui_common.h` | Auxiliaires d'interface partagés (boîtes de dialogue, saisie de texte, barre de défilement, etc.) |
| `qwk.h` / `qwk.cpp` | Analyseur et créateur de paquets QWK/REP (QWKE, pièces jointes, votes) |
| `bbs_colors.h` / `bbs_colors.cpp` | Analyseur de codes couleur/attribut BBS (ANSI, Synchronet, WWIV, PCBoard, Celerity, Renegade) |
| `utf8_util.h` / `utf8_util.cpp` | Utilitaires UTF-8 (validation, largeur d'affichage, conversion CP437-vers-UTF-8) |
| `voting.h` / `voting.cpp` | Analyseur VOTING.DAT, décompte des votes, interface d'affichage des sondages |
| `remote_systems.h` / `remote_systems.cpp` | Répertoire des systèmes distants, navigation FTP/SFTP, persistance JSON, utilitaires de répertoire personnel |
| `settings.h` | Persistance des paramètres utilisateur |
| `settings_dialog.h` | Boîtes de dialogue des paramètres (éditeur, lecteur, bascules de code d'attribut) |
| `file_browser.h` | Navigateur et sélecteur de fichiers QWK |
| `msg_list.h` | Vues de liste de conférences et de messages |
| `msg_reader.h` | Lecteur de messages (style DDMsgReader) avec interface de vote et de pièces jointes |
| `msg_editor.h` | Éditeur de messages (style SlyEdit Ice/DCT) avec sélecteur de couleur |
| `main.cpp` | Point d'entrée de l'application SlyMail et boucle principale |
| `config.cpp` | Utilitaire de configuration autonome |

## Configuration

### Fichier de paramètres

Les paramètres sont stockés dans un fichier INI nommé `slymail.ini` dans le même répertoire que l'exécutable SlyMail. Ce fichier est partagé entre SlyMail et l'utilitaire `config`. Le fichier est bien commenté avec des descriptions de chaque paramètre.

Exemple `slymail.ini` :
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

### Fichiers de thème

Les thèmes de couleurs sont des fichiers `.ini` dans le répertoire `config_files/` :

**Thèmes Ice** (`EditorIceColors_*.ini`) :
- BlueIce (par défaut), EmeraldCity, FieryInferno, Fire-N-Ice, GeneralClean, GenericBlue, PurpleHaze, ShadesOfGrey

**Thèmes DCT** (`EditorDCTColors_*.ini`) :
- Default (par défaut), Default-Modified, Midnight

Les valeurs de couleur du thème utilisent un format compact dérivé des codes d'attribut Synchronet :
- `n` = normal (réinitialiser)
- Premier plan : `k`=noir, `r`=rouge, `g`=vert, `y`=jaune, `b`=bleu, `m`=magenta, `c`=cyan, `w`=blanc
- `h` = haute intensité/brillant
- Chiffre d'arrière-plan : `0`=noir, `1`=rouge, `2`=vert, `3`=marron, `4`=bleu, `5`=magenta, `6`=cyan, `7`=gris clair

Exemple : `nbh` = bleu brillant normal, `n4wh` = blanc brillant sur fond bleu

### Taglines

Les taglines sont de courtes citations ou maximes ajoutées à la fin des messages lors de l'enregistrement. La fonctionnalité de taglines peut être activée via Ctrl-U dans l'éditeur ou l'utilitaire `config`.

Les taglines sont stockées dans `tagline_files/taglines.txt`, une par ligne. Les lignes commençant par `#` ou `;` sont traitées comme des commentaires et ignorées. Lors de l'enregistrement d'un message avec les taglines activées, l'utilisateur est invité à sélectionner une tagline spécifique ou à en choisir une au hasard. Les taglines sélectionnées sont ajoutées au message avec le préfixe `...` (par ex., `...To err is human, to really foul things up requires a computer.`).

### Vérificateur orthographique

SlyMail inclut un vérificateur orthographique intégré qui utilise des fichiers de dictionnaire en texte brut. Le vérificateur orthographique peut être configuré pour demander lors de l'enregistrement via Ctrl-U dans l'éditeur ou l'utilitaire `config`.

**Les fichiers de dictionnaire** sont des fichiers texte brut (un mot par ligne) stockés dans `dictionary_files/`. Plusieurs dictionnaires peuvent être sélectionnés simultanément pour une couverture combinée des mots. SlyMail est livré avec :
- `dictionary_en.txt` - Anglais (général, ~130K mots)
- `dictionary_en-US-supplemental.txt` - Supplément anglais américain
- `dictionary_en-GB-supplemental.txt` - Supplément anglais britannique
- `dictionary_en-AU-supplemental.txt` - Supplément anglais australien
- `dictionary_en-CA-supplemental.txt` - Supplément anglais canadien

Lorsque la vérification orthographique est déclenchée, le vérificateur analyse le message à la recherche de mots mal orthographiés et présente une boîte de dialogue interactive pour chacun d'eux, proposant des options pour **R**emplacer le mot, le **P**asser, l'**A**jouter (futur) ou **Q**uitter la vérification.

## Crédits

- Interface inspirée de [DDMsgReader](https://github.com/SynchronetBBS/sbbs) et [SlyEdit](https://github.com/SynchronetBBS/sbbs) par [Nightfox (Eric Oulashin)](https://github.com/EricOulashin)
- Compatibilité du format QWK basée sur le code source de [Synchronet BBS](https://www.synchro.net/)
- Définitions des caractères CP437 de Synchronet

## Licence

Ce projet est un logiciel open source.
