#include <atari7800.h>
#include <stdint.h>

#include "assets/astrowing_fighter_160a.h"

/*
 * Slice B.1 demo:
 * - fighter pixels come from imported Astrowing fighter.png
 * - data is converted to 160A packed bytes at build-time/tool-time
 * - MARIA renders one real object from that converted payload
 */

static atari7800_maria_null_header_t null_zone;
static atari7800_maria_dll_entry_t
    display_list[ATARI7800_MARIA_NTSC_DLL_ENTRIES];
static uint8_t fighter_zone[7];
static uint8_t fighter_rows_strided[256u * ASTROWING_FIGHTER_160A_HEIGHT_LINES];

#define ATARI7800_P3C1 ATARI7800_MMIO8(0x002du)
#define ATARI7800_P3C2 ATARI7800_MMIO8(0x002eu)
#define ATARI7800_P3C3 ATARI7800_MMIO8(0x002fu)

static void init_fighter_data_strided(void) {
  uint16_t i;
  uint8_t row;

  for (i = 0; i < sizeof(fighter_rows_strided); ++i) {
    fighter_rows_strided[i] = 0x00u;
  }

  for (row = 0; row < ASTROWING_FIGHTER_160A_HEIGHT_LINES; ++row) {
    const uint16_t src = (uint16_t)(ASTROWING_FIGHTER_160A_HEIGHT_LINES - 1u - row) *
                         ASTROWING_FIGHTER_160A_WIDTH_BYTES;
    const uint16_t dst = (uint16_t)row << 8;
    fighter_rows_strided[dst] = astrowing_fighter_160a[src];
    fighter_rows_strided[(uint16_t)(dst + 1u)] = astrowing_fighter_160a[(uint16_t)(src + 1u)];
  }
}

static void init_display_list(void) {
  uint8_t i;

  atari7800_maria_init_null_header(&null_zone, 0);

  /* Top non-visible segment. */
  atari7800_maria_init_dll_entry(&display_list[0], 8, &null_zone, 0);

  /* Visible zones default to blank. */
  for (i = 0; i < 28; ++i) {
    atari7800_maria_init_dll_entry(&display_list[1u + i], 7, &null_zone, 0);
  }

  /* Render fighter in one 8-line zone near screen center. */
  atari7800_maria_init_dll_entry(&display_list[13], 7, fighter_zone, 0);

  /* Bottom non-visible segment. */
  atari7800_maria_init_dll_entry(&display_list[29], 15, &null_zone, 0);
  atari7800_maria_init_dll_entry(&display_list[30], 9, &null_zone, 0);
  atari7800_maria_init_dll_entry(&display_list[31], 15, &null_zone, 0);
  atari7800_maria_init_dll_entry(&display_list[32], 8, &null_zone, 0);
}

static void init_fighter_zone(uint8_t x_pos) {
  const uint16_t sprite_addr = atari7800_ptr16(fighter_rows_strided);

  /* Zone format follows cc65/7800basic object list: object then null header. */
  fighter_zone[0] = (uint8_t)(sprite_addr & 0xffu);
  fighter_zone[1] = 0x40u;
  fighter_zone[2] = (uint8_t)(sprite_addr >> 8);
  fighter_zone[3] = (uint8_t)(0x60u | ASTROWING_FIGHTER_160A_WIDTH_TWOS_COMP);
  fighter_zone[4] = x_pos;
  fighter_zone[5] = 0x00u;
  fighter_zone[6] = 0x00u;
}

int main(void) {
  uint8_t x_pos = 76;
  uint8_t dir = 1;
  uint8_t frame_div = 0;

  init_fighter_data_strided();
  init_fighter_zone(x_pos);
  init_display_list();

  atari7800_init_system();
  atari7800_configure_video(atari7800_ptr16(display_list),
                            ATARI7800_CTRL_MODE_160A | ATARI7800_CTRL_DMA_ENABLE,
                            0x02u);

  /* Match Astrowing fighter palette (palette 3): green, red, yellow. */
  ATARI7800_P3C1 = 0xb4u;
  ATARI7800_P3C2 = 0x46u;
  ATARI7800_P3C3 = 0x1cu;

  for (;;) {
    atari7800_wait_vblank();
    if (++frame_div < 4u) {
      continue;
    }
    frame_div = 0;

    if (dir != 0u) {
      if (x_pos < 120u) {
        ++x_pos;
      } else {
        dir = 0;
      }
    } else {
      if (x_pos > 32u) {
        --x_pos;
      } else {
        dir = 1;
      }
    }

    fighter_zone[4] = x_pos;
  }
}
