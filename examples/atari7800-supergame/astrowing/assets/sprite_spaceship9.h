#ifndef ATARI7800_ASSET_SPRITE_SPACESHIP9_H
#define ATARI7800_ASSET_SPRITE_SPACESHIP9_H

#include <stdint.h>
#include <atari7800.h>

#define SPRITE_SPACESHIP9_WIDTH_PIXELS 16u
#define SPRITE_SPACESHIP9_HEIGHT_LINES 16u
#define SPRITE_SPACESHIP9_WIDTH_BYTES 4u
#define SPRITE_SPACESHIP9_WIDTH_TWOS_COMP 0x1cu
#define SPRITE_SPACESHIP9_MODE 0x40u
#define SPRITE_SPACESHIP9_DEFAULT_PALETTE 3u
#define SPRITE_SPACESHIP9_DATA_LAYOUT ATARI7800_SPRITE_LAYOUT_CONTIGUOUS_160A

static const uint8_t sprite_spaceship9[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x40, 0x00, 0x00, 0x0e, 0xb0, 0x00,
    0x00, 0x0b, 0xe0, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xfe, 0xbf, 0x00,
    0x00, 0x3e, 0xbc, 0x00, 0x00, 0x1f, 0xf4, 0x00, 0x00, 0x07, 0xd0, 0x00,
    0x00, 0x07, 0xd0, 0x00, 0x00, 0x01, 0x40, 0x00, 0x00, 0x03, 0xc0, 0x00,
    0x00, 0x02, 0x80, 0x00, 0x00, 0x01, 0x40, 0x00, 0x00, 0x03, 0xc0, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

static const uint16_t sprite_spaceship9_len = 64u;

static const atari7800_sprite_asset_t sprite_spaceship9_asset = {
    .data = sprite_spaceship9,
    .width_bytes = SPRITE_SPACESHIP9_WIDTH_BYTES,
    .height_lines = SPRITE_SPACESHIP9_HEIGHT_LINES,
    .mode = SPRITE_SPACESHIP9_MODE,
    .palette = SPRITE_SPACESHIP9_DEFAULT_PALETTE,
    .width_twos_comp = SPRITE_SPACESHIP9_WIDTH_TWOS_COMP,
    .data_layout = SPRITE_SPACESHIP9_DATA_LAYOUT,
};

#endif
