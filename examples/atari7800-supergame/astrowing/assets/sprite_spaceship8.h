#ifndef ATARI7800_ASSET_SPRITE_SPACESHIP8_H
#define ATARI7800_ASSET_SPRITE_SPACESHIP8_H

#include <stdint.h>
#include <atari7800.h>

#define SPRITE_SPACESHIP8_WIDTH_PIXELS 16u
#define SPRITE_SPACESHIP8_HEIGHT_LINES 16u
#define SPRITE_SPACESHIP8_WIDTH_BYTES 4u
#define SPRITE_SPACESHIP8_WIDTH_TWOS_COMP 0x1cu
#define SPRITE_SPACESHIP8_MODE 0x40u
#define SPRITE_SPACESHIP8_DEFAULT_PALETTE 3u
#define SPRITE_SPACESHIP8_DATA_LAYOUT ATARI7800_SPRITE_LAYOUT_CONTIGUOUS_160A

static const uint8_t sprite_spaceship8[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0xcc, 0x00,
    0x00, 0x0a, 0xbc, 0x00, 0x00, 0x3b, 0xfc, 0x00, 0x00, 0x2f, 0xf0, 0x00,
    0x00, 0x3e, 0xf0, 0x00, 0x00, 0xfe, 0xd0, 0x00, 0x00, 0xbf, 0xd0, 0x00,
    0x00, 0x1f, 0xd0, 0x00, 0x00, 0x05, 0x40, 0x00, 0x00, 0x01, 0xf0, 0x00,
    0x00, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0xf0, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

static const uint16_t sprite_spaceship8_len = 64u;

static const atari7800_sprite_asset_t sprite_spaceship8_asset = {
    .data = sprite_spaceship8,
    .width_bytes = SPRITE_SPACESHIP8_WIDTH_BYTES,
    .height_lines = SPRITE_SPACESHIP8_HEIGHT_LINES,
    .mode = SPRITE_SPACESHIP8_MODE,
    .palette = SPRITE_SPACESHIP8_DEFAULT_PALETTE,
    .width_twos_comp = SPRITE_SPACESHIP8_WIDTH_TWOS_COMP,
    .data_layout = SPRITE_SPACESHIP8_DATA_LAYOUT,
};

#endif
