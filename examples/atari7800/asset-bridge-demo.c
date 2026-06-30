#include <atari7800.h>
#include <stdint.h>

#include "assets/astrowing_fighter_160a.h"
#include "assets/astrowing_unified_font_160a.h"

/*
 * Scene API demo:
 * - fighter pixels come from imported Astrowing fighter.png
 * - data is converted to 160A packed bytes at build-time/tool-time
 * - runtime uses scene lifecycle calls for frame rendering
 */

static atari7800_scene_t fighter_scene;
/* Unified font tile order: 0-9, blank, A-Z, then symbols. */
#define GLYPH_0 0u
#define GLYPH_A 11u
#define GLYPH_C 13u
#define GLYPH_E 15u
#define GLYPH_O 25u
#define GLYPH_R 28u
#define GLYPH_S 29u
#define GLYPH_T 30u

int main(void) {
  uint8_t x_pos = 76;
  uint8_t dir = 1;
  uint8_t frame_div = 0;

  atari7800_scene_init_160a(&fighter_scene, 0x02u);

  /* Match Astrowing fighter palette (palette 3): green, red, yellow. */
  atari7800_scene_set_palette(&fighter_scene, 3u,
                              (atari7800_palette3_t){0xb4u, 0x46u, 0x1cu});
  /* Unified font palette used for HUD text. */
  atari7800_scene_set_palette(&fighter_scene, 5u,
                              (atari7800_palette3_t){0x0eu, 0x1cu, 0x00u});

  /* Draw the full HUD once; per-frame loop will only refresh row anchors. */
  atari7800_scene_begin_frame(&fighter_scene);
  /* ASTRO */
  atari7800_scene_draw_sprite(&fighter_scene,
                              &astrowing_unified_font_160a_glyph_assets[GLYPH_A],
                              16u, 0u);
  atari7800_scene_draw_sprite(&fighter_scene,
                              &astrowing_unified_font_160a_glyph_assets[GLYPH_S],
                              24u, 0u);
  atari7800_scene_draw_sprite(&fighter_scene,
                              &astrowing_unified_font_160a_glyph_assets[GLYPH_T],
                              32u, 0u);
  atari7800_scene_draw_sprite(&fighter_scene,
                              &astrowing_unified_font_160a_glyph_assets[GLYPH_R],
                              40u, 0u);
  atari7800_scene_draw_sprite(&fighter_scene,
                              &astrowing_unified_font_160a_glyph_assets[GLYPH_O],
                              48u, 0u);

  /* SCORE */
  atari7800_scene_draw_sprite(&fighter_scene,
                              &astrowing_unified_font_160a_glyph_assets[GLYPH_S],
                              56u, 8u);
  atari7800_scene_draw_sprite(&fighter_scene,
                              &astrowing_unified_font_160a_glyph_assets[GLYPH_C],
                              64u, 8u);
  atari7800_scene_draw_sprite(&fighter_scene,
                              &astrowing_unified_font_160a_glyph_assets[GLYPH_O],
                              72u, 8u);
  atari7800_scene_draw_sprite(&fighter_scene,
                              &astrowing_unified_font_160a_glyph_assets[GLYPH_R],
                              80u, 8u);
  atari7800_scene_draw_sprite(&fighter_scene,
                              &astrowing_unified_font_160a_glyph_assets[GLYPH_E],
                              88u, 8u);

  /* 000000 */
  atari7800_scene_draw_sprite(&fighter_scene,
                              &astrowing_unified_font_160a_glyph_assets[GLYPH_0],
                              56u, 16u);
  atari7800_scene_draw_sprite(&fighter_scene,
                              &astrowing_unified_font_160a_glyph_assets[GLYPH_0],
                              64u, 16u);
  atari7800_scene_draw_sprite(&fighter_scene,
                              &astrowing_unified_font_160a_glyph_assets[GLYPH_0],
                              72u, 16u);
  atari7800_scene_draw_sprite(&fighter_scene,
                              &astrowing_unified_font_160a_glyph_assets[GLYPH_0],
                              80u, 16u);
  atari7800_scene_draw_sprite(&fighter_scene,
                              &astrowing_unified_font_160a_glyph_assets[GLYPH_0],
                              88u, 16u);
  atari7800_scene_draw_sprite(&fighter_scene,
                              &astrowing_unified_font_160a_glyph_assets[GLYPH_0],
                              96u, 16u);
  atari7800_scene_draw_sprite(&fighter_scene, &astrowing_fighter_160a_asset,
                              x_pos, 96u);
  atari7800_scene_end_frame(&fighter_scene);

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
    /* Keep HUD rows active with one cheap anchor glyph per row. */
    atari7800_scene_draw_sprite(&fighter_scene,
                  &astrowing_unified_font_160a_glyph_assets
                     [GLYPH_A],
                  16u, 0u);
    atari7800_scene_draw_sprite(&fighter_scene,
                  &astrowing_unified_font_160a_glyph_assets[GLYPH_S],
                  56u, 8u);
    atari7800_scene_draw_sprite(&fighter_scene,
                  &astrowing_unified_font_160a_glyph_assets[GLYPH_0],
                  56u, 16u);

    atari7800_scene_draw_sprite(&fighter_scene, &astrowing_fighter_160a_asset,
                                x_pos, 96u);
    atari7800_scene_end_frame(&fighter_scene);
  }
}
