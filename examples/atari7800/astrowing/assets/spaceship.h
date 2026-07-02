#ifndef ATARI7800_ASSET_SPACESHIP_H
#define ATARI7800_ASSET_SPACESHIP_H

#include <stdint.h>

#define SPACESHIP_WIDTH_BYTES 4u
#define SPACESHIP_HEIGHT_LINES 16u
#define SPACESHIP_DEFAULT_PALETTE 5u
#define SPACESHIP_NUM_FRAMES 16u

extern const uint8_t game_block_0[];
#define spaceship_data (game_block_0 + 112)

#endif
