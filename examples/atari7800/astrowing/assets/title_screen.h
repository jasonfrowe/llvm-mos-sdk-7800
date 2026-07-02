#ifndef ATARI7800_ASSET_TITLE_SCREEN_H
#define ATARI7800_ASSET_TITLE_SCREEN_H

#include <stdint.h>

extern const uint8_t title_block[];

#define title_screen_zone0 (title_block + 0)
#define title_screen_zone1 (title_block + 40)
#define title_screen_zone2 (title_block + 80)
#define title_screen_zone3 (title_block + 120)
#define title_screen_zone4 (title_block + 160)
#define title_screen_zone5 (title_block + 200)

#endif
