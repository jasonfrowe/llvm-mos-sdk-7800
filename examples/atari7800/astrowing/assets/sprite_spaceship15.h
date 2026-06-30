#ifndef ATARI7800_ASSET_SPRITE_SPACESHIP15_H
#define ATARI7800_ASSET_SPRITE_SPACESHIP15_H

#include <stdint.h>
#include <atari7800.h>

#define SPRITE_SPACESHIP15_WIDTH_PIXELS 16u
#define SPRITE_SPACESHIP15_HEIGHT_LINES 16u
#define SPRITE_SPACESHIP15_WIDTH_BYTES 4u
#define SPRITE_SPACESHIP15_WIDTH_TWOS_COMP 0x1cu
#define SPRITE_SPACESHIP15_MODE 0x40u
#define SPRITE_SPACESHIP15_DEFAULT_PALETTE 3u
#define SPRITE_SPACESHIP15_DATA_LAYOUT ATARI7800_SPRITE_LAYOUT_CONTIGUOUS_160A

static const uint8_t sprite_spaceship15[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
    0x00, 0x34, 0x00, 0x00, 0x00, 0x19, 0x5f, 0x00, 0x00, 0x0d, 0x7f, 0x00,
    0x00, 0x07, 0xfc, 0x00, 0x00, 0x07, 0xfc, 0x00, 0x00, 0x07, 0xb8, 0x00,
    0x00, 0x07, 0xb8, 0x00, 0x00, 0x07, 0xf4, 0x00, 0x00, 0x07, 0xe4, 0x00,
    0x00, 0x0f, 0x90, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x0c, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

static const uint16_t sprite_spaceship15_len = 64u;

static const atari7800_sprite_asset_t sprite_spaceship15_asset = {
    .data = sprite_spaceship15,
    .width_bytes = SPRITE_SPACESHIP15_WIDTH_BYTES,
    .height_lines = SPRITE_SPACESHIP15_HEIGHT_LINES,
    .mode = SPRITE_SPACESHIP15_MODE,
    .palette = SPRITE_SPACESHIP15_DEFAULT_PALETTE,
    .width_twos_comp = SPRITE_SPACESHIP15_WIDTH_TWOS_COMP,
    .data_layout = SPRITE_SPACESHIP15_DATA_LAYOUT,
};

#endif
