#ifndef ATARI7800_ASSET_SPRITE_SPACESHIP4_H
#define ATARI7800_ASSET_SPRITE_SPACESHIP4_H

#include <stdint.h>
#include <atari7800.h>

#define SPRITE_SPACESHIP4_WIDTH_PIXELS 16u
#define SPRITE_SPACESHIP4_HEIGHT_LINES 16u
#define SPRITE_SPACESHIP4_WIDTH_BYTES 4u
#define SPRITE_SPACESHIP4_WIDTH_TWOS_COMP 0x1cu
#define SPRITE_SPACESHIP4_MODE 0x40u
#define SPRITE_SPACESHIP4_DEFAULT_PALETTE 3u
#define SPRITE_SPACESHIP4_DATA_LAYOUT ATARI7800_SPRITE_LAYOUT_CONTIGUOUS_160A

static const uint8_t sprite_spaceship4[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00,
    0x00, 0x3d, 0x00, 0x00, 0x00, 0x3d, 0x74, 0x00, 0x00, 0x3f, 0x74, 0x00,
    0x00, 0x2f, 0xf4, 0x00, 0x00, 0x2b, 0xd0, 0x00, 0x00, 0x2e, 0xd0, 0x00,
    0x00, 0x1e, 0xc0, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x00, 0x0b, 0x40, 0x00,
    0x00, 0x0b, 0x40, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

static const uint16_t sprite_spaceship4_len = 64u;

static const atari7800_sprite_asset_t sprite_spaceship4_asset = {
    .data = sprite_spaceship4,
    .width_bytes = SPRITE_SPACESHIP4_WIDTH_BYTES,
    .height_lines = SPRITE_SPACESHIP4_HEIGHT_LINES,
    .mode = SPRITE_SPACESHIP4_MODE,
    .palette = SPRITE_SPACESHIP4_DEFAULT_PALETTE,
    .width_twos_comp = SPRITE_SPACESHIP4_WIDTH_TWOS_COMP,
    .data_layout = SPRITE_SPACESHIP4_DATA_LAYOUT,
};

#endif
