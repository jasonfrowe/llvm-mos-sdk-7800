/* astrowing.bas (the reference 7800basic source being ported) uses
 * "set zoneheight 16" -- 16-line zones halve the zone count (28->14),
 * cutting per-zone overhead, and let a full 16-line-tall sprite (the
 * player ship) live in a single zone as one real object. */
#define ATARI7800_ZONE_HEIGHT 16
/* DIFFICULTY+PRO and VERSION+date (title screen) each need more than the
 * default 64-byte/12-glyph zone budget -- see title_screen_loop below and
 * 7800port.md #14 for how this was confirmed, not assumed. */
#define ATARI7800_SCENE_ZONE_BYTES 96
#include <atari7800.h>
#include <atari7800_pokey.h>
#include <mapper.h>

/* Include packed spaceship frames */
#include "assets/spaceship.h"

/* Include HUD font. This example lives under examples/atari7800-supergame/
 * (a different platform's example tree from these shared assets, which
 * stay under examples/atari7800/assets/ since they're also used by
 * title-demo.c, still built against the flat atari7800 platform). */
#include "../../atari7800/assets/fighter.sprite.h"
#include "../../atari7800/assets/hud_font.h"

/* Title screen banner + music, shared with title-demo.c (see 7800port.md
 * #13-#19 for how these were derived/verified) rather than duplicated. */
#include "../../atari7800/assets/song_title.h"
#include "../../atari7800/assets/title_screen.h"
#include "assets/song_level1.h"

/* Aligned 16-bit sine/cosine tables for ship physics */
static const int16_t sin_table[16] = {
  0, 8, 12, 16, 16, 16, 12, 8, 0, -8, -12, -16, -16, -16, -12, -8
};

static const int16_t cos_table[16] = {
  24, 24, 16, 8, 0, -8, -16, -24, -24, -24, -16, -8, 0, 8, 16, 24
};

/* Page-aligned 1-pixel star sprite with a single reference point, positioned
 * anywhere in its 16-line zone via draw_sprite_fine's y&15 shift trick (see
 * fighter_sprite_data's comment in fighter.sprite.h for the underlying
 * "vertically flipped scanline countdown" mechanism this relies on -- the
 * same relationship generalized from the platform's old, broken 8-line
 * zones: pixel at the LAST page, shift range covering the full zone
 * height). A single Direct Mode object's render window always spans the
 * *zone's* full declared height (16 pages here) regardless of the object's
 * own declared height_lines, no matter which shift is used -- this used to
 * be two separate 8-line-window references (one per zone half) to work
 * around a since-fixed platform bug where zones were secretly only 8 lines
 * tall; with real 16-line zones, both references' windows always spanned
 * both pixels at once, doubling every star. Needs pages 0-30 (shift ranges
 * 0-15, window is [shift, shift+15], so the pixel at page 15 must stay
 * reachable up to shift=15 i.e. window [15,30]) -- pages 16-30 are explicit
 * trailing padding, not reachable ROM garbage.
 *
 * Shares its 31-page block with bullet_sprite (below) rather than each
 * getting an independent 7936-byte array: both are single-byte-wide
 * (4px) MARIA_STRIDED assets needing the same 31-page fine-Y envelope,
 * so a second, byte-1 "column" in the same pages costs nothing extra --
 * same technique as spaceship_data's 16 frames or title_screen_data's
 * chunks sharing one block, just with 2 tiny sprites instead of many
 * animation frames. This one change was needed to fit the level-1 music
 * (song_level1) within 48K's actual usable ROM budget -- see
 * 7800port.md #22. */
static const uint8_t misc_sprite_data[31 * 256] __attribute__((aligned(256)))
    __attribute__((section(".cart_rom_fixed_hi_extra.rodata"))) = {
  [15 * 256 + 0] = 0x40, /* star, page 15: pixel */
  [12 * 256 + 1] = 0x14, /* bullet, page 12: row 3 */
  [13 * 256 + 1] = 0x14, /* bullet, page 13: row 2 */
  [14 * 256 + 1] = 0x14, /* bullet, page 14: row 1 */
  [15 * 256 + 1] = 0x14, /* bullet, page 15: row 0 (top) */
};

static const atari7800_sprite_asset_t star_sprite = {
  .data = &misc_sprite_data[0],
  .width_bytes = 1u,
  .height_lines = 8u,
  .mode = 0x40u,
  .palette = 4u,
  .width_twos_comp = 0x1fu,
  .data_layout = ATARI7800_SPRITE_LAYOUT_MARIA_STRIDED
};

/* astrowing.bas's bullet_conv.png (4x16, indexed): real ink is only a
 * 2px-wide, 4-row mark at the top (rows 0-3, packed byte 0x14 --
 * pack_160a([0,1,1,0])), the rest of the 16-line canvas is blank.
 * Palette 1 ("Player Bullets"), matching `plotsprite bullet_conv 1 ...`. */
static const atari7800_sprite_asset_t bullet_sprite = {
  .data = &misc_sprite_data[1],
  .width_bytes = 1u,
  .height_lines = 8u,
  .mode = 0x40u,
  .palette = 1u,
  .width_twos_comp = 0x1fu,
  .data_layout = ATARI7800_SPRITE_LAYOUT_MARIA_STRIDED
};

/* Starfield stars. Struct-of-arrays, not array-of-structs, per LLVM-MOS's
 * own optimization guide (https://llvm-mos.org/wiki/Optimization_guide):
 * indexing a 1-byte array is a plain offset, while indexing an
 * array-of-3-byte-structs needs a multiply first. Also dropped the
 * original's third field (per-star "color"): it was write-only -- set in
 * init_stars but never actually read anywhere, since star_sprite always
 * draws with palette 4 regardless, and cycle_stars cycles that palette's
 * own colors globally rather than per-star. */
static uint8_t star_x[4];
static uint8_t star_y[4];
static uint16_t frame_count = 0;

/* Player spaceship state */
static int16_t player_vx = 0;
static int16_t player_vy = 0;
static int16_t sub_x = 0;
static int16_t sub_y = 0;
static uint8_t angle = 0;
static uint8_t rot_timer = 0;

/* Scrolling delta globals */
static int8_t scroll_x = 0;
static int8_t scroll_y = 0;

/* Enemy state (single enemy for now; the reference 7800basic source pools
 * up to 4 with randomized spawns, deferred until this pass proves out).
 * Reuses fighter_sprite (see fighter.sprite.h) exactly as the original
 * game's regular enemies do: the player's own (non-rotating) ship shape,
 * recolored via the "Enemy" palette (index 3, matching P3C1-3 in
 * astrowing.bas) rather than needing new sprite art. Y is fixed:
 * fighter_sprite_data has no fine-Y padding (unlike star_sprite_data), so
 * only zone-aligned Y positions are safe without extending that asset. */
static uint8_t enemy_x = 130;
static const uint8_t enemy_y = 32;
static int8_t enemy_dx = -1;

/* Player bullets (astrowing.bas:78/818-880's bul_x/bul_y/bul_vx/bul_vy/
 * blife arrays, a pool of 4 -- "Screen Space" bullets per the reference's
 * own comment: spawned near the ship's fixed screen position and moved by
 * a constant per-frame velocity, independent of the starfield's own
 * scroll_x/scroll_y (the ship itself never actually moves on screen
 * either, see draw_sprite_fine(&ship, 72, 80) below -- movement is the
 * *world* scrolling under a stationary ship, and bullets fly through that
 * same screen-space view). blife is a countdown (0 = free slot); bul_vx/
 * bul_vy are read directly from sin_table/cos_table[angle] like the
 * player's own thrust vector (update_player_input), so a bullet always
 * fires in the direction the ship is facing -- matching the reference's
 * `bul_vx[iter] = sin_table[angle]` / `bul_vy[iter] = -cos_table[angle]`
 * in direction, but scaled down by 4 in fire_bullet() below: this
 * platform's existing table has magnitudes up to 24 (vs. the reference's
 * own, differently-scaled table's 0-6), and at full scale a bullet
 * crosses the whole screen in under 10 frames and dies off-edge almost
 * immediately (confirmed hands-on: invisible in every post-fire
 * screenshot tried). /4 lands close to the reference's own ~6px/frame
 * feel, reusing the one table already in
 * this file rather than adding a second, differently-scaled one purely
 * for bullets. */
static uint8_t bul_x[4];
static uint8_t bul_y[4];
static int8_t bul_vx[4];
static int8_t bul_vy[4];
static uint8_t blife[4];
static uint8_t bcooldown = 0;

/* Scene context global */
static atari7800_scene_t scene;

/* Stars, ship, and enemy are all drawn with atari7800_scene_draw_sprite
 * (begin_frame/end_frame each fully re-derive their zones' object lists
 * every frame) rather than the persistent-object API
 * (atari7800_scene_sprite): they cross zone boundaries often enough while
 * scrolling that the persistent model's ever-growing per-zone slot usage
 * (see the caveat on atari7800_scene_sprite in atari7800.h) exhausts a
 * zone's slot budget after a bit of play, permanently hiding whatever
 * hit ATARI7800_ERR_BUDGET_FULL. The HUD is the one thing still using a
 * persistent zone below, via atari7800_scene_set_zone_static -- it never
 * moves, so it never crosses a zone boundary and the caveat doesn't apply. */

/* HUD text is resolved into a glyph run once at startup instead of being
 * re-parsed every frame; the hot loop just blits the pre-resolved glyphs. */
static atari7800_glyph_run_entry_t hud_glyph_entries[16];
static atari7800_glyph_run_t hud_glyph_run;

static void init_stars(void) {
  uint8_t i;
  for (i = 0; i < 4; ++i) {
    star_x[i] = (uint8_t)((frame_count + i * 37) % 160);
    star_y[i] = (uint8_t)((frame_count + i * 59) % 192);
  }
}

/* cycle_state used to be its own persistent global, incremented mod 3 --
 * but it's a pure function of frame_count (already ticking every frame
 * regardless), so it doesn't need its own permanent zp/bss byte. Dropped
 * as a genuine simplification (found while chasing the zp overflow that
 * examples/atari7800-supergame/CMakeLists.txt's -mlto-zp=95 actually
 * fixes -- LTO's zp promotion turned out to be adaptive enough that
 * trimming one named global alone didn't reduce total zp usage at all,
 * since it just promoted a different temporary into the freed byte). */
static void cycle_stars(void) {
  uint8_t cycle_state;

  if ((frame_count & 7) != 0) return;

  cycle_state = (uint8_t)((frame_count >> 3) % 3);

  if (cycle_state == 0) {
    atari7800_set_palette3(4, (atari7800_palette3_t){0x08, 0x0c, 0x0f});
  } else if (cycle_state == 1) {
    atari7800_set_palette3(4, (atari7800_palette3_t){0x0c, 0x0f, 0x08});
  } else {
    atari7800_set_palette3(4, (atari7800_palette3_t){0x0f, 0x08, 0x0c});
  }
}

/* Throttled drift, bouncing off the screen edges. Matches the frame-mask
 * throttle technique astrowing.bas uses for enemy movement speed
 * (`temp_v = frame & enemy_move_mask`), simplified to one enemy. */
static void update_enemy(void) {
  if ((frame_count & 3u) != 0) return;

  enemy_x = (uint8_t)(enemy_x + enemy_dx);
  if (enemy_x <= 16u || enemy_x >= 144u) {
    enemy_dx = (int8_t)-enemy_dx;
  }
}

/* astrowing.bas:799-819's update_bullets: per-slot move, then off-screen
 * and lifetime expiry. Bounds checks transcribed exactly, including the
 * reference's own unsigned-wraparound idiom for "off the left edge"
 * (`if bul_x[iter] > 170 then if bul_x[iter] < 240 then blife[iter] = 0`
 * -- a small negative bul_x wraps to just under 256, so ">170 and <240"
 * catches that wrapped range without needing a signed coordinate type). */
static void update_bullets(void) {
  uint8_t i;
  for (i = 0; i < 4; ++i) {
    if (blife[i] == 0) continue;

    bul_x[i] = (uint8_t)(bul_x[i] + bul_vx[i]);
    if (bul_x[i] > 170u && bul_x[i] < 240u) {
      blife[i] = 0;
      continue;
    }

    bul_y[i] = (uint8_t)(bul_y[i] + bul_vy[i]);
    if (bul_y[i] > 200u) {
      blife[i] = 0;
      continue;
    }

    --blife[i];
  }
}

/* astrowing.bas:824-880's fire_bullet/spawn_bullet, merged into one
 * function since this port has no separate gosub/return control flow to
 * preserve. Spawns from the ship's own fixed screen position (72, 80;
 * matches draw_sprite_fine(&ship, 72, 80) below -- see the bul_* fields'
 * own comment for why bullets are screen-space, not world-space) offset
 * by +6,+6 to originate from roughly the ship sprite's center, same as
 * the reference's `bul_x[iter] = px + 6`. Cooldown thresholds
 * (astrowing.bas:881-883) are given per current_level, which this port
 * doesn't have yet -- using the level-1 value (25 frames) unconditionally
 * until levels exist. */
static void fire_bullet(void) {
  uint8_t i;
  for (i = 0; i < 4; ++i) {
    if (blife[i] == 0) break;
  }
  if (i == 4) return; /* no free slot */

  blife[i] = 60;
  bul_x[i] = (uint8_t)(72 + 6);
  bul_y[i] = (uint8_t)(80 + 6);
  bul_vx[i] = (int8_t)(sin_table[angle] / 4);
  bul_vy[i] = (int8_t)(-cos_table[angle] / 4);

  bcooldown = 25;
}

static void update_player_input(void) {
  if (rot_timer > 0) {
    rot_timer--;
  } else {
    if (!(ATARI7800_SWCHA & ATARI7800_SWCHA_LEFT)) {
      angle = (uint8_t)(angle - 1u) & 0x0fu;
      rot_timer = 4;
    } else if (!(ATARI7800_SWCHA & ATARI7800_SWCHA_RIGHT)) {
      angle = (uint8_t)(angle + 1u) & 0x0fu;
      rot_timer = 4;
    }
  }

  if (!(ATARI7800_SWCHA & ATARI7800_SWCHA_UP)) {
    player_vx += sin_table[angle];
    player_vy -= cos_table[angle];

    int16_t max_speed = 480;
    if (player_vx > max_speed) player_vx = max_speed;
    if (player_vx < -max_speed) player_vx = -max_speed;
    if (player_vy > max_speed) player_vy = max_speed;
    if (player_vy < -max_speed) player_vy = -max_speed;
  }
}

static void apply_friction(void) {
  if (player_vx > 8) player_vx -= 4;
  else if (player_vx < -8) player_vx += 4;
  else player_vx = 0;

  if (player_vy > 8) player_vy -= 4;
  else if (player_vy < -8) player_vy += 4;
  else player_vy = 0;
}

static void update_scrolling(void) {
  scroll_x = 0;
  scroll_y = 0;

  sub_x += player_vx;
  if (sub_x >= 512) {
    scroll_x = 2;
    sub_x -= 512;
  } else if (sub_x >= 256) {
    scroll_x = 1;
    sub_x -= 256;
  } else if (sub_x <= -512) {
    scroll_x = -2;
    sub_x += 512;
  } else if (sub_x <= -256) {
    scroll_x = -1;
    sub_x += 256;
  }

  sub_y += player_vy;
  if (sub_y >= 512) {
    scroll_y = 2;
    sub_y -= 512;
  } else if (sub_y >= 256) {
    scroll_y = 1;
    sub_y -= 256;
  } else if (sub_y <= -512) {
    scroll_y = -2;
    sub_y += 512;
  } else if (sub_y <= -256) {
    scroll_y = -1;
    sub_y += 256;
  }
}

static void shift_stars(void) {
  uint8_t i;
  for (i = 0; i < 4; ++i) {
    star_x[i] = (uint8_t)(star_x[i] - scroll_x);
    if (star_x[i] > 160 && star_x[i] < 240) star_x[i] = 0;
    else if (star_x[i] >= 240) star_x[i] = 159;

    star_y[i] = (uint8_t)(star_y[i] - scroll_y);
    if (star_y[i] > 192 && star_y[i] < 240) star_y[i] = 0;
    else if (star_y[i] >= 240) star_y[i] = 191;
  }
}

/* Draws a sprite with pixel-fine vertical positioning inside its zone.
 * Shift range matches the full 16-line zone height (y&15, not y&7) -- see
 * star_sprite_data's comment above for why. */
static void draw_sprite_fine(const atari7800_sprite_asset_t *asset, uint8_t x, uint8_t y) {
  uint8_t y_offset = y & 15;
  atari7800_sprite_asset_t shifted_asset = *asset;
  shifted_asset.data = (const uint8_t *)((uintptr_t)shifted_asset.data + ((uint16_t)y_offset << 8));

  (void)atari7800_scene_draw_sprite(&scene, &shifted_asset, x, y);
}

/* ---- Title screen (astrowing.bas:326-397's title_loop, ported) ----
 * Runs to completion before gameplay's own atari7800_scene_init_160a call
 * below re-initializes the scene from scratch, so nothing here needs to be
 * torn down explicitly -- see title-demo.c (examples/atari7800/title-demo/)
 * for this same banner/text/music layout as a standalone, more heavily
 * commented reference.
 *
 * The one genuinely one-shot piece of this program -- rendered once at
 * boot, never touched again -- so it and its two shared asset arrays
 * (title_screen.h/song_title.h, tagged via ATARI7800_TITLE_BANK) are the
 * first real content moved into a SuperGame switchable bank (bank 0),
 * freeing that ROM budget for the gameplay content still to come. See
 * 7800port.md #25. */
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

/* .cart_rom_bank_0.text, not the bare .cart_rom_bank_0 that
 * ATARI7800_TITLE_BANK's data arrays use below (title_screen.h/
 * song_title.h): mos-platform/atari7800-supergame/link.ld's
 * `*(.cart_rom_bank_0 .cart_rom_bank_0.*)` wildcard places both into the
 * same output region, but code and const data can't share one exact
 * section name (a real "section type conflict" from the compiler, found
 * by just trying it -- not a hypothetical).
 *
 * noinline matters even though nothing calls this directly today except
 * through banked_call_8000 below (an opaque-asm indirect call LTO can't
 * see through, so the hazard doesn't fire yet): it's cheap insurance
 * against a future stray direct call letting LTO inline this body into
 * fixed-region code that would then read title_screen_data0/1 at their
 * bank-0-only addresses without a bank switch having happened -- a
 * silent wrong-bank-read bug, not just bloat. Confirmed cheap: removing
 * it to chase an unrelated zp overflow (see cycle_stars's own comment)
 * made no difference to zp usage at all -- non-inlined-function locals
 * don't compete for this platform's named-global zp budget the way a
 * persistent global does. */
__attribute__((noinline, section(".cart_rom_bank_0.text")))
static void title_screen_loop(void) {
  uint8_t zone_index;
  uint8_t chunk_index;
  uint16_t title_frame = 0;
  const uint8_t *music_cursor = song_title;
  atari7800_font_descriptor_t hud_font_p0 = hud_font; /* Background/UI */
  atari7800_font_descriptor_t hud_font_p1 = hud_font; /* Player Bullets, green */
  atari7800_font_descriptor_t hud_font_p7 = hud_font; /* Title Screen, hue-cycled */
  hud_font_p0.glyph_palette = 0u;
  hud_font_p1.glyph_palette = 1u;
  hud_font_p7.glyph_palette = 7u;

  atari7800_pokey_init();
  atari7800_scene_init_160a(&scene, 0x00u /* BACKGRND, astrowing.bas:281 */);

  atari7800_scene_set_palette(&scene, 0u,
      (atari7800_palette3_t){0x26, 0x24, 0x04}); /* Background/UI */
  atari7800_scene_set_palette(&scene, 1u,
      (atari7800_palette3_t){0xc2, 0xc6, 0xca}); /* Player Bullets (Green) */
  atari7800_scene_set_palette(&scene, 5u,
      (atari7800_palette3_t){0x34, 0x86, 0x0a}); /* Spaceship */
  atari7800_scene_set_palette(&scene, TITLE_SCREEN_DEFAULT_PALETTE,
      (atari7800_palette3_t){0xc8, 0x46, 0x1c}); /* Title Screen (Vibrant) */

  atari7800_scene_begin_frame(&scene);

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

  atari7800_build_glyph_run(&hud_font_p0, "000000", score_glyphs, 8u,
      &score_run);
  atari7800_scene_draw_glyph_run(&scene, &hud_font_p0, 56u, 0u, &score_run);
  atari7800_scene_set_zone_static(&scene, 0u, 1u);

  atari7800_build_glyph_run(&hud_font_p7, "*+-/<", icon_glyphs, 8u,
      &icon_run);
  atari7800_scene_draw_glyph_run(&scene, &hud_font_p7, 60u, 16u, &icon_run);
  atari7800_scene_set_zone_static(&scene, 16u, 1u);

  atari7800_build_glyph_run(&hud_font_p1, "DIFFICULTY", difficulty_glyphs,
      16u, &difficulty_run);
  atari7800_scene_draw_glyph_run(&scene, &hud_font_p1, 20u, 144u,
      &difficulty_run);
  atari7800_build_glyph_run(&hud_font, "PRO", pro_glyphs, 8u, &pro_run);
  atari7800_scene_draw_glyph_run(&scene, &hud_font, 108u, 144u, &pro_run);
  atari7800_scene_set_zone_static(&scene, 144u, 1u);

  atari7800_build_glyph_run(&hud_font_p1, "VERSION", version_glyphs, 8u,
      &version_run);
  atari7800_scene_draw_glyph_run(&scene, &hud_font_p1, 12u, 176u,
      &version_run);
  atari7800_build_glyph_run(&hud_font_p1, "20260126", date_glyphs, 8u,
      &date_run);
  atari7800_scene_draw_glyph_run(&scene, &hud_font_p1, 84u, 176u, &date_run);
  atari7800_scene_set_zone_static(&scene, 176u, 1u);

  atari7800_scene_end_frame(&scene);

  /* astrowing.bas:330 `if joy0fire1 || switchreset then goto
   * title_release_wait` -- exits on fire, then debounces until release
   * (title_release_wait, astrowing.bas:399-404) so the same press doesn't
   * immediately register as an in-game action (e.g. firing a bullet) the
   * instant gameplay starts. switchreset (the console RESET switch) isn't
   * wired up on this platform yet -- fire is the only way in for now. */
  for (;;) {
    atari7800_wait_vblank();
    ++title_frame;

    if ((title_frame & 1u) == 0u) {
      atari7800_pokey_step(&music_cursor, song_title);
    }

    {
      uint8_t temp_acc = (uint8_t)(title_frame >> 2);
      uint8_t c1 = (uint8_t)(((12u + temp_acc) & 15u) << 4 | 8u);
      uint8_t c2 = (uint8_t)(((4u + temp_acc) & 15u) << 4 | 6u);
      uint8_t c3 = (uint8_t)(((1u + temp_acc) & 15u) << 4 | 12u);
      atari7800_scene_set_palette(&scene, TITLE_SCREEN_DEFAULT_PALETTE,
          (atari7800_palette3_t){c1, c2, c3});
    }

    if (ATARI7800_JOY0FIRE1()) {
      break;
    }
  }
  while (ATARI7800_JOY0FIRE1()) {
    atari7800_wait_vblank();
  }
}

int main(void) {
  uint8_t i;
  const uint8_t *music_cursor = song_level1;

  /* title_screen_loop lives in switchable bank 0 (see its own comment
   * above) -- main() itself runs from the fixed region, so reaching it
   * needs banked_call_8000 (mapper.h) rather than a plain call. */
  banked_call_8000(0, title_screen_loop);

  /* atari7800_pokey_init() re-clears every POKEY register (AUDC1-4
   * included), silencing whatever note the title music was on when the
   * player pressed fire -- title_screen_loop's own atari7800_pokey_step
   * calls stop the instant its loop exits, but POKEY itself keeps
   * generating whatever tone AUDF/AUDC were last set to until something
   * tells it otherwise (confirmed hands-on: without this, gameplay starts
   * with the title screen's last note still ringing). astrowing.bas
   * doesn't hit this itself since 7800basic's PlayMusic is switching which
   * song feeds the interpreter, never fully stopping and restarting
   * POKEY -- this port's title screen and gameplay are more separated
   * (different loops, different sync strategy) so the silence has to be
   * explicit here instead. */
  atari7800_pokey_init();

  /* Initialize system with background color */
  atari7800_scene_init_160a(&scene, ATARI7800_BG_DARKGRAY);

  /* Set Palettes */
  atari7800_scene_set_palette(&scene, 0, (atari7800_palette3_t){0x26, 0x24, 0x04}); /* UI */
  atari7800_scene_set_palette(&scene, 1, (atari7800_palette3_t){0xc2, 0xc6, 0xca}); /* Bullets */
  atari7800_scene_set_palette(&scene, 2, (atari7800_palette3_t){0x04, 0x08, 0x0c}); /* Asteroids */
  atari7800_scene_set_palette(&scene, 3, (atari7800_palette3_t){0xb4, 0x46, 0x1c}); /* Enemy */
  atari7800_scene_set_palette(&scene, 4, (atari7800_palette3_t){0x08, 0x0c, 0x0f}); /* Stars */
  atari7800_scene_set_palette(&scene, 5, (atari7800_palette3_t){0x34, 0x86, 0x0a}); /* Spaceship */

  init_stars();

  /* A non-OK status here means hud_glyph_entries is sized too small for the
   * HUD string -- a build-time bug, not a runtime budget issue. */
  (void)atari7800_build_glyph_run(&hud_font, "SHLD:100 L:3",
      hud_glyph_entries,
      (uint8_t)(sizeof(hud_glyph_entries) / sizeof(hud_glyph_entries[0])),
      &hud_glyph_run);

  /* HUD text never changes in this demo, so draw it once and pin its zone
   * static: begin_frame/end_frame then leave it alone every frame instead
   * of re-blitting unchanged glyphs. */
  atari7800_scene_begin_frame(&scene);
  (void)atari7800_scene_draw_glyph_run(&scene, &hud_font, 4, 8, &hud_glyph_run);
  atari7800_scene_set_zone_static(&scene, 8, 1);
  atari7800_scene_end_frame(&scene);

  /* NMI-driven sync (see atari7800.h): game logic below runs between
   * wait_active_start() and wait_display_safe(), overlapping MARIA's
   * active-display DMA time instead of being serialized before it the
   * way a fully-polled atari7800_wait_vblank()-at-the-top loop forces.
   * Only draw calls need to wait for the display to actually go quiet. */
  atari7800_scene_enable_nmi_sync(&scene);

  for (;;) {
    atari7800_scene_wait_active_start();
    frame_count++;

    /* No frame-budget marker here: writing ATARI7800_BACKGRND during
     * this phase is, by design, sometimes during active display (that's
     * the overlap this sync is for), and doing so repaints the live
     * picture instead of measuring anything -- this phase's cost can't
     * be visualized this way anymore. */
    update_player_input();
    apply_friction();
    update_scrolling();
    shift_stars();
    cycle_stars();
    update_enemy();

    /* astrowing.bas:560-562: `if bcooldown > 0 then bcooldown = bcooldown
     * - 1` then `if joy0fire1 && bcooldown = 0 then gosub fire_bullet`. */
    if (bcooldown > 0) --bcooldown;
    if (ATARI7800_JOY0FIRE1() && bcooldown == 0) fire_bullet();
    update_bullets();

    /* Level 1 music (astrowing.bas:3508-3510/3576-3578's PlayMusic branch
     * for current_level=1 -- Song_02_Data/Song_02_30hz.bin; this port has
     * no level progression yet, so it's the only gameplay track for now).
     * Same 30Hz-authored, every-other-frame cadence as the title screen's
     * own atari7800_pokey_step call. */
    if ((frame_count & 1u) == 0u) {
      atari7800_pokey_step(&music_cursor, song_level1);
    }

    atari7800_scene_wait_display_safe();

#ifdef ATARI7800_DEBUG_FRAME_BUDGET
    /* Safe to mark from here on: atari7800_scene_wait_display_safe() just
     * confirmed the display is quiet, so this and the restore below both
     * land in the actual blanking window. */
    ATARI7800_BACKGRND = 0x8fu; /* draw calls below */
#endif

    /* begin_frame resets every non-static zone's object list (the HUD
     * zone is pinned static and untouched); end_frame below then clears
     * any zone that goes from active to unused this frame. Stars/ship/
     * enemy all get fully re-derived from scratch this way every frame --
     * see the comment above their declarations for why the persistent-
     * object API isn't safe for them. */
    atari7800_scene_begin_frame(&scene);

    /* Render stars, clipping to gameplay area Y >= 16 to keep the HUD
     * zone clean. */
    for (i = 0; i < 4; ++i) {
      if (star_y[i] >= 16) {
        draw_sprite_fine(&star_sprite, star_x[i], star_y[i]);
      }
    }

    /* Draw player spaceship as a single real 16-line-tall object.
     * spaceship_frames[] is genuinely 16 lines of pixel data
     * (SPACESHIP_HEIGHT_LINES) -- the earlier two-stacked-8-line-object
     * workaround here was compensating for a since-fixed platform bug
     * (ATARI7800_ZONE_HEIGHT wasn't actually reaching the compiled scene
     * code -- see 7800port.md), not a real MARIA per-object height cap. */
    atari7800_sprite_asset_t ship = spaceship_frames[angle];
    ship.palette = 5; /* Use Palette 5 (Spaceship) */
    draw_sprite_fine(&ship, 72, 80);

    /* astrowing.bas:2445-2459's draw_player_bullets. */
    for (i = 0; i < 4; ++i) {
      if (blife[i] != 0) {
        draw_sprite_fine(&bullet_sprite, bul_x[i], bul_y[i]);
      }
    }

    /* Enemy: fighter_sprite has no fine-Y padding (unlike star_sprite_data
     * / spaceship_data), so it's drawn at a fixed, zone-aligned Y directly
     * rather than draw_sprite_fine's page-shift trick, which would read
     * past the asset's declared size for a non-zone-aligned Y. */
    (void)atari7800_scene_draw_sprite(&scene, &fighter_sprite, enemy_x, enemy_y);

    /* HUD text is pinned static residency (see setup above) -- nothing to
     * draw here every frame. */
    atari7800_scene_end_frame(&scene);

#ifdef ATARI7800_DEBUG_FRAME_BUDGET
    /* Marks the end of this frame's CPU work: restore the real background
     * color before the next atari7800_wait_vblank() call above. */
    ATARI7800_BACKGRND = ATARI7800_BG_DARKGRAY;
#endif
  }
}
