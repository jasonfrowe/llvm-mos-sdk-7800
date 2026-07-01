#include <atari7800.h>

/* Include packed spaceship frames */
#include "assets/spaceship.h"

/* Include HUD font */
#include "../assets/fighter.sprite.h"

/* Aligned 8-bit sine/cosine tables from astrowing.bas */
static const int16_t sin_table[16] = {
  0, 2, 3, 4, 4, 4, 3, 2, 0, -2, -3, -4, -4, -4, -3, -2
};

static const int16_t cos_table[16] = {
  6, 6, 4, 2, 0, -2, -4, -6, -6, -6, -4, -2, 0, 2, 4, 6
};

/* Page-aligned 1-pixel star sprite (16 pages data + 16 pages padding = 32 pages) */
static const uint8_t star_sprite_data[] __attribute__((aligned(256))) = {
  [3840] = 0x40, /* Row 0 of Page 15 contains the pixel */
  [8191] = 0x00  /* Pad up to 32 pages */
};

static const atari7800_sprite_asset_t star_sprite = {
  .data = star_sprite_data,
  .width_bytes = 1u,
  .height_lines = 16u, /* 16 scanlines high for 16-line zone */
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

/* Player spaceship state (exact dual-velocity structure from basic source) */
static uint8_t vx_p = 0;
static uint8_t vx_m = 0;
static uint8_t vy_p = 0;
static uint8_t vy_m = 0;
static uint8_t rx = 0;
static uint8_t ry = 0;
static uint8_t acc_mx = 0;
static uint8_t acc_my = 0;

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
    int16_t temp_acc;

    /* X Axis */
    temp_acc = sin_table[angle];
    if (temp_acc >= 0) {
      uint16_t new_val = (uint16_t)vx_p + temp_acc;
      vx_p = (new_val > 120) ? 120 : (uint8_t)new_val;
    } else {
      uint16_t new_val = (uint16_t)vx_m - temp_acc;
      vx_m = (new_val > 120) ? 120 : (uint8_t)new_val;
    }

    /* Y Axis (Inverted: subtract Cos) */
    temp_acc = cos_table[angle];
    if (temp_acc >= 0) {
      uint16_t new_val = (uint16_t)vy_m + temp_acc;
      vy_m = (new_val > 120) ? 120 : (uint8_t)new_val;
    } else {
      uint16_t new_val = (uint16_t)vy_p - temp_acc;
      vy_p = (new_val > 120) ? 120 : (uint8_t)new_val;
    }
  }
}

void neutralize_forces(void) {
  uint8_t common;

  /* X Axis */
  if (vx_p > 0 && vx_m > 0) {
    common = (vx_p < vx_m) ? vx_p : vx_m;
    vx_p -= common;
    vx_m -= common;
  }

  /* Y Axis */
  if (vy_p > 0 && vy_m > 0) {
    common = (vy_p < vy_m) ? vy_p : vy_m;
    vy_p -= common;
    vy_m -= common;
  }
}

void apply_friction(void) {
  if (vx_p < 2) vx_p = 0; else vx_p -= 1;
  if (vx_m < 2) vx_m = 0; else vx_m -= 1;
  if (vy_p < 2) vy_p = 0; else vy_p -= 1;
  if (vy_m < 2) vy_m = 0; else vy_m -= 1;
}

void update_scrolling(void) {
  scroll_x = 0;
  scroll_y = 0;

  /* X Axis scroll accumulation (div-by-64) */
  if (vx_p > 0) {
    uint16_t temp_v = (uint16_t)vx_p + rx;
    rx = (uint8_t)(temp_v & 63u);
    uint8_t temp_w = (uint8_t)(temp_v >> 6);
    if (temp_w > 0) scroll_x = (int8_t)temp_w;
  } else if (vx_m > 0) {
    uint16_t temp_v = (uint16_t)vx_m + acc_mx;
    acc_mx = (uint8_t)(temp_v & 63u);
    uint8_t temp_w = (uint8_t)(temp_v >> 6);
    if (temp_w > 0) scroll_x = -(int8_t)temp_w;
  }

  /* Y Axis scroll accumulation (div-by-64) */
  if (vy_p > 0) {
    uint16_t temp_v = (uint16_t)vy_p + ry;
    ry = (uint8_t)(temp_v & 63u);
    uint8_t temp_w = (uint8_t)(temp_v >> 6);
    if (temp_w > 0) scroll_y = (int8_t)temp_w;
  } else if (vy_m > 0) {
    uint16_t temp_v = (uint16_t)vy_m + acc_my;
    acc_my = (uint8_t)(temp_v & 63u);
    uint8_t temp_w = (uint8_t)(temp_v >> 6);
    if (temp_w > 0) scroll_y = -(int8_t)temp_w;
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

int main(void) {
  uint8_t i;

  /* Initialize system using 16-line zone height */
  atari7800_scene_init_160a_ex(&scene, ATARI7800_BG_DARKGRAY, 16);

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
    neutralize_forces();
    apply_friction();
    update_scrolling();
    shift_stars();
    cycle_stars();

    atari7800_scene_begin_frame(&scene);

    /* Render stars (clipping to gameplay area Y >= 16 to keep HUD zone clean) */
    for (i = 0; i < 4; ++i) {
      if (stars[i].y >= 16) {
        atari7800_scene_draw_sprite_fine(&scene, &star_sprite, stars[i].x, stars[i].y);
      }
    }

    /* Draw player spaceship (16 lines high, split dynamically across 16-line zones) */
    atari7800_scene_draw_sprite_16(&scene, &spaceship_frames[angle], 72, 90);

    /* Draw HUD text (fits cleanly in 16-line zone) */
    atari7800_scene_draw_text(&scene, &hud_font, 4, 8, "SHLD:100 L:3");

    atari7800_scene_end_frame(&scene);
  }
}
