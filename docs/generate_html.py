#!/usr/bin/env python3
"""Generate HTML user manual from Markdown source."""

import markdown
import re
import os
import sys

script_dir = os.environ.get('SCRIPT_DIR', os.environ.get('SCRIPT_DIR_ENV', os.path.dirname(os.path.abspath(__file__))))
project_dir = os.environ.get('PROJECT_DIR', os.environ.get('PROJECT_DIR_ENV', os.path.dirname(script_dir)))
html_dir = os.path.join(script_dir, 'html')
source = os.path.join(script_dir, 'SlyMail_User_Manual.md')
output = os.path.join(html_dir, 'SlyMail_User_Manual.html')

with open(source, 'r') as f:
    raw = f.read()

# Get version and date from environment (set by generate_docs.sh/.bat from program_info.h)
# Falls back to parsing YAML frontmatter, then hardcoded defaults
version = os.environ.get('SLYMAIL_VERSION', '')
date = os.environ.get('SLYMAIL_DATE', '')
if not version or not date:
    fm_match = re.match(r'^---\n(.*?)\n---\n', raw, flags=re.DOTALL)
    if fm_match:
        fm = fm_match.group(1)
        if not version:
            vm = re.search(r'version:\s*"?([^"\n]+)"?', fm)
            if vm: version = vm.group(1).strip()
        if not date:
            dm = re.search(r'date:\s*"?([^"\n]+)"?', fm)
            if dm: date = dm.group(1).strip()
if not version: version = "0.54"
if not date: date = "2026-04-02"

# Strip YAML frontmatter
content = re.sub(r'^---\n.*?\n---\n', '', raw, flags=re.DOTALL)

# Fix image paths: ../screenshots/ -> screenshots/ (since we copied them)
content = content.replace('](../screenshots/', '](screenshots/')

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
img { max-width: 100%; height: auto; border: 1px solid #dfe2e5; border-radius: 6px; margin: 1em 0; }
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
    table, pre, img { page-break-inside: avoid; }
    img { max-width: 80%; }
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

os.makedirs(html_dir, exist_ok=True)
with open(output, 'w') as f:
    f.write(html)
print(f'  HTML generated: {output} ({len(html)} bytes)')
