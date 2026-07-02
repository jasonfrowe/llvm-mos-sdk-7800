#ifndef ATARI7800_ASSET_UNIFIED_FONT_H
#define ATARI7800_ASSET_UNIFIED_FONT_H

#include <stdint.h>

#define UNIFIED_FONT_WIDTH_BYTES 112u
#define UNIFIED_FONT_HEIGHT_LINES 16u
#define UNIFIED_FONT_DEFAULT_PALETTE 7u

extern const uint8_t game_block_0[];
#define unified_font_data (game_block_0 + 0)

#endif
