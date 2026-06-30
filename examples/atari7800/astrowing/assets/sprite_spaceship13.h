#ifndef ATARI7800_ASSET_SPRITE_SPACESHIP13_H
#define ATARI7800_ASSET_SPRITE_SPACESHIP13_H

#include <stdint.h>
#include <atari7800.h>

#define SPRITE_SPACESHIP13_WIDTH_PIXELS 16u
#define SPRITE_SPACESHIP13_HEIGHT_LINES 16u
#define SPRITE_SPACESHIP13_WIDTH_BYTES 4u
#define SPRITE_SPACESHIP13_WIDTH_TWOS_COMP 0x1cu
#define SPRITE_SPACESHIP13_MODE 0x40u
#define SPRITE_SPACESHIP13_DEFAULT_PALETTE 3u
#define SPRITE_SPACESHIP13_DATA_LAYOUT ATARI7800_SPRITE_LAYOUT_CONTIGUOUS_160A

static const uint8_t sprite_spaceship13[] = {
    0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0xf0, 0x00,
    0x00, 0x00, 0xf0, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x01, 0xfc, 0x00,
    0x00, 0xe7, 0xb9, 0x00, 0x00, 0xe7, 0xb9, 0x00, 0x00, 0xe7, 0xb9, 0x00,
    0x00, 0xe7, 0xb9, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x01, 0xfc, 0x00,
    0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x30, 0x00,
    0x00, 0x00, 0x30, 0x00,
};

static const uint16_t sprite_spaceship13_len = 64u;

static const atari7800_sprite_asset_t sprite_spaceship13_asset = {
    .data = sprite_spaceship13,
    .width_bytes = SPRITE_SPACESHIP13_WIDTH_BYTES,
    .height_lines = SPRITE_SPACESHIP13_HEIGHT_LINES,
    .mode = SPRITE_SPACESHIP13_MODE,
    .palette = SPRITE_SPACESHIP13_DEFAULT_PALETTE,
    .width_twos_comp = SPRITE_SPACESHIP13_WIDTH_TWOS_COMP,
    .data_layout = SPRITE_SPACESHIP13_DATA_LAYOUT,
};

#endif
