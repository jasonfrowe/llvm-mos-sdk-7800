// Minimal SuperGame bankswitching smoke test: fixed-bank main() calls into
// three different switchable banks via banked_call_8000 (mapper.h), each
// setting a visibly distinct background color, cycling forever. Proves the
// bank-switch mechanism works end-to-end before any real game logic is
// migrated into the new layout (see mos-platform/atari7800-supergame/link.ld
// and 7800port.md).
//
// Deliberately does not touch MARIA/display-list setup or
// atari7800_wait_vblank(): this is a pure test of set_prg_8000/
// banked_call_8000, paced by a plain software delay loop instead of vblank,
// so it isn't entangled with unrelated video-timing behavior.
#include <atari7800.h>
#include <mapper.h>
#include <stdint.h>

__attribute__((section(".cart_rom_bank_0")))
static void set_color_bank0(void) {
  ATARI7800_BACKGRND = 0x1a;
}

__attribute__((section(".cart_rom_bank_1")))
static void set_color_bank1(void) {
  ATARI7800_BACKGRND = 0x5a;
}

__attribute__((section(".cart_rom_bank_2")))
static void set_color_bank2(void) {
  ATARI7800_BACKGRND = 0x9a;
}

static void delay(void) {
  volatile uint16_t i;
  for (i = 0; i < 20000u; i++) {
  }
}

int main(void) {
  atari7800_init_system();

  for (;;) {
    banked_call_8000(0, set_color_bank0);
    delay();
    banked_call_8000(1, set_color_bank1);
    delay();
    banked_call_8000(2, set_color_bank2);
    delay();
  }
}
