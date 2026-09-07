/* Title screen shakedown: renders astrowing.bas's real title-screen banner
 * (title_screen_conv.png, converted via
 * utils/png_banner_to_zones_header.py) as a set of zone-aligned static
 * Direct Mode objects, replicates its live palette hue-cycle animation and
 * VERSION/DIFFICULTY text overlay, and plays its real title theme
 * (Song_01_30hz.bin) through POKEY -- exercising the platform's
 * static-zone/memory-management path across every visible zone at once
 * (unlike astrowing.c, which only ever pins a single HUD zone), plus new
 * POKEY sound-chip support, before committing to the larger gameplay
 * systems (levels, collision) 7800port.md still has open.
 *
 * Values below (palette bytes, hue-cycle formula, text strings/positions)
 * are transcribed directly from astrowing.bas's title_loop (around line
 * 271 for the palette table, 335 for the banner, 341-347 for text, 349-365
 * for the cycle), not guessed or approximated.
 *
 * Deliberate simplifications from the reference:
 * - The banner is drawn at y=64 (zone-aligned), not the reference's y=46.
 *   A non-zone-aligned multi-slice banner needs cross-zone fine-Y
 *   splitting, which nothing in this SDK implements yet (see 7800port.md);
 *   y=64 was chosen because it centers the 96-line/6-zone banner exactly
 *   within this platform's 14-zone (224-line) total display space, close
 *   to the reference's own visual proportions.
 * - The score readout ("000000") is a static placeholder, not wired to
 *   real game state (no scoring system exists in this port yet), and the
 *   small icon row the reference draws above it is omitted entirely (needs
 *   sprite assets not yet examined) -- both are natural follow-ups once
 *   gameplay systems exist to back them.
 * - "DIFFICULTY PRO" and "VERSION  20260126" are each drawn as two
 *   separate zone rows instead of the reference's compact single-row
 *   layout: a zone's 64-byte object budget caps a single glyph run at 12
 *   characters, and both combined strings need more (13 and 15) -- see the
 *   comment at the draw calls below for how this was confirmed, not
 *   assumed.
 */
#define ATARI7800_ZONE_HEIGHT 16
#include <atari7800.h>
#include <atari7800_pokey.h>

#include "../assets/fighter.sprite.h" /* hud_font */
#include "assets/song_title.h"
#include "assets/title_screen.h"

#define BANNER_Y 64u /* zone 4 -- see the file comment for why not y=46 */

static atari7800_scene_t scene;
static atari7800_glyph_run_entry_t version_glyphs[8];
static atari7800_glyph_run_t version_run;
static atari7800_glyph_run_entry_t date_glyphs[8];
static atari7800_glyph_run_t date_run;
static atari7800_glyph_run_entry_t difficulty_glyphs[16];
static atari7800_glyph_run_t difficulty_run;
static atari7800_glyph_run_entry_t pro_glyphs[8];
static atari7800_glyph_run_t pro_run;
static atari7800_glyph_run_entry_t score_glyphs[8];
static atari7800_glyph_run_t score_run;

int main(void) {
  uint8_t zone_index;
  uint8_t chunk_index;
  uint16_t frame_count = 0;
  const uint8_t *music_cursor = song_title;
  /* hud_font's own glyph_palette (5) is baked into its descriptor; the
   * reference draws VERSION/DIFFICULTY in palette 1 ("Player Bullets",
   * green) instead -- same glyph data, different palette slot, so just a
   * struct copy with one field overridden rather than a new font asset. */
  atari7800_font_descriptor_t hud_font_green = hud_font;
  hud_font_green.glyph_palette = 1u;

  atari7800_scene_init_160a(&scene, 0x00u /* BACKGRND, astrowing.bas:281 */);

  /* Palette table transcribed from astrowing.bas:271-278 (only the
   * palettes this screen actually uses: 1 for VERSION/DIFFICULTY text, 5
   * for hud_font's own default-palette glyphs (the SCORE line here), 7 for
   * the title banner, whose base values also seed the hue-cycle below). */
  atari7800_scene_set_palette(&scene, 1u,
      (atari7800_palette3_t){0xc2, 0xc6, 0xca}); /* Player Bullets (Green) */
  atari7800_scene_set_palette(&scene, 5u,
      (atari7800_palette3_t){0x34, 0x86, 0x0a}); /* Spaceship */
  atari7800_scene_set_palette(&scene, TITLE_SCREEN_DEFAULT_PALETTE,
      (atari7800_palette3_t){0xc8, 0x46, 0x1c}); /* Title Screen (Vibrant) */

  atari7800_scene_begin_frame(&scene);

  for (zone_index = 0; zone_index < TITLE_SCREEN_NUM_ZONES; ++zone_index) {
    uint8_t y = (uint8_t)(BANNER_Y + zone_index * TITLE_SCREEN_ZONE_HEIGHT);
    for (chunk_index = 0; chunk_index < TITLE_SCREEN_NUM_CHUNKS; ++chunk_index) {
      atari7800_scene_draw_sprite(&scene,
          &title_screen_zones[zone_index][chunk_index],
          title_screen_chunk_x_offsets[chunk_index], y);
    }
    atari7800_scene_set_zone_static(&scene, y, 1u);
  }

  /* astrowing.bas draws "DIFFICULTY PRO" and "VERSION  20260126" as two
   * compact rows (astrowing.bas:343-348). Each single-zone glyph run here
   * is capped at 12 glyphs by the zone's 64-byte object budget (5-byte
   * header stride, terminator sharing the next slot's first 2 bytes --
   * see atari7800_scene_draw_glyph_run) -- confirmed hands-on: both
   * combined strings (13 and 15 non-space glyphs) silently truncated
   * exactly at the 12th glyph. Rather than abbreviate the text, split each
   * onto its own zone row instead, preserving exact wording at the cost
   * of the reference's compact 2-row layout -- zones 10-13 are free
   * (below the banner, which occupies 4-9). */
  atari7800_build_glyph_run(&hud_font_green, "DIFFICULTY",
      difficulty_glyphs, 16u, &difficulty_run);
  atari7800_scene_draw_glyph_run(&scene, &hud_font_green, 20u, 160u,
      &difficulty_run); /* zone 10 */
  atari7800_scene_set_zone_static(&scene, 160u, 1u);

  atari7800_build_glyph_run(&hud_font_green, "PRO", pro_glyphs, 8u, &pro_run);
  atari7800_scene_draw_glyph_run(&scene, &hud_font_green, 20u, 176u,
      &pro_run); /* zone 11 */
  atari7800_scene_set_zone_static(&scene, 176u, 1u);

  atari7800_build_glyph_run(&hud_font_green, "VERSION", version_glyphs, 20u,
      &version_run);
  atari7800_scene_draw_glyph_run(&scene, &hud_font_green, 12u, 192u,
      &version_run); /* zone 12 */
  atari7800_scene_set_zone_static(&scene, 192u, 1u);

  atari7800_build_glyph_run(&hud_font_green, "20260126", date_glyphs, 20u,
      &date_run);
  atari7800_scene_draw_glyph_run(&scene, &hud_font_green, 12u, 208u,
      &date_run); /* zone 13 */
  atari7800_scene_set_zone_static(&scene, 208u, 1u);

  /* Static "000000" placeholder for astrowing.bas:350's live score
   * readout -- no scoring system exists in this port yet (see file
   * comment). Uses hud_font's own default palette (5). */
  atari7800_build_glyph_run(&hud_font, "000000", score_glyphs, 8u,
      &score_run);
  atari7800_scene_draw_glyph_run(&scene, &hud_font, 56u, 16u, &score_run);
  atari7800_scene_set_zone_static(&scene, 16u, 1u);

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
