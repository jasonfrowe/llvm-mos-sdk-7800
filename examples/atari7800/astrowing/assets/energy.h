#ifndef ATARI7800_ASSET_ENERGY_H
#define ATARI7800_ASSET_ENERGY_H

#include <stdint.h>

#define ENERGY_WIDTH_BYTES 1u
#define ENERGY_HEIGHT_LINES 16u
#define ENERGY_DEFAULT_PALETTE 5u

extern const uint8_t game_block_1[];
#define energy_data (game_block_1 + 38)

#endif
