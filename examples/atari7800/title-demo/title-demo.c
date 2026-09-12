/* Title screen shakedown: renders astrowing.bas's real title-screen banner
 * (title_screen_conv.png, converted via
 * utils/png_banner_to_zones_header.py) as a set of zone-aligned static
 * Direct Mode objects, replicates its live palette hue-cycle animation and
 * SCORE/icon/DIFFICULTY/VERSION text overlay, and plays its real title theme
 * (Song_01_30hz.bin) through POKEY -- exercising the platform's
 * static-zone/memory-management path across every visible zone at once
 * (unlike astrowing.c, which only ever pins a single HUD zone), plus new
 * POKEY sound-chip support, before committing to the larger gameplay
 * systems (levels, collision) 7800port.md still has open.
 *
 * Values below (palette bytes, banner position, text strings/positions/
 * palettes, hue-cycle formula) are transcribed directly from astrowing.bas's
 * title_loop (palette table around line 271, banner at 341, text at
 * 344-351, cycle at 349-365), not guessed or approximated. In 7800basic,
 * plotchars/plotvalue's "y" is a coarse *line* (zone index), not a pixel
 * coordinate -- confirmed via the 7800basic Guide's wording ("the Y
 * coordinates in character functions refer to a coarse/line position, not
 * the screen Y coordinate") and cross-checked against plotbanner's own
 * wording ("y is the y *screen* coordinate"), which is why the banner's
 * y=46 is pixel-exact but every plotchars/plotvalue y below is multiplied
 * by the zone height to get a pixel Y.
 *
 * Deliberate simplifications from the reference:
 * - The score readout ("000000") is a static placeholder, not wired to
 *   real game state (no scoring system exists in this port yet) -- a
 *   natural follow-up once gameplay systems exist to back it.
 * - "DIFFICULTY" always shows "PRO" (astrowing.bas's switchleftb branch
 *   for "EASY" isn't wired to any input yet).
 */
#define ATARI7800_ZONE_HEIGHT 16
#define ATARI7800_SCENE_ZONE_BYTES 96 /* DIFFICULTY+PRO (13 glyphs) and
                                       * VERSION+20260126 (15 glyphs) each
                                       * share one zone row -- the default
                                       * 64-byte/12-glyph budget silently
                                       * truncates at glyph 12 (confirmed
                                       * hands-on); 96 bytes covers up to 18. */
#include <atari7800.h>
#include <atari7800_pokey.h>

#include "../assets/hud_font.h"
#include "../assets/song_title.h"
#include "../assets/title_screen.h"

static atari7800_scene_t scene;
static atari7800_glyph_run_entry_t score_glyphs[8];
static atari7800_glyph_run_t score_run;
static atari7800_glyph_run_entry_t icon_glyphs[8];
static atari7800_glyph_run_t icon_run;
static atari7800_glyph_run_entry_t difficulty_glyphs[16];
static atari7800_glyph_run_t difficulty_run;
static atari7800_glyph_run_entry_t pro_glyphs[8];
static atari7800_glyph_run_t pro_run;
static atari7800_glyph_run_entry_t version_glyphs[8];
static atari7800_glyph_run_t version_run;
static atari7800_glyph_run_entry_t date_glyphs[8];
static atari7800_glyph_run_t date_run;

int main(void) {
  uint8_t zone_index;
  uint8_t chunk_index;
  uint16_t frame_count = 0;
  const uint8_t *music_cursor = song_title;
  /* hud_font's own glyph_palette (5, "Spaceship") is baked into its
   * descriptor; the reference draws several other lines in different
   * palettes (astrowing.bas:344-350) -- same glyph data, different palette
   * slot, so just struct copies with one field overridden. */
  atari7800_font_descriptor_t hud_font_p0 = hud_font; /* Background/UI */
  atari7800_font_descriptor_t hud_font_p1 = hud_font; /* Player Bullets, green */
  atari7800_font_descriptor_t hud_font_p7 = hud_font; /* Title Screen, hue-cycled */
  hud_font_p0.glyph_palette = 0u;
  hud_font_p1.glyph_palette = 1u;
  hud_font_p7.glyph_palette = 7u;

  atari7800_pokey_init();
  atari7800_scene_init_160a(&scene, 0x00u /* BACKGRND, astrowing.bas:281 */);

  /* Palette table transcribed from astrowing.bas:271-278 (only the
   * palettes this screen actually uses). */
  atari7800_scene_set_palette(&scene, 0u,
      (atari7800_palette3_t){0x26, 0x24, 0x04}); /* Background/UI */
  atari7800_scene_set_palette(&scene, 1u,
      (atari7800_palette3_t){0xc2, 0xc6, 0xca}); /* Player Bullets (Green) */
  atari7800_scene_set_palette(&scene, 5u,
      (atari7800_palette3_t){0x34, 0x86, 0x0a}); /* Spaceship */
  atari7800_scene_set_palette(&scene, TITLE_SCREEN_DEFAULT_PALETTE,
      (atari7800_palette3_t){0xc8, 0x46, 0x1c}); /* Title Screen (Vibrant) */

  atari7800_scene_begin_frame(&scene);

  /* Banner: astrowing.bas:341's `plotbanner title_screen_conv 7 0 46` --
   * title_screen.h was generated with --y-offset 46, so TITLE_SCREEN_
   * FIRST_ZONE (2) is already the real on-screen starting zone; each slice
   * is still drawn zone-aligned (shift=0), just remapped row-by-row at
   * conversion time (see png_banner_to_zones_header.py). */
  for (zone_index = 0; zone_index < TITLE_SCREEN_NUM_ZONES; ++zone_index) {
    uint8_t y = (uint8_t)((TITLE_SCREEN_FIRST_ZONE + zone_index) *
                           TITLE_SCREEN_ZONE_HEIGHT);
    for (chunk_index = 0; chunk_index < TITLE_SCREEN_NUM_CHUNKS; ++chunk_index) {
      atari7800_scene_draw_sprite(&scene,
          &title_screen_zones[zone_index][chunk_index],
          title_screen_chunk_x_offsets[chunk_index], y);
    }
    atari7800_scene_set_zone_static(&scene, y, 1u);
  }

  /* SCORE: astrowing.bas:704's `plotvalue unified_font 0 score0 6 56 0` --
   * palette 0, x=56, line 0. No scoring system exists in this port yet, so
   * this is a static "000000" placeholder rather than a live BCD readout. */
  atari7800_build_glyph_run(&hud_font_p0, "000000", score_glyphs, 8u,
      &score_run);
  atari7800_scene_draw_glyph_run(&scene, &hud_font_p0, 56u, 0u, &score_run);
  atari7800_scene_set_zone_static(&scene, 0u, 1u);

  /* Icon row: astrowing.bas:346's `plotchars '*+-/<' 7 60 1` -- these are
   * literal glyphs from unified_font.png's own character set (the font's
   * alphachars table, astrowing.bas:59, assigns '*','+','-','/','<' to its
   * 5 special icon tiles), drawn in palette 7 so they hue-cycle with the
   * banner -- not separate sprites. */
  atari7800_build_glyph_run(&hud_font_p7, "*+-/<", icon_glyphs, 8u,
      &icon_run);
  atari7800_scene_draw_glyph_run(&scene, &hud_font_p7, 60u, 16u, &icon_run);
  atari7800_scene_set_zone_static(&scene, 16u, 1u);

  /* DIFFICULTY + PRO, one line: astrowing.bas:350-351's
   * `plotchars 'DIFFICULTY' 1 20 9` + `plotchars 'PRO ' 5 108 9` -- same
   * zone (line 9 -> pixel 144), different x/palette. */
  atari7800_build_glyph_run(&hud_font_p1, "DIFFICULTY", difficulty_glyphs,
      16u, &difficulty_run);
  atari7800_scene_draw_glyph_run(&scene, &hud_font_p1, 20u, 144u,
      &difficulty_run);
  atari7800_build_glyph_run(&hud_font, "PRO", pro_glyphs, 8u, &pro_run);
  atari7800_scene_draw_glyph_run(&scene, &hud_font, 108u, 144u, &pro_run);
  atari7800_scene_set_zone_static(&scene, 144u, 1u);

  /* VERSION + date, one line: astrowing.bas:345,347's
   * `plotchars 'VERSION' 1 12 11` + `plotchars '20260126' 1 84 11` -- same
   * zone (line 11 -> pixel 176), both palette 1. */
  atari7800_build_glyph_run(&hud_font_p1, "VERSION", version_glyphs, 8u,
      &version_run);
  atari7800_scene_draw_glyph_run(&scene, &hud_font_p1, 12u, 176u,
      &version_run);
  atari7800_build_glyph_run(&hud_font_p1, "20260126", date_glyphs, 8u,
      &date_run);
  atari7800_scene_draw_glyph_run(&scene, &hud_font_p1, 84u, 176u, &date_run);
  atari7800_scene_set_zone_static(&scene, 176u, 1u);

  atari7800_scene_end_frame(&scene);

  for (;;) {
    atari7800_wait_vblank();
    ++frame_count;

    /* Song data is authored at 30Hz (half the NTSC frame rate) -- see
     * atari7800_pokey_step's doc comment. */
    if ((frame_count & 1u) == 0u) {
      atari7800_pokey_step(&music_cursor, song_title);
    }

    /* Palette-7 hue-cycle animation, transcribed directly from
     * astrowing.bas:349-365 (temp_acc/temp_v naming kept to make the
     * correspondence obvious). Only ever writes palette registers -- the
     * banner's own zones/objects, pinned static above, are never touched
     * again after the initial draw. */
    {
      uint8_t temp_acc = (uint8_t)(frame_count >> 2); /* frame / 4 */
      uint8_t c1 = (uint8_t)(((12u + temp_acc) & 15u) << 4 | 8u);
      uint8_t c2 = (uint8_t)(((4u + temp_acc) & 15u) << 4 | 6u);
      uint8_t c3 = (uint8_t)(((1u + temp_acc) & 15u) << 4 | 12u);
      atari7800_scene_set_palette(&scene, TITLE_SCREEN_DEFAULT_PALETTE,
          (atari7800_palette3_t){c1, c2, c3});
    }
  }
}
