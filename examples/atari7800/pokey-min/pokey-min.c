#include <atari7800.h>
#include <atari7800_pokey.h>

int main(void) {
  atari7800_pokey_init();
  for (;;) {
    atari7800_wait_vblank();
  }
}
