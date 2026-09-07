#ifndef FIGHTER_SPRITE_H
#define FIGHTER_SPRITE_H

#include <stdint.h>
#include <atari7800.h>

/* Fighter Sprite Data (Strided layout - vertically flipped for MARIA scanline
 * countdown). Explicitly sized to 16 pages (4096 bytes) rather than letting
 * the initializer list imply a size covering only the real 8 pages: a
 * 16-line zone's Direct Mode object genuinely reads all 16 pages regardless
 * of this sprite's own 8-line height, so the extra 8 pages must be explicit
 * zero bytes (rendering as blank) rather than whatever ROM bytes happen to
 * follow a shorter array -- otherwise they show up as a stray garbage line.
 * The extra pages are harmless in an 8-line zone (e.g. asset-bridge-demo,
 * which doesn't override ATARI7800_ZONE_HEIGHT): only the first 8 ever get
 * read there. */
static const uint8_t fighter_sprite_data[4096] __attribute__((aligned(256))) = {
    [1792] = 0x00, 0x00,
    [1536] = 0x02, 0x40,
    [1280] = 0x0a, 0x50,
    [1024] = 0x0f, 0xf0,
    [768] = 0x3f, 0xfc,
    [512] = 0x3f, 0xfc,
    [256] = 0x03, 0xc0,
    [0] = 0x00, 0x00,
};

static const atari7800_sprite_asset_t fighter_sprite = {
    .data = fighter_sprite_data,
    .width_bytes = 2u,
    .height_lines = 8u,
    .mode = 0x40u, // 160A mode
    .palette = 3u,
    .width_twos_comp = 0x1eu,
    .data_layout = ATARI7800_SPRITE_LAYOUT_MARIA_STRIDED,
};

#endif
