@echo off
REM makeWindowsReleaseArchive.bat - Create a Windows release ZIP archive
REM This is the Windows equivalent of makeNixReleaseArchive.sh

setlocal enabledelayedexpansion

set OSName=Windows
set releaseDirName=SlyMail_%OSName%

REM Extract version from program_info.h
for /f "tokens=3 delims= " %%a in ('findstr PROGRAM_VERSION src\program_info.h') do set version=%%~a
REM Remove quotes from version
set version=%version:"=%
REM Version without dot for filename
set versionWithoutDot=%version:.=%

REM Clean up any previous release directory
if exist "%releaseDirName%" rmdir /s /q "%releaseDirName%"

REM Create release directory structure
mkdir "%releaseDirName%"
mkdir "%releaseDirName%\config_files"
mkdir "%releaseDirName%\dictionary_files"
mkdir "%releaseDirName%\tagline_files"
mkdir "%releaseDirName%\docs"
mkdir "%releaseDirName%\docs\html"

REM Copy config, dictionary, and tagline files
xcopy /q /y config_files\* "%releaseDirName%\config_files\" >nul
xcopy /q /y dictionary_files\* "%releaseDirName%\dictionary_files\" >nul
xcopy /q /y tagline_files\* "%releaseDirName%\tagline_files\" >nul

REM Copy settings template
copy /y slymail.ini "%releaseDirName%\" >nul

REM Copy Windows executables (Release x64 build)
copy /y vs\x64\Release\slymail.exe "%releaseDirName%\" >nul
copy /y vs\x64\Release\config.exe "%releaseDirName%\" >nul

REM Copy docs (if they exist)
if exist docs\SlyMail_User_Manual.pdf copy /y docs\SlyMail_User_Manual.pdf "%releaseDirName%\docs\" >nul
if exist docs\html xcopy /q /y /e docs\html\* "%releaseDirName%\docs\html\" >nul

REM Create the FILE_ID.DIZ for the release package
REM Get today's date in YYYY-MM-DD format
for /f "tokens=2 delims==" %%I in ('wmic os get localdatetime /format:list 2^>nul') do set datetime=%%I
set TODAY=%datetime:~0,4%-%datetime:~4,2%-%datetime:~6,2%

REM Build FILE_ID.DIZ from template using PowerShell for string replacement
powershell -Command "(Get-Content 'FILE_ID_Template.DIZ') -replace '<VERSION>','%version%' -replace '<OS>','%OSName%' -replace '<DATE>','%TODAY%' | Set-Content 'FILE_ID.DIZ'"

REM Make the zip file using tar (available on Windows 10+)
set zipName=SlyMail_%versionWithoutDot%_%OSName%.zip
if exist "%zipName%" del "%zipName%"
tar -a -cf "%zipName%" FILE_ID.DIZ "%releaseDirName%"

REM Clean up
rmdir /s /q "%releaseDirName%"
del FILE_ID.DIZ

echo.
echo Release archive created: %zipName%
