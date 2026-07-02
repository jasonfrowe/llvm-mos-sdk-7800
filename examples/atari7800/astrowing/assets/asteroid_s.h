#ifndef ATARI7800_ASSET_ASTEROID_S_H
#define ATARI7800_ASSET_ASTEROID_S_H

#include <stdint.h>

#define ASTEROID_S_WIDTH_BYTES 2u
#define ASTEROID_S_HEIGHT_LINES 16u
#define ASTEROID_S_DEFAULT_PALETTE 2u

extern const uint8_t game_block_0[];
#define asteroid_s_data (game_block_0 + 180)

#endif
