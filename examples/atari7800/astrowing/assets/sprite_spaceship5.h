#ifndef ATARI7800_ASSET_SPRITE_SPACESHIP5_H
#define ATARI7800_ASSET_SPRITE_SPACESHIP5_H

#include <stdint.h>
#include <atari7800.h>

#define SPRITE_SPACESHIP5_WIDTH_PIXELS 16u
#define SPRITE_SPACESHIP5_HEIGHT_LINES 16u
#define SPRITE_SPACESHIP5_WIDTH_BYTES 4u
#define SPRITE_SPACESHIP5_WIDTH_TWOS_COMP 0x1cu
#define SPRITE_SPACESHIP5_MODE 0x40u
#define SPRITE_SPACESHIP5_DEFAULT_PALETTE 3u
#define SPRITE_SPACESHIP5_DATA_LAYOUT ATARI7800_SPRITE_LAYOUT_CONTIGUOUS_160A

static const uint8_t sprite_spaceship5[] = {
    0x00, 0x0c, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00,
    0x00, 0x0f, 0x00, 0x00, 0x00, 0x3f, 0x40, 0x00, 0x00, 0x3f, 0x40, 0x00,
    0x00, 0x6e, 0xdb, 0x00, 0x00, 0x6e, 0xdb, 0x00, 0x00, 0x6e, 0xdb, 0x00,
    0x00, 0x6e, 0xdb, 0x00, 0x00, 0x3f, 0x40, 0x00, 0x00, 0x3f, 0x40, 0x00,
    0x00, 0x0f, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00,
    0x00, 0x0c, 0x00, 0x00,
};

static const uint16_t sprite_spaceship5_len = 64u;

static const atari7800_sprite_asset_t sprite_spaceship5_asset = {
    .data = sprite_spaceship5,
    .width_bytes = SPRITE_SPACESHIP5_WIDTH_BYTES,
    .height_lines = SPRITE_SPACESHIP5_HEIGHT_LINES,
    .mode = SPRITE_SPACESHIP5_MODE,
    .palette = SPRITE_SPACESHIP5_DEFAULT_PALETTE,
    .width_twos_comp = SPRITE_SPACESHIP5_WIDTH_TWOS_COMP,
    .data_layout = SPRITE_SPACESHIP5_DATA_LAYOUT,
};

#endif
