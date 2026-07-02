#ifndef ATARI7800_ASSET_HEART_H
#define ATARI7800_ASSET_HEART_H

#include <stdint.h>

#define HEART_WIDTH_BYTES 2u
#define HEART_HEIGHT_LINES 16u
#define HEART_DEFAULT_PALETTE 5u

extern const uint8_t game_block_1[];
#define heart_data (game_block_1 + 40)

#endif
