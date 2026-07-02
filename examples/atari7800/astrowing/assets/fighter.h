#ifndef ATARI7800_ASSET_FIGHTER_H
#define ATARI7800_ASSET_FIGHTER_H

#include <stdint.h>

#define FIGHTER_WIDTH_BYTES 1u
#define FIGHTER_HEIGHT_LINES 16u
#define FIGHTER_DEFAULT_PALETTE 3u

extern const uint8_t game_block_1[];
#define fighter_data (game_block_1 + 34)

#endif
