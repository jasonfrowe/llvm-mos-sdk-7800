#ifndef ASTROWING_FIGHTER_160A_H
#define ASTROWING_FIGHTER_160A_H

#include <stdint.h>

#define ASTROWING_FIGHTER_160A_WIDTH_PIXELS 8u
#define ASTROWING_FIGHTER_160A_HEIGHT_LINES 8u
#define ASTROWING_FIGHTER_160A_WIDTH_BYTES 2u
#define ASTROWING_FIGHTER_160A_WIDTH_TWOS_COMP 0x1eu

static const uint8_t astrowing_fighter_160a[] = {
    0x00, 0x00, 0x02, 0x40, 0x0a, 0x50, 0x0f, 0xf0, 0x3f, 0xfc, 0x3f, 0xfc,
    0x03, 0xc0, 0x00, 0x00,
};

static const uint16_t astrowing_fighter_160a_len = 16u;

#endif
