*(Tradução automática — para a versão original em inglês, veja [README.md](README.md))*

# SlyMail

Este é um leitor de e-mail offline baseado em texto e multiplataforma para o formato de pacotes [QWK](https://en.wikipedia.org/wiki/QWK_(file_format)). O formato de pacotes QWK era/é frequentemente usado para trocar correspondência em [sistemas de boletim eletrônico](https://en.wikipedia.org/wiki/Bulletin_board_system).

O SlyMail fornece uma interface completa para ler e responder mensagens de pacotes de correio QWK de BBS (Sistema de Boletim Eletrônico). Sua interface de usuário é inspirada no [Digital Distortion Message Reader (DDMsgReader)](https://github.com/SynchronetBBS/sbbs/tree/master/xtrn/DDMsgReader) para leitura de mensagens e no [SlyEdit](https://github.com/SynchronetBBS/sbbs/tree/master/exec) para edição de mensagens, ambos originalmente criados para o [Synchronet BBS](https://www.synchro.net/).

O SlyMail foi criado com a ajuda do Claude AI.

## Recursos

### Suporte a pacotes QWK
- Abre e lê pacotes de correio QWK padrão (arquivos .qwk)
- Analisa CONTROL.DAT, MESSAGES.DAT e arquivos de índice NDX
- Suporte completo a QWKE (QWK estendido) via HEADERS.DAT — correspondência baseada em deslocamento para campos estendidos precisos To/From/Subject, flag UTF-8 e RFC822 Message-ID
- Análise de kludge de corpo QWKE (`To:`, `From:`, `Subject:` no início da mensagem)
- Lida com a numeração de conferências no estilo Synchronet
- Cria pacotes de resposta REP (arquivos .rep) para envio de volta ao BBS, incluindo HEADERS.DAT para campos estendidos e VOTING.DAT para votos pendentes
- Suporta codificação de ponto flutuante no Formato Binário da Microsoft (MBF) em arquivos NDX
- Lembra o último arquivo QWK aberto e o diretório entre sessões

### Leitura de mensagens (estilo DDMsgReader)
- Lista de conferências com contagens de mensagens
- Lista de mensagens rolável com navegação por barra de luz
- Leitor de mensagens completo com exibição de cabeçalho (De, Para, Assunto, Data)
- Realce de linhas de citação (suporta citações em múltiplos níveis)
- Exibição de linhas kludge (opcional)
- Indicador de barra de rolagem
- Navegação pelo teclado: Primeira/Última/Próxima/Mensagem anterior, Página acima/abaixo
- Telas de ajuda acessíveis com `?` ou `F1` em todas as visualizações

### Suporte a códigos de cor e atributo BBS
O SlyMail interpreta códigos de cor/atributo de vários pacotes de software BBS, renderizando-os como texto colorido tanto no leitor de mensagens quanto no editor de mensagens. Formatos suportados:
- **Códigos de escape ANSI** — sempre habilitados; sequências SGR padrão (ESC[...m) para primeiro plano, plano de fundo, negrito
- **Códigos Ctrl-A do Synchronet** — `\x01` + caractere de atributo (p. ex., `\x01c` para ciano, `\x01h` para brilhante)
- **Códigos de coração WWIV** — `\x03` + dígito de 0 a 9
- **Códigos @X do PCBoard/Wildcat** — `@X##` onde os dois dígitos hexadecimais codificam a cor de fundo e primeiro plano
- **Códigos de pipe do Celerity** — `|` + letra (p. ex., `|c` para ciano, `|W` para branco brilhante)
- **Códigos de pipe do Renegade** — `|` + número de dois dígitos de 00 a 31

Cada tipo de código BBS pode ser habilitado ou desabilitado individualmente através do subdiálogo **Alternadores de código de atributo** nas Configurações do leitor ou no utilitário `config`. Esses alternadores afetam tanto o leitor quanto o editor. Uma opção separada **Remover códigos ANSI** remove todas as sequências ANSI das mensagens quando habilitada.

### Anexos de arquivo
- Detecta anexos de arquivo referenciados via linhas kludge `@ATTACH:` nos corpos das mensagens
- Mostra um indicador **[ATT]** no cabeçalho da mensagem quando há anexos
- Pressione **D** ou **Ctrl-D** no leitor para baixar anexos — mostra uma lista de arquivos com tamanhos e solicita um diretório de destino

### Votações e enquetes (Synchronet QWKE)
O SlyMail suporta a extensão Synchronet VOTING.DAT para enquetes e votações de mensagens:
- **Enquetes**: Mensagens identificadas como enquetes exibem suas opções de resposta com contagens de votos e barras de porcentagem. Pressione **V** para abrir um diálogo de cédula onde você pode alternar as seleções de resposta e emitir seu voto.
- **Votos para cima/baixo**: Para mensagens regulares (não enquetes), pressione **V** para votar a favor ou contra. As contagens de votos atuais e a pontuação são exibidas no cabeçalho da mensagem.
- **Contagens de votos**: O cabeçalho da mensagem exibe as contagens de votos para cima/baixo e a pontuação líquida, com um indicador se você já votou.
- **Fila de votos**: Os votos são enfileirados junto com as respostas de mensagens e gravados em VOTING.DAT no pacote REP para upload ao BBS.
- **Navegador de enquetes**: Pressione **V** na lista de conferências para navegar por todas as enquetes do pacote.

### Suporte a UTF-8
- Detecta conteúdo UTF-8 nas mensagens (via flag `Utf8` do HEADERS.DAT e detecção automática de sequências de bytes UTF-8)
- Exibe caracteres UTF-8 corretamente em terminais compatíveis
- Mostra um indicador **[UTF8]** no cabeçalho da mensagem para mensagens UTF-8
- Salva novas mensagens com codificação adequada
- Conversão de CP437 para UTF-8 para conteúdo BBS legado
- Define a localidade no Linux/macOS/BSD (`setlocale(LC_ALL, "")`) e a página de código UTF-8 no Windows para renderização correta no terminal

### Editor de mensagens (inspirado no SlyEdit)
- **Dois modos visuais**: Ice e DCT, cada um com esquemas de cores e layouts distintos
- **Modo aleatório**: Seleciona aleatoriamente Ice ou DCT em cada sessão de edição
- **Cores de borda alternantes**: Os caracteres de borda alternam aleatoriamente entre duas cores do tema, combinando com o estilo visual do SlyEdit
- **Suporte a temas**: Temas de cor configuráveis carregados de arquivos .ini
- Editor de texto em tela cheia com quebra de linha automática
- Janela de citação para selecionar e inserir texto citado (Ctrl-Q para abrir/fechar)
- Composição de respostas e novas mensagens
- Menu ESC para salvar, abortar, alternar inserir/sobrescrever e mais
- **Seletor de cor Ctrl-K**: Abre um diálogo para selecionar cores de primeiro plano e plano de fundo, inserindo um código de escape ANSI na posição do cursor. Suporta 16 cores de primeiro plano (8 normais + 8 brilhantes) e 8 planos de fundo, com uma prévia ao vivo. Pressione **N** para inserir um código de reset.
- **Renderização com reconhecimento de cor**: A área de edição renderiza os códigos ANSI e de atributos BBS em linha, para que o texto colorido seja exibido enquanto você digita
- **Diálogo de configurações de usuário Ctrl-U** para configurar as preferências do editor em tempo real
- **Prompts de sim/não específicos do estilo**: O modo Ice usa um prompt em linha na parte inferior da tela; o modo DCT usa uma caixa de diálogo centralizada com cores temáticas

### Configurações do editor (via Ctrl-U)
- **Escolher modo de interface**: Diálogo para alternar entre os estilos Ice, DCT e Aleatório (tem efeito imediato)
- **Selecionar arquivo de tema**: Escolha entre os temas de cor Ice ou DCT disponíveis
- **Taglines**: Quando habilitado, solicita a seleção de tagline ao salvar (de `tagline_files/taglines.txt`)
- **Dicionário/dicionários de verificação ortográfica**: Selecione quais dicionários usar
- **Solicitar verificador ortográfico ao salvar**: Quando habilitado, oferece verificação ortográfica antes de salvar
- **Ajustar linhas de citação à largura do terminal**: Quebrar linhas citadas automaticamente
- **Citar com as iniciais do autor**: Antepor linhas de citação com as iniciais do autor (p. ex., `MP> `)
- **Recuar linhas de citação que contêm iniciais**: Adicionar espaço antes das iniciais (p. ex., ` MP> `)
- **Remover espaços das linhas de citação**: Eliminar espaços em branco iniciais do texto citado

### Temas de cor
- Os arquivos de tema são arquivos de configuração (`.ini`) no diretório `config_files/`
- Temas Ice: `EditorIceColors_*.ini` (BlueIce, EmeraldCity, FieryInferno, etc.)
- Temas DCT: `EditorDCTColors_*.ini` (Default, Default-Modified, Midnight)
- As cores do tema usam um formato simples: letra de primeiro plano (`r`/`g`/`b`/`c`/`y`/`m`/`w`/`k`), `h` opcional para brilhante, dígito de plano de fundo opcional (`0`-`7`)
- Os temas controlam todas as cores dos elementos da interface: bordas, rótulos, valores, janela de citação, barra de ajuda, diálogos de sim/não

### Verificador ortográfico
- Verificador ortográfico integrado usando arquivos de dicionário de texto simples
- Vem com dicionários em inglês (suplementos en, en-US, en-GB, en-AU, en-CA)
- Diálogo de correção interativo: Substituir, Ignorar ou Sair
- Arquivos de dicionário armazenados no diretório `dictionary_files/`

### Taglines
- Os arquivos de taglines são armazenados no diretório `tagline_files/`
- O arquivo de taglines padrão é `tagline_files/taglines.txt`, uma tagline por linha
- Linhas que começam com `#` ou `;` são tratadas como comentários e ignoradas
- Selecione uma tagline específica ou escolha uma aleatoriamente ao salvar uma mensagem
- As taglines são adicionadas às mensagens com o prefixo `...`

### Criação de pacotes REP
- Atualmente, apenas ZIP é suportado (quero adicionar suporte para mais tipos de compressão no futuro)
- Quando você escreve respostas ou novas mensagens, elas são enfileiradas como pendentes
- Os votos (cédulas de enquete, votos para cima/baixo) também são enfileirados junto com as respostas
- Ao sair (ou ao abrir um novo arquivo QWK), o SlyMail solicita salvar todos os itens pendentes
- Cria um arquivo `.rep` padrão (arquivo ZIP) para upload ao BBS, contendo:
  - `<BBSID>.MSG` — mensagens de resposta no formato QWK padrão
  - `HEADERS.DAT` — cabeçalhos estendidos QWKE para campos que excedem 25 caracteres
  - `VOTING.DAT` — votos pendentes no formato INI compatível com Synchronet
- O arquivo REP é salvo como `<BBS-ID>.rep` no diretório de resposta configurado (ou o diretório do arquivo QWK)

### Sistemas remotos (Ctrl-R)
O SlyMail pode baixar pacotes QWK diretamente de sistemas remotos via FTP ou SFTP (SSH):
- Pressione **Ctrl-R** no navegador de arquivos para abrir o diretório de sistemas remotos
- **Adicionar/Editar/Excluir** entradas de sistemas remotos com: nome, host, porta, tipo de conexão (FTP ou SSH), nome de usuário, senha, alternador de FTP passivo e caminho remoto inicial
- **Navegar em diretórios remotos** com um navegador de arquivos/diretórios similar ao navegador de arquivos local — navegar nos diretórios, subir com `..`, ir para a raiz com `/`
- **Baixar arquivos QWK** do sistema remoto diretamente para o subdiretório `QWK` do diretório de dados do SlyMail
- As entradas de sistemas remotos são salvas em `remote_systems.json` no diretório de dados do SlyMail
- A última data/hora de conexão é rastreada para cada sistema
- Usa o comando `curl` do sistema para transferências FTP e SFTP (sem dependências de biblioteca em tempo de compilação)

### Configurações do aplicativo
- As configurações persistentes são salvas em `slymail.ini` no diretório de dados do SlyMail (`~/.slymail` no Linux/macOS/BSD, ou o diretório pessoal do usuário no Windows)
- O diretório de dados do SlyMail e seu subdiretório `QWK` são criados automaticamente na primeira execução
- O diretório padrão para navegar em arquivos QWK e salvar pacotes REP é `~/.slymail/QWK`
- Lembra o último diretório navegado e o nome do arquivo QWK
- Atalho de teclado Ctrl-L para carregar um arquivo QWK diferente a partir das visualizações de lista de conferências ou mensagens
- Prefixo de citação, largura de linha de citação, nome de usuário configuráveis
- Opções do leitor: mostrar/ocultar linhas kludge, linhas de rasgo/origem, barra de rolagem, remover códigos ANSI
- Alternadores de código de atributo por BBS (Synchronet, WWIV, Celerity, Renegade, PCBoard/Wildcat) — afetam tanto o leitor quanto o editor
- Diretório de saída de pacotes REP

## Capturas de tela

<p align="center">
	<a href="screenshots/SlyMail_01_OpeningScreen.png" target='_blank'><img src="screenshots/SlyMail_01_OpeningScreen.png" alt="Tela de abertura" width="800"></a>
	<a href="screenshots/SlyMail_02_File_Chooser.png" target='_blank'><img src="screenshots/SlyMail_02_File_Chooser.png" alt="Seletor de arquivos: Pesquisando arquivo QWK" width="800"></a>
	<a href="screenshots/SlyMail_03_remote_system_list.png" target='_blank'><img src="screenshots/SlyMail_03_remote_system_list.png" alt="Lista de sistemas remotos" width="800"></a>
	<a href="screenshots/SlyMail_04_Remote_System_Edit.png" target='_blank'><img src="screenshots/SlyMail_04_Remote_System_Edit.png" alt="Editando um sistema remoto" width="800"></a>
	<a href="screenshots/SlyMail_05_Remote_System_Browsing.png" target='_blank'><img src="screenshots/SlyMail_05_Remote_System_Browsing.png" alt="Navegação em sistema remoto" width="800"></a>
	<a href="screenshots/SlyMail_06_msg_area_list.png" target='_blank'><img src="screenshots/SlyMail_06_msg_area_list.png" alt="Lista de áreas de mensagens" width="800"></a>
	<a href="screenshots/SlyMail_07_msg_list.png" target='_blank'><img src="screenshots/SlyMail_07_msg_list.png" alt="Lista de mensagens" width="800"></a>
	<a href="screenshots/SlyMail_08_reading_msg.png" target='_blank'><img src="screenshots/SlyMail_08_reading_msg.png" alt="Lendo uma mensagem" width="800"></a>
	<a href="screenshots/SlyMail_09_msg_edit_start.png" target='_blank'><img src="screenshots/SlyMail_09_msg_edit_start.png" alt="Início da edição de uma mensagem" width="800"></a>
	<a href="screenshots/SlyMail_10_quote_line_selection.png" target='_blank'><img src="screenshots/SlyMail_10_quote_line_selection.png" alt="Editor: Seleção de linha de citação" width="800"></a>
	<a href="screenshots/SlyMail_11_writing_reply_msg.png" target='_blank'><img src="screenshots/SlyMail_11_writing_reply_msg.png" alt="Editor: Editando uma mensagem" width="800"></a>
	<a href="screenshots/SlyMail_12_editor_color_picker.png" target='_blank'><img src="screenshots/SlyMail_12_editor_color_picker.png" alt="Editor: Seletor de cor" width="800"></a>
	<a href="screenshots/SlyMail_13_Sync_poll_msg.png" target='_blank'><img src="screenshots/SlyMail_13_Sync_poll_msg.png" alt="Mensagem de enquete do Synchronet" width="800"></a>
	<a href="screenshots/SlyMail_14_reader_settings.png" target='_blank'><img src="screenshots/SlyMail_14_reader_settings.png" alt="Configurações do leitor" width="800"></a>
	<a href="screenshots/SlyMail_15_editor_settings.png" target='_blank'><img src="screenshots/SlyMail_15_editor_settings.png" alt="Configurações do editor" width="800"></a>
	<a href="screenshots/SlyMail_16_msg_search.png" target='_blank'><img src="screenshots/SlyMail_16_msg_search.png" alt="Pesquisa de mensagens" width="800"></a>
	<a href="screenshots/SlyMail_17_Advanced_msg_search.png" target='_blank'><img src="screenshots/SlyMail_17_Advanced_msg_search.png" alt="Pesquisa avançada de mensagens" width="800"></a>
	<a href="screenshots/SlyMail_18_advanced_msg_search_date_picker" target='_blank'><img src="screenshots/SlyMail_18_advanced_msg_search_date_picker" alt="Seletor de data na pesquisa avançada de mensagens" width="800"></a>
	<a href="screenshots/SlyMail_19_config_program.png" target='_blank'><img src="screenshots/SlyMail_19_config_program.png" alt="Programa de configuração" width="800"></a>
	<a href="screenshots/SlyMail_20_reading_ANSI_art.png" target='_blank'><img src="screenshots/SlyMail_20_reading_ANSI_art.png" alt="Lendo uma mensagem com arte ANSI" width="800"></a>
	<a href="screenshots/SlyMail_21_reading_ANSI_art.png" target='_blank'><img src="screenshots/SlyMail_21_reading_ANSI_art.png" alt="Lendo uma mensagem com arte ANSI" width="800"></a>
	<a href="screenshots/SlyMail_22_reading_ANSI_art.png" target='_blank'><img src="screenshots/SlyMail_22_reading_ANSI_art.png" alt="Lendo uma mensagem com arte ANSI" width="800"></a>
	<a href="screenshots/SlyMail_23_reading_ANSI_art.png" target='_blank'><img src="screenshots/SlyMail_23_reading_ANSI_art.png" alt="Lendo uma mensagem com arte ANSI" width="800"></a>
</p>

## Configurações do Synchronet para um pacote QWK
Em um BBS Synchronet, nas configurações do pacote QWK, o SlyMail é (ou deveria ser) compatível com os códigos de cor Ctrl-A, VOTING.DAT, anexos de arquivo e o formato de pacote QWKE. O SlyMail também deveria ser compatível com UTF-8. Por exemplo:
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

## Compilação

### Requisitos

**Linux / macOS / BSD:**
- Compilador compatível com C++17 (GCC 8+, Clang 7+)
- Biblioteca de desenvolvimento ncurses (`libncurses-dev` no Debian/Ubuntu, `ncurses-devel` no Fedora/RHEL)
- Comando `unzip` (para extrair pacotes QWK)
- Comando `zip` (para criar pacotes REP)
- Comando `curl` (para transferências FTP/SFTP de sistemas remotos — opcional, necessário apenas para o recurso de sistemas remotos)

**Windows (Visual Studio 2022):**
- Visual Studio 2022 com a carga de trabalho "Desenvolvimento para desktop com C++"
- Windows SDK 10.0 (incluído com o VS)
- Nenhuma biblioteca adicional necessária — usa a API de Console Win32 integrada para a interface de terminal, e `tar.exe` ou PowerShell para tratamento de ZIP dos pacotes QWK/REP (veja a nota abaixo)

**Windows (MinGW/MSYS2):**
- MinGW-w64 ou MSYS2 com GCC (suporte a C++17)
- API de Console do Windows (integrada)

> **Nota — Tratamento de ZIP QWK/REP no Windows:** O SlyMail detecta em tempo de execução qual ferramenta está disponível e usa a melhor opção:
>
> - **`tar.exe` (preferido):** Vem com o Windows 10 versão 1803 (Atualização de abril de 2018) e posterior, e com todas as versões do Windows 11. O `tar` lê arquivos ZIP pelo seu conteúdo em vez de pela extensão do arquivo, então os pacotes `.qwk` são extraídos diretamente e os pacotes `.rep` são criados por meio de um arquivo `.zip` temporário que é então renomeado. Nenhuma configuração adicional é necessária.
> - **PowerShell (alternativa):** Se `tar.exe` não for encontrado no PATH, o SlyMail recorre ao PowerShell. Para extração, usa a classe .NET `ZipFile` (`System.IO.Compression`) em vez de `Expand-Archive`, porque `Expand-Archive` rejeita extensões de arquivo que não sejam `.zip` mesmo quando o arquivo é um arquivo ZIP válido. Para a criação de pacotes REP, usa `Compress-Archive`, também gravando em um arquivo `.zip` temporário que é então renomeado para `.rep`.

### Compilar no Linux/macOS/BSD

```bash
make
```

Isso compila dois programas:
- `slymail` - a aplicação principal do leitor QWK
- `config_slymail` - o utilitário de configuração independente

### Compilar com símbolos de depuração

```bash
make debug
```

### Instalar (opcional)

```bash
sudo make install    # Installs slymail and config to /usr/local/bin/
sudo make uninstall  # Remove
```

### Compilar no Windows com o Visual Studio 2022

Abra o arquivo de solução no Visual Studio 2022:

```
vs\SlyMail.sln
```

Ou compile a partir da linha de comando usando o MSBuild:

```powershell
# Release build (output in vs\x64\Release\)
msbuild vs\SlyMail.sln /p:Configuration=Release /p:Platform=x64

# Debug build (output in vs\x64\Debug\)
msbuild vs\SlyMail.sln /p:Configuration=Debug /p:Platform=x64
```

Isso compila dois executáveis:
- `x64\Release\slymail.exe` — o leitor QWK principal
- `x64\Release\config_slymail.exe` — o utilitário de configuração independente

A solução contém dois projetos (`SlyMail.vcxproj` e `Config.vcxproj`) visando x64, C++17, com o conjunto de ferramentas MSVC v143.

### Compilar no Windows (MinGW/MSYS2)

```bash
make
```

O Makefile detecta automaticamente a plataforma e usa a implementação de terminal apropriada:
- **Linux/macOS/BSD**: ncurses (`terminal_ncurses.cpp`)
- **Windows**: conio + Win32 Console API (`terminal_win32.cpp`)

## Uso

```bash
# Launch SlyMail with file browser
./slymail

# Open a specific QWK packet
./slymail MYBBS.qwk

# Run the standalone configuration utility
./config_slymail
```

### Programa de configuração

O utilitário `config` fornece uma interface de texto independente para configurar as definições do SlyMail sem abrir o aplicativo principal. Ele oferece quatro categorias de configuração:

- **Configurações do editor** - Todas as mesmas configurações disponíveis via Ctrl-U no editor (estilo de editor, taglines, verificação ortográfica, opções de citação, etc.)
- **Configurações do leitor** - Alternar linhas kludge, linhas de rasgo, barra de rolagem, remoção de ANSI, modo lightbar, ordem inversa e alternadores de código de atributo (habilitar/desabilitar por BBS)
- **Configurações de tema** - Selecionar arquivos de tema de cor Ice e DCT do diretório `config_files/`
- **Configurações gerais** - Definir seu nome para respostas e o diretório de saída de pacotes REP

As configurações são salvas automaticamente ao sair de cada categoria. Tanto o SlyMail quanto o utilitário de configuração leem e escrevem no mesmo arquivo de configuração.

### Atalhos de teclado

#### Navegador de arquivos
| Tecla | Ação |
|-------|------|
| Cima/Baixo | Navegar nos arquivos e diretórios |
| Enter | Abrir diretório / Selecionar arquivo QWK |
| Ctrl-R | Abrir diretório de sistemas remotos |
| Q / ESC | Sair |

#### Lista de conferências
| Tecla | Ação |
|-------|------|
| Cima/Baixo | Navegar nas conferências |
| Enter | Abrir a conferência selecionada |
| V | Ver enquetes/votos no pacote |
| O / Ctrl-L | Abrir um arquivo QWK diferente |
| S / Ctrl-U | Configurações |
| Q / ESC | Sair do SlyMail |
| ? / F1 | Ajuda |

#### Lista de mensagens
| Tecla | Ação |
|-------|------|
| Cima/Baixo | Navegar nas mensagens |
| Enter / R | Ler a mensagem selecionada |
| N | Escrever uma nova mensagem |
| G | Ir para o número da mensagem |
| Ctrl-L | Abrir um arquivo QWK diferente |
| S / Ctrl-U | Configurações |
| C / ESC | Voltar para a lista de conferências |
| Q | Sair |
| ? / F1 | Ajuda |

#### Leitor de mensagens
| Tecla | Ação |
|-------|------|
| Cima/Baixo | Rolar a mensagem |
| Esquerda/Direita | Mensagem anterior / próxima |
| F / L | Primeira / última mensagem |
| R | Responder à mensagem |
| V | Votar (voto para cima/baixo ou cédula de enquete) |
| D / Ctrl-D | Baixar anexos de arquivo |
| H | Mostrar informações de cabeçalho da mensagem |
| S / Ctrl-U | Configurações |
| C / Q / ESC | Voltar para a lista de mensagens |
| ? / F1 | Ajuda |

#### Editor de mensagens
| Tecla | Ação |
|-------|------|
| ESC | Menu do editor (Salvar, Abortar, etc.) |
| Ctrl-U | Diálogo de configurações de usuário |
| Ctrl-Q | Abrir/fechar janela de citação |
| Ctrl-K | Seletor de cor (inserir código de cor ANSI na posição do cursor) |
| Ctrl-G | Inserir caractere gráfico (CP437) por código |
| Ctrl-W | Pesquisa de palavra/texto |
| Ctrl-S | Alterar assunto |
| Ctrl-D | Excluir a linha atual |
| Ctrl-Z | Salvar mensagem |
| Ctrl-A | Abortar mensagem |
| F1 | Tela de ajuda |
| Insert | Alternar modo Inserir/Sobrescrever |

#### Janela de citação
| Tecla | Ação |
|-------|------|
| Cima/Baixo | Navegar nas linhas de citação |
| Enter | Inserir a linha de citação selecionada |
| Ctrl-Q / ESC | Fechar janela de citação |

## Arquitetura

O SlyMail usa uma camada de abstração de plataforma para sua interface de usuário de texto:

```
ITerminal (abstract base class)
    ├── NCursesTerminal  (Linux/macOS/BSD - ncurses)
    └── Win32Terminal    (Windows - conio + Win32 Console API)
```

Os caracteres de desenho de caixas e caracteres especiais CP437 são definidos em `cp437defs.h` e renderizados através do método `putCP437()`, que mapeia os códigos CP437 para equivalentes nativos da plataforma (caracteres ACS no ncurses, bytes CP437 diretos no Windows).

### Arquivos fonte

| Arquivo | Descrição |
|---------|-----------|
| `terminal.h` | Interface abstrata `ITerminal`, constantes de teclas/cores, fábrica |
| `terminal_ncurses.cpp` | Implementação ncurses com mapeamento CP437-para-ACS |
| `terminal_win32.cpp` | API de Console do Windows + implementação conio |
| `cp437defs.h` | Definições de caracteres da página de código IBM 437 |
| `colors.h` | Definições de esquemas de cores (Ice, DCT, leitor, lista) |
| `theme.h` | Analisador de arquivo de configuração de tema (códigos de atributo estilo Synchronet) |
| `ui_common.h` | Auxiliares de interface compartilhados (diálogos, entrada de texto, barra de rolagem, etc.) |
| `qwk.h` / `qwk.cpp` | Analisador e criador de pacotes QWK/REP (QWKE, anexos, votações) |
| `bbs_colors.h` / `bbs_colors.cpp` | Analisador de códigos de cor/atributo BBS (ANSI, Synchronet, WWIV, PCBoard, Celerity, Renegade) |
| `utf8_util.h` / `utf8_util.cpp` | Utilitários UTF-8 (validação, largura de exibição, conversão CP437-para-UTF-8) |
| `voting.h` / `voting.cpp` | Analisador VOTING.DAT, contagem de votos, interface de exibição de enquetes |
| `remote_systems.h` / `remote_systems.cpp` | Diretório de sistemas remotos, navegação FTP/SFTP, persistência JSON, utilitários de diretório pessoal |
| `settings.h` | Persistência de configurações de usuário |
| `settings_dialog.h` | Diálogos de configurações (editor, leitor, alternadores de código de atributo) |
| `file_browser.h` | Navegador e seletor de arquivos QWK |
| `msg_list.h` | Visualizações de lista de conferências e mensagens |
| `msg_reader.h` | Leitor de mensagens (estilo DDMsgReader) com interface de votação e anexos |
| `msg_editor.h` | Editor de mensagens (estilo SlyEdit Ice/DCT) com seletor de cor |
| `main.cpp` | Ponto de entrada do aplicativo SlyMail e loop principal |
| `config.cpp` | Utilitário de configuração independente |

## Configuração

### Arquivo de configuração

As configurações são armazenadas em um arquivo INI chamado `slymail.ini` no mesmo diretório que o executável do SlyMail. Este arquivo é compartilhado entre o SlyMail e o utilitário `config`. O arquivo está bem comentado com descrições de cada configuração.

Exemplo `slymail.ini`:
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

### Arquivos de tema

Os temas de cor são arquivos `.ini` no diretório `config_files/`:

**Temas Ice** (`EditorIceColors_*.ini`):
- BlueIce (padrão), EmeraldCity, FieryInferno, Fire-N-Ice, GeneralClean, GenericBlue, PurpleHaze, ShadesOfGrey

**Temas DCT** (`EditorDCTColors_*.ini`):
- Default (padrão), Default-Modified, Midnight

Os valores de cor do tema usam um formato compacto derivado dos códigos de atributo do Synchronet:
- `n` = normal (redefinir)
- Primeiro plano: `k`=preto, `r`=vermelho, `g`=verde, `y`=amarelo, `b`=azul, `m`=magenta, `c`=ciano, `w`=branco
- `h` = alta intensidade/brilhante
- Dígito de plano de fundo: `0`=preto, `1`=vermelho, `2`=verde, `3`=marrom, `4`=azul, `5`=magenta, `6`=ciano, `7`=cinza claro

Exemplo: `nbh` = azul brilhante normal, `n4wh` = branco brilhante sobre fundo azul

### Taglines

As taglines são citações curtas ou ditados adicionados ao final das mensagens quando salvas. O recurso de taglines pode ser habilitado via Ctrl-U no editor ou no utilitário `config`.

As taglines são armazenadas em `tagline_files/taglines.txt`, uma por linha. Linhas que começam com `#` ou `;` são tratadas como comentários e ignoradas. Ao salvar uma mensagem com taglines habilitadas, o usuário é solicitado a selecionar uma tagline específica ou escolher uma aleatoriamente. As taglines selecionadas são adicionadas à mensagem com o prefixo `...` (p. ex., `...To err is human, to really foul things up requires a computer.`).

### Verificador ortográfico

O SlyMail inclui um verificador ortográfico integrado que usa arquivos de dicionário de texto simples. O verificador ortográfico pode ser configurado para solicitar ao salvar via Ctrl-U no editor ou no utilitário `config`.

**Os arquivos de dicionário** são arquivos de texto simples (uma palavra por linha) armazenados em `dictionary_files/`. Vários dicionários podem ser selecionados simultaneamente para uma cobertura combinada de palavras. O SlyMail é fornecido com:
- `dictionary_en.txt` - Inglês (geral, ~130K palavras)
- `dictionary_en-US-supplemental.txt` - Suplemento de inglês dos EUA
- `dictionary_en-GB-supplemental.txt` - Suplemento de inglês britânico
- `dictionary_en-AU-supplemental.txt` - Suplemento de inglês australiano
- `dictionary_en-CA-supplemental.txt` - Suplemento de inglês canadense

Quando a verificação ortográfica é acionada, o verificador examina a mensagem em busca de palavras com erros ortográficos e apresenta um diálogo interativo para cada uma, oferecendo opções para **S**ubstituir a palavra, **I**gnorar, **A**dicionar (futuro) ou **E**ncerrar a verificação.

## Créditos

- Interface inspirada em [DDMsgReader](https://github.com/SynchronetBBS/sbbs) e [SlyEdit](https://github.com/SynchronetBBS/sbbs) por [Nightfox (Eric Oulashin)](https://github.com/EricOulashin)
- Compatibilidade com o formato QWK baseada no código-fonte do [Synchronet BBS](https://www.synchro.net/)
- Definições de caracteres CP437 do Synchronet

## Licença

Este projeto é software de código aberto.
