#ifndef ATARI7800_ASSET_BULLET_H
#define ATARI7800_ASSET_BULLET_H

#include <stdint.h>

#define BULLET_WIDTH_BYTES 1u
#define BULLET_HEIGHT_LINES 16u
#define BULLET_DEFAULT_PALETTE 1u

extern const uint8_t game_block_1[];
#define bullet_data (game_block_1 + 32)

#endif
