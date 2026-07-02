#ifndef ATARI7800_ASSET_BOSS_H
#define ATARI7800_ASSET_BOSS_H

#include <stdint.h>

#define BOSS_WIDTH_BYTES 4u
#define BOSS_HEIGHT_LINES 16u
#define BOSS_DEFAULT_PALETTE 3u
#define BOSS_NUM_FRAMES 4u

extern const uint8_t game_block_1[];
#define boss_data (game_block_1 + 0)

#endif
