#ifndef ATARI7800_ASSET_SPRITE_SPACESHIP3_H
#define ATARI7800_ASSET_SPRITE_SPACESHIP3_H

#include <stdint.h>
#include <atari7800.h>

#define SPRITE_SPACESHIP3_WIDTH_PIXELS 16u
#define SPRITE_SPACESHIP3_HEIGHT_LINES 16u
#define SPRITE_SPACESHIP3_WIDTH_BYTES 4u
#define SPRITE_SPACESHIP3_WIDTH_TWOS_COMP 0x1cu
#define SPRITE_SPACESHIP3_MODE 0x40u
#define SPRITE_SPACESHIP3_DEFAULT_PALETTE 3u
#define SPRITE_SPACESHIP3_DATA_LAYOUT ATARI7800_SPRITE_LAYOUT_CONTIGUOUS_160A

static const uint8_t sprite_spaceship3[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
    0x00, 0x00, 0x1c, 0x00, 0x00, 0xf5, 0x64, 0x00, 0x00, 0xfd, 0x70, 0x00,
    0x00, 0x3f, 0xd0, 0x00, 0x00, 0x3f, 0xd0, 0x00, 0x00, 0x2e, 0xd0, 0x00,
    0x00, 0x2e, 0xd0, 0x00, 0x00, 0x1f, 0xd0, 0x00, 0x00, 0x1b, 0xd0, 0x00,
    0x00, 0x06, 0xf0, 0x00, 0x00, 0x03, 0xf0, 0x00, 0x00, 0x00, 0x30, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

static const uint16_t sprite_spaceship3_len = 64u;

static const atari7800_sprite_asset_t sprite_spaceship3_asset = {
    .data = sprite_spaceship3,
    .width_bytes = SPRITE_SPACESHIP3_WIDTH_BYTES,
    .height_lines = SPRITE_SPACESHIP3_HEIGHT_LINES,
    .mode = SPRITE_SPACESHIP3_MODE,
    .palette = SPRITE_SPACESHIP3_DEFAULT_PALETTE,
    .width_twos_comp = SPRITE_SPACESHIP3_WIDTH_TWOS_COMP,
    .data_layout = SPRITE_SPACESHIP3_DATA_LAYOUT,
};

#endif
