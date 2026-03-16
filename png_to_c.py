#!/usr/bin/env python3
"""
png_to_c_tile.py  —  Convert a PNG image to an RGB565 C array for CPulator/DE1-SoC
Usage:
    python3 png_to_c_tile.py image.png array_name [tile_x tile_y]

Arguments:
    image.png   — Any PNG file (16x16 for a single tile, or a tileset)
    array_name  — The C identifier to use (e.g. dirt_sprite, grass_sprite)
    tile_x      — (optional) 0-based column index if image is a tileset
    tile_y      — (optional) 0-based row index  if image is a tileset

The script always extracts a 16x16 region.
Transparent pixels (alpha < 128 in RGBA images) are output as 0xF81F (magenta).

Examples:
    python3 png_to_c_tile.py DirtTile.png dirt_sprite
    python3 png_to_c_tile.py GrassTile.png grass_sprite 9 1   # interior tile
"""

import sys
from PIL import Image
import numpy as np

TRANSPARENT = 0xF81F
TILE_W = 16
TILE_H = 16


def to_rgb565(r, g, b) -> int:
    # Cast to plain Python int to avoid numpy uint8 overflow:
    # numpy uint8 arithmetic wraps at 255, so (uint8 & 0xF8) << 8 = 0 always.
    r, g, b = int(r), int(g), int(b)
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def extract_tile(arr: np.ndarray, tx: int, ty: int) -> np.ndarray:
    x0, y0 = tx * TILE_W, ty * TILE_H
    return arr[y0:y0 + TILE_H, x0:x0 + TILE_W]


def tile_to_c_array(tile: np.ndarray, name: str, has_alpha: bool) -> str:
    lines = [
        f"/* {name}: {TILE_W}x{TILE_H} px, RGB565, 0xF81F = transparent */",
        f"static const short {name}[TILE_W * TILE_H] = {{",
    ]
    for y in range(TILE_H):
        row = []
        for x in range(TILE_W):
            if has_alpha and int(tile[y, x, 3]) < 128:
                row.append(f"0x{TRANSPARENT:04X}")
            else:
                row.append(f"0x{to_rgb565(tile[y,x,0], tile[y,x,1], tile[y,x,2]):04X}")
        suffix = "," if y < TILE_H - 1 else ""
        lines.append("    " + ",".join(row) + suffix)
    lines.append("};")
    return "\n".join(lines)


def list_tiles(arr: np.ndarray):
    h, w = arr.shape[:2]
    cols = w // TILE_W
    rows = h // TILE_H
    print(f"Tileset is {w}x{h} px — {cols} cols x {rows} rows of 16x16 tiles:")
    for ty in range(rows):
        for tx in range(cols):
            region = arr[ty*TILE_H:(ty+1)*TILE_H, tx*TILE_W:(tx+1)*TILE_W]
            avg = region[:, :, :3].mean()
            tag = "EMPTY" if avg < 5 else f"avg={avg:.0f}"
            print(f"  tile[{ty}][{tx}]  {tag}")


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)

    path = sys.argv[1]
    name = sys.argv[2]
    tx   = int(sys.argv[3]) if len(sys.argv) > 3 else 0
    ty   = int(sys.argv[4]) if len(sys.argv) > 4 else 0

    img = Image.open(path)
    has_alpha = img.mode == "RGBA"
    img = img.convert("RGBA")
    arr = np.array(img)

    h, w = arr.shape[:2]

    if w == TILE_W and h == TILE_H:
        tile = arr
    else:
        print(f"# Detected tileset ({w}x{h})")
        list_tiles(arr)
        print()
        tile = extract_tile(arr, tx, ty)

    print(tile_to_c_array(tile, name, has_alpha))


if __name__ == "__main__":
    main()