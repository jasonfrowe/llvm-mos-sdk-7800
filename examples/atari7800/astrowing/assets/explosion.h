#ifndef ATARI7800_ASSET_EXPLOSION_H
#define ATARI7800_ASSET_EXPLOSION_H

#include <stdint.h>

#define EXPLOSION_WIDTH_BYTES 1u
#define EXPLOSION_HEIGHT_LINES 16u
#define EXPLOSION_DEFAULT_PALETTE 3u
#define EXPLOSION_NUM_FRAMES 8u

extern const uint8_t game_block_1[];
#define explosion_data (game_block_1 + 24)

#endif
