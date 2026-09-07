#!/usr/bin/env python3
"""Slice a tall indexed-color PNG into one MARIA-strided sprite asset per
zone-height-tall band, for use as a full-screen static background (the same
technique 7800basic's `plotbanner` uses: N zone-aligned Direct Mode objects
stacked vertically, not a new MARIA mode -- see 7800port.md).

Each output slice is zone-aligned (drawn with no fine-Y shift), so unlike
pack_sprites_to_strided.py's frame format there is no leading/trailing
padding for a shift range -- a slice needs exactly `zone_height` pages.
Row 0 (top of the slice) is packed at the *last* page and row
(zone_height-1) (bottom) at page 0 -- empirically confirmed (7800port.md,
"Title screen + POKEY music" entry) to be the correct direction for a
zone-aligned object: MARIA's per-scanline auto-increment renders the
*highest* page of a zero-shift object's window first (top of the zone).

Usage:
  python3 png_banner_to_zones_header.py --output assets/title_screen.h \
      --symbol title_screen --zone-height 16 --palette 0 input.png
"""

import argparse
import struct
import zlib
from pathlib import Path

PNG_SIG = b"\x89PNG\r\n\x1a\n"


def paeth(a: int, b: int, c: int) -> int:
    p = a + b - c
    pa = abs(p - a)
    pb = abs(p - b)
    pc = abs(p - c)
    if pa <= pb and pa <= pc:
        return a
    if pb <= pc:
        return b
    return c


def parse_png(path: Path):
    data = path.read_bytes()
    if not data.startswith(PNG_SIG):
        raise ValueError(f"{path.name} is not a PNG file")

    pos = len(PNG_SIG)
    width = height = bit_depth = color_type = None
    idat = bytearray()

    while pos < len(data):
        if pos + 8 > len(data):
            raise ValueError("truncated PNG chunk")
        length = struct.unpack(">I", data[pos : pos + 4])[0]
        ctype = data[pos + 4 : pos + 8]
        pos += 8
        chunk = data[pos : pos + length]
        pos += length
        pos += 4  # crc

        if ctype == b"IHDR":
            width, height, bit_depth, color_type, comp, flt, interlace = struct.unpack(
                ">IIBBBBB", chunk
            )
            if comp != 0 or flt != 0 or interlace != 0:
                raise ValueError("unsupported PNG encoding")
        elif ctype == b"IDAT":
            idat.extend(chunk)
        elif ctype == b"IEND":
            break

    if width is None or height is None:
        raise ValueError("missing IHDR")
    if color_type != 3:
        raise ValueError("only indexed-color PNG (color_type=3) is supported")
    if bit_depth not in (1, 2, 4, 8):
        raise ValueError("unsupported bit depth")

    raw = zlib.decompress(bytes(idat))
    row_bytes = (width * bit_depth + 7) // 8
    stride = row_bytes + 1
    if len(raw) != stride * height:
        raise ValueError("unexpected decompressed size")

    rows = []
    prev = bytearray(row_bytes)
    for y in range(height):
        row = bytearray(raw[y * stride + 1 : y * stride + 1 + row_bytes])
        ftype = raw[y * stride]

        if ftype == 0:
            pass
        elif ftype == 1:
            for i in range(row_bytes):
                left = row[i - 1] if i > 0 else 0
                row[i] = (row[i] + left) & 0xFF
        elif ftype == 2:
            for i in range(row_bytes):
                row[i] = (row[i] + prev[i]) & 0xFF
        elif ftype == 3:
            for i in range(row_bytes):
                left = row[i - 1] if i > 0 else 0
                up = prev[i]
                row[i] = (row[i] + ((left + up) >> 1)) & 0xFF
        elif ftype == 4:
            for i in range(row_bytes):
                left = row[i - 1] if i > 0 else 0
                up = prev[i]
                up_left = prev[i - 1] if i > 0 else 0
                row[i] = (row[i] + paeth(left, up, up_left)) & 0xFF
        else:
            raise ValueError(f"unsupported PNG filter type: {ftype}")

        rows.append(bytes(row))
        prev = row

    return width, height, bit_depth, rows


def row_indices(row: bytes, width: int, bit_depth: int):
    out = []
    if bit_depth == 8:
        return list(row[:width])

    mask = (1 << bit_depth) - 1
    bits = 8
    for b in row:
        shift = bits - bit_depth
        while shift >= 0 and len(out) < width:
            out.append((b >> shift) & mask)
            shift -= bit_depth
    return out[:width]


def pack_160a(indices):
    packed = []
    pad = (-len(indices)) % 4
    if pad:
        indices = indices + [0] * pad
    for i in range(0, len(indices), 4):
        p0 = indices[i] & 0x03
        p1 = indices[i + 1] & 0x03
        p2 = indices[i + 2] & 0x03
        p3 = indices[i + 3] & 0x03
        packed.append((p0 << 6) | (p1 << 4) | (p2 << 2) | p3)
    return packed


MAX_OBJECT_WIDTH_BYTES = 31  # MARIA's 5-bit two's-complement width field caps a
                             # single Direct Mode object at 31 bytes (124px in
                             # 160A's 4px/byte packing) -- wider content needs
                             # multiple objects side by side per zone.


def split_chunks(width_bytes: int):
    """Split width_bytes into as-even-as-possible chunks, each <= the max
    single-object width. Returns a list of (start_byte, chunk_width_bytes)."""
    num_chunks = -(-width_bytes // MAX_OBJECT_WIDTH_BYTES)  # ceil div
    base = width_bytes // num_chunks
    extra = width_bytes % num_chunks
    chunks = []
    start = 0
    for i in range(num_chunks):
        w = base + (1 if i < extra else 0)
        chunks.append((start, w))
        start += w
    return chunks


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    parser.add_argument("--symbol", required=True)
    parser.add_argument("--zone-height", type=int, default=16)
    parser.add_argument("--palette", type=int, default=0)
    parser.add_argument("input_png")
    args = parser.parse_args()

    zh = args.zone_height
    width, height, bit_depth, rows = parse_png(Path(args.input_png))
    if height % zh != 0:
        raise ValueError(
            f"image height {height} is not a multiple of --zone-height {zh}"
        )
    num_zones = height // zh

    packed_rows = [pack_160a(row_indices(row, width, bit_depth)) for row in rows]
    width_bytes = len(packed_rows[0]) if packed_rows else 0
    chunks = split_chunks(width_bytes)
    num_chunks = len(chunks)

    # Every (zone, chunk) pair is zone-aligned (shift=0), so -- exactly like
    # pack_sprites_to_strided.py packs multiple animation frames into one
    # set of pages at different per-frame byte offsets -- all num_zones *
    # num_chunks slices can share a single zone_height-page block, each at
    # its own byte offset within every page, instead of each getting its
    # own full zone_height-page block. This is the difference between
    # needing zone_height*256 bytes total (a few KB) and
    # num_zones*num_chunks*zone_height*256 bytes (tens of KB, blowing a
    # 32KB ROM for a banner this size).
    frame_offsets = {}
    offset = 0
    for z in range(num_zones):
        for c, (_start_byte, chunk_w) in enumerate(chunks):
            frame_offsets[(z, c)] = offset
            offset += chunk_w
    bytes_per_page = offset
    if bytes_per_page > 256:
        raise ValueError(
            f"all zones/chunks together need {bytes_per_page} bytes per page, "
            "which exceeds MARIA's 256-byte page size -- this image is too wide "
            "to pack this way; split it into multiple banners/palettes"
        )

    symbol = args.symbol
    symbol_upper = symbol.upper()

    lines = []
    lines.append(f"#ifndef ATARI7800_ASSET_{symbol_upper}_H")
    lines.append(f"#define ATARI7800_ASSET_{symbol_upper}_H")
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append("#include <atari7800.h>")
    lines.append("")
    lines.append(f"#define {symbol_upper}_WIDTH_PIXELS {width}u")
    lines.append(f"#define {symbol_upper}_ZONE_HEIGHT {zh}u")
    lines.append(f"#define {symbol_upper}_NUM_ZONES {num_zones}u")
    lines.append(f"#define {symbol_upper}_NUM_CHUNKS {num_chunks}u")
    lines.append(f"#define {symbol_upper}_MODE 0x40u")
    lines.append(f"#define {symbol_upper}_DEFAULT_PALETTE {args.palette}u")
    lines.append(f"#define {symbol_upper}_DATA_LAYOUT ATARI7800_SPRITE_LAYOUT_MARIA_STRIDED")
    lines.append("")
    lines.append(
        f"/* Pixel X offset of each of the {num_chunks} side-by-side chunks a "
        f"{width}px-wide row is split into (MARIA's width field caps a single "
        f"object at {MAX_OBJECT_WIDTH_BYTES * 4}px -- see MAX_OBJECT_WIDTH_BYTES "
        "in png_banner_to_zones_header.py). Draw chunk c of zone z at "
        "(base_x + chunk_x_offsets[c], base_y + z * ZONE_HEIGHT). */"
    )
    offsets = ", ".join(f"{start * 4}u" for start, _w in chunks)
    lines.append(
        f"static const uint8_t {symbol}_chunk_x_offsets[{symbol_upper}_NUM_CHUNKS] = {{ {offsets} }};"
    )
    lines.append("")

    # One shared zone_height-page block (no shift padding -- everything here
    # is drawn zone-aligned) holding every (zone, chunk) slice at its own
    # byte offset within every page -- see the frame_offsets comment above.
    # Row r (0 = top of slice) goes at page (zone_height - 1 - r): see the
    # module docstring for why.
    lines.append(
        f"static const uint8_t {symbol}_data[{zh} * 256] "
        "__attribute__((aligned(256))) = {"
    )
    for r in range(zh):
        page = zh - 1 - r
        page_bytes = []
        for z in range(num_zones):
            src_row = z * zh + r
            for c, (start_byte, chunk_w) in enumerate(chunks):
                page_bytes.extend(packed_rows[src_row][start_byte : start_byte + chunk_w])
        if any(b != 0 for b in page_bytes):
            vals = ", ".join(f"0x{b:02x}" for b in page_bytes)
            lines.append(f"  [{page} * 256] = {vals},")
    lines.append("};")
    lines.append("")

    lines.append(
        f"static const atari7800_sprite_asset_t "
        f"{symbol}_zones[{symbol_upper}_NUM_ZONES][{symbol_upper}_NUM_CHUNKS] = {{"
    )
    for z in range(num_zones):
        lines.append("  {")
        for c, (_start_byte, chunk_w) in enumerate(chunks):
            width_twos_comp = (0x20 - chunk_w) & 0xFF
            lines.append("    {")
            lines.append(f"      .data = &{symbol}_data[{frame_offsets[(z, c)]}u],")
            lines.append(f"      .width_bytes = {chunk_w}u,")
            lines.append(f"      .height_lines = {symbol_upper}_ZONE_HEIGHT,")
            lines.append(f"      .mode = {symbol_upper}_MODE,")
            lines.append(f"      .palette = {symbol_upper}_DEFAULT_PALETTE,")
            lines.append(f"      .width_twos_comp = 0x{width_twos_comp:02x}u,")
            lines.append(f"      .data_layout = {symbol_upper}_DATA_LAYOUT,")
            lines.append("    },")
        lines.append("  },")
    lines.append("};")
    lines.append("")
    lines.append("#endif")
    lines.append("")

    Path(args.output).write_text("\n".join(lines), encoding="ascii")


if __name__ == "__main__":
    main()
