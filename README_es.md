*(Traducción automática — para la versión original en inglés véase [README.md](README.md))*

# SlyMail

Es un lector de correo sin conexión basado en texto y multiplataforma para el formato de paquetes [QWK](https://en.wikipedia.org/wiki/QWK_(file_format)). El formato de paquetes QWK se usaba/usa frecuentemente para intercambiar correo en [sistemas de tablón de anuncios](https://en.wikipedia.org/wiki/Bulletin_board_system).

SlyMail proporciona una interfaz completa para leer y responder mensajes de paquetes de correo QWK de BBS (Sistema de Tablón de Anuncios). Su interfaz de usuario está inspirada en [Digital Distortion Message Reader (DDMsgReader)](https://github.com/SynchronetBBS/sbbs/tree/master/xtrn/DDMsgReader) para la lectura de mensajes y en [SlyEdit](https://github.com/SynchronetBBS/sbbs/tree/master/exec) para la edición de mensajes, ambos creados originalmente para [Synchronet BBS](https://www.synchro.net/).

SlyMail fue creado con la ayuda de Claude AI.

## Características

### Soporte de paquetes QWK
- Abre y lee paquetes de correo QWK estándar (archivos .qwk)
- Analiza CONTROL.DAT, MESSAGES.DAT y archivos de índice NDX
- Soporte completo de QWKE (QWK extendido) a través de HEADERS.DAT — coincidencia basada en desplazamiento para campos extendidos precisos To/From/Subject, bandera UTF-8 y RFC822 Message-ID
- Análisis de kludge de cuerpo QWKE (`To:`, `From:`, `Subject:` al inicio del mensaje)
- Maneja la numeración de conferencias estilo Synchronet
- Crea paquetes de respuesta REP (archivos .rep) para subir de nuevo al BBS, incluyendo HEADERS.DAT para campos extendidos y VOTING.DAT para votos pendientes
- Soporta codificación de flotante en Formato Binario de Microsoft (MBF) en archivos NDX
- Recuerda el último archivo QWK abierto y el directorio entre sesiones

### Lectura de mensajes (estilo DDMsgReader)
- Lista de conferencias con recuentos de mensajes
- Lista de mensajes desplazable con navegación por barra de luz
- Lector de mensajes completo con visualización de encabezado (De, Para, Asunto, Fecha)
- Resaltado de líneas de cita (admite citas de múltiples niveles)
- Visualización de líneas kludge (opcional)
- Indicador de barra de desplazamiento
- Navegación por teclado: Primer/Último/Siguiente/Mensaje anterior, Página arriba/abajo
- Pantallas de ayuda accesibles con `?` o `F1` en todas las vistas

### Soporte de códigos de color y atributos BBS
SlyMail interpreta códigos de color/atributo de múltiples paquetes de software BBS, renderizándolos como texto en color tanto en el lector de mensajes como en el editor de mensajes. Formatos soportados:
- **Códigos de escape ANSI** — siempre habilitados; secuencias SGR estándar (ESC[...m) para primer plano, fondo, negrita
- **Códigos Ctrl-A de Synchronet** — `\x01` + carácter de atributo (p. ej., `\x01c` para cyan, `\x01h` para brillante)
- **Códigos de corazón WWIV** — `\x03` + dígito del 0 al 9
- **Códigos @X de PCBoard/Wildcat** — `@X##` donde los dos dígitos hexadecimales codifican el color de fondo y primer plano
- **Códigos de tubería Celerity** — `|` + letra (p. ej., `|c` para cyan, `|W` para blanco brillante)
- **Códigos de tubería Renegade** — `|` + número de dos dígitos del 00 al 31

Cada tipo de código BBS puede habilitarse o deshabilitarse individualmente mediante el subdiálogo **Alternadores de código de atributo** en Configuración del lector o la utilidad `config`. Estos alternadores afectan tanto al lector como al editor. Una opción separada **Eliminar códigos ANSI** elimina todas las secuencias ANSI de los mensajes cuando está habilitada.

### Archivos adjuntos
- Detecta archivos adjuntos referenciados mediante líneas kludge `@ATTACH:` en el cuerpo de los mensajes
- Muestra un indicador **[ATT]** en el encabezado del mensaje cuando hay archivos adjuntos
- Presione **D** o **Ctrl-D** en el lector para descargar archivos adjuntos — muestra una lista de archivos con tamaños y solicita un directorio de destino

### Votaciones y encuestas (Synchronet QWKE)
SlyMail soporta la extensión Synchronet VOTING.DAT para encuestas y votaciones de mensajes:
- **Encuestas**: Los mensajes identificados como encuestas muestran sus opciones de respuesta con recuentos de votos y barras de porcentaje. Presione **V** para abrir un diálogo de papeleta donde puede alternar las selecciones de respuesta y emitir su voto.
- **Votos arriba/abajo**: Para mensajes regulares (no encuestas), presione **V** para votar a favor o en contra. Los recuentos de votos actuales y la puntuación se muestran en el encabezado del mensaje.
- **Recuentos de votos**: El encabezado del mensaje muestra los recuentos de votos arriba/abajo y la puntuación neta, con un indicador si ya ha votado.
- **Cola de votos**: Los votos se ponen en cola junto con las respuestas de mensajes y se escriben en VOTING.DAT en el paquete REP para subirlos al BBS.
- **Navegador de encuestas**: Presione **V** desde la lista de conferencias para examinar todas las encuestas del paquete.

### Soporte UTF-8
- Detecta contenido UTF-8 en los mensajes (mediante la bandera `Utf8` de HEADERS.DAT y detección automática de secuencias de bytes UTF-8)
- Muestra caracteres UTF-8 correctamente en terminales compatibles
- Muestra un indicador **[UTF8]** en el encabezado del mensaje para mensajes UTF-8
- Guarda nuevos mensajes con la codificación adecuada
- Conversión de CP437 a UTF-8 para contenido BBS heredado
- Establece la configuración regional en Linux/macOS/BSD (`setlocale(LC_ALL, "")`) y la página de códigos UTF-8 en Windows para una correcta representación en el terminal

### Editor de mensajes (inspirado en SlyEdit)
- **Dos modos visuales**: Ice y DCT, cada uno con distintos esquemas de colores y diseños
- **Modo aleatorio**: Selecciona aleatoriamente Ice o DCT en cada sesión de edición
- **Colores de borde alternantes**: Los caracteres del borde alternan aleatoriamente entre dos colores del tema, coincidiendo con el estilo visual de SlyEdit
- **Soporte de temas**: Temas de color configurables cargados desde archivos .ini
- Editor de texto a pantalla completa con ajuste de línea
- Ventana de citas para seleccionar e insertar texto citado (Ctrl-Q para abrir/cerrar)
- Respuesta y composición de nuevos mensajes
- Menú ESC para guardar, abortar, alternar insertar/sobreescribir y más
- **Selector de color Ctrl-K**: Abre un diálogo para seleccionar colores de primer plano y fondo, insertando un código de escape ANSI en la posición del cursor. Admite 16 colores de primer plano (8 normales + 8 brillantes) y 8 fondos, con una vista previa en vivo. Presione **N** para insertar un código de reinicio.
- **Renderizado con reconocimiento de color**: El área de edición renderiza los códigos ANSI y de atributos BBS en línea, para que el texto en color se muestre mientras escribe
- **Diálogo de configuración de usuario Ctrl-U** para configurar las preferencias del editor sobre la marcha
- **Indicadores de sí/no específicos del estilo**: El modo Ice usa un indicador en línea en la parte inferior de la pantalla; el modo DCT usa un cuadro de diálogo centrado con colores temáticos

### Configuración del editor (a través de Ctrl-U)
- **Elegir modo de interfaz**: Diálogo para cambiar entre los estilos Ice, DCT y Aleatorio (tiene efecto inmediatamente)
- **Seleccionar archivo de tema**: Elija entre los temas de color Ice o DCT disponibles
- **Taglines**: Cuando está habilitado, solicita la selección de tagline al guardar (desde `tagline_files/taglines.txt`)
- **Diccionario/diccionarios de corrección ortográfica**: Seleccione qué diccionarios usar
- **Solicitar corrector ortográfico al guardar**: Cuando está habilitado, ofrece verificar la ortografía antes de guardar
- **Ajustar líneas de cita al ancho del terminal**: Ajustar líneas citadas con salto de línea
- **Citar con las iniciales del autor**: Anteponer líneas de cita con las iniciales del autor (p. ej., `MP> `)
- **Sangrar líneas de cita que contienen iniciales**: Añadir espacio inicial antes de las iniciales (p. ej., ` MP> `)
- **Eliminar espacios de las líneas de cita**: Eliminar espacios en blanco iniciales del texto citado

### Temas de color
- Los archivos de tema son archivos de configuración (`.ini`) en el directorio `config_files/`
- Temas Ice: `EditorIceColors_*.ini` (BlueIce, EmeraldCity, FieryInferno, etc.)
- Temas DCT: `EditorDCTColors_*.ini` (Default, Default-Modified, Midnight)
- Los colores del tema usan un formato simple: letra de primer plano (`r`/`g`/`b`/`c`/`y`/`m`/`w`/`k`), `h` opcional para brillante, dígito de fondo opcional (`0`-`7`)
- Los temas controlan todos los colores de elementos de la interfaz: bordes, etiquetas, valores, ventana de citas, barra de ayuda, diálogos de sí/no

### Corrector ortográfico
- Corrector ortográfico integrado usando archivos de diccionario de texto plano
- Se distribuye con diccionarios en inglés (suplementos en, en-US, en-GB, en-AU, en-CA)
- Diálogo de corrección interactivo: Reemplazar, Omitir o Salir
- Archivos de diccionario almacenados en el directorio `dictionary_files/`

### Taglines
- Los archivos de taglines se almacenan en el directorio `tagline_files/`
- El archivo de taglines predeterminado es `tagline_files/taglines.txt`, una tagline por línea
- Las líneas que comienzan con `#` o `;` se tratan como comentarios y se ignoran
- Seleccione una tagline específica o elija una al azar al guardar un mensaje
- Las taglines se añaden a los mensajes con el prefijo `...`

### Creación de paquetes REP
- Actualmente, solo se admite ZIP (quiero agregar soporte para más tipos de compresión en el futuro)
- Cuando escribe respuestas o nuevos mensajes, se ponen en cola como pendientes
- Los votos (papeletas de encuestas, votos arriba/abajo) también se ponen en cola junto con las respuestas
- Al salir (o al abrir un nuevo archivo QWK), SlyMail solicita guardar todos los elementos pendientes
- Crea un archivo `.rep` estándar (archivo ZIP) para subir al BBS, que contiene:
  - `<BBSID>.MSG` — mensajes de respuesta en formato QWK estándar
  - `HEADERS.DAT` — encabezados extendidos QWKE para campos que superan los 25 caracteres
  - `VOTING.DAT` — votos pendientes en formato INI compatible con Synchronet
- El archivo REP se guarda como `<BBS-ID>.rep` en el directorio de respuestas configurado (o el directorio del archivo QWK)

### Sistemas remotos (Ctrl-R)
SlyMail puede descargar paquetes QWK directamente desde sistemas remotos a través de FTP o SFTP (SSH):
- Presione **Ctrl-R** desde el explorador de archivos para abrir el directorio de sistemas remotos
- **Agregar/Editar/Eliminar** entradas de sistemas remotos con: nombre, host, puerto, tipo de conexión (FTP o SSH), nombre de usuario, contraseña, alternador de FTP pasivo y ruta remota inicial
- **Explorar directorios remotos** con un explorador de archivos/directorios similar al explorador de archivos local — navegar por directorios, subir con `..`, saltar a la raíz con `/`
- **Descargar archivos QWK** del sistema remoto directamente al subdirectorio `QWK` del directorio de datos de SlyMail
- Las entradas de sistemas remotos se guardan en `remote_systems.json` en el directorio de datos de SlyMail
- La última fecha/hora de conexión se rastrea para cada sistema
- Usa el comando `curl` del sistema para transferencias FTP y SFTP (sin dependencias de bibliotecas en tiempo de compilación)

### Configuración de la aplicación
- La configuración persistente se guarda en `slymail.ini` en el directorio de datos de SlyMail (`~/.slymail` en Linux/macOS/BSD, o el directorio de inicio del usuario en Windows)
- El directorio de datos de SlyMail y su subdirectorio `QWK` se crean automáticamente en el primer inicio
- El directorio predeterminado para explorar archivos QWK y guardar paquetes REP es `~/.slymail/QWK`
- Recuerda el último directorio explorado y el nombre del archivo QWK
- Atajo de teclado Ctrl-L para cargar un archivo QWK diferente desde las vistas de lista de conferencias o mensajes
- Prefijo de cita configurable, ancho de línea de cita, nombre de usuario
- Opciones del lector: mostrar/ocultar líneas kludge, líneas de desgarro/origen, barra de desplazamiento, eliminar códigos ANSI
- Alternadores de código de atributo por BBS (Synchronet, WWIV, Celerity, Renegade, PCBoard/Wildcat) — afectan tanto al lector como al editor
- Directorio de salida de paquetes REP

## Capturas de pantalla

<p align="center">
	<a href="screenshots/SlyMail_01_OpeningScreen.png" target='_blank'><img src="screenshots/SlyMail_01_OpeningScreen.png" alt="Pantalla de inicio" width="800"></a>
	<a href="screenshots/SlyMail_02_File_Chooser.png" target='_blank'><img src="screenshots/SlyMail_02_File_Chooser.png" alt="Selector de archivos: Búsqueda de archivo QWK" width="800"></a>
	<a href="screenshots/SlyMail_03_remote_system_list.png" target='_blank'><img src="screenshots/SlyMail_03_remote_system_list.png" alt="Lista de sistemas remotos" width="800"></a>
	<a href="screenshots/SlyMail_04_Remote_System_Edit.png" target='_blank'><img src="screenshots/SlyMail_04_Remote_System_Edit.png" alt="Editar un sistema remoto" width="800"></a>
	<a href="screenshots/SlyMail_05_Remote_System_Browsing.png" target='_blank'><img src="screenshots/SlyMail_05_Remote_System_Browsing.png" alt="Exploración de sistema remoto" width="800"></a>
	<a href="screenshots/SlyMail_06_msg_area_list.png" target='_blank'><img src="screenshots/SlyMail_06_msg_area_list.png" alt="Lista de áreas de mensajes" width="800"></a>
	<a href="screenshots/SlyMail_07_msg_list.png" target='_blank'><img src="screenshots/SlyMail_07_msg_list.png" alt="Lista de mensajes" width="800"></a>
	<a href="screenshots/SlyMail_08_reading_msg.png" target='_blank'><img src="screenshots/SlyMail_08_reading_msg.png" alt="Leyendo un mensaje" width="800"></a>
	<a href="screenshots/SlyMail_09_msg_edit_start.png" target='_blank'><img src="screenshots/SlyMail_09_msg_edit_start.png" alt="Inicio de edición de mensaje" width="800"></a>
	<a href="screenshots/SlyMail_10_quote_line_selection.png" target='_blank'><img src="screenshots/SlyMail_10_quote_line_selection.png" alt="Editor: Selección de línea de cita" width="800"></a>
	<a href="screenshots/SlyMail_11_writing_reply_msg.png" target='_blank'><img src="screenshots/SlyMail_11_writing_reply_msg.png" alt="Editor: Editando un mensaje" width="800"></a>
	<a href="screenshots/SlyMail_12_editor_color_picker.png" target='_blank'><img src="screenshots/SlyMail_12_editor_color_picker.png" alt="Editor: Selector de color" width="800"></a>
	<a href="screenshots/SlyMail_13_Sync_poll_msg.png" target='_blank'><img src="screenshots/SlyMail_13_Sync_poll_msg.png" alt="Mensaje de encuesta de Synchronet" width="800"></a>
	<a href="screenshots/SlyMail_14_reader_settings.png" target='_blank'><img src="screenshots/SlyMail_14_reader_settings.png" alt="Configuración del lector" width="800"></a>
	<a href="screenshots/SlyMail_15_editor_settings.png" target='_blank'><img src="screenshots/SlyMail_15_editor_settings.png" alt="Configuración del editor" width="800"></a>
	<a href="screenshots/SlyMail_16_msg_search.png" target='_blank'><img src="screenshots/SlyMail_16_msg_search.png" alt="Búsqueda de mensajes" width="800"></a>
	<a href="screenshots/SlyMail_17_Advanced_msg_search.png" target='_blank'><img src="screenshots/SlyMail_17_Advanced_msg_search.png" alt="Búsqueda avanzada de mensajes" width="800"></a>
	<a href="screenshots/SlyMail_18_advanced_msg_search_date_picker" target='_blank'><img src="screenshots/SlyMail_18_advanced_msg_search_date_picker" alt="Selector de fecha en búsqueda avanzada de mensajes" width="800"></a>
	<a href="screenshots/SlyMail_19_config_program.png" target='_blank'><img src="screenshots/SlyMail_19_config_program.png" alt="Programa de configuración" width="800"></a>
	<a href="screenshots/SlyMail_20_reading_ANSI_art.png" target='_blank'><img src="screenshots/SlyMail_20_reading_ANSI_art.png" alt="Leyendo un mensaje con arte ANSI" width="800"></a>
	<a href="screenshots/SlyMail_21_reading_ANSI_art.png" target='_blank'><img src="screenshots/SlyMail_21_reading_ANSI_art.png" alt="Leyendo un mensaje con arte ANSI" width="800"></a>
	<a href="screenshots/SlyMail_22_reading_ANSI_art.png" target='_blank'><img src="screenshots/SlyMail_22_reading_ANSI_art.png" alt="Leyendo un mensaje con arte ANSI" width="800"></a>
	<a href="screenshots/SlyMail_23_reading_ANSI_art.png" target='_blank'><img src="screenshots/SlyMail_23_reading_ANSI_art.png" alt="Leyendo un mensaje con arte ANSI" width="800"></a>
</p>

## Configuración de Synchronet para un paquete QWK
En un BBS Synchronet, en la configuración del paquete QWK, SlyMail es (o debería ser) compatible con los códigos de color Ctrl-A, VOTING.DAT, archivos adjuntos y el formato de paquete QWKE. SlyMail también debería ser compatible con UTF-8. Por ejemplo:
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

## Compilación

### Requisitos

**Linux / macOS / BSD:**
- Compilador compatible con C++17 (GCC 8+, Clang 7+)
- Biblioteca de desarrollo ncurses (`libncurses-dev` en Debian/Ubuntu, `ncurses-devel` en Fedora/RHEL)
- Comando `unzip` (para extraer paquetes QWK)
- Comando `zip` (para crear paquetes REP)
- Comando `curl` (para transferencias FTP/SFTP de sistemas remotos — opcional, solo necesario para la función de sistemas remotos)

**Windows (Visual Studio 2022):**
- Visual Studio 2022 con la carga de trabajo "Desarrollo de escritorio con C++"
- Windows SDK 10.0 (incluido con VS)
- No se requieren bibliotecas adicionales — usa la API de consola Win32 integrada para la interfaz de terminal, y `tar.exe` o PowerShell para el manejo de ZIP de paquetes QWK/REP (ver nota a continuación)

**Windows (MinGW/MSYS2):**
- MinGW-w64 o MSYS2 con GCC (soporte de C++17)
- API de consola de Windows (integrada)

> **Nota — Manejo de ZIP QWK/REP en Windows:** SlyMail detecta en tiempo de ejecución qué herramienta está disponible y usa la mejor opción:
>
> - **`tar.exe` (preferido):** Se incluye con Windows 10 versión 1803 (Actualización de abril de 2018) y posterior, y con todas las versiones de Windows 11. `tar` lee archivos ZIP por su contenido en lugar de su extensión de archivo, por lo que los paquetes `.qwk` se extraen directamente y los paquetes `.rep` se crean a través de un archivo `.zip` temporal que luego se renombra. No se necesita configuración adicional.
> - **PowerShell (alternativa):** Si `tar.exe` no se encuentra en el PATH, SlyMail recurre a PowerShell. Para la extracción usa la clase .NET `ZipFile` (`System.IO.Compression`) en lugar de `Expand-Archive`, porque `Expand-Archive` rechaza extensiones de archivo que no sean `.zip` incluso cuando el archivo es un archivo ZIP válido. Para la creación de paquetes REP usa `Compress-Archive`, escribiendo también a un archivo `.zip` temporal que luego se renombra a `.rep`.

### Compilar en Linux/macOS/BSD

```bash
make
```

Esto compila dos programas:
- `slymail` - la aplicación principal del lector QWK
- `config` - la utilidad de configuración independiente

### Compilar con símbolos de depuración

```bash
make debug
```

### Instalar (opcional)

```bash
sudo make install    # Installs slymail and config to /usr/local/bin/
sudo make uninstall  # Remove
```

### Compilar en Windows con Visual Studio 2022

Abra el archivo de solución en Visual Studio 2022:

```
vs\SlyMail.sln
```

O compile desde la línea de comandos usando MSBuild:

```powershell
# Release build (output in vs\x64\Release\)
msbuild vs\SlyMail.sln /p:Configuration=Release /p:Platform=x64

# Debug build (output in vs\x64\Debug\)
msbuild vs\SlyMail.sln /p:Configuration=Debug /p:Platform=x64
```

Esto compila dos ejecutables:
- `x64\Release\slymail.exe` — el lector QWK principal
- `x64\Release\config.exe` — la utilidad de configuración independiente

La solución contiene dos proyectos (`SlyMail.vcxproj` y `Config.vcxproj`) orientados a x64, C++17, con el conjunto de herramientas MSVC v143.

### Compilar en Windows (MinGW/MSYS2)

```bash
make
```

El Makefile detecta automáticamente la plataforma y usa la implementación de terminal apropiada:
- **Linux/macOS/BSD**: ncurses (`terminal_ncurses.cpp`)
- **Windows**: conio + Win32 Console API (`terminal_win32.cpp`)

## Uso

```bash
# Launch SlyMail with file browser
./slymail

# Open a specific QWK packet
./slymail MYBBS.qwk

# Run the standalone configuration utility
./config
```

### Programa de configuración

La utilidad `config` proporciona una interfaz de texto independiente para configurar los ajustes de SlyMail sin abrir la aplicación principal. Ofrece cuatro categorías de configuración:

- **Configuración del editor** - Todos los mismos ajustes disponibles a través de Ctrl-U en el editor (estilo de editor, taglines, corrección ortográfica, opciones de cita, etc.)
- **Configuración del lector** - Alternar líneas kludge, líneas de desgarro, barra de desplazamiento, eliminación de ANSI, modo lightbar, orden inverso y alternadores de código de atributo (habilitar/deshabilitar por BBS)
- **Configuración de temas** - Seleccionar archivos de tema de color Ice y DCT del directorio `config_files/`
- **Configuración general** - Establecer su nombre para las respuestas y el directorio de salida de paquetes REP

Los ajustes se guardan automáticamente al salir de cada categoría. Tanto SlyMail como la utilidad de configuración leen y escriben el mismo archivo de configuración.

### Atajos de teclado

#### Explorador de archivos
| Tecla | Acción |
|-------|--------|
| Arriba/Abajo | Navegar por archivos y directorios |
| Enter | Abrir directorio / Seleccionar archivo QWK |
| Ctrl-R | Abrir directorio de sistemas remotos |
| Q / ESC | Salir |

#### Lista de conferencias
| Tecla | Acción |
|-------|--------|
| Arriba/Abajo | Navegar por conferencias |
| Enter | Abrir la conferencia seleccionada |
| V | Ver encuestas/votos en el paquete |
| O / Ctrl-L | Abrir un archivo QWK diferente |
| S / Ctrl-U | Configuración |
| Q / ESC | Salir de SlyMail |
| ? / F1 | Ayuda |

#### Lista de mensajes
| Tecla | Acción |
|-------|--------|
| Arriba/Abajo | Navegar por mensajes |
| Enter / R | Leer el mensaje seleccionado |
| N | Escribir un nuevo mensaje |
| G | Ir al número de mensaje |
| Ctrl-L | Abrir un archivo QWK diferente |
| S / Ctrl-U | Configuración |
| C / ESC | Volver a la lista de conferencias |
| Q | Salir |
| ? / F1 | Ayuda |

#### Lector de mensajes
| Tecla | Acción |
|-------|--------|
| Arriba/Abajo | Desplazar mensaje |
| Izquierda/Derecha | Mensaje anterior / siguiente |
| F / L | Primer / último mensaje |
| R | Responder al mensaje |
| V | Votar (voto arriba/abajo o papeleta de encuesta) |
| D / Ctrl-D | Descargar archivos adjuntos |
| H | Mostrar información del encabezado del mensaje |
| S / Ctrl-U | Configuración |
| C / Q / ESC | Volver a la lista de mensajes |
| ? / F1 | Ayuda |

#### Editor de mensajes
| Tecla | Acción |
|-------|--------|
| ESC | Menú del editor (Guardar, Abortar, etc.) |
| Ctrl-U | Diálogo de configuración de usuario |
| Ctrl-Q | Abrir/cerrar ventana de citas |
| Ctrl-K | Selector de color (insertar código de color ANSI en la posición del cursor) |
| Ctrl-G | Insertar carácter gráfico (CP437) por código |
| Ctrl-W | Búsqueda de palabra/texto |
| Ctrl-S | Cambiar asunto |
| Ctrl-D | Eliminar la línea actual |
| Ctrl-Z | Guardar mensaje |
| Ctrl-A | Abortar mensaje |
| F1 | Pantalla de ayuda |
| Insert | Alternar modo Insertar/Sobreescribir |

#### Ventana de citas
| Tecla | Acción |
|-------|--------|
| Arriba/Abajo | Navegar por líneas de cita |
| Enter | Insertar la línea de cita seleccionada |
| Ctrl-Q / ESC | Cerrar ventana de citas |

## Arquitectura

SlyMail utiliza una capa de abstracción de plataforma para su interfaz de usuario de texto:

```
ITerminal (abstract base class)
    ├── NCursesTerminal  (Linux/macOS/BSD - ncurses)
    └── Win32Terminal    (Windows - conio + Win32 Console API)
```

Los caracteres de dibujo de caja y caracteres especiales CP437 están definidos en `cp437defs.h` y se renderizan a través del método `putCP437()`, que mapea los códigos CP437 a equivalentes nativos de la plataforma (caracteres ACS en ncurses, bytes CP437 directos en Windows).

### Archivos fuente

| Archivo | Descripción |
|---------|-------------|
| `terminal.h` | Interfaz abstracta `ITerminal`, constantes de teclas/colores, fábrica |
| `terminal_ncurses.cpp` | Implementación de ncurses con mapeo CP437-a-ACS |
| `terminal_win32.cpp` | API de consola de Windows + implementación de conio |
| `cp437defs.h` | Definiciones de caracteres de la página de códigos IBM 437 |
| `colors.h` | Definiciones de esquemas de colores (Ice, DCT, lector, lista) |
| `theme.h` | Analizador de archivos de configuración de temas (códigos de atributo estilo Synchronet) |
| `ui_common.h` | Auxiliares de interfaz compartidos (diálogos, entrada de texto, barra de desplazamiento, etc.) |
| `qwk.h` / `qwk.cpp` | Analizador y creador de paquetes QWK/REP (QWKE, adjuntos, votaciones) |
| `bbs_colors.h` / `bbs_colors.cpp` | Analizador de códigos de color/atributo BBS (ANSI, Synchronet, WWIV, PCBoard, Celerity, Renegade) |
| `utf8_util.h` / `utf8_util.cpp` | Utilidades UTF-8 (validación, ancho de visualización, conversión CP437-a-UTF-8) |
| `voting.h` / `voting.cpp` | Analizador VOTING.DAT, recuento de votos, interfaz de visualización de encuestas |
| `remote_systems.h` / `remote_systems.cpp` | Directorio de sistemas remotos, exploración FTP/SFTP, persistencia JSON, utilidades de directorio de inicio |
| `settings.h` | Persistencia de configuración de usuario |
| `settings_dialog.h` | Diálogos de configuración (editor, lector, alternadores de código de atributo) |
| `file_browser.h` | Explorador y selector de archivos QWK |
| `msg_list.h` | Vistas de lista de conferencias y mensajes |
| `msg_reader.h` | Lector de mensajes (estilo DDMsgReader) con interfaz de votación y adjuntos |
| `msg_editor.h` | Editor de mensajes (estilo SlyEdit Ice/DCT) con selector de color |
| `main.cpp` | Punto de entrada de la aplicación SlyMail y bucle principal |
| `config.cpp` | Utilidad de configuración independiente |

## Configuración

### Archivo de configuración

Los ajustes se almacenan en un archivo INI llamado `slymail.ini` en el mismo directorio que el ejecutable de SlyMail. Este archivo es compartido entre SlyMail y la utilidad `config`. El archivo está bien comentado con descripciones de cada ajuste.

Ejemplo `slymail.ini`:
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

### Archivos de tema

Los temas de color son archivos `.ini` en el directorio `config_files/`:

**Temas Ice** (`EditorIceColors_*.ini`):
- BlueIce (predeterminado), EmeraldCity, FieryInferno, Fire-N-Ice, GeneralClean, GenericBlue, PurpleHaze, ShadesOfGrey

**Temas DCT** (`EditorDCTColors_*.ini`):
- Default (predeterminado), Default-Modified, Midnight

Los valores de color del tema usan un formato compacto derivado de los códigos de atributo de Synchronet:
- `n` = normal (restablecer)
- Primer plano: `k`=negro, `r`=rojo, `g`=verde, `y`=amarillo, `b`=azul, `m`=magenta, `c`=cyan, `w`=blanco
- `h` = alta/brillante intensidad
- Dígito de fondo: `0`=negro, `1`=rojo, `2`=verde, `3`=marrón, `4`=azul, `5`=magenta, `6`=cyan, `7`=gris claro

Ejemplo: `nbh` = azul brillante normal, `n4wh` = blanco brillante sobre fondo azul

### Taglines

Las taglines son citas cortas o dichos que se añaden al final de los mensajes al guardarlos. La función de taglines se puede habilitar a través de Ctrl-U en el editor o la utilidad `config`.

Las taglines se almacenan en `tagline_files/taglines.txt`, una por línea. Las líneas que comienzan con `#` o `;` se tratan como comentarios y se ignoran. Al guardar un mensaje con taglines habilitadas, el usuario recibe un aviso para seleccionar una tagline específica o elegir una al azar. Las taglines seleccionadas se añaden al mensaje con el prefijo `...` (p. ej., `...To err is human, to really foul things up requires a computer.`).

### Corrector ortográfico

SlyMail incluye un corrector ortográfico integrado que usa archivos de diccionario de texto plano. El corrector ortográfico se puede configurar para que solicite al guardar a través de Ctrl-U en el editor o la utilidad `config`.

**Los archivos de diccionario** son archivos de texto plano (una palabra por línea) almacenados en `dictionary_files/`. Se pueden seleccionar múltiples diccionarios simultáneamente para una cobertura de palabras combinada. SlyMail se incluye con:
- `dictionary_en.txt` - Inglés (general, ~130K palabras)
- `dictionary_en-US-supplemental.txt` - Suplemento de inglés de EE.UU.
- `dictionary_en-GB-supplemental.txt` - Suplemento de inglés británico
- `dictionary_en-AU-supplemental.txt` - Suplemento de inglés australiano
- `dictionary_en-CA-supplemental.txt` - Suplemento de inglés canadiense

Cuando se activa la corrección ortográfica, el verificador escanea el mensaje en busca de palabras mal escritas y presenta un diálogo interactivo para cada una, ofreciendo opciones para **R**eemplazar la palabra, **O**mitirla, **A**gregarla (futuro) o **S**alir de la verificación.

## Créditos

- Interfaz inspirada en [DDMsgReader](https://github.com/SynchronetBBS/sbbs) y [SlyEdit](https://github.com/SynchronetBBS/sbbs) por [Nightfox (Eric Oulashin)](https://github.com/EricOulashin)
- Compatibilidad con el formato QWK basada en el código fuente de [Synchronet BBS](https://www.synchro.net/)
- Definiciones de caracteres CP437 de Synchronet

## Licencia

Este proyecto es software de código abierto.
