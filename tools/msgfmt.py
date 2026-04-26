#!/usr/bin/env python3
"""
Minimal msgfmt replacement (GNU gettext .po -> .mo).

Supports the subset of PO files used by SlyMail (no contexts, no plurals).
This avoids requiring external gettext tools (msgfmt) on developer machines.
"""

from __future__ import annotations

import argparse
import ast
import struct
from pathlib import Path


def _unquote_po_string(s: str) -> str:
    # PO uses C-style escapes inside double-quoted strings.
    # ast.literal_eval on a Python string literal gives us the right behavior.
    return ast.literal_eval(s)


def parse_po(po_text: str) -> dict[str, str]:
    """
    Parse a PO file into msgid -> msgstr mapping.
    Ignores fuzzy entries and untranslated entries.
    """
    entries: dict[str, str] = {}

    msgid_parts: list[str] | None = None
    msgstr_parts: list[str] | None = None
    in_msgid = False
    in_msgstr = False
    fuzzy = False

    def flush():
        nonlocal msgid_parts, msgstr_parts, in_msgid, in_msgstr, fuzzy
        if not msgid_parts or msgstr_parts is None:
            msgid_parts = None
            msgstr_parts = None
            in_msgid = in_msgstr = False
            fuzzy = False
            return
        msgid = "".join(msgid_parts)
        msgstr = "".join(msgstr_parts)
        # Skip header and empty/untranslated/fuzzy
        if msgid and msgstr and not fuzzy:
            entries[msgid] = msgstr
        msgid_parts = None
        msgstr_parts = None
        in_msgid = in_msgstr = False
        fuzzy = False

    for raw_line in po_text.splitlines():
        line = raw_line.strip()
        if not line:
            flush()
            continue
        if line.startswith("#,") and "fuzzy" in line:
            fuzzy = True
            continue
        if line.startswith("#"):
            continue

        if line.startswith("msgid "):
            flush()
            in_msgid = True
            in_msgstr = False
            msgid_parts = [_unquote_po_string(line[5:].strip())]
            msgstr_parts = None
            continue

        if line.startswith("msgstr "):
            in_msgid = False
            in_msgstr = True
            if msgid_parts is None:
                # malformed, ignore
                msgid_parts = []
            msgstr_parts = [_unquote_po_string(line[6:].strip())]
            continue

        # Continuation lines: "...."
        if line.startswith('"') and line.endswith('"'):
            if in_msgid and msgid_parts is not None:
                msgid_parts.append(_unquote_po_string(line))
            elif in_msgstr and msgstr_parts is not None:
                msgstr_parts.append(_unquote_po_string(line))
            continue

        # Not supported (msgctxt, plurals, etc.) — ignore.

    flush()
    return entries


def build_mo(mapping: dict[str, str]) -> bytes:
    """
    Build a GNU .mo file from a msgid->msgstr mapping.
    Format reference: GNU gettext MO file format.
    """
    # Sort by msgid
    items = sorted(mapping.items(), key=lambda kv: kv[0])
    ids = [k.encode("utf-8") for k, _ in items]
    strs = [v.encode("utf-8") for _, v in items]

    # Header
    magic = 0x950412de
    revision = 0
    n = len(items)
    # Offsets: header(7*4)=28 bytes, then two tables of n*8 each
    off_orig_table = 28
    off_trans_table = off_orig_table + n * 8
    off_strings = off_trans_table + n * 8

    # Build string blocks and record offsets
    orig_table = []
    trans_table = []
    orig_block = b""
    trans_block = b""

    # Originals
    cur = off_strings
    for b in ids:
        orig_table.append((len(b), cur))
        orig_block += b + b"\x00"
        cur += len(b) + 1

    # Translations
    trans_start = off_strings + len(orig_block)
    cur = trans_start
    for b in strs:
        trans_table.append((len(b), cur))
        trans_block += b + b"\x00"
        cur += len(b) + 1

    # Assemble
    out = bytearray()
    out += struct.pack("<I", magic)
    out += struct.pack("<I", revision)
    out += struct.pack("<I", n)
    out += struct.pack("<I", off_orig_table)
    out += struct.pack("<I", off_trans_table)
    out += struct.pack("<I", 0)  # hash table size
    out += struct.pack("<I", 0)  # hash table offset

    for length, offset in orig_table:
        out += struct.pack("<II", length, offset)
    for length, offset in trans_table:
        out += struct.pack("<II", length, offset)

    out += orig_block
    out += trans_block
    return bytes(out)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("po_file", type=Path)
    ap.add_argument("-o", "--output", required=True, type=Path)
    args = ap.parse_args()

    po_text = args.po_file.read_text(encoding="utf-8")
    mapping = parse_po(po_text)

    # Ensure at least the header exists (gettext tolerates empty catalog),
    # but we intentionally omit msgid "" header entry from mapping.
    mo_bytes = build_mo(mapping)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(mo_bytes)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

