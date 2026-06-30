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

/* Draws a sprite with pixel-fine vertical positioning inside the zone */
void draw_sprite_fine(const atari7800_sprite_asset_t *asset, uint8_t x, uint8_t y) {
  uint8_t y_offset = y & 7;
  atari7800_sprite_asset_t shifted_asset = *asset;
  shifted_asset.data = (const uint8_t *)((uintptr_t)shifted_asset.data + ((uint16_t)y_offset << 8));
  
  atari7800_scene_draw_sprite(&scene, &shifted_asset, x, y);
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

  for (;;) {
    atari7800_wait_vblank();
    frame_count++;

    update_player_input();
    apply_friction();
    update_scrolling();
    shift_stars();
    cycle_stars();

    atari7800_scene_begin_frame(&scene);

    /* Render stars (clipping to gameplay area Y >= 16 to keep HUD zone clean) */
    for (i = 0; i < 4; ++i) {
      if (stars[i].y >= 16) {
        draw_sprite_fine(&star_sprite, stars[i].x, stars[i].y);
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

    draw_sprite_fine(&top_half, 72, 90);
    draw_sprite_fine(&bottom_half, 72, 90 + 8);

    /* Draw HUD text */
    atari7800_scene_draw_text(&scene, &hud_font, 4, 8, "SHLD:100 L:3");

    atari7800_scene_end_frame(&scene);
  }
}
