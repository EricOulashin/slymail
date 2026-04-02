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
cd "$releaseDirName/docs"
cp ../../docs/SlyMail_User_Manual.pdf . 2>/dev/null || true
cp ../../docs/SlyMail_User_Manual.txt . 2>/dev/null || true
cp -rf ../../docs/html . 2>/dev/null || true
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
