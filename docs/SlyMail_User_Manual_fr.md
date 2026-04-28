---
title: "Manuel d’utilisation de SlyMail"
subtitle: "Lecteur de courrier hors ligne QWK"
version: "0.55"
date: "2026-04-09"
author: "Eric Oulashin"
---

# Manuel d’utilisation de SlyMail

## Introduction

SlyMail est un lecteur de courrier hors ligne multiplateforme, en mode texte, pour le format de paquets QWK. Les paquets QWK servent à échanger du courrier sur les systèmes de type Bulletin Board System (BBS). SlyMail fournit une interface complète pour lire, rechercher et répondre aux messages contenus dans des paquets QWK.

Son interface utilisateur s’inspire de Digital Distortion Message Reader (DDMsgReader) pour la lecture des messages et de SlyEdit pour l’édition des messages, deux programmes créés à l’origine pour Synchronet BBS.

SlyMail fonctionne sous Linux, macOS, BSD et Windows.

## Bien démarrer

### Lancer SlyMail

![Écran d’ouverture de SlyMail](../screenshots/SlyMail_01_OpeningScreen.png)

Lancez SlyMail depuis la ligne de commande :

    slymail

Cela affiche l’écran de démarrage (si activé) et ouvre le navigateur de fichiers afin de sélectionner un paquet QWK.

Pour ouvrir directement un paquet QWK spécifique :

    slymail MYBBS.qwk

Ou utilisez le paramètre nommé :

    slymail -qwk_file=/chemin/vers/MYBBS.qwk

Autres options en ligne de commande :

- `-v` ou `--version` - Afficher les informations de version
- `-?` ou `--help` - Afficher l’aide

### Le répertoire de données de SlyMail

Au premier lancement, SlyMail crée un répertoire de données dans `~/.slymail` (sous Linux, macOS et BSD) ou dans votre dossier personnel sous Windows. Ce répertoire contient :

- `slymail.ini` - Vos réglages
- `QWK/` - Répertoire par défaut des paquets QWK
- `REP/` - Répertoire par défaut des paquets de réponse
- `config_files/` - Fichiers de thèmes de couleurs
- `dictionary_files/` - Dictionnaires du correcteur orthographique
- `tagline_files/` - Fichiers de « taglines »
- `remote_systems.json` - Connexions aux systèmes distants enregistrées
- `lastread_<BBSID>.json` - Pointeurs « dernier lu » par conférence

### Le programme de configuration

SlyMail inclut un programme de configuration autonome nommé `config` (ou `config_slymail` si installé via `make install`). Ce programme fournit une interface en mode texte permettant de configurer SlyMail sans ouvrir l’application principale.

Exécutez-le depuis la ligne de commande :

    config_slymail

Le programme de configuration propose ces catégories :

- **Réglages de l’éditeur** - Style de l’éditeur, taglines, correcteur, options de citation, sélection du thème
- **Réglages du lecteur** - Lignes kludge, ligne tear/origin, barre de défilement, suppression ANSI, ordre inversé, bascules des codes d’attribut
- **Réglages des thèmes** - Sélection des thèmes de couleurs Ice et DCT
- **Réglages généraux** - Nom pour les réponses, répertoire des paquets REP, éditeur externe, mode lightbar, écran de démarrage

Les réglages sont enregistrés automatiquement lorsque vous quittez chaque catégorie. SlyMail et le programme `config` lisent et écrivent le même fichier `slymail.ini`.

## Navigateur de fichiers

Au démarrage, SlyMail affiche un navigateur de fichiers pour sélectionner un paquet QWK à ouvrir.

![Navigateur de fichiers](../screenshots/SlyMail_02_File_Chooser.png)

### Touches du navigateur de fichiers

| Touche | Action |
|-----|--------|
| Haut / Bas | Naviguer dans les fichiers et dossiers |
| Entrée | Ouvrir un dossier ou sélectionner un fichier QWK |
| PgUp / PgDn | Faire défiler la liste |
| Home / End | Aller au début ou à la fin |
| Ctrl-R | Ouvrir le répertoire des systèmes distants |
| Q / ESC | Quitter SlyMail |
| ? / F1 | Aide |

Seuls les fichiers `.qwk` peuvent être sélectionnés. Les dossiers apparaissent en bleu et peuvent être ouverts en appuyant sur Entrée.

## Systèmes distants (Ctrl-R)

![Liste des systèmes distants](../screenshots/SlyMail_03_remote_system_list.png)

SlyMail peut télécharger des paquets QWK directement depuis des BBS distantes via FTP ou SFTP (SSH).

Appuyez sur **Ctrl-R** depuis le navigateur de fichiers, la liste des conférences ou la liste des messages pour ouvrir le gestionnaire des systèmes distants. Vous pouvez :

- **Ajouter** un système distant (hôte, port, type de connexion FTP/SSH, utilisateur, mot de passe, FTP passif, chemin distant initial)
- **Modifier** ou **Supprimer** des entrées existantes
- **Se connecter** à un système distant et parcourir ses dossiers
- **Télécharger** des fichiers QWK directement dans votre dossier `QWK/`
- **Envoyer** des paquets REP au système distant

Les entrées sont enregistrées dans `remote_systems.json` dans le répertoire de données SlyMail. La date/heure de dernière connexion est mémorisée.

SlyMail utilise la commande système `curl` pour les transferts FTP et SFTP.

## Liste des conférences

![Liste des conférences](../screenshots/SlyMail_06_msg_area_list.png)

Après l’ouverture d’un paquet QWK, la liste des conférences affiche toutes les zones de messages (conférences) avec leurs compteurs. Une colonne « New » affiche une coche à côté des conférences ayant de nouveaux messages. Si l’option « Only show areas with new mail » est activée, les conférences sans messages sont masquées.

### Touches de la liste des conférences

| Touche | Action |
|-----|--------|
| Haut / Bas | Naviguer dans les conférences |
| PgUp / PgDn | Faire défiler la liste |
| Home / End | Aller à la première / dernière conférence |
| Entrée (E) | Ouvrir la conférence sélectionnée |
| G | Aller à un numéro de conférence |
| / | Rechercher une conférence par nom |
| V | Voir les sondages et votes du paquet |
| O / Ctrl-L | Ouvrir un autre fichier QWK |
| Ctrl-R | Ouvrir les systèmes distants |
| Ctrl-P | Enregistrer le paquet REP |
| S / Ctrl-U | Ouvrir les réglages |
| Q / ESC | Quitter SlyMail |
| ? / F1 | Aide |

Lorsqu’un filtre de recherche est actif, **Q** efface le filtre au lieu de quitter.

## Liste des messages

![Liste des messages](../screenshots/SlyMail_07_msg_list.png)

La liste des messages affiche tous les messages de la conférence sélectionnée, avec les colonnes numéro, de, à, sujet, date et heure.

### Suivi du dernier message lu

SlyMail mémorise le dernier message lu dans chaque conférence. Lorsque vous entrez dans une conférence, le curseur est automatiquement positionné sur le premier message non lu (le message suivant celui que vous avez lu en dernier). Ces informations sont enregistrées par BBS dans le répertoire de données.

### Touches de la liste des messages

| Touche | Action |
|-----|--------|
| Haut / Bas | Naviguer dans les messages |
| PgUp / PgDn | Faire défiler la liste |
| Home / End | Aller au premier / dernier message |
| Entrée / R | Lire le message sélectionné |
| N | Écrire un nouveau message |
| G | Aller à un numéro de message |
| / | Rechercher des messages |
| Ctrl-A | Recherche avancée (plage de dates) |
| Ctrl-L | Ouvrir un autre fichier QWK |
| Ctrl-R | Ouvrir les systèmes distants |
| Ctrl-P | Enregistrer le paquet REP |
| S / Ctrl-U | Ouvrir les réglages |
| C / ESC | Retour à la liste des conférences |
| Q | Quitter |
| ? / F1 | Aide |

### Rechercher des messages

![Recherche de messages](../screenshots/SlyMail_16_msg_search.png)

Appuyez sur **/** pour rechercher des messages par sujet, texte, expéditeur ou destinataire. Appuyez sur **Ctrl-A** pour la recherche avancée, qui inclut un filtrage par plage de dates avec un sélecteur de calendrier. Quand un filtre est actif, **Q** efface le filtre.

La recherche peut utiliser une sous-chaîne simple ou des expressions régulières selon « Search using regular expression ».

## Lecteur de messages

![Lecture d’un message](../screenshots/SlyMail_08_reading_msg.png)

Le lecteur affiche le contenu avec un en-tête contenant From, To, Subject et Date.

### Touches du lecteur de messages

| Touche | Action |
|-----|--------|
| Haut / Bas | Faire défiler le contenu |
| PgUp / PgDn | Page précédente / suivante |
| Home / End | Haut / bas du message |
| Gauche / Droite | Message précédent / suivant |
| F / L | Premier / dernier message |
| R | Répondre au message |
| V | Voter (haut/bas ou bulletin de sondage) |
| D / Ctrl-D | Télécharger les pièces jointes |
| H | Afficher l’en-tête détaillé |
| S / Ctrl-U | Ouvrir les réglages |
| C / Q / ESC / M | Retour à la liste |
| ? / F1 | Aide |

### Art ANSI

![Art ANSI](../screenshots/SlyMail_20_reading_ANSI_art.png)

SlyMail rend l’art ANSI dans les messages, avec prise en charge du positionnement du curseur, des couleurs et des caractères CP437. Les messages contenant des séquences de contrôle ANSI sont détectés et rendus via un tampon d’écran virtuel.

### Pièces jointes

Lorsqu’un message possède des pièces jointes (indiqué par **[ATT]**), appuyez sur **D** ou **Ctrl-D** pour voir et télécharger. Une liste des fichiers s’affiche, puis vous choisissez un dossier de destination.

### Votes et sondages

SlyMail prend en charge le vote de type Synchronet :

- **Sondages** : les messages identifiés comme sondages affichent des options avec compteurs et barres de pourcentage. Appuyez sur **V** pour ouvrir le bulletin et sélectionner vos choix.
- **Votes haut/bas** : pour un message normal, appuyez sur **V** pour voter pour/contre. Les totaux et le score net s’affichent dans l’en-tête.
- **Navigateur de sondages** : appuyez sur **V** depuis la liste des conférences pour parcourir les sondages.

Les votes sont mis en file d’attente avec les réponses et écrits dans le paquet REP.

## Éditeur de messages

![Éditeur de messages](../screenshots/SlyMail_09_msg_edit_start.png)

L’éditeur sert à composer des réponses et des messages. Il propose deux modes visuels inspirés de SlyEdit : **Ice** et **DCT**.

### Touches de l’éditeur

| Touche | Action |
|-----|--------|
| Flèches | Déplacer le curseur |
| Home / End | Début / fin de ligne |
| PgUp / PgDn | Page précédente / suivante |
| Insert | Basculer insertion/écrasement |
| Delete | Supprimer le caractère au curseur |
| Retour arrière | Supprimer le caractère avant le curseur |
| ESC | Ouvrir le menu de l’éditeur |
| Ctrl-Z | Enregistrer le message |
| Ctrl-A | Abandonner le message |
| Ctrl-Q | Ouvrir/fermer la fenêtre de citation |
| Ctrl-K | Ouvrir le sélecteur de couleurs |
| Ctrl-G | Insérer un caractère graphique CP437 |
| Ctrl-O | Importer un fichier texte à la position du curseur |
| Ctrl-W | Rechercher du texte |
| Ctrl-S | Changer le sujet |
| Ctrl-D | Supprimer la ligne courante |
| Ctrl-T | Lister les remplacements de texte |
| Ctrl-U | Ouvrir les réglages de l’éditeur |
| F1 | Aide |

### Commandes slash

Vous pouvez aussi taper des commandes sur une ligne vide puis Entrée :

| Commande | Action |
|---------|--------|
| /S | Enregistrer le message |
| /A | Abandonner le message |
| /Q | Ouvrir la fenêtre de citation |
| /U | Ouvrir les réglages de l’éditeur |
| /? | Afficher l’aide |

### Fenêtre de citation

![Fenêtre de citation](../screenshots/SlyMail_10_quote_line_selection.png)

En répondant à un message, appuyez sur **Ctrl-Q** ou tapez `/Q` sur une ligne vide. La fenêtre affiche le texte original avec les préfixes de citation.

| Touche | Action |
|-----|--------|
| Haut / Bas | Naviguer dans les lignes citées |
| PgUp / PgDn | Page précédente / suivante |
| Home / End | Première / dernière ligne |
| Entrée | Insérer la ligne citée sélectionnée |
| Ctrl-Q / ESC | Fermer la fenêtre |

Si le nombre de lignes dépasse la hauteur de la fenêtre, une barre de défilement apparaît à droite.

### Sélecteur de couleurs (Ctrl-K)

Appuyez sur **Ctrl-K** pour ouvrir un sélecteur interactif. Choisissez une couleur de premier plan (16 options) et d’arrière-plan (8 options). Une prévisualisation affiche le résultat. Appuyez sur **Entrée** pour insérer le code ANSI. Appuyez sur **N** pour insérer un code de réinitialisation.

### Import de fichier (Ctrl-O)

Appuyez sur **Ctrl-O** pour importer le contenu d’un fichier texte à la position du curseur. Un navigateur de fichiers s’ouvre, puis le contenu est inséré ligne par ligne.

![Écriture d’une réponse](../screenshots/SlyMail_11_writing_reply_msg.png)

### Gestion des lignes citées

Les lignes insérées depuis la fenêtre de citation sont suivies comme lignes citées. Si une ligne citée est modifiée et se replie, le débordement va sur une nouvelle ligne (également citée) au lieu de fusionner. Supprimer du texte d’une ligne citée ne remonte pas le texte de la ligne suivante si celle-ci est aussi citée. Cela préserve l’intégrité des citations.

### Enregistrement par paragraphes

Lors de l’enregistrement, SlyMail rassemble les lignes « soft-wrappées » du texte nouvellement saisi en longs paragraphes afin que d’autres lecteurs puissent replier le texte selon leur largeur. Les lignes citées sont toujours préservées telles quelles.

### Messages vides

Si vous enregistrez un message sans contenu (vide ou seulement des espaces), SlyMail annule le message et affiche une notification au lieu de le poster.

### Éditeurs externes

SlyMail permet d’utiliser des éditeurs externes. Pour en configurer un :

1. Ouvrez les réglages (Ctrl-U ou le programme `config`)
2. Allez dans **External Editors...** pour ajouter/configurer
3. Réglez **External Editor** pour choisir lequel utiliser
4. Activez **Use external editor**

Chaque éditeur externe peut être configuré avec :

- **Name** - Nom descriptif
- **Startup Directory** - Dossier contenant l’exécutable
- **Command Line** - Commande (utilisez `%f` pour le fichier temporaire)
- **Word-wrap Quoted Text** - Repli du texte cité et largeur
- **Auto Quoted Text** - Toujours / Demander / Jamais
- **Editor Information Files** - Type de fichier d’info (None, MSGINF, EDITOR.INF, DOOR.SYS, DOOR32.SYS)
- **Strip FidoNet Kludges** - Retirer les lignes de contrôle FidoNet

En mode éditeur externe, SlyMail crée un fichier temporaire, lance l’éditeur et lit le résultat à la fermeture. Si l’éditeur produit `RESULT.ED`, SlyMail y lit le nouveau sujet et des détails.

## Réglages

### Réglages du lecteur (Ctrl-U)

![Réglages du lecteur](../screenshots/SlyMail_14_reader_settings.png)

Accédez aux réglages du lecteur en appuyant sur **Ctrl-U** ou **S** depuis la liste des conférences, la liste des messages ou le lecteur.

Réglages disponibles :

- **Show kludge lines** - Afficher/masquer les lignes kludge/contrôle
- **Show tear/origin lines** - Afficher/masquer tear/origin
- **Scrollbar in reader** - Afficher/masquer la barre de défilement
- **Only show areas with new mail** - Masquer les conférences vides
- **Strip ANSI codes from messages** - Retirer les codes ANSI
- **Attribute code toggles** - Bascules des codes d’attribut
- **Search using regular expression** - Recherche par regex
- **List messages in reversed** - Messages les plus récents en premier
- **Show splash screen on startup** - Écran de démarrage
- **Reply packet directory** - Dossier des paquets REP
- **Use external editor** - Activer l’éditeur externe
- **External Editors...** - Configurer les éditeurs externes
- **External Editor** - Sélectionner l’éditeur
- **Language** - Choisir la langue (ou « OS Default »)

Appuyez sur **S** pour enregistrer, ou **ESC/Q** pour fermer.

### Réglages de l’éditeur (Ctrl-U dans l’éditeur)

Accédez aux réglages de l’éditeur en appuyant sur **Ctrl-U** dans l’éditeur de messages.

Réglages disponibles :

- **Choose UI mode** - Ice / DCT / Random
- **Taglines** - Activer les taglines
- **Spell-check dictionary/dictionaries** - Choisir les dictionnaires
- **Prompt for spell checker on save** - Demander la correction
- **Wrap quote lines to terminal width** - Replier les citations à la largeur du terminal
- **Quote with author's initials** - Préfixe de citation avec initiales
- **Indent quote lines containing initials** - Indenter les citations avec initiales
- **Trim spaces from quote lines** - Retirer les espaces en début de citation
- **Language** - Choisir la langue (ou « OS Default »)

## Prise en charge des codes couleur BBS

SlyMail interprète les codes couleur/attribut de plusieurs logiciels de BBS :

- **Codes ANSI** - Toujours activés (SGR standard)
- **Synchronet Ctrl-A** - Ctrl-A + caractère
- **WWIV (cœurs)** - Ctrl-C + chiffre 0-9
- **PCBoard/Wildcat @X** - @X + deux chiffres hex
- **Celerity (pipe)** - | + lettre
- **Renegade (pipe)** - | + deux chiffres

Chaque type peut être activé/désactivé via **Attribute Code Toggles**.

## Thèmes de couleurs

L’éditeur de SlyMail utilise des thèmes configurables dans `config_files/` sous forme de fichiers `.ini`.

### Thèmes Ice

- BlueIce (par défaut)
- EmeraldCity
- FieryInferno
- Fire-N-Ice
- GeneralClean
- GenericBlue
- PurpleHaze
- ShadesOfGrey

### Thèmes DCT

- Default
- Default-Modified
- Midnight

### Format des couleurs

Les thèmes utilisent un format compact basé sur les codes d’attribut Synchronet :

- `n` - Normal (réinitialise tout)
- Lettres de premier plan : `k` noir, `r` rouge, `g` vert, `y` jaune, `b` bleu, `m` magenta, `c` cyan, `w` blanc
- `h` - Intensité haute
- Chiffres d’arrière-plan : `0` noir, `1` rouge, `2` vert, `3` brun, `4` bleu, `5` magenta, `6` cyan, `7` gris clair

Exemples : `nbh` = bleu clair, `n4wh` = blanc clair sur fond bleu, `nk7` = noir sur gris clair.

## Correcteur orthographique

SlyMail inclut un correcteur orthographique basé sur des dictionnaires texte.

### Dictionnaires

Les dictionnaires se trouvent dans `dictionary_files/` :

- `dictionary_en.txt` - Anglais (général)
- `dictionary_en-US-supplemental.txt` - Supplément anglais US
- `dictionary_en-GB-supplemental.txt` - Supplément anglais UK
- `dictionary_en-AU-supplemental.txt` - Supplément anglais AU
- `dictionary_en-CA-supplemental.txt` - Supplément anglais CA

Vous pouvez sélectionner plusieurs dictionnaires en même temps.

### Utilisation du correcteur

Activez « Prompt for spell checker on save ». À l’enregistrement, le correcteur propose :

- **R**eplace - Remplacer
- **S**kip - Ignorer
- **Q**uit - Quitter

## Taglines

Les taglines sont de courtes citations ajoutées à la fin des messages.

### Mise en place

1. Activez « Taglines »
2. Placez les taglines dans `tagline_files/taglines.txt`, une par ligne
3. Les lignes commençant par `#` ou `;` sont des commentaires

### Utilisation

Lors de l’enregistrement, vous choisissez une tagline ou une au hasard. Elle est ajoutée avec le préfixe `...`.

## Paquets de réponse REP

Lorsque vous écrivez des réponses ou de nouveaux messages, ils sont mis en attente. Les votes sont aussi mis en attente.

### Enregistrer les paquets REP

- Appuyez sur **Ctrl-P** depuis la liste des conférences/messages
- SlyMail peut demander d’enregistrer après la rédaction
- À la sortie, SlyMail propose d’enregistrer les éléments en attente

Le paquet REP est enregistré sous `<BBS-ID>.rep` dans le répertoire REP (par défaut `~/.slymail/REP/`). Envoyez ce fichier au BBS pour poster vos réponses.

### Contenu d’un paquet REP

- Réponses au format QWK standard
- `HEADERS.DAT` pour les champs étendus (> 25 caractères)
- `VOTING.DAT` pour les votes en attente (compatible Synchronet)

## Prise en charge UTF-8

SlyMail détecte et affiche correctement l’UTF-8 :

- Détection via les indicateurs `HEADERS.DAT` et l’analyse automatique
- Indicateur **[UTF8]** dans l’en-tête
- Conversion des caractères CP437 vers Unicode

## Référence du fichier de réglages

Les réglages sont stockés dans `slymail.ini` (format INI) avec les sections :

### [General]

| Réglage | Défaut | Description |
|---------|---------|-------------|
| showSplashScreen | true | Afficher l’écran de démarrage |
| userName | (vide) | Nom pour le champ From |
| replyDir | (vide) | Répertoire des paquets REP |
| useExternalEditor | false | Utiliser un éditeur externe |
| selectedEditor | (vide) | Nom de l’éditeur externe sélectionné |
| language | (vide) | Code langue UI (vide = OS Default) |

### [Reader]

| Réglage | Défaut | Description |
|---------|---------|-------------|
| showKludgeLines | false | Afficher les lignes kludge/contrôle |
| showTearLine | true | Afficher tear/origin |
| useScrollbar | true | Afficher la barre de défilement |
| onlyShowAreasWithNewMail | false | Masquer les conférences vides |
| stripAnsi | false | Retirer les codes ANSI |
| attrSynchronet | true | Interpréter Ctrl-A Synchronet |
| attrWWIV | true | Interpréter WWIV |
| attrCelerity | true | Interpréter Celerity |
| attrRenegade | true | Interpréter Renegade |
| attrPCBoard | true | Interpréter PCBoard/Wildcat |
| useRegexSearch | false | Recherche par regex |

### [MessageList]

| Réglage | Défaut | Description |
|---------|---------|-------------|
| lightbarMode | true | Navigation lightbar |
| reverseOrder | false | Plus récents d’abord |

### [Editor]

| Réglage | Défaut | Description |
|---------|---------|-------------|
| editorStyle | Ice | Mode : Ice, Dct ou Random |
| insertMode | true | Insertion par défaut |
| wrapQuoteLines | true | Replier les citations |
| quoteLineWidth | 76 | Largeur max des citations |
| quotePrefix | > (espace) | Préfixe de citation |
| taglines | false | Activer les taglines |
| promptSpellCheck | false | Demander la correction |
| quoteWithInitials | false | Initiales dans le préfixe |
| indentQuoteInitials | true | Indenter avec initiales |
| trimQuoteSpaces | false | Retirer espaces de début |

### [Themes]

| Réglage | Défaut | Description |
|---------|---------|-------------|
| iceThemeFile | EditorIceColors_BlueIce.ini | Thème Ice |
| dctThemeFile | EditorDCTColors_Default.ini | Thème DCT |

## Réglages QWK recommandés (Synchronet)

| Réglage | Valeur recommandée |
|---------|-------------------|
| Ctrl-A Color Codes | Leave in |
| Archive Type | ZIP |
| Include File Attachments | Yes |
| Include Index Files | Yes |
| Include Control Files | Yes |
| Include VOTING.DAT File | Yes |
| Include HEADERS.DAT File | Yes |
| Include UTF-8 Characters | Yes |
| Extended (QWKE) Packet Format | Yes |

## Historique des versions

### Version 0.54 (2026-03-31)

#### Ajouts

- Suivi du dernier message lu par conférence avec positionnement automatique
- Indicateur de nouveaux messages (coche) dans la liste des conférences
- Import de fichiers dans l’éditeur (Ctrl-O)
- Suivi des lignes citées pour préserver l’intégrité
- Enregistrement par paragraphes
- Détection du redimensionnement du terminal

#### Modifications

- Le repli des citations respecte la largeur du terminal si activé
- Première exécution : si aucune configuration n’existe, `config` est lancé au démarrage et le nom est demandé

#### Corrections

- L’édition des lignes citées ne fusionne plus le débordement avec les lignes adjacentes

### Version 0.53 (2026-03-29)

#### Ajouts

- Option pour n’afficher que les conférences avec nouveaux messages
- Éditeur externe avec plusieurs configurations
- Support des fichiers d’info (MSGINF, EDITOR.INF, DOOR.SYS, DOOR32.SYS)
- Lecture de RESULT.ED après la fermeture de l’éditeur externe
- Aller à une conférence (touche G)
- Validation de saisie numérique

#### Corrections

- Les commandes slash (/Q, /S, etc.) fonctionnent sur la première ligne
- Couleurs de la fenêtre de citation DCT corrigées
- Barre de défilement restaurée pour les listes longues
- Rafraîchissement d’écran amélioré dans les dialogues

### Version 0.52 (2026-03-26)

#### Ajouts

- Création automatique du répertoire REP
- Touche Ctrl-P pour enregistrer le REP
- Envoi de paquets REP aux systèmes distants
- Repli des lignes citées
- Sélecteur de répertoire pour REP

#### Modifications

- Mises à jour d’écran plus rapides dans l’éditeur
- Enregistrement par paragraphes amélioré
- Rendu ANSI amélioré (256 couleurs / truecolor)

#### Corrections

- Gestion des codes couleur dans toutes les opérations
- Comportement de word-wrap de l’éditeur
- Préservation de l’espace final du préfixe de citation
- Suivi de l’enregistrement REP

### Version 0.51 (2026-03-25)

#### Ajouts

- Support QWKE (QWK étendu)
- Codes couleur/attribut pour plusieurs BBS
- Vote Synchronet (sondages et votes haut/bas)
- Pièces jointes
- UTF-8
- Sélecteur de couleurs (Ctrl-K)
- Systèmes distants (Ctrl-R)
- Recherche et filtrage (recherche avancée)
- Écran de démarrage

### Version 0.50 (2026-03-24)

- Première version
- Lecteur QWK hors ligne multiplateforme
- Interface inspirée de DDMsgReader
- Éditeur inspiré de SlyEdit (Ice/DCT)
- Thèmes, correcteur, taglines
- Création de paquets REP
- Navigateur de fichiers et utilitaire `config`

