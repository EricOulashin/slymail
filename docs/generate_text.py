#!/usr/bin/env python3
"""Generate plain text user manual from Markdown source."""

import re
import os
import textwrap

script_dir = os.environ.get('SCRIPT_DIR', os.environ.get('SCRIPT_DIR_ENV', os.path.dirname(os.path.abspath(__file__))))
source = os.path.join(script_dir, 'SlyMail_User_Manual.md')
output = os.path.join(script_dir, 'SlyMail_User_Manual.txt')

LINE_WIDTH = 79

with open(source, 'r') as f:
    raw = f.read()

# Get version and date from environment (set by generate_docs.sh/.bat from program_info.h)
# Falls back to parsing YAML frontmatter, then hardcoded defaults
version = os.environ.get('SLYMAIL_VERSION', '')
date = os.environ.get('SLYMAIL_DATE', '')
author = "Eric Oulashin"
fm_match = re.match(r'^---\n(.*?)\n---\n', raw, flags=re.DOTALL)
if fm_match:
    fm = fm_match.group(1)
    if not version:
        vm = re.search(r'version:\s*"?([^"\n]+)"?', fm)
        if vm: version = vm.group(1).strip()
    if not date:
        dm = re.search(r'date:\s*"?([^"\n]+)"?', fm)
        if dm: date = dm.group(1).strip()
    am = re.search(r'author:\s*"?([^"\n]+)"?', fm)
    if am: author = am.group(1).strip()
if not version: version = "0.54"
if not date: date = "2026-04-02"

# Strip YAML frontmatter
content = re.sub(r'^---\n.*?\n---\n', '', raw, flags=re.DOTALL)

# Remove image links (not useful in plain text)
content = re.sub(r'!\[.*?\]\(.*?\)\n*', '', content)

lines = content.split('\n')
output_lines = []


def center(text):
    return text.center(LINE_WIDTH).rstrip()


def wrap_text(text, width=LINE_WIDTH):
    if not text.strip():
        return [""]
    return textwrap.wrap(text, width=width) or [""]


def clean_inline(text):
    text = re.sub(r'\*\*(.+?)\*\*', r'\1', text)
    text = re.sub(r'\*(.+?)\*', r'\1', text)
    text = re.sub(r'`(.+?)`', r'\1', text)
    text = re.sub(r'\[(.+?)\]\(.+?\)', r'\1', text)
    return text


# Title block (centered)
output_lines.append(center("SlyMail User Manual"))
output_lines.append(center("QWK Offline Mail Reader"))
output_lines.append(center("Version " + version))
output_lines.append(center("Release date: " + date))
output_lines.append("")
output_lines.append(center("by"))
output_lines.append("")
output_lines.append(center(author))
output_lines.append("")
output_lines.append("")

# Build table of contents
toc_entries = []
section_num = 0
subsection_num = 0
for line in lines:
    if line.startswith('## ') and not line.startswith('###'):
        section_num += 1
        subsection_num = 0
        title = line[3:].strip()
        toc_entries.append((section_num, 0, title))
    elif line.startswith('### '):
        subsection_num += 1
        title = line[4:].strip()
        toc_entries.append((section_num, subsection_num, title))

output_lines.append("Contents")
output_lines.append("=" * 8)
for sec, sub, title in toc_entries:
    if sub == 0:
        output_lines.append(f"{sec:>2}. {title}")
    else:
        output_lines.append(f"    {sec}.{sub}. {title}")
output_lines.append("")
output_lines.append("")

# Process content
section_num = 0
subsection_num = 0
in_table = False
in_code = False
table_rows = []


def flush_table():
    global table_rows
    if not table_rows:
        return
    parsed = []
    for row in table_rows:
        cells = [c.strip() for c in row.strip('|').split('|')]
        parsed.append(cells)
    parsed = [r for r in parsed if not all(re.match(r'^[-:]+$', c) for c in r)]
    if not parsed:
        table_rows = []
        return
    num_cols = max(len(r) for r in parsed)
    col_widths = [0] * num_cols
    for row in parsed:
        for i, cell in enumerate(row):
            if i < num_cols:
                col_widths[i] = max(col_widths[i], len(cell))
    for ri, row in enumerate(parsed):
        parts = []
        for i in range(num_cols):
            cell = row[i] if i < len(row) else ""
            parts.append(cell.ljust(col_widths[i]))
        line = "  ".join(parts).rstrip()
        output_lines.append(line)
        if ri == 0:
            parts = []
            for i in range(num_cols):
                parts.append("-" * col_widths[i])
            output_lines.append("  ".join(parts).rstrip())
    output_lines.append("")
    table_rows = []


i = 0
while i < len(lines):
    line = lines[i]

    # Code blocks
    if line.strip().startswith('```'):
        if in_code:
            in_code = False
            output_lines.append("")
        else:
            in_code = True
        i += 1
        continue
    if in_code:
        output_lines.append("    " + line)
        i += 1
        continue

    # Tables
    if '|' in line and line.strip().startswith('|'):
        if not in_table:
            in_table = True
            table_rows = []
        table_rows.append(line)
        i += 1
        continue
    elif in_table:
        in_table = False
        flush_table()

    # H1 heading - skip (title block already output)
    if line.startswith('# ') and not line.startswith('##'):
        i += 1
        continue

    # H2 heading
    if line.startswith('## ') and not line.startswith('###'):
        section_num += 1
        subsection_num = 0
        title = clean_inline(line[3:].strip())
        output_lines.append("")
        heading = f"{section_num}. {title}"
        output_lines.append(heading)
        output_lines.append("=" * len(heading))
        i += 1
        continue

    # H3 heading
    if line.startswith('### '):
        subsection_num += 1
        title = clean_inline(line[4:].strip())
        output_lines.append("")
        heading = f"{section_num}.{subsection_num}. {title}"
        output_lines.append(heading)
        output_lines.append("-" * len(heading))
        i += 1
        continue

    # H4 heading
    if line.startswith('#### '):
        title = clean_inline(line[5:].strip())
        output_lines.append("")
        output_lines.append(title)
        output_lines.append("-" * len(title))
        i += 1
        continue

    # Horizontal rule
    if line.strip() in ('---', '***', '___'):
        output_lines.append("-" * LINE_WIDTH)
        i += 1
        continue

    # List items
    if re.match(r'^[-*] ', line.strip()):
        text = clean_inline(re.sub(r'^[-*] ', '', line.strip()))
        wrapped = textwrap.wrap(text, width=LINE_WIDTH - 4)
        for j, wl in enumerate(wrapped):
            if j == 0:
                output_lines.append("  - " + wl)
            else:
                output_lines.append("    " + wl)
        i += 1
        continue

    # Numbered list items
    m = re.match(r'^(\d+)\. ', line.strip())
    if m:
        num = m.group(1)
        text = clean_inline(line.strip()[len(num) + 2:])
        prefix = f"  {num}. "
        wrapped = textwrap.wrap(text, width=LINE_WIDTH - len(prefix))
        for j, wl in enumerate(wrapped):
            if j == 0:
                output_lines.append(prefix + wl)
            else:
                output_lines.append(" " * len(prefix) + wl)
        i += 1
        continue

    # Indented code (4 spaces)
    if line.startswith('    ') and line.strip():
        output_lines.append(line)
        i += 1
        continue

    # Empty line
    if not line.strip():
        output_lines.append("")
        i += 1
        continue

    # Regular paragraph text
    text = clean_inline(line.strip())
    for wl in wrap_text(text):
        output_lines.append(wl)
    i += 1

# Flush any remaining table
if in_table:
    flush_table()

# Write output, collapsing multiple blank lines
with open(output, 'w') as f:
    prev_blank = False
    for line in output_lines:
        is_blank = (line.strip() == "")
        if is_blank and prev_blank:
            continue
        f.write(line.rstrip() + "\n")
        prev_blank = is_blank

print(f'  Plain text generated: {output}')
