#!/usr/bin/env python3
"""Convert an indexed-color PNG glyph strip into a MARIA-strided
atari7800_font_descriptor_t asset (the layout atari7800.h's
atari7800_font_descriptor_t actually uses: one shared `data` blob, glyph i's
column at byte offset i*glyph_width_bytes within every page, vertically
flipped like every other MARIA_STRIDED asset -- row 0 (top) at the highest
page, row (height-1) at page 0. See fighter.sprite.h's hud_font for a
hand-transcribed example of this same shape).

Usage:
  python3 png_indexed_font_to_strided_header.py unified_font.png \
      out.h hud_font --tile-width 8 --tile-height 8 --palette 5 \
      --charmap '0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ.!?,"$():*+-/<>'
"""

import argparse
from pathlib import Path

from png_indexed_to_160a_header import parse_png, pack_160a, row_indices


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("input_png")
    parser.add_argument("output_header")
    parser.add_argument("symbol")
    parser.add_argument("--tile-width", type=int, default=8)
    parser.add_argument("--tile-height", type=int, default=8)
    parser.add_argument("--mode", type=lambda x: int(x, 0), default=0x40)
    parser.add_argument("--palette", type=int, default=5)
    parser.add_argument("--glyph-advance", type=int, default=8)
    parser.add_argument("--space-advance", type=int, default=8)
    parser.add_argument("--line-advance", type=int, default=8)
    parser.add_argument("--charmap", required=True)
    args = parser.parse_args()

    width, height, bit_depth, rows = parse_png(Path(args.input_png))
    tw, th = args.tile_width, args.tile_height
    if width % tw != 0:
        raise ValueError("image width must be an exact multiple of tile width")
    if th > height:
        raise ValueError("tile height exceeds image height")

    tile_count = width // tw
    if len(args.charmap) > tile_count:
        raise ValueError("charmap length exceeds tile count")

    width_bytes = (tw + 3) // 4
    unpacked_rows = [row_indices(row, width, bit_depth) for row in rows]

    # packed_rows[y] is the whole strip's row y packed into 160A bytes
    # (width_bytes * tile_count bytes long); glyph i's column within it is
    # byte range [i*width_bytes, (i+1)*width_bytes).
    packed_rows = [pack_160a(unpacked_rows[y][:width]) for y in range(th)]

    symbol = args.symbol
    symbol_upper = symbol.upper()
    width_twos_comp = (0x20 - width_bytes) & 0xFF

    lines = []
    lines.append(f"#ifndef ATARI7800_ASSET_{symbol_upper}_H")
    lines.append(f"#define ATARI7800_ASSET_{symbol_upper}_H")
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append("#include <atari7800.h>")
    lines.append("")
    lines.append(f"#define {symbol_upper}_TILE_WIDTH_PIXELS {tw}u")
    lines.append(f"#define {symbol_upper}_TILE_HEIGHT_LINES {th}u")
    lines.append(f"#define {symbol_upper}_TILE_WIDTH_BYTES {width_bytes}u")
    lines.append(f"#define {symbol_upper}_TILE_COUNT {tile_count}u")
    lines.append("")

    # Vertically flipped, page-strided: row r (0=top) at page (th-1-r), each
    # glyph i at column i*width_bytes within that page -- see module
    # docstring and fighter.sprite.h's hud_font_data for precedent.
    lines.append(
        f"static const uint8_t {symbol}_data[{th} * 256] "
        "__attribute__((aligned(256))) = {"
    )
    for r in range(th):
        page = th - 1 - r
        row_bytes = packed_rows[r]
        if any(b != 0 for b in row_bytes):
            vals = ", ".join(f"0x{b:02x}" for b in row_bytes)
            lines.append(f"    [{page} * 256] = {vals},")
    lines.append("};")
    lines.append("")

    lookup = [0xFF] * 128
    for index, ch in enumerate(args.charmap):
        code = ord(ch)
        if code < 128:
            lookup[code] = index
    lines.append(f"static const uint8_t {symbol}_char_to_glyph[128] = {{")
    for i in range(0, 128, 16):
        chunk = ", ".join(f"0x{v:02x}" for v in lookup[i : i + 16])
        lines.append(f"    {chunk},")
    lines.append("};")
    lines.append("")

    escaped = args.charmap.replace("\\", "\\\\").replace('"', '\\"')
    lines.append(f'/* Character order: "{escaped}" */')
    lines.append(f"static const atari7800_font_descriptor_t {symbol} = {{")
    lines.append(f"    .data = {symbol}_data,")
    lines.append(f"    .char_to_glyph = {symbol}_char_to_glyph,")
    lines.append("    .char_to_glyph_len = 128u,")
    lines.append("    .first_char = 0u,")
    lines.append(f"    .glyph_count = {symbol_upper}_TILE_COUNT,")
    lines.append(f"    .glyph_width_bytes = {symbol_upper}_TILE_WIDTH_BYTES,")
    lines.append(f"    .glyph_height_lines = {symbol_upper}_TILE_HEIGHT_LINES,")
    lines.append(f"    .glyph_mode = 0x{args.mode:02x}u,")
    lines.append(f"    .glyph_palette = {args.palette}u,")
    lines.append(f"    .glyph_width_twos_comp = 0x{width_twos_comp:02x}u,")
    lines.append(f"    .glyph_advance = {args.glyph_advance}u,")
    lines.append(f"    .space_advance = {args.space_advance}u,")
    lines.append(f"    .line_advance = {args.line_advance}u,")
    lines.append("};")
    lines.append("")
    lines.append(f"#endif /* ATARI7800_ASSET_{symbol_upper}_H */")
    lines.append("")

    Path(args.output_header).write_text("\n".join(lines), encoding="ascii")


if __name__ == "__main__":
    main()
