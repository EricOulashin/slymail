@echo off
REM generate_docs.bat - Generate HTML, PDF, and plain text user manual from Markdown source
REM
REM This is the Windows equivalent of generate_docs.sh.
REM
REM Prerequisites:
REM   Python 3 with: pip install markdown weasyprint
REM
REM Usage:
REM   generate_docs.bat          - Generate all formats (HTML, PDF, text)
REM   generate_docs.bat html     - Generate HTML only
REM   generate_docs.bat pdf      - Generate PDF only
REM   generate_docs.bat text     - Generate plain text only

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%
for %%i in ("%SCRIPT_DIR%\..") do set PROJECT_DIR=%%~fi
set SOURCE=%SCRIPT_DIR%\SlyMail_User_Manual.md
set HTML_DIR=%SCRIPT_DIR%\html
set HTML_OUTPUT=%HTML_DIR%\SlyMail_User_Manual.html
set PDF_OUTPUT=%SCRIPT_DIR%\SlyMail_User_Manual.pdf
set TEXT_OUTPUT=%SCRIPT_DIR%\SlyMail_User_Manual.txt

if not exist "%SOURCE%" (
    echo Error: Source file not found: %SOURCE%
    exit /b 1
)

REM Extract version and date from program_info.h (authoritative source)
set PROGRAM_INFO=%PROJECT_DIR%\src\program_info.h
if exist "%PROGRAM_INFO%" (
    for /f "tokens=3 delims= " %%a in ('findstr PROGRAM_VERSION "%PROGRAM_INFO%"') do set SLYMAIL_VERSION=%%~a
    for /f "tokens=3 delims= " %%a in ('findstr PROGRAM_DATE "%PROGRAM_INFO%"') do set SLYMAIL_DATE=%%~a
    set SLYMAIL_VERSION=!SLYMAIL_VERSION:"=!
    set SLYMAIL_DATE=!SLYMAIL_DATE:"=!
    echo Version: !SLYMAIL_VERSION!  Date: !SLYMAIL_DATE! ^(from program_info.h^)

    REM Update the Markdown source frontmatter to match program_info.h
    powershell -Command "(Get-Content '%SOURCE%') -replace '^version: .*', 'version: \"!SLYMAIL_VERSION!\"' -replace '^date: .*', 'date: \"!SLYMAIL_DATE!\"' | Set-Content '%SOURCE%'"
)

set MODE=%~1
if "%MODE%"=="" set MODE=all

if /i "%MODE%"=="html" goto do_html
if /i "%MODE%"=="pdf" goto do_pdf
if /i "%MODE%"=="text" goto do_text
if /i "%MODE%"=="all" goto do_all
echo Usage: %~nx0 [html^|pdf^|text^|all]
exit /b 1

:do_all
call :gen_html
call :gen_pdf
call :gen_text
echo Done.
goto :eof

:do_html
call :gen_html
echo Done.
goto :eof

:do_pdf
call :gen_pdf
echo Done.
goto :eof

:do_text
call :gen_text
echo Done.
goto :eof

REM All translated language codes
set LANGS=cy da de el es fi fr ga ja nb pl pt-BR ru sv zh-CN zh-TW

:gen_html
echo Generating HTML user manual...
if not exist "%HTML_DIR%" mkdir "%HTML_DIR%"

REM Copy screenshots
if exist "%PROJECT_DIR%\screenshots" (
    if not exist "%HTML_DIR%\screenshots" mkdir "%HTML_DIR%\screenshots"
    xcopy /q /y "%PROJECT_DIR%\screenshots\*.png" "%HTML_DIR%\screenshots\" >nul 2>&1
    echo   Copied screenshots to %HTML_DIR%\screenshots\
)

REM Use Python markdown
python -c "import markdown" >nul 2>&1
if %errorlevel%==0 (
    set SCRIPT_DIR_ENV=%SCRIPT_DIR%
    set PROJECT_DIR_ENV=%PROJECT_DIR%
    python "%SCRIPT_DIR%\generate_html.py"
    REM Generate translated HTML variants
    for %%L in (%LANGS%) do (
        if exist "%SCRIPT_DIR%\SlyMail_User_Manual_%%L.md" (
            echo   Generating HTML for language: %%L
            set SOURCE_MD=%SCRIPT_DIR%\SlyMail_User_Manual_%%L.md
            set OUTPUT_HTML=%HTML_DIR%\SlyMail_User_Manual_%%L.html
            set LANG_CODE=%%L
            python "%SCRIPT_DIR%\generate_html.py"
        )
    )
) else (
    python3 -c "import markdown" >nul 2>&1
    if %errorlevel%==0 (
        set SCRIPT_DIR_ENV=%SCRIPT_DIR%
        set PROJECT_DIR_ENV=%PROJECT_DIR%
        python3 "%SCRIPT_DIR%\generate_html.py"
        for %%L in (%LANGS%) do (
            if exist "%SCRIPT_DIR%\SlyMail_User_Manual_%%L.md" (
                echo   Generating HTML for language: %%L
                set SOURCE_MD=%SCRIPT_DIR%\SlyMail_User_Manual_%%L.md
                set OUTPUT_HTML=%HTML_DIR%\SlyMail_User_Manual_%%L.html
                set LANG_CODE=%%L
                python3 "%SCRIPT_DIR%\generate_html.py"
            )
        )
    ) else (
        echo Error: Python 'markdown' module not available.
        echo Install with: pip install markdown
    )
)
set SOURCE_MD=
set OUTPUT_HTML=
set LANG_CODE=
goto :eof

:gen_pdf
echo Generating PDF user manual...

if not exist "%HTML_OUTPUT%" call :gen_html

REM Try weasyprint
python -c "from weasyprint import HTML; HTML(filename=r'%HTML_OUTPUT%', base_url=r'%HTML_DIR%').write_pdf(r'%PDF_OUTPUT%'); print('  PDF generated: %PDF_OUTPUT%')" 2>nul
if %errorlevel% neq 0 (
    python3 -c "from weasyprint import HTML; HTML(filename=r'%HTML_OUTPUT%', base_url=r'%HTML_DIR%').write_pdf(r'%PDF_OUTPUT%'); print('  PDF generated: %PDF_OUTPUT%')" 2>nul
    if %errorlevel% neq 0 (
        echo Warning: Cannot generate PDF. Install weasyprint: pip install weasyprint
    )
)
REM Generate translated PDFs
for %%L in (%LANGS%) do (
    if exist "%HTML_DIR%\SlyMail_User_Manual_%%L.html" (
        echo   Generating PDF for language: %%L
        python -c "from weasyprint import HTML; HTML(filename=r'%HTML_DIR%\SlyMail_User_Manual_%%L.html', base_url=r'%HTML_DIR%').write_pdf(r'%SCRIPT_DIR%\SlyMail_User_Manual_%%L.pdf')" 2>nul || python3 -c "from weasyprint import HTML; HTML(filename=r'%HTML_DIR%\SlyMail_User_Manual_%%L.html', base_url=r'%HTML_DIR%').write_pdf(r'%SCRIPT_DIR%\SlyMail_User_Manual_%%L.pdf')" 2>nul || echo   Warning: PDF generation failed for %%L
    )
)
goto :eof

:gen_text
echo Generating plain text user manual...

REM Try python then python3
python "%SCRIPT_DIR%\generate_text.py" 2>nul
if %errorlevel% neq 0 (
    python3 "%SCRIPT_DIR%\generate_text.py" 2>nul
    if %errorlevel% neq 0 (
        echo Warning: Cannot generate plain text. Python 3 is required.
    )
)
REM Generate translated plain-text variants
for %%L in (%LANGS%) do (
    if exist "%SCRIPT_DIR%\SlyMail_User_Manual_%%L.md" (
        echo   Generating plain text for language: %%L
        set SOURCE_MD=%SCRIPT_DIR%\SlyMail_User_Manual_%%L.md
        set OUTPUT_TXT=%SCRIPT_DIR%\SlyMail_User_Manual_%%L.txt
        python "%SCRIPT_DIR%\generate_text.py" 2>nul || python3 "%SCRIPT_DIR%\generate_text.py" 2>nul || echo   Warning: text generation failed for %%L
    )
)
set SOURCE_MD=
set OUTPUT_TXT=
goto :eof
