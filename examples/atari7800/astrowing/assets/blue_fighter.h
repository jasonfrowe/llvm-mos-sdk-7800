#ifndef ATARI7800_ASSET_BLUE_FIGHTER_H
#define ATARI7800_ASSET_BLUE_FIGHTER_H

#include <stdint.h>

#define BLUE_FIGHTER_WIDTH_BYTES 2u
#define BLUE_FIGHTER_HEIGHT_LINES 16u
#define BLUE_FIGHTER_DEFAULT_PALETTE 3u

extern const uint8_t game_block_1[];
#define blue_fighter_data (game_block_1 + 36)

#endif
