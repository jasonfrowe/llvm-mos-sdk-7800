/* astrowing.bas (the reference 7800basic source being ported) uses
 * "set zoneheight 16" -- 16-line zones halve the zone count (28->14),
 * cutting per-zone overhead, and let a full 16-line-tall sprite (the
 * player ship) live in a single zone as one real object. */
#define ATARI7800_ZONE_HEIGHT 16
#include <atari7800.h>

/* Include packed spaceship frames */
#include "assets/spaceship.h"

/* Include HUD font */
#include "../assets/fighter.sprite.h"

/* Aligned 16-bit sine/cosine tables for ship physics */
static const int16_t sin_table[16] = {
  0, 8, 12, 16, 16, 16, 12, 8, 0, -8, -12, -16, -16, -16, -12, -8
};

static const int16_t cos_table[16] = {
  24, 24, 16, 8, 0, -8, -16, -24, -24, -24, -16, -8, 0, 8, 16, 24
};

/* Page-aligned 1-pixel star sprite, with two reference points (one per
 * 8-line half of the 16-line zone) so draw_sprite_fine's y&7 shift trick
 * can position it anywhere. This predates a since-fixed platform bug
 * (see 7800port.md, "Fix ATARI7800_ZONE_HEIGHT actually taking effect") --
 * a single reference with a y&15 shift would likely work fine now that
 * zones are genuinely 16 lines tall, but this two-reference form is
 * proven correct and hasn't been revisited yet:
 *   - pages 0-6: leading padding, page 7: pixel (bottom-half reference)
 *   - pages 8-14: padding (doubles as the top half's leading padding),
 *     page 15: pixel (top-half reference), pages 16-22: trailing padding
 * See star_sprite (bottom half, y&15 < 8) and star_sprite_upper (top
 * half, y&15 >= 8) below; the call site picks between them. */
static const uint8_t star_sprite_data[] __attribute__((aligned(256))) = {
  [1792] = 0x40, /* Page 7: bottom-half pixel */
  [3840] = 0x40, /* Page 15: top-half pixel */
  [5887] = 0x00  /* Pad up to 23 pages (5888 bytes) */
};

static const atari7800_sprite_asset_t star_sprite = {
  .data = star_sprite_data,
  .width_bytes = 1u,
  .height_lines = 8u,
  .mode = 0x40u,
  .palette = 4u,
  .width_twos_comp = 0x1fu,
  .data_layout = ATARI7800_SPRITE_LAYOUT_MARIA_STRIDED
};

static const atari7800_sprite_asset_t star_sprite_upper = {
  .data = &star_sprite_data[8 * 256],
  .width_bytes = 1u,
  .height_lines = 8u,
  .mode = 0x40u,
  .palette = 4u,
  .width_twos_comp = 0x1fu,
  .data_layout = ATARI7800_SPRITE_LAYOUT_MARIA_STRIDED
};

/* Starfield stars */
typedef struct {
  uint8_t x;
  uint8_t y;
  uint8_t color;
} star_t;

static star_t stars[4];
static uint8_t cycle_state = 0;
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

/* Saturating counter of dropped draws (status != ATARI7800_OK) from any
 * scene zone. Harmless for stars (a missed star this frame is invisible),
 * but a nonzero count from the ship or HUD draws would mean a zone is
 * genuinely over budget and needs attention. Inspect via the emulator's
 * memory/watch view; not surfaced on screen. */
static uint8_t budget_drops = 0;

static void note_status(uint8_t status) {
  if (status != ATARI7800_OK && budget_drops < 0xffu) {
    ++budget_drops;
  }
}

void init_stars(void) {
  uint8_t i;
  for (i = 0; i < 4; ++i) {
    stars[i].x = (uint8_t)((frame_count + i * 37) % 160);
    stars[i].y = (uint8_t)((frame_count + i * 59) % 192);
    stars[i].color = (uint8_t)((frame_count + i) & 3);
    if (stars[i].color == 0) stars[i].color = 1;
  }
}

void cycle_stars(void) {
  if ((frame_count & 7) != 0) return;

  cycle_state++;
  if (cycle_state > 2) cycle_state = 0;

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
void update_enemy(void) {
  if ((frame_count & 3u) != 0) return;

  enemy_x = (uint8_t)(enemy_x + enemy_dx);
  if (enemy_x <= 16u || enemy_x >= 144u) {
    enemy_dx = (int8_t)-enemy_dx;
  }
}

void update_player_input(void) {
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

void apply_friction(void) {
  if (player_vx > 8) player_vx -= 4;
  else if (player_vx < -8) player_vx += 4;
  else player_vx = 0;

  if (player_vy > 8) player_vy -= 4;
  else if (player_vy < -8) player_vy += 4;
  else player_vy = 0;
}

void update_scrolling(void) {
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

void shift_stars(void) {
  uint8_t i;
  for (i = 0; i < 4; ++i) {
    stars[i].x = (uint8_t)(stars[i].x - scroll_x);
    if (stars[i].x > 160 && stars[i].x < 240) stars[i].x = 0;
    else if (stars[i].x >= 240) stars[i].x = 159;

    stars[i].y = (uint8_t)(stars[i].y - scroll_y);
    if (stars[i].y > 192 && stars[i].y < 240) stars[i].y = 0;
    else if (stars[i].y >= 240) stars[i].y = 191;
  }
}

/* Draws a sprite with pixel-fine vertical positioning inside its zone. */
void draw_sprite_fine(const atari7800_sprite_asset_t *asset, uint8_t x, uint8_t y) {
  uint8_t y_offset = y & 7;
  atari7800_sprite_asset_t shifted_asset = *asset;
  shifted_asset.data = (const uint8_t *)((uintptr_t)shifted_asset.data + ((uint16_t)y_offset << 8));

  note_status(atari7800_scene_draw_sprite(&scene, &shifted_asset, x, y));
}

int main(void) {
  uint8_t i;

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
  note_status(atari7800_build_glyph_run(&hud_font, "SHLD:100 L:3",
      hud_glyph_entries,
      (uint8_t)(sizeof(hud_glyph_entries) / sizeof(hud_glyph_entries[0])),
      &hud_glyph_run));

  /* HUD text never changes in this demo, so draw it once and pin its zone
   * static: begin_frame/end_frame then leave it alone every frame instead
   * of re-blitting unchanged glyphs. */
  atari7800_scene_begin_frame(&scene);
  note_status(atari7800_scene_draw_glyph_run(&scene, &hud_font, 4, 8, &hud_glyph_run));
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
      if (stars[i].y >= 16) {
        /* Pick the reference matching which half of the 16-line zone this
         * star falls in -- see star_sprite/star_sprite_upper above. */
        const atari7800_sprite_asset_t *star_ref =
            ((stars[i].y & 15u) < 8u) ? &star_sprite : &star_sprite_upper;
        draw_sprite_fine(star_ref, stars[i].x, stars[i].y);
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

    /* Enemy: fighter_sprite has no fine-Y padding (unlike star_sprite_data
     * / spaceship_data), so it's drawn at a fixed, zone-aligned Y directly
     * rather than draw_sprite_fine's page-shift trick, which would read
     * past the asset's declared size for a non-zone-aligned Y. */
    note_status(atari7800_scene_draw_sprite(&scene, &fighter_sprite, enemy_x, enemy_y));

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
