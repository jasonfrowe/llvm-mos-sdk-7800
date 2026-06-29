#include <atari7800.h>

static atari7800_maria_null_header_t null_zone;
static atari7800_maria_dll_entry_t
    display_list[ATARI7800_MARIA_NTSC_DLL_ENTRIES];

int main(void) {
  atari7800_maria_init_null_header(&null_zone, 0);
  atari7800_maria_build_blank_ntsc(display_list, &null_zone);

  atari7800_init_system();
  atari7800_configure_video(atari7800_ptr16(display_list),
                            ATARI7800_CTRL_MODE_160A | ATARI7800_CTRL_DMA_ENABLE,
                            0x00);

  return 0;
}
