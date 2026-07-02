#ifndef ATARI7800_ASSET_LIVES_ICON_H
#define ATARI7800_ASSET_LIVES_ICON_H

#include <stdint.h>

#define LIVES_ICON_WIDTH_BYTES 2u
#define LIVES_ICON_HEIGHT_LINES 16u
#define LIVES_ICON_DEFAULT_PALETTE 5u

extern const uint8_t game_block_1[];
#define lives_icon_data (game_block_1 + 42)

#endif
