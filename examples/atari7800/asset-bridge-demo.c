#include <atari7800.h>
#include "assets/fighter.sprite.h"

int main(void) {
  atari7800_scene_t scene;
  int16_t x = 76;
  int8_t dx = 1;

  atari7800_scene_init_160a(&scene, ATARI7800_BG_DARKGRAY);
  atari7800_scene_set_palette(&scene, 3, (atari7800_palette3_t){0xb4, 0x46, 0x1c});

  for (;;) {
    atari7800_wait_vblank();

    x += dx;
    if (x >= 120 || x <= 32) dx = -dx;

    atari7800_scene_begin_frame(&scene);
    atari7800_scene_draw_sprite(&scene, &fighter_sprite, x, 96);
    atari7800_scene_draw_text(&scene, &hud_font, 4, 8, "ASTROWING");
    atari7800_scene_end_frame(&scene);
  }
}
