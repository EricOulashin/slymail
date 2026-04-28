---
title: "Manual de Usuario de SlyMail"
subtitle: "Lector de correo sin conexión QWK"
version: "0.55"
date: "2026-04-09"
author: "Eric Oulashin"
---

# Manual de Usuario de SlyMail

## Introducción

SlyMail es un lector de correo sin conexión, multiplataforma y basado en texto, para el formato de paquetes QWK. Los paquetes QWK se usan para intercambiar correo en Sistemas de Tablón de Anuncios (BBS). SlyMail ofrece una interfaz completa para leer, buscar y responder a mensajes de paquetes de correo QWK.

Su interfaz de usuario está inspirada en Digital Distortion Message Reader (DDMsgReader) para la lectura de mensajes y en SlyEdit para la edición de mensajes, ambos creados originalmente para Synchronet BBS.

SlyMail funciona en Linux, macOS, BSD y Windows.

## Primeros pasos

### Iniciar SlyMail

![Pantalla de inicio de SlyMail](../screenshots/SlyMail_01_OpeningScreen.png)

Inicia SlyMail desde la línea de comandos:

    slymail

Esto muestra la pantalla de presentación (si está habilitada) y abre el explorador de archivos para seleccionar un paquete QWK.

Para abrir directamente un paquete QWK específico:

    slymail MYBBS.qwk

O usa el parámetro con nombre:

    slymail -qwk_file=/ruta/a/MYBBS.qwk

Otras opciones de línea de comandos:

- `-v` o `--version` - Mostrar información de versión
- `-?` o `--help` - Mostrar ayuda

### El directorio de datos de SlyMail

En el primer inicio, SlyMail crea un directorio de datos en `~/.slymail` (en Linux, macOS y BSD) o en tu directorio personal en Windows. Este directorio contiene:

- `slymail.ini` - Tu configuración
- `QWK/` - Directorio predeterminado para paquetes de correo QWK
- `REP/` - Directorio predeterminado para paquetes de respuesta
- `config_files/` - Archivos de temas de color
- `dictionary_files/` - Diccionarios del corrector ortográfico
- `tagline_files/` - Archivos de “taglines”
- `remote_systems.json` - Conexiones a sistemas remotos guardadas
- `lastread_<BBSID>.json` - Punteros de “último leído” por conferencia

### El programa de configuración

SlyMail incluye un programa de configuración independiente llamado `config` (o `config_slymail` si se instaló con `make install`). Este programa ofrece una interfaz en modo texto para configurar todas las opciones de SlyMail sin abrir la aplicación principal.

Ejecútalo desde la línea de comandos:

    config_slymail

El programa de configuración ofrece estas categorías:

- **Configuración del editor** - Estilo del editor, taglines, corrector ortográfico, opciones de citado, selección de tema
- **Configuración del lector** - Líneas “kludge”, líneas de despedida, barra de desplazamiento, eliminación de ANSI, orden inverso, alternancias de códigos de atributo
- **Configuración de temas** - Seleccionar archivos de tema de color Ice y DCT
- **Configuración general** - Tu nombre para respuestas, directorio de paquetes de respuesta, configuración de editor externo, modo “lightbar”, pantalla de presentación

La configuración se guarda automáticamente al salir de cada categoría. Tanto SlyMail como el programa `config` leen y escriben el mismo archivo `slymail.ini`.

## Explorador de archivos

Cuando SlyMail se inicia, se muestra un explorador de archivos para seleccionar un paquete QWK para abrir.

![Explorador de archivos](../screenshots/SlyMail_02_File_Chooser.png)

### Teclas del explorador de archivos

| Tecla | Acción |
|-----|--------|
| Arriba / Abajo | Navegar por archivos y directorios |
| Enter | Abrir directorio o seleccionar un archivo QWK |
| RePág / AvPág | Desplazar la lista de archivos |
| Inicio / Fin | Ir a la primera o última entrada |
| Ctrl-R | Abrir el directorio de sistemas remotos |
| Q / ESC | Salir de SlyMail |
| ? / F1 | Ayuda |

Solo se pueden seleccionar archivos `.qwk`. Los directorios se muestran en azul y se puede entrar en ellos pulsando Enter.

## Sistemas remotos (Ctrl-R)

![Lista de sistemas remotos](../screenshots/SlyMail_03_remote_system_list.png)

SlyMail puede descargar paquetes QWK directamente desde BBS remotas mediante FTP o SFTP (SSH).

Pulsa **Ctrl-R** desde el explorador de archivos, la lista de conferencias o la lista de mensajes para abrir el administrador de sistemas remotos. Desde aquí puedes:

- **Añadir** un nuevo sistema remoto con hostname, puerto, tipo de conexión (FTP o SSH), usuario, contraseña, alternancia de FTP pasivo y ruta remota inicial
- **Editar** o **Eliminar** entradas existentes
- **Conectar** a un sistema remoto y explorar sus directorios
- **Descargar** archivos QWK directamente en tu directorio `QWK/`
- **Subir** paquetes de respuesta REP al sistema remoto

Las entradas de sistemas remotos se guardan en `remote_systems.json` dentro del directorio de datos de SlyMail. Se registra la fecha y hora de la última conexión para cada sistema.

SlyMail utiliza el comando `curl` del sistema para transferencias de archivos por FTP y SFTP.

## Lista de conferencias

![Lista de conferencias](../screenshots/SlyMail_06_msg_area_list.png)

Tras abrir un paquete QWK, la lista de conferencias muestra todas las áreas de mensajes (conferencias) con sus recuentos. Una columna “New” muestra una marca de verificación junto a las conferencias que tienen mensajes nuevos. Si la opción “Only show areas with new mail” está habilitada, las conferencias con cero mensajes se ocultan.

### Teclas de la lista de conferencias

| Tecla | Acción |
|-----|--------|
| Arriba / Abajo | Navegar por conferencias |
| RePág / AvPág | Desplazar la lista |
| Inicio / Fin | Ir a la primera o última conferencia |
| Enter (E) | Abrir la conferencia seleccionada |
| G | Ir a un número de conferencia específico |
| / | Buscar conferencias por nombre |
| V | Ver encuestas y votos en el paquete |
| O / Ctrl-L | Abrir otro archivo QWK |
| Ctrl-R | Abrir sistemas remotos |
| Ctrl-P | Guardar el paquete de respuesta REP |
| S / Ctrl-U | Abrir configuración |
| Q / ESC | Salir de SlyMail |
| ? / F1 | Ayuda |

Cuando hay un filtro de búsqueda activo, al pulsar **Q** se borra el filtro en lugar de salir.

## Lista de mensajes

![Lista de mensajes](../screenshots/SlyMail_07_msg_list.png)

La lista de mensajes muestra todos los mensajes de la conferencia seleccionada con columnas para número de mensaje, de, para, asunto, fecha y hora.

### Seguimiento de “último leído”

SlyMail recuerda el último mensaje que leíste en cada conferencia. Cuando entras en una conferencia, el cursor se posiciona automáticamente en el primer mensaje no leído (el mensaje posterior al último que leíste). Esta información se guarda por BBS en el directorio de datos de SlyMail.

### Teclas de la lista de mensajes

| Tecla | Acción |
|-----|--------|
| Arriba / Abajo | Navegar por mensajes |
| RePág / AvPág | Desplazar la lista |
| Inicio / Fin | Ir al primero o último mensaje |
| Enter / R | Leer el mensaje seleccionado |
| N | Escribir un mensaje nuevo |
| G | Ir a un número de mensaje específico |
| / | Buscar mensajes |
| Ctrl-A | Búsqueda avanzada (con rango de fechas) |
| Ctrl-L | Abrir otro archivo QWK |
| Ctrl-R | Abrir sistemas remotos |
| Ctrl-P | Guardar el paquete de respuesta REP |
| S / Ctrl-U | Abrir configuración |
| C / ESC | Volver a la lista de conferencias |
| Q | Salir |
| ? / F1 | Ayuda |

### Buscar mensajes

![Búsqueda de mensajes](../screenshots/SlyMail_16_msg_search.png)

Pulsa **/** para buscar mensajes por asunto, texto del cuerpo, remitente o destinatario. Pulsa **Ctrl-A** para la búsqueda avanzada, que incluye filtrado por rango de fechas con un selector visual de calendario. Cuando hay un filtro activo, al pulsar **Q** se borra el filtro.

La búsqueda puede usar coincidencia por subcadenas o expresiones regulares, según la opción “Search using regular expression”.

## Lector de mensajes

![Leyendo un mensaje](../screenshots/SlyMail_08_reading_msg.png)

El lector de mensajes muestra el contenido con un encabezado que incluye De, Para, Asunto y Fecha.

### Teclas del lector de mensajes

| Tecla | Acción |
|-----|--------|
| Arriba / Abajo | Desplazar el contenido |
| RePág / AvPág | Página arriba o abajo |
| Inicio / Fin | Ir al inicio o al final |
| Izquierda / Derecha | Mensaje anterior o siguiente |
| F / L | Primero o último mensaje |
| R | Responder al mensaje |
| V | Votar (voto a favor/en contra o papeleta de encuesta) |
| D / Ctrl-D | Descargar archivos adjuntos |
| H | Mostrar encabezado detallado |
| S / Ctrl-U | Abrir configuración |
| C / Q / ESC / M | Volver a la lista de mensajes |
| ? / F1 | Ayuda |

### Arte ANSI

![Arte ANSI](../screenshots/SlyMail_20_reading_ANSI_art.png)

SlyMail representa arte ANSI en mensajes, con soporte de posicionamiento de cursor, colores y caracteres de bloque CP437. Los mensajes que contienen secuencias de control de cursor ANSI se detectan automáticamente y se renderizan mediante un búfer de pantalla virtual para mostrarse correctamente.

### Archivos adjuntos

Cuando un mensaje tiene adjuntos (indicados por **[ATT]** en el encabezado), pulsa **D** o **Ctrl-D** para ver y descargar. Se mostrará una lista de archivos adjuntos con sus tamaños y se te pedirá elegir un directorio de destino.

### Votaciones y encuestas

SlyMail admite votación al estilo Synchronet:

- **Encuestas**: los mensajes identificados como encuestas muestran opciones de respuesta con recuentos y barras de porcentaje. Pulsa **V** para abrir un diálogo de papeleta donde puedes alternar selecciones y emitir tu voto.
- **Votos a favor/en contra**: en mensajes normales, pulsa **V** para votar a favor o en contra. Los totales y la puntuación neta se muestran en el encabezado.
- **Navegador de encuestas**: pulsa **V** desde la lista de conferencias para explorar todas las encuestas del paquete.

Los votos se ponen en cola junto con las respuestas y se escriben en el paquete REP.

## Editor de mensajes

![Editor de mensajes](../screenshots/SlyMail_09_msg_edit_start.png)

El editor de mensajes se usa para redactar respuestas y mensajes nuevos. Ofrece dos modos visuales inspirados en SlyEdit: **Ice** y **DCT**.

### Teclas del editor

| Tecla | Acción |
|-----|--------|
| Flechas | Mover el cursor |
| Inicio / Fin | Inicio o fin de la línea |
| RePág / AvPág | Página arriba o abajo |
| Insert | Alternar Insertar/Sobrescribir |
| Delete | Borrar el carácter en el cursor |
| Retroceso | Borrar el carácter anterior |
| ESC | Abrir el menú del editor |
| Ctrl-Z | Guardar el mensaje |
| Ctrl-A | Abortar el mensaje |
| Ctrl-Q | Abrir o cerrar la ventana de cita |
| Ctrl-K | Abrir el selector de colores |
| Ctrl-G | Insertar un carácter gráfico CP437 |
| Ctrl-O | Importar un archivo de texto en la posición del cursor |
| Ctrl-W | Buscar texto |
| Ctrl-S | Cambiar el asunto |
| Ctrl-D | Eliminar la línea actual |
| Ctrl-T | Listar reemplazos de texto |
| Ctrl-U | Abrir configuración del editor |
| F1 | Pantalla de ayuda |

### Comandos con barra

También puedes escribir comandos en una línea vacía y pulsar Enter:

| Comando | Acción |
|---------|--------|
| /S | Guardar el mensaje |
| /A | Abortar el mensaje |
| /Q | Abrir la ventana de cita |
| /U | Abrir configuración del editor |
| /? | Mostrar ayuda |

### Ventana de cita

![Ventana de cita](../screenshots/SlyMail_10_quote_line_selection.png)

Al responder a un mensaje, pulsa **Ctrl-Q** o escribe `/Q` en una línea vacía para abrir la ventana de cita. La ventana muestra el texto original con prefijos de cita.

| Tecla | Acción |
|-----|--------|
| Arriba / Abajo | Navegar por líneas citadas |
| RePág / AvPág | Página arriba o abajo |
| Inicio / Fin | Ir a la primera o última línea |
| Enter | Insertar la línea citada seleccionada en tu mensaje |
| Ctrl-Q / ESC | Cerrar la ventana de cita |

Si el número de líneas citadas excede la altura de la ventana, aparece una barra de desplazamiento en el borde derecho.

### Selector de colores (Ctrl-K)

Pulsa **Ctrl-K** para abrir un diálogo interactivo de selección de color. Selecciona un color de primer plano (16 opciones: 8 normales + 8 brillantes) y un color de fondo (8 opciones). Una vista previa muestra la combinación. Pulsa **Enter** para insertar el código ANSI en la posición del cursor. Pulsa **N** para insertar un código de reinicio (normal).

### Importar archivo (Ctrl-O)

Pulsa **Ctrl-O** para importar el contenido de un archivo de texto en la posición actual del cursor. Se abre un explorador de archivos para elegir el archivo. El contenido se inserta línea por línea, preservando saltos de línea.

![Editando un mensaje](../screenshots/SlyMail_11_writing_reply_msg.png)

### Manejo de líneas citadas

Al responder, las líneas insertadas desde la ventana de cita se marcan como líneas citadas. Si editas una línea citada y se ajusta, el desbordamiento pasa a una línea nueva (también marcada como citada) en vez de fusionarse con el texto adyacente. Eliminar texto de una línea citada no “sube” texto desde la línea siguiente si también es citada. Esto preserva la integridad del texto citado.

### Guardado por párrafos

Al guardar un mensaje, SlyMail une líneas ajustadas “suavemente” de texto recién escrito en párrafos largos. Esto permite que otros lectores vuelvan a ajustar el texto a su anchura de terminal. Las líneas citadas se preservan tal cual y nunca se unen.

### Mensajes vacíos

Si guardas un mensaje sin contenido (vacío o solo espacios), SlyMail aborta el mensaje y muestra una notificación en lugar de publicarlo.

### Editores externos

SlyMail permite usar editores de texto externos en lugar del editor integrado. Para configurar un editor externo:

1. Abre Configuración (Ctrl-U o el programa `config`)
2. Ve a **External Editors...** para añadir y configurar editores
3. Establece **External Editor** para elegir cuál usar
4. Habilita **Use external editor**

Cada editor externo puede configurarse con:

- **Name** - Un nombre descriptivo
- **Startup Directory** - Directorio que contiene el ejecutable
- **Command Line** - Comando a ejecutar (usa `%f` para la ruta del archivo temporal)
- **Word-wrap Quoted Text** - Ajustar texto citado y el ancho en columnas
- **Auto Quoted Text** - Incluir citas siempre, preguntar o nunca
- **Editor Information Files** - Tipo de “drop file” (None, MSGINF, EDITOR.INF, DOOR.SYS, DOOR32.SYS)
- **Strip FidoNet Kludges** - Eliminar líneas de control FidoNet del texto citado

Al usar un editor externo, SlyMail crea un archivo temporal, lanza el editor y lee el resultado al salir. Si el editor produce un archivo `RESULT.ED` (compatible con Synchronet), SlyMail lee el nuevo asunto y detalles desde allí.

## Configuración

### Configuración del lector (Ctrl-U)

![Configuración del lector](../screenshots/SlyMail_14_reader_settings.png)

Accede a la configuración del lector pulsando **Ctrl-U** o **S** desde la lista de conferencias, la lista de mensajes o el lector de mensajes.

Opciones disponibles:

- **Show kludge lines** - Mostrar u ocultar líneas kludge/control
- **Show tear/origin lines** - Mostrar u ocultar líneas de despedida y origen
- **Scrollbar in reader** - Mostrar u ocultar la barra de desplazamiento
- **Only show areas with new mail** - Ocultar conferencias vacías
- **Strip ANSI codes from messages** - Eliminar secuencias ANSI
- **Attribute code toggles** - Activar/desactivar interpretación de códigos de color por tipo
- **Search using regular expression** - Usar regex en búsquedas
- **List messages in reversed** - Mostrar los mensajes más nuevos primero
- **Show splash screen on startup** - Alternar la pantalla de presentación
- **Reply packet directory** - Directorio donde guardar REP
- **Use external editor** - Habilitar modo de editor externo
- **External Editors...** - Configurar editores externos
- **External Editor** - Seleccionar qué editor usar
- **Language** - Elegir el idioma de la interfaz (o “OS Default”)

Pulsa **S** para guardar, o **ESC/Q** para cerrar (con aviso de guardar si hubo cambios).

### Configuración del editor (Ctrl-U en el editor)

Accede a la configuración del editor pulsando **Ctrl-U** mientras editas un mensaje.

Opciones disponibles:

- **Choose UI mode** - Cambiar entre Ice, DCT y Random
- **Taglines** - Habilitar taglines al guardar
- **Spell-check dictionary/dictionaries** - Seleccionar diccionarios a usar
- **Prompt for spell checker on save** - Pedir corrección al guardar
- **Wrap quote lines to terminal width** - Ajustar líneas citadas al ancho de terminal
- **Quote with author's initials** - Usar iniciales en el prefijo de cita
- **Indent quote lines containing initials** - Añadir espacio antes de iniciales
- **Trim spaces from quote lines** - Eliminar espacios iniciales en citas
- **Language** - Elegir el idioma de la interfaz (o “OS Default”)

## Compatibilidad con códigos de color de BBS

SlyMail interpreta códigos de color y atributo de múltiples paquetes de BBS:

- **Códigos de escape ANSI** - Siempre habilitados; secuencias SGR estándar
- **Códigos Ctrl-A de Synchronet** - Ctrl-A + carácter de atributo
- **Códigos “corazón” de WWIV** - Ctrl-C + dígito 0-9
- **Códigos @X de PCBoard/Wildcat** - @X seguido de dos dígitos hex
- **Códigos “pipe” de Celerity** - Pipe + letra
- **Códigos “pipe” de Renegade** - Pipe + número de dos dígitos

Cada tipo puede habilitarse o deshabilitarse individualmente en **Attribute Code Toggles**.

## Temas de color

El editor de SlyMail usa temas de color configurables cargados desde archivos `.ini` en `config_files/`.

### Temas Ice

Los temas Ice controlan la apariencia del modo Ice:

- BlueIce (predeterminado)
- EmeraldCity
- FieryInferno
- Fire-N-Ice
- GeneralClean
- GenericBlue
- PurpleHaze
- ShadesOfGrey

### Temas DCT

Los temas DCT controlan la apariencia del modo DCT:

- Default
- Default-Modified
- Midnight

### Formato de color del tema

Los colores usan un formato compacto basado en códigos de atributo de Synchronet:

- `n` - Normal (reinicia atributos)
- Letras de primer plano: `k`=negro, `r`=rojo, `g`=verde, `y`=amarillo, `b`=azul, `m`=magenta, `c`=cian, `w`=blanco
- `h` - Intensidad alta/brillante
- Dígitos de fondo: `0`=negro, `1`=rojo, `2`=verde, `3`=marrón, `4`=azul, `5`=magenta, `6`=cian, `7`=gris claro

Ejemplos: `nbh` = azul brillante, `n4wh` = blanco brillante sobre fondo azul, `nk7` = negro sobre gris claro.

## Corrector ortográfico

SlyMail incluye un corrector ortográfico integrado usando diccionarios de texto plano.

### Diccionarios

Los diccionarios se almacenan en `dictionary_files/`:

- `dictionary_en.txt` - Inglés (general)
- `dictionary_en-US-supplemental.txt` - Suplemento inglés (EE. UU.)
- `dictionary_en-GB-supplemental.txt` - Suplemento inglés (Reino Unido)
- `dictionary_en-AU-supplemental.txt` - Suplemento inglés (Australia)
- `dictionary_en-CA-supplemental.txt` - Suplemento inglés (Canadá)

Se pueden seleccionar varios diccionarios simultáneamente.

### Usar el corrector

Habilita “Prompt for spell checker on save”. Al guardar, el corrector busca palabras mal escritas y ofrece opciones:

- **R**eplace - Escribir un reemplazo
- **S**kip - Omitir
- **Q**uit - Salir del corrector

## Taglines

Las “taglines” son citas cortas que se agregan al final del mensaje al guardar.

### Configuración

1. Habilita “Taglines” en la configuración del editor
2. Coloca taglines en `tagline_files/taglines.txt`, una por línea
3. Las líneas que comienzan con `#` o `;` son comentarios

### Uso

Al guardar un mensaje con taglines habilitadas, se te pide seleccionar una específica o elegir una al azar. La tagline se añade con el prefijo `...`.

## Paquetes de respuesta REP

Cuando escribes respuestas o mensajes nuevos, se ponen en cola como pendientes. Los votos también se ponen en cola.

### Guardar paquetes REP

- Pulsa **Ctrl-P** desde la lista de conferencias o mensajes para guardar el paquete en cualquier momento
- SlyMail solicita guardar tras redactar un mensaje
- Al salir, SlyMail solicita guardar elementos pendientes no guardados

El paquete REP se guarda como `<BBS-ID>.rep` en tu directorio de respuestas (por defecto `~/.slymail/REP/`). Sube este archivo de vuelta al BBS para publicar tus respuestas.

### Contenido del paquete REP

- Respuestas en formato QWK estándar
- `HEADERS.DAT` para campos extendidos que exceden 25 caracteres
- `VOTING.DAT` para votos pendientes (compatible con Synchronet)

## Compatibilidad UTF-8

SlyMail detecta y muestra contenido UTF-8 correctamente:

- Los mensajes UTF-8 se detectan mediante flags de `HEADERS.DAT` y análisis automático
- Aparece un indicador **[UTF8]** en el encabezado del mensaje
- Los caracteres CP437 de contenido heredado se convierten a Unicode

## Referencia del archivo de configuración

La configuración se guarda en `slymail.ini` en el directorio de datos de SlyMail. Es un INI con secciones:

### [General]

| Opción | Predeterminado | Descripción |
|---------|---------|-------------|
| showSplashScreen | true | Mostrar pantalla de presentación al iniciar |
| userName | (vacío) | Tu nombre para el campo From |
| replyDir | (vacío) | Directorio para guardar REP |
| useExternalEditor | false | Usar editor externo |
| selectedEditor | (vacío) | Nombre del editor externo seleccionado |
| language | (vacío) | Código de idioma de UI (vacío = OS Default) |

### [Reader]

| Opción | Predeterminado | Descripción |
|---------|---------|-------------|
| showKludgeLines | false | Mostrar líneas kludge/control |
| showTearLine | true | Mostrar líneas de despedida/origen |
| useScrollbar | true | Mostrar barra de desplazamiento |
| onlyShowAreasWithNewMail | false | Ocultar conferencias vacías |
| stripAnsi | false | Eliminar códigos ANSI |
| attrSynchronet | true | Interpretar Ctrl-A de Synchronet |
| attrWWIV | true | Interpretar “corazones” WWIV |
| attrCelerity | true | Interpretar pipes de Celerity |
| attrRenegade | true | Interpretar pipes de Renegade |
| attrPCBoard | true | Interpretar @X PCBoard/Wildcat |
| useRegexSearch | false | Usar regex en búsquedas |

### [MessageList]

| Opción | Predeterminado | Descripción |
|---------|---------|-------------|
| lightbarMode | true | Navegación con lightbar en lista |
| reverseOrder | false | Mostrar mensajes más nuevos primero |

### [Editor]

| Opción | Predeterminado | Descripción |
|---------|---------|-------------|
| editorStyle | Ice | Modo del editor: Ice, Dct o Random |
| insertMode | true | Modo insertar predeterminado |
| wrapQuoteLines | true | Ajustar líneas citadas |
| quoteLineWidth | 76 | Ancho máximo de líneas citadas |
| quotePrefix | > (espacio) | Prefijo de cita |
| taglines | false | Habilitar taglines |
| promptSpellCheck | false | Pedir corrector al guardar |
| quoteWithInitials | false | Usar iniciales en prefijo |
| indentQuoteInitials | true | Añadir espacio antes de iniciales |
| trimQuoteSpaces | false | Recortar espacios en citas |

### [Themes]

| Opción | Predeterminado | Descripción |
|---------|---------|-------------|
| iceThemeFile | EditorIceColors_BlueIce.ini | Tema Ice |
| dctThemeFile | EditorDCTColors_Default.ini | Tema DCT |

## Ajustes recomendados de QWK para Synchronet BBS

Si usas una BBS Synchronet, se recomiendan estos ajustes para mejor compatibilidad:

| Ajuste | Valor recomendado |
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

## Registro de cambios

### Versión 0.54 (2026-03-31)

#### Añadido

- Seguimiento de “último leído” por conferencia con posicionamiento automático del cursor
- Indicador de mensajes nuevos (marca) en la lista de conferencias
- Importación de archivo en el editor (Ctrl-O) para insertar texto
- Seguimiento de líneas citadas para preservar integridad al editar
- Guardado por párrafos para mejor reflujo de texto
- Detección de cambio de tamaño del terminal

#### Cambiado

- El ajuste de líneas citadas respeta el ancho del terminal cuando está habilitado
- Configuración en el primer inicio: si el usuario no tiene configuración, `config` se ejecuta al inicio y se solicita el nombre antes de continuar

#### Corregido

- La edición de líneas citadas ya no fusiona el desbordamiento con líneas adyacentes

### Versión 0.53 (2026-03-29)

#### Añadido

- Opción para mostrar solo conferencias con mensajes nuevos
- Soporte de editor externo con múltiples configuraciones
- Soporte de “drop file” (MSGINF, EDITOR.INF, DOOR.SYS, DOOR32.SYS)
- Análisis de RESULT.ED tras salir del editor externo
- Tecla G para ir a un número de conferencia
- Validación de entrada numérica para “ir a”

#### Corregido

- Los comandos con barra (/Q, /S, etc.) ahora funcionan en la primera línea
- Los colores de la ventana de cita en modo DCT usan el tema cargado
- Barra de desplazamiento restaurada para listas largas
- Mejoras en el refresco de pantalla en diálogos de configuración

### Versión 0.52 (2026-03-26)

#### Añadido

- El directorio REP se crea automáticamente
- Tecla Ctrl-P para guardar el paquete REP
- Subida de paquetes de respuesta a sistemas remotos
- Reajuste de líneas citadas
- Selector de directorio para el directorio de REP

#### Cambiado

- Actualizaciones de pantalla del editor más rápidas, con menos parpadeo
- Guardado de mensajes consciente de párrafos
- Renderizado ANSI mejorado con soporte de 256 colores y truecolor

#### Corregido

- Manejo de códigos de color durante todas las operaciones de edición
- Comportamiento de ajuste de línea del editor
- Preservación del espacio final en el prefijo de cita
- Seguimiento del guardado de REP

### Versión 0.51 (2026-03-25)

#### Añadido

- Soporte QWKE (QWK extendido)
- Soporte de códigos de color/atributo de múltiples BBS
- Votación tipo Synchronet (encuestas y votos a favor/en contra)
- Soporte de adjuntos
- Soporte UTF-8
- Selector de colores (Ctrl-K)
- Directorio de sistemas remotos (Ctrl-R)
- Búsqueda y filtrado con búsqueda avanzada
- Alternancia de pantalla de presentación

### Versión 0.50 (2026-03-24)

- Lanzamiento inicial
- Lector QWK offline multiplataforma
- Interfaz inspirada en DDMsgReader para lectura
- Editor inspirado en SlyEdit con modos Ice y DCT
- Soporte de temas, corrector ortográfico y taglines
- Creación de paquetes de respuesta REP
- Explorador de archivos y utilidad de configuración independiente

