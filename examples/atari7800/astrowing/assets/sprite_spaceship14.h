#ifndef ATARI7800_ASSET_SPRITE_SPACESHIP14_H
#define ATARI7800_ASSET_SPRITE_SPACESHIP14_H

#include <stdint.h>
#include <atari7800.h>

#define SPRITE_SPACESHIP14_WIDTH_PIXELS 16u
#define SPRITE_SPACESHIP14_HEIGHT_LINES 16u
#define SPRITE_SPACESHIP14_WIDTH_BYTES 4u
#define SPRITE_SPACESHIP14_WIDTH_TWOS_COMP 0x1cu
#define SPRITE_SPACESHIP14_MODE 0x40u
#define SPRITE_SPACESHIP14_DEFAULT_PALETTE 3u
#define SPRITE_SPACESHIP14_DATA_LAYOUT ATARI7800_SPRITE_LAYOUT_CONTIGUOUS_160A

static const uint8_t sprite_spaceship14[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00,
    0x00, 0x00, 0x7c, 0x00, 0x00, 0x1d, 0x7c, 0x00, 0x00, 0x1d, 0xfc, 0x00,
    0x00, 0x1f, 0xf8, 0x00, 0x00, 0x07, 0xe8, 0x00, 0x00, 0x07, 0xb8, 0x00,
    0x00, 0x03, 0xb4, 0x00, 0x00, 0x03, 0xf4, 0x00, 0x00, 0x01, 0xe0, 0x00,
    0x00, 0x01, 0xe0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

static const uint16_t sprite_spaceship14_len = 64u;

static const atari7800_sprite_asset_t sprite_spaceship14_asset = {
    .data = sprite_spaceship14,
    .width_bytes = SPRITE_SPACESHIP14_WIDTH_BYTES,
    .height_lines = SPRITE_SPACESHIP14_HEIGHT_LINES,
    .mode = SPRITE_SPACESHIP14_MODE,
    .palette = SPRITE_SPACESHIP14_DEFAULT_PALETTE,
    .width_twos_comp = SPRITE_SPACESHIP14_WIDTH_TWOS_COMP,
    .data_layout = SPRITE_SPACESHIP14_DATA_LAYOUT,
};

#endif
