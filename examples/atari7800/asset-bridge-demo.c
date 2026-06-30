#include <atari7800.h>
#include <stdint.h>

#include "assets/astrowing_fighter_160a.h"

/*
 * Scene API demo:
 * - fighter pixels come from imported Astrowing fighter.png
 * - data is converted to 160A packed bytes at build-time/tool-time
 * - runtime uses scene lifecycle calls for frame rendering
 */

static atari7800_scene_t fighter_scene;

int main(void) {
  uint8_t x_pos = 76;
  uint8_t dir = 1;
  uint8_t frame_div = 0;

  atari7800_scene_init_160a(&fighter_scene, 0x02u);

  /* Match Astrowing fighter palette (palette 3): green, red, yellow. */
  atari7800_scene_set_palette(&fighter_scene, 3u,
                              (atari7800_palette3_t){0xb4u, 0x46u, 0x1cu});

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

    atari7800_scene_begin_frame(&fighter_scene);
    atari7800_scene_draw_sprite(&fighter_scene, &astrowing_fighter_160a_asset,
                                x_pos, 96u);
    atari7800_scene_end_frame(&fighter_scene);
  }
}
