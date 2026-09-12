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

/* Explosion animation (regular enemy sprite itself is folded into
 * misc_sprite_data below, alongside star/bullet -- see its own comment). */
#include "assets/fighter_explode.h"

/* Include HUD font. This example lives under examples/atari7800-supergame/
 * (a different platform's example tree from these shared assets, which
 * stay under examples/atari7800/assets/ since they're also used by
 * title-demo.c, still built against the flat atari7800 platform). */
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
 * getting an independent 7936-byte array: both need the exact same
 * 31-page fine-Y envelope (utils/pack_sprites_to_strided.py generates
 * this identical envelope regardless of a sprite's own height -- see its
 * own comment, fixed in 7800port.md #27), so an extra byte-column
 * "channel" in the same pages costs nothing extra -- same technique as
 * spaceship_data's 16 frames or title_screen_data's chunks sharing one
 * block, just with 2 tiny sprites instead of many animation frames. This
 * one change was needed to fit the level-1 music (song_level1) within
 * 48K's actual usable ROM budget -- see 7800port.md #22.
 *
 * enemy_sprite used to share a third byte-column here (7800port.md #27),
 * but moving under real AI exercises the *whole* y&15 shift range,
 * including positions that cross a zone boundary -- correctly rendering
 * those needs a second Direct Mode object reading data from *before* this
 * array's own page 0 (see draw_sprite_at_shift's comment), which isn't
 * expressible while sharing this block's fixed page range with star/
 * bullet. Given its own dedicated, differently-padded array instead:
 * enemy_sprite_tall_data, bank 1, below. */
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

/* Enemy pool (astrowing.bas:886-1118's update_enemy, ported in full --
 * see 7800port.md #26). Pool of 4, struct-of-arrays like everything else
 * in this file. enemy_life: 0=dead/empty, 1=alive, 2-18=exploding (counts
 * down to 0 on death). Reuses the player's own ship shape recolored via
 * the "Enemy" palette (index 3, matching P3C1-3 in astrowing.bas) --
 * see assets/enemy.h for why this is a local regeneration of
 * fighter.sprite.h's asset rather than the shared header itself.
 *
 * Position is screen-relative, not the reference's true world coordinates
 * with an 8-bit low byte + hi-byte "segment" (4 segments = 1024px) and
 * explicit wraparound arithmetic: that scheme exists in the reference
 * because its camera follows a player that itself moves through world
 * space. This port's ship is always drawn at a fixed screen position (72,
 * 80, see draw_sprite_fine(&ship, 72, 80) below), so "player world
 * position" always collapses to that one point -- the AI's target-corner
 * math (in update_enemy) uses it directly instead of comparing hi-bytes.
 * Enemies get the same per-frame world-scroll compensation stars already
 * receive in shift_stars (subtracting scroll_x/scroll_y) rather than
 * reproducing the reference's wraparound scheme, which has no equivalent
 * need here. int16_t (not uint8_t, like stars/bullets use): spawn
 * positions go negative or past 255 (player_x ± 90, i.e. -18 or 162). */
static int16_t enemy_x[4];
static int16_t enemy_y[4];
static int8_t enemy_vx[4];
static int8_t enemy_vy[4];
static uint8_t enemy_life[4];

/* astrowing.bas leans on 7800basic's built-in `rand` for enemy spawn
 * side/Y; this platform has no equivalent yet, so a minimal one is added
 * here. 8-bit Galois LFSR, feedback mask 0xb8 (taps at bits 8/6/5/4) --
 * a standard maximal-length choice at this width. */
static uint8_t rand_state = 1u;

static uint8_t next_rand(void) {
  uint8_t lsb = rand_state & 1u;
  rand_state >>= 1;
  if (lsb) rand_state ^= 0xb8u;
  return rand_state;
}

/* astrowing.bas's score0 (BCD, HUD-displayed) and player_shield (starts
 * at 100, astrowing.bas:509 area) -- tracked internally starting this
 * pass since player-vs-enemy collision needs somewhere real to record a
 * hit. Neither is wired into the HUD display yet (still today's static
 * "SHLD:100 L:3" text) -- a natural follow-up, not this pass. Shield
 * reaching 0 has no consequence yet either: that's the deferred level/
 * death-flow system (astrowing.bas's own "goto coll_done" branch). */
static uint16_t score = 0;
static uint8_t player_shield = 100;

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

/* astrowing.bas:886-1118's update_enemy: spawn, "squad formation, rotating
 * corner" inertia AI, and explosion countdown -- see enemy_x's own comment
 * above for the world-space-to-screen-relative adaptation. AI/velocity
 * updates every other frame (astrowing.bas:894-895's `frame &
 * enemy_move_mask`, default 1 at astrowing.bas:285 -- no difficulty system
 * exists yet, so this is the only rate used); enemy firing (astrowing.bas:
 * 1030-1039) is deferred with the rest of the enemy-bullet system. */
static void update_enemy(void) {
  uint8_t i;

  for (i = 0; i < 4; ++i) {
    if (enemy_life[i] == 0) {
      /* astrowing.bas:1046-1049: ~4.7% spawn chance/frame per dead slot. */
      if ((uint8_t)(frame_count & 127u) <= 5u) {
        uint8_t side = next_rand();
        uint8_t y_roll = next_rand();
        if (y_roll < 10u) y_roll = 10u;
        if (y_roll > 180u) y_roll = 180u;

        enemy_life[i] = 1;
        enemy_vx[i] = 0;
        enemy_vy[i] = 0;
        /* astrowing.bas:1066-1082: player_x +/- 90 (50/50 side). */
        enemy_x[i] = (side < 128u) ? (int16_t)(72 - 90) : (int16_t)(72 + 90);
        /* astrowing.bas:1083-1114: rand[10,180] as an offset from the
         * player's own screen-center (90). */
        enemy_y[i] = (int16_t)(80 + (int16_t)y_roll - 90);
      }
      continue;
    }

    if (enemy_life[i] > 1u) {
      /* astrowing.bas:1041-1044's update_explosion_state. */
      --enemy_life[i];
      if (enemy_life[i] == 1u) enemy_life[i] = 0;
      continue;
    }

    /* Alive (enemy_life[i] == 1): AI update. */
    if ((frame_count & 1u) == 0u) {
      /* astrowing.bas:899-907: rotating target corner, offset by slot
       * index so the 4 enemies don't all turn in lockstep, Gray-coded
       * (0,1,3,2 = TL,TR,BR,BL). */
      uint8_t corner_raw = (uint8_t)(((frame_count >> 7) + i) & 3u);
      uint8_t corner = (uint8_t)(corner_raw ^ (corner_raw >> 1));
      /* astrowing.bas:909-958: target = player position (fixed screen
       * (72,80) in this port) +/- offset per corner. */
      int16_t target_x = (int16_t)(72 + ((corner & 1u) ? 30 : -30));
      int16_t target_y = (int16_t)(80 + ((corner & 2u) ? 55 : -55));

      /* astrowing.bas:940-993: accelerate every 4th frame, capped +/-3. */
      if ((frame_count & 3u) == 0u) {
        if (target_x >= enemy_x[i]) {
          if (enemy_vx[i] < 3) ++enemy_vx[i];
        } else {
          if (enemy_vx[i] > -3) --enemy_vx[i];
        }
        if (target_y >= enemy_y[i]) {
          if (enemy_vy[i] < 3) ++enemy_vy[i];
        } else {
          if (enemy_vy[i] > -3) --enemy_vy[i];
        }
      }

      /* astrowing.bas:996-1028: apply velocity (only on the same
       * every-other-frame cadence as the AI above). */
      enemy_x[i] = (int16_t)(enemy_x[i] + enemy_vx[i]);
      enemy_y[i] = (int16_t)(enemy_y[i] + enemy_vy[i]);
    }

    /* World-scroll compensation, every frame regardless of the AI's own
     * update rate -- this port's own addition (see enemy_x's comment),
     * matching how shift_stars treats stars. */
    enemy_x[i] = (int16_t)(enemy_x[i] - scroll_x);
    enemy_y[i] = (int16_t)(enemy_y[i] - scroll_y);
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

/* astrowing.bas:1319-1807's check_collisions -- this pass only ports the
 * two pairs needed once real enemies exist (bullet-vs-enemy, player-vs-
 * enemy); everything else in that subroutine (energy pickups, blue
 * fighters, asteroids, boss, enemy bullets) belongs to systems not yet
 * ported, deferred along with them. */
static void check_collisions(void) {
  uint8_t bi, ei;

  /* 1. Bullets vs enemies (astrowing.bas:1342-1379). Center-distance
   * check done independently on X and Y: bullet's own center offset +2,
   * enemy's +4 (net delta -2), threshold 15 (16 on easy difficulty, which
   * this port doesn't have yet, so always 15). */
  for (bi = 0; bi < 4; ++bi) {
    if (blife[bi] == 0) continue;

    for (ei = 0; ei < 4; ++ei) {
      int16_t dx, dy;

      if (enemy_life[ei] != 1u) continue;

      dx = (int16_t)((int16_t)bul_x[bi] - enemy_x[ei] - 2);
      if (dx < 0) dx = (int16_t)-dx;
      if (dx >= 15) continue;

      dy = (int16_t)((int16_t)bul_y[bi] - enemy_y[ei] - 2);
      if (dy < 0) dy = (int16_t)-dy;
      if (dy >= 15) continue;

      /* Hit! */
      blife[bi] = 0;
      enemy_life[ei] = 18; /* Start 18-frame explosion. */
      score = (uint16_t)(score + 100u);
      break; /* Bullet used up -- astrowing.bas's own "goto skip_bullet_coll". */
    }
  }

  /* 2. Player vs enemies (astrowing.bas:1438-1474). The player is always
   * at its fixed screen position (72, 80) in this port's model. */
  for (ei = 0; ei < 4; ++ei) {
    int16_t dx, dy;

    if (enemy_life[ei] != 1u) continue;

    dx = (int16_t)(72 - enemy_x[ei] + 4);
    if (dx < 0) dx = (int16_t)-dx;
    if (dx >= 11) continue;

    dy = (int16_t)(80 - enemy_y[ei]);
    if (dy < 0) dy = (int16_t)-dy;
    if (dy >= 11) continue;

    /* Hit! Colliding with an enemy destroys it too, same as shooting it. */
    enemy_life[ei] = 18;
    score = (uint16_t)(score + 100u);
    if (player_shield < 2u) player_shield = 0;
    else player_shield = (uint8_t)(player_shield - 2u);
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

/* Shifts asset.data by an explicit signed page count (not derived from Y)
 * and draws at (x,y). draw_sprite_fine (below) is the common case, where
 * the shift is simply y&15; the enemy zone-crossing split (draw_enemy,
 * bank 1) needs a second object whose shift is (y&15)-16 -- a *negative*
 * page offset from the same base pointer, continuing the same downward
 * page countdown into data that lives *before* the sprite's own page 0 --
 * which draw_sprite_fine's y&15-only math can't express, hence this being
 * split out. Casting the negative int16_t product to uintptr_t and adding
 * relies on standard unsigned wraparound (well-defined in C, unlike
 * signed left-shift of a negative value): -N*256 as an unsigned 16-bit
 * value equals 65536-N*256, which added to the pointer is equivalent to
 * subtracting N*256, exactly as intended. */
static void draw_sprite_at_shift(const atari7800_sprite_asset_t *asset, uint8_t x, uint8_t y, int8_t page_shift) {
  atari7800_sprite_asset_t shifted_asset = *asset;
  shifted_asset.data = (const uint8_t *)((uintptr_t)shifted_asset.data + (uintptr_t)((int16_t)page_shift * 256));

  (void)atari7800_scene_draw_sprite(&scene, &shifted_asset, x, y);
}

/* Draws a sprite with pixel-fine vertical positioning inside its zone.
 * Shift range matches the full 16-line zone height (y&15, not y&7) -- see
 * star_sprite_data's comment above for why. */
static void draw_sprite_fine(const atari7800_sprite_asset_t *asset, uint8_t x, uint8_t y) {
  draw_sprite_at_shift(asset, x, y, (int8_t)(y & 15u));
}

/* A single MARIA Direct Mode object's render window is exactly one
 * 16-line zone -- whichever zone it's placed in (y>>4), only that zone's
 * own 16 scanlines are ever scanned from it, regardless of shift. BUT this
 * is not a hardware wall: astrowing.bas's own plotsprite4.asm (the
 * reference this port is based on, and proven to have zero trouble with
 * sprites crossing zone boundaries) handles a sprite whose Y isn't
 * zone-aligned by emitting a *second* Direct Mode object in the next
 * zone, whose graphic-address high byte is exactly one zone-height (16
 * pages) less than the first object's -- continuing the same downward
 * per-scanline page countdown across the boundary rather than truncating
 * it. Confirmed against the emulator's own MARIA model
 * (third_party/a7800/src/mame/video/maria.cpp's draw_scanline/startdma):
 * each zone reloads its object's page countdown fresh from that zone's
 * own DLL entry, decrementing once per scanline, so a second object
 * placed one zone down with a base address 16 pages lower picks up
 * exactly where the first left off. See draw_enemy (bank 1, below) for
 * the real two-object implementation, and enemy_sprite_tall_data's own
 * comment for the padded layout this needs. An earlier attempt at this
 * (7800port.md #28) wrongly concluded a two-object split was infeasible
 * with a *shared* data array (the 31-page envelope other sprites use is
 * too small relative to its own 16-page read window to add continuation
 * content without ghosting into the first object's unused rows) and
 * settled for clamping the drawn Y instead of giving the enemy its own,
 * differently-padded array -- reverted; see 7800port.md #29.
 *
 * Same constraint, but a 16-line-tall sprite (fighter_explode_frames)
 * only ever fits its zone at shift 0 exactly -- any other position always
 * needs a split, so there's no partial safe range to round into like the
 * 8-line case above. Always zone-align. Acceptable for an already-brief,
 * visually chaotic 18-frame explosion in a way it wouldn't be for the
 * continuously-visible, smoothly-moving live enemy sprite. */
static uint8_t zone_safe_y_16(uint8_t y) {
  return (uint8_t)(y & 0xf0u);
}

/* fighter_explode_frames/fighter_explode_data (assets/fighter_explode.h,
 * 8KiB) don't fit in the fixed region alongside everything else needed
 * every frame -- moved to switchable bank 1, since explosions are brief
 * and relatively infrequent (up to 4 at once, ~18 frames each), unlike
 * the ship/stars/live enemies that need zero-overhead fixed-region access
 * every single frame. banked_call_8000 (mapper.h) only takes a void(void)
 * function pointer, so the frame index and position are passed via these
 * globals instead of real arguments. */
static uint8_t explode_draw_frame;
static uint8_t explode_draw_x;
static uint8_t explode_draw_y;

__attribute__((section(".cart_rom_bank_1.text")))
static void draw_explosion_bank1(void) {
  draw_sprite_fine(&fighter_explode_frames[explode_draw_frame], explode_draw_x, explode_draw_y);
}

/* Regular enemy sprite (astrowing.bas's own fighter.png, recolored via
 * the "Enemy" palette, index 3, matching P3C1-3) -- 8x8, 2 bytes/row.
 * Moving under real AI exercises the full y&15 shift range, including
 * shifts that cross a zone boundary (in_zone > 8), which needs a *second*
 * Direct Mode object one zone down whose base address is 16 pages (one
 * zone height) lower than the first -- see draw_sprite_at_shift's and
 * draw_enemy's own comments, and 7800port.md #29 for the derivation.
 *
 * That second object's own page countdown, at its own zone-local shift
 * (in_zone - 16, negative), reads pages *before* this array's own page 0
 * -- so unlike star/bullet's shared 31-page envelope (content anchored at
 * the *end* of the span, pages [16-H, 15], trailing padding only), this
 * array needs LEADING padding too: 7 pages before page 0, on top of the
 * usual 31-page envelope (0-30), for 38 pages total. Content stays
 * anchored the same way (page 15-row, rows 0/7 blank and omitted) at
 * pages 8-15 of the *unshifted* numbering -- i.e. array pages 15-22 once
 * the 7-page lead is folded in; .data below points at array page 7 (this
 * sprite's own "unshifted page 0") so a plain (int8_t) shift of -7..+15
 * lands correctly across both objects without further adjustment.
 * Dedicated (not shared with star/bullet in misc_sprite_data): the
 * 31-page shared envelope has no room to add this leading padding without
 * the extra pages becoming reachable -- and visible -- from star/bullet's
 * own unused shift range too. 9728 bytes doesn't fit alongside bank 1's
 * own fighter_explode_data (8KiB) in one 16KiB bank, so this gets its own
 * bank (2, otherwise unused) rather than bank 1. */
static const uint8_t enemy_sprite_tall_data[38 * 256] __attribute__((aligned(256)))
    __attribute__((section(".cart_rom_bank_2.rodata"))) = {
  [16 * 256 + 0] = 0x03, [16 * 256 + 1] = 0xc0, /* row 6 */
  [17 * 256 + 0] = 0x3f, [17 * 256 + 1] = 0xfc, /* row 5 */
  [18 * 256 + 0] = 0x3f, [18 * 256 + 1] = 0xfc, /* row 4 */
  [19 * 256 + 0] = 0x0f, [19 * 256 + 1] = 0xf0, /* row 3 */
  [20 * 256 + 0] = 0x0a, [20 * 256 + 1] = 0x50, /* row 2 */
  [21 * 256 + 0] = 0x02, [21 * 256 + 1] = 0x40, /* row 1 */
};

__attribute__((section(".cart_rom_bank_2.rodata")))
static const atari7800_sprite_asset_t enemy_sprite_tall = {
  .data = &enemy_sprite_tall_data[7 * 256],
  .width_bytes = 2u,
  .height_lines = 8u,
  .mode = 0x40u,
  .palette = 3u,
  .width_twos_comp = 0x1eu,
  .data_layout = ATARI7800_SPRITE_LAYOUT_MARIA_STRIDED
};

static uint8_t enemy_draw_x;
static uint8_t enemy_draw_y;

/* Draws the live enemy sprite, splitting into a second Direct Mode object
 * in the next zone whenever it doesn't fit the rest of the current one --
 * astrowing.bas's own plotsprite4.asm technique (7800port.md #29),
 * replacing the Y-clamp workaround from #28. in_zone > 8 is the same
 * "more than half the sprite's 8 lines would be cut off" threshold #28's
 * zone_safe_y_8 used, but here it triggers a real second object instead
 * of snapping position. */
__attribute__((section(".cart_rom_bank_2.text")))
static void draw_enemy(void) {
  uint8_t in_zone = (uint8_t)(enemy_draw_y & 15u);
  draw_sprite_at_shift(&enemy_sprite_tall, enemy_draw_x, enemy_draw_y, (int8_t)in_zone);
  if (in_zone > 8u) {
    draw_sprite_at_shift(&enemy_sprite_tall, enemy_draw_x, (uint8_t)(enemy_draw_y + 16u), (int8_t)(in_zone - 16));
  }
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
    check_collisions();

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

    /* astrowing.bas:2445-2459's draw_player_bullets. Clipped to gameplay
     * area Y >= 16, same guard and reason as the star loop above: a
     * bullet can fly straight up through the pinned-static HUD zone
     * (nothing stops it, unlike the ship which never moves on screen),
     * and drawing into a static zone doesn't get cleared next frame like
     * a normal zone does -- confirmed hands-on as real stuck "residual"
     * sprites, not hypothetical. */
    for (i = 0; i < 4; ++i) {
      if (blife[i] != 0 && bul_y[i] >= 16) {
        draw_sprite_fine(&bullet_sprite, bul_x[i], bul_y[i]);
      }
    }

    /* astrowing.bas's draw_enemies/draw_explosions (referenced 2462+,
     * 2476-2491): alive enemies draw the ship sprite; exploding ones draw
     * an 8-frame animation selected by remaining countdown. Skip drawing
     * (not updating -- they may come back on screen) enemies currently
     * outside the visible area, since enemy_x/y are int16_t and can go
     * negative or past 255 (spawn positions do, by design). */
    for (i = 0; i < 4; ++i) {
      if (enemy_life[i] == 0) continue;
      if (enemy_x[i] < 0 || enemy_x[i] > 159 || enemy_y[i] < 16 || enemy_y[i] > 191) {
        continue;
      }

      if (enemy_life[i] == 1u) {
        /* enemy_sprite_tall/draw_enemy live in switchable bank 1 (see
         * their own comments) -- reach via banked_call_8000, passing
         * position through globals, same pattern as the explosion draw
         * below. The enemy_y <= 191 filter above keeps y+16 (the second
         * object's zone, when the sprite crosses a boundary) within the
         * scene's 14 visible zones (224 lines) -- never clamped into the
         * first object's own zone. */
        enemy_draw_x = (uint8_t)enemy_x[i];
        enemy_draw_y = (uint8_t)enemy_y[i];
        banked_call_8000(2, draw_enemy);
      } else {
        /* astrowing.bas:2484-2489: frame = (18 - elife) / 2. Clamped to 7
         * (the last real frame): the reference's own arithmetic reaches 8
         * one frame before death (elife=2), one past fighter_explode's 8
         * frames (0-7) -- clamp rather than reproduce that as an
         * out-of-bounds asset read. */
        uint8_t frame_index = (uint8_t)((18u - enemy_life[i]) / 2u);
        if (frame_index > 7u) frame_index = 7u;
        /* fighter_explode_frames lives in switchable bank 1 (see
         * draw_explosion_bank1's own comment) -- reach it via
         * banked_call_8000, passing arguments through globals. */
        explode_draw_frame = frame_index;
        explode_draw_x = (uint8_t)enemy_x[i];
        explode_draw_y = zone_safe_y_16((uint8_t)enemy_y[i]);
        banked_call_8000(1, draw_explosion_bank1);
      }
    }

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
