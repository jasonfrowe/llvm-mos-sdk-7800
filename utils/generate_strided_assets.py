#!/usr/bin/env python3
import sys
from pathlib import Path

# Add utils to path so we can import png_indexed_to_160a_header
sys.path.append(str(Path(__file__).parent))
from png_indexed_to_160a_header import parse_png, pack_160a, row_indices

def main():
    utils_dir = Path(__file__).parent
    assets_dir = utils_dir.parent / "examples" / "atari7800" / "assets"
    
    # 1. Process fighter sprite
    fighter_png = assets_dir / "astrowing" / "fighter.png"
    width, height, bit_depth, rows = parse_png(fighter_png)
    
    unpacked_rows = [row_indices(row, width, bit_depth) for row in rows]
    fighter_packed = [pack_160a(idx_row) for idx_row in unpacked_rows]
    
    # 2. Process unified font
    font_png = assets_dir / "unified_font.png"
    f_width, f_height, f_bit_depth, f_rows = parse_png(font_png)
    
    tile_width = 8
    tile_height = 8
    tile_count = f_width // tile_width
    
    f_unpacked = [row_indices(row, f_width, f_bit_depth) for row in f_rows]
    
    # Group into tiles: packed_tiles[tile][row] = bytes
    packed_tiles = []
    for tile in range(tile_count):
        x0 = tile * tile_width
        tile_rows = []
        for y in range(tile_height):
            tile_indices = f_unpacked[y][x0 : x0 + tile_width]
            tile_rows.append(pack_160a(tile_indices))
        packed_tiles.append(tile_rows)
        
    # Write to fighter.sprite.h
    out_path = assets_dir / "fighter.sprite.h"
    
    lines = []
    lines.append("#ifndef FIGHTER_SPRITE_H")
    lines.append("#define FIGHTER_SPRITE_H")
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append("#include <atari7800.h>")
    lines.append("")
    
    # Emit fighter sprite data
    lines.append("/* Fighter Sprite Data (Strided layout - vertically flipped for MARIA scanline countdown) */")
    lines.append("static const uint8_t fighter_sprite_data[] __attribute__((aligned(256))) = {")
    for y, row_bytes in enumerate(fighter_packed):
        row_str = ", ".join(f"0x{b:02x}" for b in row_bytes)
        offset = (height - 1 - y) * 256
        lines.append(f"    [{offset}] = {row_str},")
    lines.append("};")
    lines.append("")
    
    # Emit fighter sprite descriptor
    width_bytes = len(fighter_packed[0])
    width_twos_comp = (0x20 - width_bytes) & 0xFF
    lines.append("static const atari7800_sprite_asset_t fighter_sprite = {")
    lines.append("    .data = fighter_sprite_data,")
    lines.append(f"    .width_bytes = {width_bytes}u,")
    lines.append(f"    .height_lines = {height}u,")
    lines.append("    .mode = 0x40u, // 160A mode")
    lines.append("    .palette = 3u,")
    lines.append(f"    .width_twos_comp = 0x{width_twos_comp:02x}u,")
    lines.append("    .data_layout = ATARI7800_SPRITE_LAYOUT_MARIA_STRIDED,")
    lines.append("};")
    lines.append("")
    
    # Emit font data (Strided layout: tile_height pages, flipped for MARIA scanline countdown)
    lines.append("/* Unified Font Data (Strided layout: 8 pages of 256 bytes) */")
    lines.append("static const uint8_t hud_font_data[] __attribute__((aligned(256))) = {")
    for y in range(tile_height):
        # Gather row y of all tiles
        page_bytes = []
        for t in range(tile_count):
            page_bytes.extend(packed_tiles[t][y])
        
        # Output page y at offset (tile_height - 1 - y)*256
        offset = (tile_height - 1 - y) * 256
        lines.append(f"    /* Page {tile_height - 1 - y} (offset {offset}) */")
        first = True
        for i in range(0, len(page_bytes), 12):
            chunk = page_bytes[i : i + 12]
            chunk_str = ", ".join(f"0x{b:02x}" for b in chunk)
            if first:
                lines.append(f"    [{offset}] = {chunk_str},")
                first = False
            else:
                lines.append(f"    {chunk_str},")
    lines.append("};")
    lines.append("")
    
    # Emit charmap lookup table
    charmap = "0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ.!?,\"$():*+-/<>"
    lookup = [0xFF] * 128
    for index, ch in enumerate(charmap):
        code = ord(ch)
        if code < 128:
            lookup[code] = index
            
    lines.append("static const uint8_t hud_font_char_to_glyph[128] = {")
    for i in range(0, 128, 16):
        chunk = ", ".join(f"0x{v:02x}" for v in lookup[i : i + 16])
        lines.append(f"    {chunk},")
    lines.append("};")
    lines.append("")
    
    # Emit font descriptor
    f_width_bytes = len(packed_tiles[0][0])
    f_width_twos_comp = (0x20 - f_width_bytes) & 0xFF
    lines.append("static const atari7800_font_descriptor_t hud_font = {")
    lines.append("    .data = hud_font_data,")
    lines.append("    .char_to_glyph = hud_font_char_to_glyph,")
    lines.append("    .char_to_glyph_len = 128u,")
    lines.append("    .first_char = 0u,")
    lines.append(f"    .glyph_count = {tile_count}u,")
    lines.append(f"    .glyph_width_bytes = {f_width_bytes}u,")
    lines.append(f"    .glyph_height_lines = {tile_height}u,")
    lines.append("    .glyph_mode = 0x40u, // 160A mode")
    lines.append("    .glyph_palette = 5u,")
    lines.append(f"    .glyph_width_twos_comp = 0x{f_width_twos_comp:02x}u,")
    lines.append("    .glyph_advance = 8u,")
    lines.append("    .space_advance = 8u,")
    lines.append("    .line_advance = 8u,")
    lines.append("};")
    lines.append("")
    lines.append("#endif")
    lines.append("")
    
    out_path.write_text("\n".join(lines), encoding="ascii")
    print(f"Generated {out_path} successfully!")

if __name__ == "__main__":
    main()
