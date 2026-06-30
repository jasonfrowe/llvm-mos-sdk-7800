#!/usr/bin/env python3
"""Convert indexed-color PNG tileset to 160A glyph assets + font descriptor.

The output header contains:
- packed tile bytes in 160A 2bpp format
- one atari7800_sprite_asset_t per tile
- optional ASCII lookup table populated from a provided charmap string
- one atari7800_font_descriptor_t
"""

import argparse
from pathlib import Path

from png_indexed_to_160a_header import parse_png, pack_160a, row_indices


def c_escape(ch: str) -> str:
    if ch == "\\":
        return "\\\\"
    if ch == "'":
        return "\\'"
    if ch == '"':
        return '\\"'
    if ch == "\n":
        return "\\n"
    if ch == "\t":
        return "\\t"
    code = ord(ch)
    if code < 32 or code > 126:
        return f"\\x{code:02x}"
    return ch


def emit_header(
    out_path: Path,
    symbol: str,
    tile_width: int,
    tile_height: int,
    tile_count: int,
    packed_tiles,
    mode: int,
    palette: int,
    glyph_advance: int,
    space_advance: int,
    line_advance: int,
    charmap: str,
):
    guard = f"{symbol.upper()}_H"
    width_bytes = len(packed_tiles[0][0]) if packed_tiles and packed_tiles[0] else 0
    width_twos_comp = (0x20 - width_bytes) & 0xFF
    bytes_per_tile = width_bytes * tile_height

    flat = []
    for tile in packed_tiles:
        for row in tile:
            flat.extend(row)

    lookup = [0xFF] * 128
    for index, ch in enumerate(charmap):
        code = ord(ch)
        if code < 128:
            lookup[code] = index

    lines = []
    lines.append(f"#ifndef {guard}")
    lines.append(f"#define {guard}")
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append("#include <atari7800.h>")
    lines.append("")
    lines.append(f"#define {symbol.upper()}_TILE_WIDTH_PIXELS {tile_width}u")
    lines.append(f"#define {symbol.upper()}_TILE_HEIGHT_LINES {tile_height}u")
    lines.append(f"#define {symbol.upper()}_TILE_WIDTH_BYTES {width_bytes}u")
    lines.append(f"#define {symbol.upper()}_TILE_WIDTH_TWOS_COMP 0x{width_twos_comp:02x}u")
    lines.append(f"#define {symbol.upper()}_TILE_COUNT {tile_count}u")
    lines.append(f"#define {symbol.upper()}_MODE 0x{mode:02x}u")
    lines.append(f"#define {symbol.upper()}_DEFAULT_PALETTE {palette}u")
    lines.append(
        f"#define {symbol.upper()}_DATA_LAYOUT ATARI7800_SPRITE_LAYOUT_CONTIGUOUS_160A"
    )
    lines.append("")
    lines.append(f"static const uint8_t {symbol}_tile_data[] = {{")
    for i in range(0, len(flat), 12):
        chunk = ", ".join(f"0x{v:02x}" for v in flat[i : i + 12])
        lines.append(f"    {chunk},")
    lines.append("};")
    lines.append("")
    lines.append(
        f"static const atari7800_sprite_asset_t {symbol}_glyph_assets[{tile_count}] = {{"
    )
    for i in range(tile_count):
        offset = i * bytes_per_tile
        lines.append("    {")
        lines.append(f"        .data = &{symbol}_tile_data[{offset}u],")
        lines.append(f"        .width_bytes = {symbol.upper()}_TILE_WIDTH_BYTES,")
        lines.append(f"        .height_lines = {symbol.upper()}_TILE_HEIGHT_LINES,")
        lines.append(f"        .mode = {symbol.upper()}_MODE,")
        lines.append(f"        .palette = {symbol.upper()}_DEFAULT_PALETTE,")
        lines.append(f"        .width_twos_comp = {symbol.upper()}_TILE_WIDTH_TWOS_COMP,")
        lines.append(f"        .data_layout = {symbol.upper()}_DATA_LAYOUT,")
        lines.append("    },")
    lines.append("};")
    lines.append("")
    lines.append(f"static const uint8_t {symbol}_char_to_glyph[128] = {{")
    for i in range(0, 128, 16):
        chunk = ", ".join(f"0x{v:02x}" for v in lookup[i : i + 16])
        lines.append(f"    {chunk},")
    lines.append("};")
    lines.append("")
    if charmap:
        escaped = "".join(c_escape(ch) for ch in charmap)
        lines.append(f"/* Character order: \"{escaped}\" */")
    lines.append(f"static const atari7800_font_descriptor_t {symbol}_font = {{")
    lines.append(f"    .glyphs = {symbol}_glyph_assets,")
    lines.append(f"    .char_to_glyph = {symbol}_char_to_glyph,")
    lines.append(f"    .char_to_glyph_len = 128u,")
    lines.append("    .first_char = 0u,")
    lines.append(f"    .glyph_count = {symbol.upper()}_TILE_COUNT,")
    lines.append(f"    .glyph_advance = {glyph_advance}u,")
    lines.append(f"    .space_advance = {space_advance}u,")
    lines.append(f"    .line_advance = {line_advance}u,")
    lines.append("};")
    lines.append("")
    lines.append(f"#endif /* {guard} */")
    lines.append("")

    out_path.write_text("\n".join(lines), encoding="ascii")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("input_png")
    parser.add_argument("output_header")
    parser.add_argument("symbol")
    parser.add_argument("--tile-width", type=int, default=8)
    parser.add_argument("--tile-height", type=int, default=16)
    parser.add_argument("--tile-offset-y", type=int, default=0)
    parser.add_argument("--mode", type=lambda x: int(x, 0), default=0x40)
    parser.add_argument("--palette", type=int, default=5)
    parser.add_argument("--glyph-advance", type=int, default=8)
    parser.add_argument("--space-advance", type=int, default=8)
    parser.add_argument("--line-advance", type=int, default=16)
    parser.add_argument("--charmap", default="")
    args = parser.parse_args()

    width, height, bit_depth, rows = parse_png(Path(args.input_png))

    if args.tile_width <= 0 or args.tile_height <= 0:
        raise ValueError("tile dimensions must be positive")
    if width % args.tile_width != 0:
        raise ValueError("image width must be an exact multiple of tile width")
    if args.tile_offset_y < 0:
        raise ValueError("tile_offset_y must be >= 0")
    if args.tile_offset_y + args.tile_height > height:
        raise ValueError("tile area exceeds image height")

    tile_count = width // args.tile_width
    if args.charmap and len(args.charmap) > tile_count:
        raise ValueError("charmap length exceeds tile count")

    unpacked_rows = [row_indices(row, width, bit_depth) for row in rows]
    packed_tiles = []

    for tile in range(tile_count):
        x0 = tile * args.tile_width
        tile_rows = []
        for y in range(args.tile_offset_y, args.tile_offset_y + args.tile_height):
            tile_indices = unpacked_rows[y][x0 : x0 + args.tile_width]
            tile_rows.append(pack_160a(tile_indices))
        packed_tiles.append(tile_rows)

    emit_header(
        Path(args.output_header),
        args.symbol,
        args.tile_width,
        args.tile_height,
        tile_count,
        packed_tiles,
        args.mode,
        args.palette,
        args.glyph_advance,
        args.space_advance,
        args.line_advance,
        args.charmap,
    )


if __name__ == "__main__":
    main()