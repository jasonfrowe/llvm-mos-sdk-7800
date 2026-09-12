#ifndef ATARI7800_ASSET_SPRITE_SPACESHIP10_H
#define ATARI7800_ASSET_SPRITE_SPACESHIP10_H

#include <stdint.h>
#include <atari7800.h>

#define SPRITE_SPACESHIP10_WIDTH_PIXELS 16u
#define SPRITE_SPACESHIP10_HEIGHT_LINES 16u
#define SPRITE_SPACESHIP10_WIDTH_BYTES 4u
#define SPRITE_SPACESHIP10_WIDTH_TWOS_COMP 0x1cu
#define SPRITE_SPACESHIP10_MODE 0x40u
#define SPRITE_SPACESHIP10_DEFAULT_PALETTE 3u
#define SPRITE_SPACESHIP10_DATA_LAYOUT ATARI7800_SPRITE_LAYOUT_CONTIGUOUS_160A

static const uint8_t sprite_spaceship10[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x50, 0x00,
    0x00, 0x3e, 0xa0, 0x00, 0x00, 0x3f, 0xec, 0x00, 0x00, 0x0f, 0xf8, 0x00,
    0x00, 0x0f, 0xbc, 0x00, 0x00, 0x07, 0xbf, 0x00, 0x00, 0x07, 0xff, 0x00,
    0x00, 0x07, 0xf4, 0x00, 0x00, 0x01, 0x50, 0x00, 0x00, 0x0f, 0x40, 0x00,
    0x00, 0x0a, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

static const uint16_t sprite_spaceship10_len = 64u;

static const atari7800_sprite_asset_t sprite_spaceship10_asset = {
    .data = sprite_spaceship10,
    .width_bytes = SPRITE_SPACESHIP10_WIDTH_BYTES,
    .height_lines = SPRITE_SPACESHIP10_HEIGHT_LINES,
    .mode = SPRITE_SPACESHIP10_MODE,
    .palette = SPRITE_SPACESHIP10_DEFAULT_PALETTE,
    .width_twos_comp = SPRITE_SPACESHIP10_WIDTH_TWOS_COMP,
    .data_layout = SPRITE_SPACESHIP10_DATA_LAYOUT,
};

#endif
