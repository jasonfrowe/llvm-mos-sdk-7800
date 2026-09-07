/* Title screen shakedown: renders astrowing.bas's real title-screen banner
 * (title_screen_conv.png, converted via
 * utils/png_banner_to_zones_header.py) as a set of zone-aligned static
 * Direct Mode objects, and plays its real title theme (Song_01_30hz.bin)
 * through POKEY -- exercising the platform's static-zone/memory-management
 * path across every visible zone at once (unlike astrowing.c, which only
 * ever pins a single HUD zone), plus new POKEY sound-chip support, before
 * committing to the larger gameplay systems (levels, collision) 7800port.md
 * still has open.
 *
 * Deliberate simplifications from the reference (astrowing.bas:341 draws
 * this banner at y=46, not zone-aligned): placed here at y=0 instead, since
 * a non-zone-aligned multi-slice banner would need cross-zone fine-Y
 * splitting, which nothing in this SDK implements yet (see 7800port.md).
 */
#define ATARI7800_ZONE_HEIGHT 16
#include <atari7800.h>
#include <atari7800_pokey.h>

#include "assets/song_title.h"
#include "assets/title_screen.h"

static atari7800_scene_t scene;

int main(void) {
  uint8_t zone_index;
  uint8_t chunk_index;
  uint16_t frame_count = 0;
  const uint8_t *music_cursor = song_title;

  atari7800_scene_init_160a(&scene, 0x00u /* black */);

  /* title_screen_conv.png's 4 indices: 0 = background (shows BACKGRND,
   * set above), 1-3 = the 3 real colors this palette carries. Approximate
   * NTSC hue/luminance bytes for the source PNG's yellow/orange/white --
   * not colorimetrically calibrated, just picked to look right; this is a
   * rendering-pipeline shakedown; exact color fidelity isn't the point. */
  atari7800_scene_set_palette(&scene, TITLE_SCREEN_DEFAULT_PALETTE,
      (atari7800_palette3_t){0x1e, 0x2a, 0x0f});

  atari7800_scene_begin_frame(&scene);
  for (zone_index = 0; zone_index < TITLE_SCREEN_NUM_ZONES; ++zone_index) {
    uint8_t y = (uint8_t)(zone_index * TITLE_SCREEN_ZONE_HEIGHT);
    for (chunk_index = 0; chunk_index < TITLE_SCREEN_NUM_CHUNKS; ++chunk_index) {
      atari7800_scene_draw_sprite(&scene,
          &title_screen_zones[zone_index][chunk_index],
          title_screen_chunk_x_offsets[chunk_index], y);
    }
    /* Whole banner is drawn once and never changes -- pin every zone it
     * occupies static so begin_frame/end_frame leave them alone forever,
     * matching astrowing.c's HUD pattern but across all 6 zones instead
     * of just one. */
    atari7800_scene_set_zone_static(&scene, y, 1u);
  }
  atari7800_scene_end_frame(&scene);

  for (;;) {
    atari7800_wait_vblank();
    ++frame_count;

    /* Song data is authored at 30Hz (half the NTSC frame rate) -- see
     * atari7800_pokey_step's doc comment. */
    if ((frame_count & 1u) == 0u) {
      atari7800_pokey_step(&music_cursor, song_title);
    }
  }
}
