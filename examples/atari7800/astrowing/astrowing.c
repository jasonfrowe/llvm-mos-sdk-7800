#include <atari7800.h>

/* Include packed spaceship frames */
#include "assets/spaceship.h"

/* Include HUD font */
#include "../assets/fighter.sprite.h"

/* Aligned 8-bit sine/cosine tables from astrowing.bas */
static const int8_t sin_table[16] __attribute__((section(".text"))) = {
  0, 2, 3, 4, 4, 4, 3, 2, 0, -2, -3, -4, -4, -4, -3, -2
};

static const int8_t cos_table[16] __attribute__((section(".text"))) = {
  6, 6, 4, 2, 0, -2, -4, -6, -6, -6, -4, -2, 0, 2, 4, 6
};

/* One-byte star replicated on each 16-line page for pixel-precise strided fetches. */
static const uint8_t star_data[] __attribute__((aligned(256))) = {
  [0] = 0x40,
  [256] = 0x40,
  [512] = 0x40,
  [768] = 0x40,
  [1024] = 0x40,
  [1280] = 0x40,
  [1536] = 0x40,
  [1792] = 0x40,
  [2048] = 0x40,
  [2304] = 0x40,
  [2560] = 0x40,
  [2816] = 0x40,
  [3072] = 0x40,
  [3328] = 0x40,
  [3584] = 0x40,
  [3840] = 0x40,
  [4095] = 0x00,
};

static const atari7800_sprite_asset_t star_sprite __attribute__((section(".text"))) = {
  .data = star_data,
  .width_bytes = 1u,
  .height_lines = 1u,
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

/* Enemy fighters */
typedef struct {
  uint8_t x;
  uint8_t y;
  uint8_t active;
} enemy_t;

#define NUM_ENEMIES 2
static enemy_t enemies[NUM_ENEMIES];

void init_enemies(void) {
  enemies[0].x = 40;
  enemies[0].y = 30;
  enemies[0].active = 1;

  enemies[1].x = 110;
  enemies[1].y = 50;
  enemies[1].active = 1;
}

void update_enemies(void) {
  uint8_t i;
  for (i = 0; i < NUM_ENEMIES; ++i) {
    if (!enemies[i].active) continue;

    /* Move down slowly */
    if ((frame_count & 3) == 0) {
      enemies[i].y++;
    }

    /* Apply viewport scrolling */
    enemies[i].x = (uint8_t)(enemies[i].x - scroll_x);
    enemies[i].y = (uint8_t)(enemies[i].y - scroll_y);

    /* If they go off-screen vertically or horizontally, wrap them */
    if (enemies[i].y >= 192 && enemies[i].y < 240) {
      enemies[i].y = 16; /* Keep below HUD */
    } else if (enemies[i].y >= 240) {
      enemies[i].y = 191;
    }

    if (enemies[i].x > 160 && enemies[i].x < 240) {
      enemies[i].x = 0;
    } else if (enemies[i].x >= 240) {
      enemies[i].x = 159;
    }
  }
}

void init_stars(void) {
  stars[0].x = 10; stars[0].y = 20; stars[0].color = 1;
  stars[1].x = 50; stars[1].y = 80; stars[1].color = 2;
  stars[2].x = 90; stars[2].y = 140; stars[2].color = 3;
  stars[3].x = 130; stars[3].y = 170; stars[3].color = 1;
}

static const atari7800_palette3_t star_palettes[3] __attribute__((section(".text"))) = {
  {0x08, 0x0c, 0x0f},
  {0x0c, 0x0f, 0x08},
  {0x0f, 0x08, 0x0c}
};

void cycle_stars(void) {
  if ((frame_count & 7) != 0) return;

  cycle_state++;
  if (cycle_state > 2) cycle_state = 0;

  atari7800_set_palette3(4, star_palettes[cycle_state]);
}

static void adjust_velocity(uint8_t *v, int8_t acc) {
  uint8_t new_val = (uint8_t)(*v + acc);
  *v = (new_val > 120) ? 120 : new_val;
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
    int8_t sx = sin_table[angle];
    if (sx >= 0) {
      adjust_velocity(&vx_p, sx);
    } else {
      adjust_velocity(&vx_m, (int8_t)(-sx));
    }

    int8_t cy = cos_table[angle];
    if (cy >= 0) {
      adjust_velocity(&vy_m, cy);
    } else {
      adjust_velocity(&vy_p, (int8_t)(-cy));
    }
  }
}

static void neutralize(uint8_t *p, uint8_t *m) {
  if (*p > 0 && *m > 0) {
    uint8_t common = (*p < *m) ? *p : *m;
    *p -= common;
    *m -= common;
  }
}

void neutralize_forces(void) {
  neutralize(&vx_p, &vx_m);
  neutralize(&vy_p, &vy_m);
}

static void friction(uint8_t *v) {
  if (*v < 2) *v = 0; else (*v)--;
}

void apply_friction(void) {
  friction(&vx_p);
  friction(&vx_m);
  friction(&vy_p);
  friction(&vy_m);
}

void update_scrolling(void) {
  scroll_x = 0;
  scroll_y = 0;

  /* X Axis scroll accumulation (div-by-64) */
  if (vx_p > 0) {
    uint8_t temp_v = (uint8_t)(vx_p + rx);
    rx = (uint8_t)(temp_v & 63u);
    uint8_t temp_w = (uint8_t)(temp_v >> 6);
    if (temp_w > 0) scroll_x = (int8_t)temp_w;
  } else if (vx_m > 0) {
    uint8_t temp_v = (uint8_t)(vx_m + acc_mx);
    acc_mx = (uint8_t)(temp_v & 63u);
    uint8_t temp_w = (uint8_t)(temp_v >> 6);
    if (temp_w > 0) scroll_x = -(int8_t)temp_w;
  }

  /* Y Axis scroll accumulation (div-by-64) */
  if (vy_p > 0) {
    uint8_t temp_v = (uint8_t)(vy_p + ry);
    ry = (uint8_t)(temp_v & 63u);
    uint8_t temp_w = (uint8_t)(temp_v >> 6);
    if (temp_w > 0) scroll_y = (int8_t)temp_w;
  } else if (vy_m > 0) {
    uint8_t temp_v = (uint8_t)(vy_m + acc_my);
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
  init_enemies();

  for (;;) {
    atari7800_wait_vblank();
    frame_count++;

    update_player_input();
    neutralize_forces();
    apply_friction();
    update_scrolling();
    shift_stars();
    cycle_stars();
    update_enemies();

    atari7800_scene_begin_frame(&scene);

    /* Render stars (clipping to gameplay area Y >= 16 to keep HUD zone clean) */
    for (i = 0; i < 4; ++i) {
      if (stars[i].y >= 16) {
        atari7800_scene_draw_sprite(&scene, &star_sprite, stars[i].x, stars[i].y);
      }
    }

    /* Draw enemy fighters */
    for (i = 0; i < NUM_ENEMIES; ++i) {
      if (enemies[i].active && enemies[i].y >= 16) {
        atari7800_scene_draw_sprite_8(&scene, &fighter_sprite, enemies[i].x, enemies[i].y);
      }
    }

    /* Draw player spaceship (16 lines high, split dynamically across 16-line zones) */
    atari7800_scene_draw_sprite_16(&scene, &spaceship_frames[angle], 72, 90);

    /* Draw HUD text (fits cleanly in 16-line zone) */
    atari7800_scene_draw_text(&scene, &hud_font, 4, 0, "SHLD:100 L:3");

    atari7800_scene_end_frame(&scene);
  }
}
