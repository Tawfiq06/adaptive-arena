#!/usr/bin/env python3
"""
flatten.py — Combine a multi-file C project into a single .c file for CPulator.

Usage:
    python3 flatten.py                      # uses flatten.cfg in current dir
    python3 flatten.py --config my.cfg      # specify config file
    python3 flatten.py --out flat.c         # specify output file
    python3 flatten.py --verbose            # show what each pass does

Config file format (flatten.cfg):
------------------------------------------------------------
output: work/flattened.c

keep_includes:
    <stdint.h>
    <stdlib.h>

files:
    src/hardware/vga.h
    src/hardware/vga.c
    src/game/entity.h
    src/game/entity.c
    src/main.c
------------------------------------------------------------
"""

import sys
import os
import re
import argparse
from pathlib import Path

DEFAULT_CONFIG = "flatten.cfg"
DEFAULT_OUTPUT = "flattened.c"

# All project filenames — populated after reading config
_project_files = set()


def parse_config(cfg_path):
    cfg = {"output": DEFAULT_OUTPUT, "keep_includes": [], "files": []}
    if not Path(cfg_path).exists():
        print(f"ERROR: config file not found: {cfg_path}")
        sys.exit(1)
    section = None
    for raw in Path(cfg_path).read_text().splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        if line.startswith("output:"):
            cfg["output"] = line.split(":", 1)[1].strip()
            section = None
            continue
        if line.endswith(":") and " " not in line:
            section = line[:-1].strip()
            continue
        if section == "keep_includes":
            cfg["keep_includes"].append(line.lstrip("- ").strip())
        elif section == "files":
            f = line.lstrip("- ").strip()
            if f:
                cfg["files"].append(f)
    return cfg


def is_project_include(inc_name):
    bare = inc_name.strip('"<>')
    return bare in _project_files or Path(bare).name in _project_files


def process_file(filepath, keep_includes, verbose=False):
    """
    Read a .c/.h file and return cleaned lines.

    Transformations applied:
      - Strip #include "..." for project files
      - Strip header guards (#ifndef FOO_H / #define FOO_H / closing #endif)
      - Strip `extern` from top-level variable declarations
        (definition is present in the same merged TU)
      - Remove static forward array declarations (`static T foo[N];` with no `=`)
        (the definition that follows would conflict)
      - Strip `static` from top-level non-inline function/array DEFINITIONS
        so they don't conflict with the now-non-extern header declarations
      - Keep `static inline` functions as-is
      - Keep `static` on local variables inside functions (indented)
    """
    path = Path(filepath)
    if not path.exists():
        print(f"  ERROR: file not found: {filepath}")
        return []

    raw_lines = path.read_text(errors='replace').splitlines()

    # Identify header guard lines to skip
    skip_lines = set()
    guard_name = None
    guard_start = guard_define = guard_end = -1
    for i, line in enumerate(raw_lines):
        s = line.strip()
        m = re.match(r'#\s*ifndef\s+(\w+)', s)
        if m and guard_start == -1:
            guard_start = i
            guard_name = m.group(1)
        if guard_name and re.match(r'#\s*define\s+' + re.escape(guard_name) + r'\b', s):
            if guard_define == -1:
                guard_define = i
    if guard_name:
        for i in range(len(raw_lines) - 1, -1, -1):
            if re.match(r'#\s*endif', raw_lines[i].strip()):
                guard_end = i
                break
    for idx in (guard_start, guard_define, guard_end):
        if idx >= 0:
            skip_lines.add(idx)

    out_lines = []
    stripped_includes = []
    kept_includes = []

    for i, line in enumerate(raw_lines):
        if i in skip_lines:
            continue

        stripped = line.strip()

        # --- Handle #include ---
        m = re.match(r'#\s*include\s+([<"][^>"]+[>"])', stripped)
        if m:
            inc = m.group(1)
            bare = inc.strip('"<>')
            keep_bare = [k.strip('<>') for k in keep_includes]
            if inc.startswith('<') and bare in keep_bare:
                kept_includes.append(inc)
                out_lines.append(line)
            elif inc.startswith('"') or is_project_include(bare):
                stripped_includes.append(inc)
                out_lines.append(f"/* #include {inc} -- merged */")
            else:
                out_lines.append(f"/* #include {inc} -- removed */")
            continue

        # --- Linkage reconciliation (top-level lines only) ---
        is_top_level = not (line.startswith(' ') or line.startswith('\t'))

        if is_top_level and stripped and not stripped.startswith('//') and not stripped.startswith('/*'):

            # 1. Strip `extern` from variable/array declarations
            #    extern const short foo[N];  ->  const short foo[N];
            if re.match(r'extern\b', stripped) and not re.match(r'extern\s*"C"', stripped):
                line = re.sub(r'\bextern\s+', '', line, count=1)
                stripped = line.strip()
                if verbose:
                    print(f"    extern->: {stripped[:70]}")

            # 2. Remove `static` array/variable FORWARD DECLARATIONS (no `=`)
            #    e.g. `static const short foo[256];`  in a .h file
            #    The .c definition that follows would conflict — remove the decl.
            if re.match(r'static\b', stripped) and not re.match(r'static\s+inline\b', stripped):
                has_init = '=' in stripped
                is_array_fwd = bool(re.search(r'\[[^\]]*\]\s*;', stripped) and not has_init)
                is_struct_fwd = bool(re.match(r'static\s+(?:struct|enum)\s+\w+\s*;', stripped))
                if is_array_fwd or is_struct_fwd:
                    out_lines.append(f"/* removed fwd decl: {stripped} */")
                    if verbose:
                        print(f"    removed fwd decl: {stripped[:70]}")
                    continue

                # 3. Strip `static` from top-level non-inline DEFINITIONS
                #    (initialized arrays and function bodies).
                #    This prevents conflicts when the header had `extern T foo[]`
                #    and the .c has `static T foo[] = {...}`.
                is_func_def = bool(
                    re.match(r'static\s+(?:const\s+)?(?:void|int|short|char|long|unsigned|\w+)\s+\w+\s*\(', stripped)
                    and not stripped.endswith(';')
                )
                has_body = bool(re.search(r'=\s*[{(]|\)\s*\{', stripped))
                if is_func_def or has_body:
                    line = re.sub(r'\bstatic\s+', '', line, count=1)
                    stripped = line.strip()
                    if verbose:
                        print(f"    static->: {stripped[:70]}")

        out_lines.append(line)

    if verbose:
        if stripped_includes:
            print(f"    stripped includes: {stripped_includes}")
        if kept_includes:
            print(f"    kept includes:     {kept_includes}")

    return out_lines


def collect_system_includes(all_sections):
    """Pull all kept #include <...> lines, deduplicate, return (includes, cleaned_sections)."""
    seen = set()
    deduped = []
    cleaned = []
    for lines in all_sections:
        new_section = []
        for line in lines:
            m = re.match(r'\s*#\s*include\s+(<[^>]+>)', line.strip())
            if m:
                inc = m.group(1)
                if inc not in seen:
                    seen.add(inc)
                    deduped.append(f"#include {inc}")
            else:
                new_section.append(line)
        cleaned.append(new_section)
    return deduped, cleaned


def dedup_defines(sections):
    """Comment out duplicate #define occurrences (keeps first, removes rest)."""
    seen = {}
    result = []
    for lines in sections:
        new_lines = []
        for line in lines:
            m = re.match(r'\s*#\s*define\s+(\w+)', line)
            if m:
                name = m.group(1)
                if name in seen:
                    new_lines.append(f"/* #define {name} -- duplicate removed */")
                    continue
                seen[name] = True
            new_lines.append(line)
        result.append(new_lines)
    return result


def main():
    ap = argparse.ArgumentParser(
        description="Flatten a multi-file C project into one .c file",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    ap.add_argument("--config",  default=DEFAULT_CONFIG)
    ap.add_argument("--out",     default="")
    ap.add_argument("--verbose", action="store_true")
    args = ap.parse_args()

    cfg = parse_config(args.config)
    output_path = Path(args.out) if args.out else Path(cfg["output"])

    print(f"Config  : {args.config}")
    print(f"Output  : {output_path}")
    print(f"Files   : {len(cfg['files'])}")

    global _project_files
    _project_files = {Path(f).name for f in cfg["files"]}

    all_sections = []
    for filepath in cfg["files"]:
        if args.verbose:
            print(f"  Processing: {filepath}")
        lines = process_file(filepath, cfg["keep_includes"], args.verbose)
        all_sections.append((filepath, lines))

    just_lines = [ls for _, ls in all_sections]
    sys_includes, cleaned = collect_system_includes(just_lines)
    cleaned = dedup_defines(cleaned)

    output_lines = [
        "/* ==========================================================================",
        f" * flattened.c — auto-generated by flatten.py",
        f" * Source files ({len(cfg['files'])}):",
    ] + [f" *   {f}" for f in cfg["files"]] + [
        " * DO NOT EDIT — regenerate with:  python3 tools/flatten.py",
        " * =========================================================================*/",
        "",
    ]

    if sys_includes:
        output_lines.append("/* === System includes === */")
        output_lines += sys_includes
        output_lines.append("")

    for (filepath, _), section_lines in zip(all_sections, cleaned):
        output_lines.append("/* ===========================================================================")
        output_lines.append(f" * {filepath}")
        output_lines.append(" * =========================================================================*/")
        while section_lines and not section_lines[0].strip():
            section_lines.pop(0)
        while section_lines and not section_lines[-1].strip():
            section_lines.pop()
        output_lines += section_lines
        output_lines += ["", ""]

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text("\n".join(output_lines))
    print(f"Done — {len(output_lines)} lines written to {output_path}")


if __name__ == "__main__":
    main()