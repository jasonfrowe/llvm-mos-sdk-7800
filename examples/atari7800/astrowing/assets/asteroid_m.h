#ifndef ATARI7800_ASSET_ASTEROID_M_H
#define ATARI7800_ASSET_ASTEROID_M_H

#include <stdint.h>

#define ASTEROID_M_WIDTH_BYTES 4u
#define ASTEROID_M_HEIGHT_LINES 16u
#define ASTEROID_M_DEFAULT_PALETTE 2u

extern const uint8_t game_block_0[];
#define asteroid_m_data (game_block_0 + 176)

#endif
