#!/usr/bin/env python3
"""
img_to_c.py — Convert PNG or Aseprite sprite sheets to RGB565 C arrays.

Usage:
    python3 img_to_c.py <image> [options]

Options:
    --name      NAME        C array name (default: filename stem)
    --out       FILE        Output .c file (default: <name>.c)
    --header                Also write a .h file
    --ox        N           X offset into source image (pixels, default 0)
    --oy        N           Y offset into source image (pixels, default 0)
    --w         N           Width to extract (default: full image width)
    --h         N           Height to extract (default: full image height)
    --fw        N           Frame width — splits sheet into frames (optional)
    --fh        N           Frame height — splits sheet into frames (optional)
    --fcount    N           Max frames to extract (default: all that fit)
    --row       N           Row index in a tile grid (0-based, alternative to --oy)
    --col       N           Col index in a tile grid (0-based, alternative to --ox)
    --tile-w    N           Tile width for --row/--col indexing (default 16)
    --tile-h    N           Tile height for --row/--col indexing (default 16)
    --transparent R,G,B     RGB colour to treat as transparent (default: none)
    --magenta               Treat magenta (255,0,255) as transparent (common default)
    --scale     N           Upscale output by N for visual preview only (not C output)
    --preview   FILE        Save a PNG preview to FILE
    --verbose               Print extracted region info

Transparent pixel value in output: 0xF81F (RGB565 magenta)

Examples:
    # Single tile at grid position row=10, col=2
    python3 img_to_c.py Floors_Tiles.png --name grass_sprite --row 10 --col 2 --tile-w 16 --tile-h 16

    # Extract a 32x32 region at pixel offset (16, 48)
    python3 img_to_c.py Rocks.png --name rock_big --ox 16 --oy 16 --w 32 --h 32 --magenta

    # Sprite sheet: 4 frames of 32x32
    python3 img_to_c.py Knight_Idle.png --name knight_idle --fw 32 --fh 32 --fcount 4

    # Full sprite sheet auto-split into 64x64 frames
    python3 img_to_c.py Body_A_Walk.png --name walk_frames --fw 64 --fh 64

    # With header file
    python3 img_to_c.py grass.png --name grass_sprite --header
"""

import sys
import os
import argparse
from pathlib import Path

try:
    from PIL import Image
    import numpy as np
except ImportError:
    print("ERROR: Pillow and numpy required.  pip install pillow numpy")
    sys.exit(1)


TRANSPARENT_565 = 0xF81F   # magenta in RGB565 — used as transparent sentinel


def to_rgb565(r, g, b, a=255, transparent_rgb=None):
    """Convert RGBA to RGB565. Returns TRANSPARENT_565 if pixel is transparent."""
    if a < 32:
        return TRANSPARENT_565
    if transparent_rgb is not None:
        if (r, g, b) == transparent_rgb:
            return TRANSPARENT_565
    v = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
    # Avoid accidentally producing the transparent sentinel value
    if v == TRANSPARENT_565:
        v = 0xF820
    return v


def load_image(path):
    """Load PNG or Aseprite file as RGBA numpy array."""
    p = Path(path)
    if not p.exists():
        print(f"ERROR: file not found: {path}")
        sys.exit(1)

    if p.suffix.lower() == '.aseprite' or p.suffix.lower() == '.ase':
        # Try pyaseprite if available, otherwise error
        try:
            import aseprite
            ase = aseprite.AsepriteFile(str(p))
            frame = ase.frames[0]
            img = frame.image.convert('RGBA')
        except ImportError:
            print("ERROR: .aseprite files need pyaseprite:  pip install pyaseprite")
            print("       Or export to PNG from Aseprite first (File > Export Sprite Sheet)")
            sys.exit(1)
    else:
        img = Image.open(path).convert('RGBA')

    return img, np.array(img)


def extract_region(arr, ox, oy, w, h):
    """Extract a w×h region from arr starting at (ox, oy). Clamps to image bounds."""
    img_h, img_w = arr.shape[:2]
    x0, y0 = max(0, ox), max(0, oy)
    x1, y1 = min(img_w, ox + w), min(img_h, oy + h)
    region = arr[y0:y1, x0:x1]
    # Pad if region was clamped
    if region.shape[1] != w or region.shape[0] != h:
        padded = np.zeros((h, w, 4), dtype=np.uint8)
        padded[:region.shape[0], :region.shape[1]] = region
        region = padded
    return region


def crop_to_content(region, transparent_rgb=None):
    """Remove fully-transparent rows/cols from edges. Returns (cropped, x_off, y_off)."""
    h, w = region.shape[:2]
    alpha = region[:, :, 3].copy()
    if transparent_rgb is not None:
        r_eq = region[:, :, 0] == transparent_rgb[0]
        g_eq = region[:, :, 1] == transparent_rgb[1]
        b_eq = region[:, :, 2] == transparent_rgb[2]
        alpha[r_eq & g_eq & b_eq] = 0
    rows = np.any(alpha > 10, axis=1)
    cols = np.any(alpha > 10, axis=0)
    if not rows.any():
        return region, 0, 0
    r0, r1 = np.where(rows)[0][[0, -1]]
    c0, c1 = np.where(cols)[0][[0, -1]]
    return region[r0:r1+1, c0:c1+1], int(c0), int(r0)


def region_to_pixels(region, transparent_rgb=None):
    """Convert RGBA region to flat list of RGB565 values."""
    h, w = region.shape[:2]
    pixels = []
    for row in range(h):
        for col in range(w):
            r, g, b, a = (int(region[row, col, ch]) for ch in range(4))
            pixels.append(to_rgb565(r, g, b, a, transparent_rgb))
    return pixels


def pixels_to_c_array(pixels, name, w, h, frame_idx=None):
    """Format a flat pixel list as a C static const short array."""
    aname = f"{name}_f{frame_idx:02d}" if frame_idx is not None else name
    total = w * h
    lines = [f"static const short {aname}[{total}] = {{"]
    for i in range(0, total, 16):
        chunk = pixels[i:i+16]
        lines.append("    " + ",".join(f"0x{p:04X}" for p in chunk) + ",")
    lines.append("};")
    return "\n".join(lines), aname


def write_output(args, all_arrays, all_names, w, h, frame_count):
    """Write .c (and optionally .h) output file."""
    out_path = Path(args.out)

    guard = args.name.upper() + "_C"
    lines = []
    lines.append(f"/* Generated by img_to_c.py")
    lines.append(f" * Source : {args.image}")
    lines.append(f" * Region : ox={args.ox} oy={args.oy} w={w} h={h}")
    if frame_count > 1:
        lines.append(f" * Frames : {frame_count} x {args.fw}x{args.fh}")
    lines.append(f" * Format : RGB565, 0x{TRANSPARENT_565:04X} = transparent")
    lines.append(f" */")
    lines.append("")
    lines.append(f"#ifndef {guard}")
    lines.append(f"#define {guard}")
    lines.append("")

    # Size defines
    if frame_count == 1:
        lines.append(f"#define {args.name.upper()}_W  {w}")
        lines.append(f"#define {args.name.upper()}_H  {h}")
    else:
        lines.append(f"#define {args.name.upper()}_FRAME_W  {args.fw}")
        lines.append(f"#define {args.name.upper()}_FRAME_H  {args.fh}")
        lines.append(f"#define {args.name.upper()}_FRAMES   {frame_count}")

    lines.append("")

    for arr_text in all_arrays:
        lines.append(arr_text)
        lines.append("")

    # If multiple frames, emit a frames[] pointer array
    if frame_count > 1:
        lines.append(f"static const short * const {args.name}_frames[{frame_count}] = {{")
        for n in all_names:
            lines.append(f"    {n},")
        lines.append("};")
        lines.append("")

    lines.append(f"#endif /* {guard} */")

    out_path.write_text("\n".join(lines))
    print(f"Written: {out_path}  ({frame_count} array(s), {w}x{h} each)")

    if args.header:
        h_path = out_path.with_suffix('.h')
        h_guard = args.name.upper() + "_H"
        h_lines = [
            f"/* {h_path.name} — declarations for {args.name} */",
            f"#ifndef {h_guard}",
            f"#define {h_guard}",
            f'#include "{out_path.name}"',
            f"#endif /* {h_guard} */",
        ]
        h_path.write_text("\n".join(h_lines))
        print(f"Written: {h_path}")


def main():
    p = argparse.ArgumentParser(
        description="Convert PNG/Aseprite to RGB565 C arrays",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    p.add_argument("image",                 help="Input PNG or .aseprite file")
    p.add_argument("--name",    default="",  help="C array name (default: filename stem)")
    p.add_argument("--out",     default="",  help="Output .c file")
    p.add_argument("--header",  action="store_true", help="Also write .h file")
    p.add_argument("--ox",      type=int, default=0,  help="X pixel offset into source")
    p.add_argument("--oy",      type=int, default=0,  help="Y pixel offset into source")
    p.add_argument("--w",       type=int, default=0,  help="Width to extract (0=full)")
    p.add_argument("--h",       type=int, default=0,  help="Height to extract (0=full)")
    p.add_argument("--fw",      type=int, default=0,  help="Frame width (for sprite sheets)")
    p.add_argument("--fh",      type=int, default=0,  help="Frame height (for sprite sheets)")
    p.add_argument("--fcount",  type=int, default=0,  help="Max frames (0=all)")
    p.add_argument("--row",     type=int, default=-1, help="Tile grid row (0-based)")
    p.add_argument("--col",     type=int, default=-1, help="Tile grid col (0-based)")
    p.add_argument("--tile-w",  type=int, default=16, help="Tile width for --row/--col")
    p.add_argument("--tile-h",  type=int, default=16, help="Tile height for --row/--col")
    p.add_argument("--transparent", default="", help="Treat R,G,B as transparent e.g. 255,0,255")
    p.add_argument("--magenta", action="store_true", help="Treat (255,0,255) as transparent")
    p.add_argument("--crop",    action="store_true", help="Auto-crop to content bbox")
    p.add_argument("--preview", default="",  help="Save PNG preview to this path")
    p.add_argument("--verbose", action="store_true")
    args = p.parse_args()

    # Defaults
    stem = Path(args.image).stem
    if not args.name:
        args.name = stem.lower().replace("-", "_").replace(" ", "_")
    if not args.out:
        args.out = args.name + ".c"

    # Transparent colour
    transparent_rgb = None
    if args.magenta:
        transparent_rgb = (255, 0, 255)
    elif args.transparent:
        parts = [int(x.strip()) for x in args.transparent.split(",")]
        transparent_rgb = tuple(parts[:3])

    # Load image
    img, arr = load_image(args.image)
    img_h, img_w = arr.shape[:2]

    # Apply --row / --col tile grid indexing
    if args.row >= 0:
        args.oy = args.row * args.tile_h
    if args.col >= 0:
        args.ox = args.col * args.tile_w

    # Resolve extraction width/height
    # When using --row/--col tile indexing, default extraction to tile size
    default_w = args.tile_w if (args.row >= 0 or args.col >= 0) else img_w - args.ox
    default_h = args.tile_h if (args.row >= 0 or args.col >= 0) else img_h - args.oy
    ext_w = args.w if args.w > 0 else (args.fw if args.fw > 0 else default_w)
    ext_h = args.h if args.h > 0 else (args.fh if args.fh > 0 else default_h)

    if args.verbose:
        print(f"Source: {args.image}  ({img_w}x{img_h})")
        print(f"Region: ox={args.ox} oy={args.oy}  extract={ext_w}x{ext_h}")

    # Single tile vs sprite sheet
    if args.fw > 0 and args.fh > 0:
        # Sprite sheet mode
        fw, fh = args.fw, args.fh
        cols_per_row = max(1, ext_w // fw)
        rows_in_sheet = max(1, ext_h // fh)
        max_frames = cols_per_row * rows_in_sheet
        n_frames = min(max_frames, args.fcount) if args.fcount > 0 else max_frames

        all_arrays, all_names = [], []
        for fi in range(n_frames):
            fx = args.ox + (fi % cols_per_row) * fw
            fy = args.oy + (fi // cols_per_row) * fh
            region = extract_region(arr, fx, fy, fw, fh)
            if args.crop:
                region, _, _ = crop_to_content(region, transparent_rgb)
            pixels = region_to_pixels(region, transparent_rgb)
            rh, rw = region.shape[:2]
            arr_text, aname = pixels_to_c_array(pixels, args.name, rw, rh, fi)
            all_arrays.append(arr_text)
            all_names.append(aname)
            if args.verbose:
                print(f"  Frame {fi}: ({fx},{fy})  {rw}x{rh}")

        write_output(args, all_arrays, all_names, fw, fh, n_frames)

    else:
        # Single region mode
        region = extract_region(arr, args.ox, args.oy, ext_w, ext_h)
        if args.crop:
            region, cx, cy = crop_to_content(region, transparent_rgb)
            if args.verbose:
                print(f"Cropped to content: offset ({cx},{cy})  size {region.shape[1]}x{region.shape[0]}")
        pixels = region_to_pixels(region, transparent_rgb)
        rh, rw = region.shape[:2]
        arr_text, aname = pixels_to_c_array(pixels, args.name, rw, rh)
        write_output(args, [arr_text], [aname], rw, rh, 1)

    # Preview
    if args.preview:
        region = extract_region(arr, args.ox, args.oy, ext_w, ext_h)
        preview_img = Image.fromarray(region, 'RGBA')
        preview_img.save(args.preview)
        print(f"Preview: {args.preview}")


if __name__ == "__main__":
    main()