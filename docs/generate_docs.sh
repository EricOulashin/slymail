#!/bin/bash
#
# generate_docs.sh - Generate HTML, PDF, and plain text user manual from Markdown source
#
# Prerequisites (Python - preferred):
#   pip3 install markdown weasyprint
#
# Fallback (system packages):
#   sudo apt-get install pandoc wkhtmltopdf  (Ubuntu/Debian)
#   brew install pandoc wkhtmltopdf          (macOS)
#
# Usage:
#   ./generate_docs.sh          # Generate all formats (HTML, PDF, text)
#   ./generate_docs.sh html     # Generate HTML only
#   ./generate_docs.sh pdf      # Generate PDF only
#   ./generate_docs.sh text     # Generate plain text only

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
SOURCE="$SCRIPT_DIR/SlyMail_User_Manual.md"
HTML_DIR="$SCRIPT_DIR/html"
HTML_OUTPUT="$HTML_DIR/SlyMail_User_Manual.html"
PDF_OUTPUT="$SCRIPT_DIR/SlyMail_User_Manual.pdf"
TEXT_OUTPUT="$SCRIPT_DIR/SlyMail_User_Manual.txt"

# Check source file exists
if [ ! -f "$SOURCE" ]; then
    echo "Error: Source file not found: $SOURCE"
    exit 1
fi

# Export dirs for Python scripts
export SCRIPT_DIR
export PROJECT_DIR

# Extract version and date from program_info.h (authoritative source)
PROGRAM_INFO="$PROJECT_DIR/src/program_info.h"
if [ -f "$PROGRAM_INFO" ]; then
    export SLYMAIL_VERSION=$(grep 'PROGRAM_VERSION' "$PROGRAM_INFO" | sed -E 's/.*"(.*)".*/\1/')
    export SLYMAIL_DATE=$(grep 'PROGRAM_DATE' "$PROGRAM_INFO" | sed -E 's/.*"(.*)".*/\1/')
    echo "Version: $SLYMAIL_VERSION  Date: $SLYMAIL_DATE (from program_info.h)"

    # Update the Markdown source frontmatter to match program_info.h
    if [ -n "$SLYMAIL_VERSION" ] && [ -n "$SLYMAIL_DATE" ]; then
        sed -i "s/^version: .*/version: \"$SLYMAIL_VERSION\"/" "$SOURCE"
        sed -i "s/^date: .*/date: \"$SLYMAIL_DATE\"/" "$SOURCE"
    fi
fi

# All translated language codes (matching README_XX.md / SlyMail_User_Manual_XX.md)
# Keep in sync with supported UI languages in src/i18n.cpp (note: pt_BR vs pt-BR file naming).
LANGS=(cy da de el es fi fr ga ja nb pirate pl pt-BR ru sv zh-CN zh-TW)

generate_html() {
    echo "Generating HTML user manual..."
    mkdir -p "$HTML_DIR"

    # Copy screenshots into html/ so relative paths work from the HTML file
    if [ -d "$PROJECT_DIR/screenshots" ]; then
        mkdir -p "$HTML_DIR/screenshots"
        cp -u "$PROJECT_DIR/screenshots/"*.png "$HTML_DIR/screenshots/" 2>/dev/null || true
        echo "  Copied screenshots to $HTML_DIR/screenshots/"
    fi

    if python3 -c "import markdown" 2>/dev/null; then
        python3 "$SCRIPT_DIR/generate_html.py"
        # Generate HTML for each translated language variant
        for lang in "${LANGS[@]}"; do
            src="$SCRIPT_DIR/SlyMail_User_Manual_${lang}.md"
            if [ -f "$src" ]; then
                echo "  Generating HTML for language: $lang"
                SOURCE_MD="$src" \
                OUTPUT_HTML="$HTML_DIR/SlyMail_User_Manual_${lang}.html" \
                LANG_CODE="$lang" \
                python3 "$SCRIPT_DIR/generate_html.py"
            fi
        done
    elif command -v pandoc &>/dev/null; then
        sed 's|](../screenshots/|](screenshots/|g' "$SOURCE" > "/tmp/slymail_manual_tmp.md"
        pandoc "/tmp/slymail_manual_tmp.md" \
            --from markdown \
            --to html5 \
            --standalone \
            --toc \
            --toc-depth=3 \
            --metadata title="SlyMail User Manual" \
            -o "$HTML_OUTPUT"
        rm -f /tmp/slymail_manual_tmp.md
        echo "  HTML generated: $HTML_OUTPUT"
        # Generate translated HTML via pandoc
        for lang in "${LANGS[@]}"; do
            src="$SCRIPT_DIR/SlyMail_User_Manual_${lang}.md"
            if [ -f "$src" ]; then
                echo "  Generating HTML for language: $lang"
                tmp="/tmp/slymail_manual_${lang}_tmp.md"
                sed 's|](../screenshots/|](screenshots/|g' "$src" > "$tmp"
                pandoc "$tmp" \
                    --from markdown \
                    --to html5 \
                    --standalone \
                    --toc \
                    --toc-depth=3 \
                    --metadata title="SlyMail User Manual" \
                    -o "$HTML_DIR/SlyMail_User_Manual_${lang}.html"
                rm -f "$tmp"
            fi
        done
    else
        echo "Error: Neither Python 'markdown' module nor 'pandoc' is available."
        echo "Install one of:"
        echo "  pip3 install markdown"
        echo "  sudo apt-get install pandoc"
        exit 1
    fi
}

generate_pdf() {
    echo "Generating PDF user manual..."

    # Ensure HTML exists first
    if [ ! -f "$HTML_OUTPUT" ]; then
        generate_html
    fi

    # If weasyprint is not importable, try to install it into the user environment
    # using --break-system-packages (safe on modern Debian/Ubuntu with PEP 668).
    if ! python3 -c "from weasyprint import HTML" 2>/dev/null; then
        echo "  weasyprint not found — attempting: pip3 install --break-system-packages weasyprint"
        if pip3 install --break-system-packages weasyprint 2>/dev/null \
           || pip3 install weasyprint 2>/dev/null; then
            echo "  weasyprint installed successfully."
        else
            echo "  pip install failed — will try wkhtmltopdf fallback."
        fi
    fi

    if python3 -c "from weasyprint import HTML" 2>/dev/null; then
        python3 -c "
from weasyprint import HTML
import os
html_file = os.path.join('${HTML_DIR}', 'SlyMail_User_Manual.html')
pdf_file = os.path.join('${SCRIPT_DIR}', 'SlyMail_User_Manual.pdf')
HTML(filename=html_file, base_url='${HTML_DIR}').write_pdf(pdf_file)
print(f'  PDF generated: {pdf_file}')
"
        # Generate translated PDFs
        for lang in "${LANGS[@]}"; do
            html_src="$HTML_DIR/SlyMail_User_Manual_${lang}.html"
            if [ -f "$html_src" ]; then
                echo "  Generating PDF for language: $lang"
                python3 -c "
from weasyprint import HTML
import os
html_file = '${html_src}'
pdf_file = os.path.join('${SCRIPT_DIR}', 'SlyMail_User_Manual_${lang}.pdf')
HTML(filename=html_file, base_url='${HTML_DIR}').write_pdf(pdf_file)
print(f'  PDF generated: {pdf_file}')
"
            fi
        done
    elif command -v wkhtmltopdf &>/dev/null; then
        wkhtmltopdf \
            --enable-local-file-access \
            --page-size Letter \
            --margin-top 20mm \
            --margin-bottom 20mm \
            --margin-left 15mm \
            --margin-right 15mm \
            --header-center "SlyMail User Manual" \
            --header-font-size 9 \
            --header-spacing 5 \
            --footer-center "[page] / [topage]" \
            --footer-font-size 9 \
            --footer-spacing 5 \
            "$HTML_OUTPUT" \
            "$PDF_OUTPUT" 2>/dev/null
        echo "  PDF generated: $PDF_OUTPUT"
        # Generate translated PDFs via wkhtmltopdf
        for lang in "${LANGS[@]}"; do
            html_src="$HTML_DIR/SlyMail_User_Manual_${lang}.html"
            if [ -f "$html_src" ]; then
                echo "  Generating PDF for language: $lang"
                wkhtmltopdf \
                    --enable-local-file-access \
                    --page-size Letter \
                    --margin-top 20mm --margin-bottom 20mm \
                    --margin-left 15mm --margin-right 15mm \
                    --footer-center "[page] / [topage]" \
                    --footer-font-size 9 --footer-spacing 5 \
                    "$html_src" \
                    "$SCRIPT_DIR/SlyMail_User_Manual_${lang}.pdf" 2>/dev/null
            fi
        done
    else
        echo "Warning: Cannot generate PDF. Install weasyprint:"
        echo "  pip3 install --break-system-packages weasyprint"
        return 1
    fi
}

generate_text() {
    echo "Generating plain text user manual..."
    python3 "$SCRIPT_DIR/generate_text.py"
    # Generate translated plain-text variants
    for lang in "${LANGS[@]}"; do
        src="$SCRIPT_DIR/SlyMail_User_Manual_${lang}.md"
        if [ -f "$src" ]; then
            echo "  Generating plain text for language: $lang"
            SOURCE_MD="$src" \
            OUTPUT_TXT="$SCRIPT_DIR/SlyMail_User_Manual_${lang}.txt" \
            python3 "$SCRIPT_DIR/generate_text.py"
        fi
    done
}

case "${1:-all}" in
    html)
        generate_html
        ;;
    pdf)
        generate_pdf
        ;;
    text)
        generate_text
        ;;
    all)
        generate_html
        generate_pdf
        generate_text
        ;;
    *)
        echo "Usage: $0 [html|pdf|text|all]"
        exit 1
        ;;
esac

echo "Done."
