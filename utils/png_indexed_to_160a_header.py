#!/usr/bin/env python3
"""Convert an indexed-color PNG to 160A 2bpp row bytes header.

Output format packs 4 pixels per byte: p0..p3 -> bits 7..0 as 2bpp indices.
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
        raise ValueError("not a PNG file")

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


def emit_header(out_path: Path, symbol: str, width: int, height: int, packed_rows):
    width_bytes = len(packed_rows[0]) if packed_rows else 0
    width_twos_comp = (0x20 - width_bytes) & 0xFF

    lines = []
    lines.append("#ifndef ASTROWING_FIGHTER_160A_H")
    lines.append("#define ASTROWING_FIGHTER_160A_H")
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append("#include <atari7800.h>")
    lines.append("")
    lines.append(f"#define {symbol.upper()}_WIDTH_PIXELS {width}u")
    lines.append(f"#define {symbol.upper()}_HEIGHT_LINES {height}u")
    lines.append(f"#define {symbol.upper()}_WIDTH_BYTES {width_bytes}u")
    lines.append(f"#define {symbol.upper()}_WIDTH_TWOS_COMP 0x{width_twos_comp:02x}u")
    lines.append(f"#define {symbol.upper()}_MODE 0x40u")
    lines.append(f"#define {symbol.upper()}_DEFAULT_PALETTE 3u")
    lines.append(
        f"#define {symbol.upper()}_DATA_LAYOUT ATARI7800_SPRITE_LAYOUT_CONTIGUOUS_160A"
    )
    lines.append("")
    lines.append(f"static const uint8_t {symbol}[] = {{")

    flat = [b for row in packed_rows for b in row]
    for i in range(0, len(flat), 12):
        chunk = ", ".join(f"0x{v:02x}" for v in flat[i : i + 12])
        lines.append(f"    {chunk},")
    lines.append("};")
    lines.append("")
    lines.append(f"static const uint16_t {symbol}_len = {len(flat)}u;")
    lines.append("")
    lines.append(
        f"static const atari7800_sprite_asset_t {symbol}_asset = {{"
    )
    lines.append(f"    .data = {symbol},")
    lines.append(f"    .width_bytes = {symbol.upper()}_WIDTH_BYTES,")
    lines.append(f"    .height_lines = {symbol.upper()}_HEIGHT_LINES,")
    lines.append(f"    .mode = {symbol.upper()}_MODE,")
    lines.append(f"    .palette = {symbol.upper()}_DEFAULT_PALETTE,")
    lines.append(f"    .width_twos_comp = {symbol.upper()}_WIDTH_TWOS_COMP,")
    lines.append(f"    .data_layout = {symbol.upper()}_DATA_LAYOUT,")
    lines.append("};")
    lines.append("")
    lines.append("#endif")
    lines.append("")

    out_path.write_text("\n".join(lines), encoding="ascii")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("input_png")
    parser.add_argument("output_header")
    parser.add_argument("symbol")
    args = parser.parse_args()

    width, height, bit_depth, rows = parse_png(Path(args.input_png))
    packed_rows = []
    for row in rows:
        idx = row_indices(row, width, bit_depth)
        packed_rows.append(pack_160a(idx))

    emit_header(Path(args.output_header), args.symbol, width, height, packed_rows)


if __name__ == "__main__":
    main()
