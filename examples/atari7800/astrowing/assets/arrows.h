#ifndef ATARI7800_ASSET_ARROWS_H
#define ATARI7800_ASSET_ARROWS_H

#include <stdint.h>

#define ARROWS_WIDTH_BYTES 2u
#define ARROWS_HEIGHT_LINES 16u
#define ARROWS_DEFAULT_PALETTE 3u
#define ARROWS_NUM_FRAMES 4u

extern const uint8_t game_block_1[];
#define arrows_data (game_block_1 + 16)

#endif
