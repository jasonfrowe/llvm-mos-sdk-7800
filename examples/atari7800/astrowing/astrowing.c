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

/* Page-aligned 1-pixel star sprite (8 pages data + 8 pages padding) */
static const uint8_t star_sprite_data[] __attribute__((aligned(256))) = {
  [1792] = 0x40, /* Row 0 of Page 7 contains the pixel */
  [3839] = 0x00  /* Pad up to 15 pages (3840 bytes) to allow up to 7 scanlines shift */
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

/* Scene context global */
static atari7800_scene_t scene;

/* Persistent sprite objects: each retains its assigned zone/slot across
 * frames, so redrawing it every frame patches its existing header in
 * place instead of re-deriving zone membership and appending a fresh one
 * (see atari7800_scene_sprite in atari7800.h). */
static atari7800_scene_object_t star_objs[4] = {
  ATARI7800_SCENE_OBJECT_INIT, ATARI7800_SCENE_OBJECT_INIT,
  ATARI7800_SCENE_OBJECT_INIT, ATARI7800_SCENE_OBJECT_INIT
};
static atari7800_scene_object_t ship_top_obj = ATARI7800_SCENE_OBJECT_INIT;
static atari7800_scene_object_t ship_bottom_obj = ATARI7800_SCENE_OBJECT_INIT;

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

/* Draws/updates a persistent sprite object with pixel-fine vertical
 * positioning inside its zone. */
void draw_sprite_fine(atari7800_scene_object_t *object,
    const atari7800_sprite_asset_t *asset, uint8_t x, uint8_t y) {
  uint8_t y_offset = y & 7;
  atari7800_sprite_asset_t shifted_asset = *asset;
  shifted_asset.data = (const uint8_t *)((uintptr_t)shifted_asset.data + ((uint16_t)y_offset << 8));

  note_status(atari7800_scene_sprite(&scene, object, &shifted_asset, x, y));
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

  for (;;) {
    atari7800_wait_vblank();
    frame_count++;

#ifdef ATARI7800_DEBUG_FRAME_BUDGET
    /* Multi-stage frame-budget marker: each stage below sets a different
     * hue right before it starts, so a single screenshot shows one colored
     * band per stage instead of just an overrun/no-overrun bar. Whichever
     * band is tallest is where the cycles are going. Luminance is kept
     * high (0xF) on all of them so the bands are equally bright and only
     * differ by hue; the final restore to ATARI7800_BG_DARKGRAY marks
     * genuine leftover margin before the next vblank. */
    ATARI7800_BACKGRND = 0x0fu; /* stage 1: game-logic functions below */
#endif

    update_player_input();
    apply_friction();
    update_scrolling();
    shift_stars();
    cycle_stars();

#ifdef ATARI7800_DEBUG_FRAME_BUDGET
    ATARI7800_BACKGRND = 0x8fu; /* stage 2: star + ship draw calls below */
#endif

    /* Render stars as persistent objects (clipping to gameplay area Y >=
     * 16 to keep the HUD zone clean); hide any star that scrolled into
     * that band instead of just skipping its draw call, since a
     * persistent object stays displayed until told otherwise. */
    for (i = 0; i < 4; ++i) {
      if (stars[i].y >= 16) {
        draw_sprite_fine(&star_objs[i], &star_sprite, stars[i].x, stars[i].y);
      } else {
        atari7800_scene_hide_sprite(&star_objs[i]);
      }
    }

    /* Draw player spaceship (16 lines high, split into two 8-line zones) */
    atari7800_sprite_asset_t top_half = spaceship_frames[angle];
    top_half.height_lines = 8;
    top_half.palette = 5; /* Use Palette 5 (Spaceship) */
    top_half.data = (const uint8_t *)((uintptr_t)top_half.data + 8 * 256);

    atari7800_sprite_asset_t bottom_half = spaceship_frames[angle];
    bottom_half.height_lines = 8;
    bottom_half.palette = 5; /* Use Palette 5 (Spaceship) */

    draw_sprite_fine(&ship_top_obj, &top_half, 72, 88);
    draw_sprite_fine(&ship_bottom_obj, &bottom_half, 72, 88 + 8);

    /* HUD text is pinned static residency (see setup above) -- nothing to
     * draw here every frame. Stars/ship are persistent objects (see
     * setup above) -- no atari7800_scene_begin_frame/end_frame needed
     * either, since nothing in this loop uses the older append-and-diff
     * API any more. */

#ifdef ATARI7800_DEBUG_FRAME_BUDGET
    /* Marks the end of this frame's CPU work: restore the real background
     * color before the next atari7800_wait_vblank() call above. */
    ATARI7800_BACKGRND = ATARI7800_BG_DARKGRAY;
#endif
  }
}
