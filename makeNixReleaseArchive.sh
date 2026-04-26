#!/bin/bash

cd docs
./generate_docs.sh
cd ..

OSName=$(uname -s)
releaseDirName="SlyMail_${OSName}"


rm -rf "$releaseDirName"
mkdir -p "$releaseDirName/config_files"
mkdir -p "$releaseDirName/dictionary_files"
mkdir -p "$releaseDirName/tagline_files"
mkdir -p "$releaseDirName/docs"
cp config_files/* "$releaseDirName/config_files"
cp dictionary_files/* "$releaseDirName/dictionary_files"
cp tagline_files/* "$releaseDirName/tagline_files"
cp slymail.ini "$releaseDirName"
#cp README.md "$releaseDirName"
#cp CHANGELOG.md "$releaseDirName"
cp slymail "$releaseDirName"
cp config "$releaseDirName"

# Copy compiled locale (.mo) files for runtime translation
mkdir -p "$releaseDirName/locale"
for lang in cy da de el es fi fr ga ja nb pirate pl pt_BR ru sv zh_CN zh_TW; do
    mo="locale/${lang}/LC_MESSAGES/slymail.mo"
    if [ -f "$mo" ]; then
        mkdir -p "$releaseDirName/locale/${lang}/LC_MESSAGES"
        cp "$mo" "$releaseDirName/locale/${lang}/LC_MESSAGES/"
    fi
done

cd "$releaseDirName/docs"
cp ../../docs/SlyMail_User_Manual.pdf . 2>/dev/null || true
cp ../../docs/SlyMail_User_Manual.txt . 2>/dev/null || true
cp -rf ../../docs/html . 2>/dev/null || true
# Copy translated user manuals (PDF and plain text) for all supported languages
for lang in cy da de el es fi fr ga ja nb pirate pl pt-BR ru sv zh-CN zh-TW; do
    cp "../../docs/SlyMail_User_Manual_${lang}.pdf" . 2>/dev/null || true
    cp "../../docs/SlyMail_User_Manual_${lang}.txt" . 2>/dev/null || true
done
cd ../..


# Create the FILE_ID.DIZ for the release package
version=$(cat src/program_info.h |grep PROGRAM_VERSION |sed -E 's/^.*PROGRAM_VERSION "(.*)"/\1/g')
versionWithoutDot=$(cat src/program_info.h |grep PROGRAM_VERSION |sed -E 's/^.*PROGRAM_VERSION "(.*)"/\1/g' |sed -E 's/\.//g')
sed "s/<VERSION>/$version/g" FILE_ID_Template.DIZ |sed "s/<OS>/${OSName}/g" |sed "s/<DATE>/$(date '+%Y-%m-%d')/g" >FILE_ID.DIZ

# Make the zip file (unless --no-zip is passed, e.g. for CI artifact uploads)
if [ "$1" = "--no-zip" ]; then
    # Keep FILE_ID.DIZ alongside the release directory (not inside it)
    echo "Release directory prepared: $releaseDirName"
    echo "FILE_ID.DIZ created alongside the release directory"
else
    zip -r -9 "SlyMail_${versionWithoutDot}_${OSName}.zip" FILE_ID.DIZ "$releaseDirName"
    rm -rf "$releaseDirName"
    rm FILE_ID.DIZ
fi
