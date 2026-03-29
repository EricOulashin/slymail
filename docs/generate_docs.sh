#!/bin/bash
#
# generate_docs.sh - Generate HTML and PDF user manual from Markdown source
#
# This script converts SlyMail_User_Manual.md to HTML and PDF.
#
# Method 1 (Python - preferred): Uses Python 'markdown' and 'weasyprint' packages
#   pip3 install markdown weasyprint
#
# Method 2 (pandoc + wkhtmltopdf): Uses system packages
#   sudo apt-get install pandoc wkhtmltopdf  (Ubuntu/Debian)
#   brew install pandoc wkhtmltopdf          (macOS)
#
# Usage:
#   ./generate_docs.sh          # Generate both HTML and PDF
#   ./generate_docs.sh html     # Generate HTML only
#   ./generate_docs.sh pdf      # Generate PDF only

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SOURCE="$SCRIPT_DIR/SlyMail_User_Manual.md"
HTML_DIR="$SCRIPT_DIR/html"
HTML_OUTPUT="$HTML_DIR/SlyMail_User_Manual.html"
PDF_OUTPUT="$SCRIPT_DIR/SlyMail_User_Manual.pdf"

# Check source file exists
if [ ! -f "$SOURCE" ]; then
    echo "Error: Source file not found: $SOURCE"
    exit 1
fi

generate_html() {
    echo "Generating HTML user manual..."
    mkdir -p "$HTML_DIR"

    # Try Python markdown first
    if python3 -c "import markdown" 2>/dev/null; then
        python3 << 'PYEOF'
import markdown
import re
import os

script_dir = os.environ.get('SCRIPT_DIR', '.')
source = os.path.join(script_dir, 'SlyMail_User_Manual.md')
html_dir = os.path.join(script_dir, 'html')
output = os.path.join(html_dir, 'SlyMail_User_Manual.html')

with open(source, 'r') as f:
    content = f.read()

# Strip YAML frontmatter
content = re.sub(r'^---\n.*?\n---\n', '', content, flags=re.DOTALL)

# Extract version/date from frontmatter if present
version = "0.53"
date = "2026-03-29"

md = markdown.Markdown(extensions=['tables', 'toc', 'fenced_code'],
                       extension_configs={'toc': {'toc_depth': 3}})
html_body = md.convert(content)
toc = md.toc

css = """
body {
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif;
    line-height: 1.6; max-width: 900px; margin: 0 auto; padding: 20px 40px;
    color: #24292e; background-color: #fff;
}
h1 { border-bottom: 2px solid #0366d6; padding-bottom: 0.3em; color: #0366d6; }
h2 { border-bottom: 1px solid #eaecef; padding-bottom: 0.3em; margin-top: 2em; }
h3 { color: #0366d6; margin-top: 1.5em; }
h4 { color: #586069; }
table { border-collapse: collapse; width: 100%; margin: 1em 0; }
th, td { border: 1px solid #dfe2e5; padding: 8px 12px; text-align: left; }
th { background-color: #f6f8fa; font-weight: 600; }
tr:nth-child(even) { background-color: #f6f8fa; }
code { background-color: #f6f8fa; padding: 0.2em 0.4em; border-radius: 3px; font-size: 0.9em;
       font-family: "SFMono-Regular", Consolas, "Liberation Mono", Menlo, monospace; }
pre { background-color: #f6f8fa; padding: 16px; border-radius: 6px; overflow-x: auto; }
pre code { background: none; padding: 0; }
strong { color: #24292e; }
a { color: #0366d6; text-decoration: none; }
a:hover { text-decoration: underline; }
.toc { background-color: #f6f8fa; border: 1px solid #dfe2e5; border-radius: 6px;
       padding: 15px 20px; margin: 1em 0 2em 0; }
.toc h2 { margin-top: 0; border-bottom: none; font-size: 1.1em; }
.toc ul { list-style: none; padding-left: 1.2em; }
.toc > ul { padding-left: 0; }
.toc li { margin: 0.3em 0; }
.title-block { text-align: center; margin-bottom: 2em; padding-bottom: 1em;
               border-bottom: 2px solid #0366d6; }
@media print {
    body { max-width: none; padding: 0; }
    h1, h2, h3 { page-break-after: avoid; }
    table, pre { page-break-inside: avoid; }
}
"""

html = f'''<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>SlyMail User Manual</title>
<style>{css}</style>
</head>
<body>
<div class="title-block">
<h1>SlyMail User Manual</h1>
<p>QWK Offline Mail Reader &mdash; Version {version}</p>
<p>{date}</p>
</div>
<div class="toc">
<h2>Table of Contents</h2>
{toc}
</div>
{html_body}
</body>
</html>'''

with open(output, 'w') as f:
    f.write(html)
print(f'HTML generated: {output} ({len(html)} bytes)')
PYEOF
    elif command -v pandoc &>/dev/null; then
        pandoc "$SOURCE" \
            --from markdown \
            --to html5 \
            --standalone \
            --toc \
            --toc-depth=3 \
            --metadata title="SlyMail User Manual" \
            -o "$HTML_OUTPUT"
        echo "HTML generated: $HTML_OUTPUT"
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

    # Try weasyprint (Python) first
    if python3 -c "from weasyprint import HTML" 2>/dev/null; then
        python3 -c "
from weasyprint import HTML
import os
html_file = os.path.join('${SCRIPT_DIR}', 'html', 'SlyMail_User_Manual.html')
pdf_file = os.path.join('${SCRIPT_DIR}', 'SlyMail_User_Manual.pdf')
HTML(filename=html_file).write_pdf(pdf_file)
print(f'PDF generated: {pdf_file}')
"
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
        echo "PDF generated: $PDF_OUTPUT"
    else
        echo "Warning: Cannot generate PDF."
        echo "Install one of:"
        echo "  pip3 install weasyprint"
        echo "  sudo apt-get install wkhtmltopdf"
        return 1
    fi
}

# Export SCRIPT_DIR for Python subprocesses
export SCRIPT_DIR

case "${1:-all}" in
    html)
        generate_html
        ;;
    pdf)
        generate_pdf
        ;;
    all)
        generate_html
        generate_pdf
        ;;
    *)
        echo "Usage: $0 [html|pdf|all]"
        exit 1
        ;;
esac

echo "Done."
