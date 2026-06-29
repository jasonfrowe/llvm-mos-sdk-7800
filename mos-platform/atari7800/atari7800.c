#include <atari7800.h>

void atari7800_init_system(void) {
  ATARI7800_INPTCTRL = 0x07;
  ATARI7800_CTRL = 0x7f;
  ATARI7800_OFFSET = 0x00;
  ATARI7800_INPTCTRL = 0x00;
}

void atari7800_wait_vblank(void) {
  while ((ATARI7800_MSTAT & 0x80u) != 0) {
  }
  while ((ATARI7800_MSTAT & 0x80u) == 0) {
  }
}

void atari7800_configure_video(uint16_t display_list_addr, uint8_t ctrl,
                               uint8_t bgcolor) {
  atari7800_set_display_list(display_list_addr);
  atari7800_wait_vblank();
  ATARI7800_CTRL = ctrl;
  ATARI7800_BACKGRND = bgcolor;
}

void atari7800_maria_build_blank_ntsc(
    atari7800_maria_dll_entry_t *display_list,
    const atari7800_maria_null_header_t *zone_header) {
  uint8_t i;

  atari7800_maria_init_dll_entry(&display_list[0], 8, zone_header, 0);

  for (i = 0; i < 28; ++i) {
    atari7800_maria_init_dll_entry(&display_list[1u + i], 7, zone_header, 0);
  }

  atari7800_maria_init_dll_entry(&display_list[29], 15, zone_header, 0);
  atari7800_maria_init_dll_entry(&display_list[30], 9, zone_header, 0);
  atari7800_maria_init_dll_entry(&display_list[31], 15, zone_header, 0);
  atari7800_maria_init_dll_entry(&display_list[32], 8, zone_header, 0);
}
