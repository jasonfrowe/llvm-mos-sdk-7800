#include <atari7800.h>
#include <stdint.h>

static atari7800_maria_null_header_t null_zone;
static atari7800_maria_dll_entry_t
  display_list[ATARI7800_MARIA_NTSC_DLL_ENTRIES];

static void init_blank_frame_dll(void) {
  atari7800_maria_init_null_header(&null_zone, 0);
  atari7800_maria_build_blank_ntsc(display_list, &null_zone);
}

int main(void) {
  uint8_t color = 0x0a;
  uint8_t frame_div = 0;

  init_blank_frame_dll();
  atari7800_init_system();
  atari7800_configure_video(atari7800_ptr16(display_list),
                            ATARI7800_CTRL_MODE_160A | ATARI7800_CTRL_DMA_ENABLE,
                            0x00);

  for (;;) {
    atari7800_wait_vblank();
    if (++frame_div >= 8u) {
      frame_div = 0;
      ATARI7800_BACKGRND = (uint8_t)(color + 1u);
      ATARI7800_P0C1 = color;
      ATARI7800_P0C2 = (uint8_t)(color + 2u);
      ATARI7800_P0C3 = (uint8_t)(color + 4u);
      color = (uint8_t)(color + 1u);
    }
  }
}
