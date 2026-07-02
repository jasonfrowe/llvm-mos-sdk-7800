#include <atari7800.h>
#include <stddef.h>

#include "assets/bullet.h"
#include "assets/fighter.h"
#include "assets/blue_fighter.h"
#include "assets/energy.h"
#include "assets/asteroid_m.h"
#include "assets/asteroid_s.h"
#include "assets/spaceship.h"
#include "assets/explosion.h"
#include "assets/boss.h"
#include "assets/arrows.h"
#include "assets/unified_font.h"
#include "assets/title_screen.h"
#include "assets/lives_icon.h"
#include "assets/heart.h"

// Sound effect definitions
const uint8_t sfx_laser[] = {
    16, 1, 3,
    0x1F, 0x04, 0x04,
    0x1C, 0x04, 0x03,
    0x00, 0x00, 0x00
};
const uint8_t sfx_enemyfire[] = {
    16, 1, 3,
    0x0C, 0x04, 0x05,
    0x0A, 0x04, 0x04,
    0x00, 0x00, 0x00
};
const uint8_t sfx_damage[] = {
    16, 1, 5,
    0x08, 0x08, 0x0A,
    0x0A, 0x08, 0x08,
    0x0C, 0x08, 0x06,
    0x0E, 0x08, 0x04,
    0x00, 0x00, 0x00
};

// Movement lookup tables
static const int8_t sin_table[16] = {
    0, 2, 3, 4, 4, 4, 3, 2, 0, -2, -3, -4, -4, -4, -3, -2
};
static const int8_t cos_table[16] = {
    6, 6, 4, 2, 0, -2, -4, -6, -6, -6, -4, -2, 0, 2, 4, 6
};

// Game state variables
uint16_t px = 72;
uint16_t py = 90;
int16_t vx_p = 0;
int16_t vx_m = 0;
int16_t vy_p = 0;
int16_t vy_m = 0;
int16_t rx = 0;
int16_t ry = 0;
int16_t acc_mx = 0;
int16_t acc_my = 0;

uint8_t angle = 0;
uint8_t rot_timer = 0;
uint8_t shpfr = 0;

uint8_t player_lives = 3;
uint8_t player_shield = 99;
uint8_t current_level = 1;
uint8_t boss_checkpoint = 0;
uint8_t cached_lives = 255;
uint8_t cached_level = 255;
uint8_t screen_timer = 0;

uint8_t fighters_remaining = 20;
uint8_t fighters_bcd[1] = { 0x20 };

volatile uint8_t boss_state = 0;
uint8_t boss_on = 0;
uint8_t boss_hp = 100;
uint16_t boss_x = 0;
uint16_t boss_y = 0;
int16_t boss_scr_x = 0;
int16_t boss_scr_y = 0;
uint8_t boss_move_phase = 0;
uint8_t boss_fighter_timer = 0;
uint8_t boss_asteroid_cooldown = 0;
uint8_t boss_attack_timer = 0;

uint8_t energy_on = 0;
uint16_t energy_x = 0;
uint16_t energy_y = 0;
int16_t energy_scr_x = 0;
int16_t energy_scr_y = 0;

// Difficulty scaling
uint8_t enemy_move_mask = 1;
uint8_t enemy_fire_cooldown = 60;
uint8_t asteroid_move_mask = 3;
uint8_t asteroid_base_speed = 1;
uint8_t game_difficulty = 0; // 0 = Pro, 1 = Easy

// Starfield
uint16_t star_x[4];
uint16_t star_y[4];
uint8_t star_c[4];

// Player Bullets
uint8_t blife[4] = {0, 0, 0, 0};
uint8_t bul_x[4];
uint8_t bul_y[4];
uint8_t bcooldown = 0;

// Enemy Bullets
uint8_t eblife[4] = {0, 0, 0, 0};
uint8_t ebul_x[4];
uint8_t ebul_y[4];
int8_t ebul_vx[4];
int8_t ebul_vy[4];

// Enemies (Fighters & Asteroids)
uint8_t e_on[4] = {0, 0, 0, 0};
uint16_t ex[4];
uint16_t ey[4];
int16_t ex_scr[4];
int16_t ey_scr[4];
uint8_t elife[4] = {0, 0, 0, 0};
uint8_t etype[4] = {0, 0, 0, 0}; // 0 = small asteroid, 1 = medium asteroid, 2 = enemy fighter
uint8_t ecooldown = 0;

// Blue Fighters
uint8_t bflife[2] = {0, 0};
uint16_t bfx[2];
uint16_t bfy[2];
int16_t bfx_scr[2];
int16_t bfy_scr[2];
uint8_t bf_on[2] = {0, 0};
uint8_t bf_bul_life = 0;
uint16_t bf_bul_x = 0;
uint16_t bf_bul_y = 0;
uint8_t bf_fire_cooldown = 0;

// Global big asteroid (spawns in Level 6)
uint8_t alife = 0;
uint16_t ax = 0;
uint16_t ay = 0;
int16_t ax_scr = 0;
int16_t ay_scr = 0;
int8_t avx = 0;
int8_t avy = 0;
uint8_t asteroid_timer = 0;



// Forward declarations
void init_stars(void);
void draw_stars(void);
void set_level_config(void);
void init_boss(void);
void teleport_boss(void);
void update_render_coords(void);
void check_collisions(void);
void update_enemy(void);
void update_blue_fighters(void);
void update_bullets(void);
void shift_universe(int8_t scroll_x, int8_t scroll_y);
void play_sfx(const uint8_t *sfx);
void apply_thrust(void);
void apply_friction(void);
void neutralize_forces(void);

void start_level_music(uint8_t level);
void start_title_music(void);

void play_sfx(const uint8_t *sfx) {
    playsfx(sfx, 0);
}

void start_level_music(uint8_t level) {
    stop_music();
    if (boss_state >= 1 || level == 6) {
        music_ptr = Boss_30hz_bin;
        music_loop_start = Boss_30hz_bin;
        current_song = 4;
    } else if (level == 3) {
        music_ptr = Song_03_30hz_bin;
        music_loop_start = Song_03_30hz_bin;
        current_song = 3;
    } else if (level == 1 || level == 4) {
        music_ptr = Song_02_30hz_bin;
        music_loop_start = Song_02_30hz_bin;
        current_song = 2;
    } else {
        // level == 2 or 5
        music_ptr = Song_01_30hz_bin;
        music_loop_start = Song_01_30hz_bin;
        current_song = 1;
    }
    music_active = 1;
}

void start_title_music(void) {
    stop_music();
    static uint8_t title_song_idx = 1;
    if (title_song_idx == 1) {
        music_ptr = Song_01_30hz_bin;
        music_loop_start = Song_01_30hz_bin;
        current_song = 1;
        title_song_idx = 2;
    } else if (title_song_idx == 2) {
        music_ptr = Song_02_30hz_bin;
        music_loop_start = Song_02_30hz_bin;
        current_song = 2;
        title_song_idx = 3;
    } else {
        music_ptr = Song_03_30hz_bin;
        music_loop_start = Song_03_30hz_bin;
        current_song = 3;
        title_song_idx = 1;
    }
    music_active = 1;
}

void apply_thrust(void) {
    int8_t dx = sin_table[angle];
    if (dx > 0) vx_p += dx;
    else if (dx < 0) vx_m -= dx;

    int8_t dy = cos_table[angle];
    if (dy > 0) vy_m += dy;
    else if (dy < 0) vy_p -= dy;

    int16_t max_speed = (current_level >= 3) ? 190 : 120;
    if (vx_p > max_speed) vx_p = max_speed;
    if (vx_m > max_speed) vx_m = max_speed;
    if (vy_p > max_speed) vy_p = max_speed;
    if (vy_m > max_speed) vy_m = max_speed;
}

void neutralize_forces(void) {
    if (vx_p > 0 && vx_m > 0) {
        int16_t common = (vx_p < vx_m) ? vx_p : vx_m;
        vx_p -= common;
        vx_m -= common;
    }
    if (vy_p > 0 && vy_m > 0) {
        int16_t common = (vy_p < vy_m) ? vy_p : vy_m;
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

void init_stars(void) {
    for (uint8_t i = 0; i < 4; i++) {
        star_x[i] = (get_random() & 127) + (get_random() & 31);
        if (star_x[i] > 159) star_x[i] = 159;
        
        star_y[i] = (get_random() & 127) + 30;
        if (star_y[i] > 190) star_y[i] = 190;
        
        star_c[i] = (get_random() & 3) + 1;
    }
}

void draw_stars(void) {
    for (uint8_t i = 0; i < 4; i++) {
        int16_t sx = star_x[i] - px + 80;
        int16_t sy = star_y[i] - py + 90;
        if (sx >= 0 && sx <= 160 && sy >= 0 && sy <= 192) {
            plotsprite(bullet_data, star_c[i], sx, sy, BULLET_WIDTH_BYTES);
        }
    }
}

void set_level_config(void) {
    if (current_level == 1) {
        enemy_move_mask = 2; enemy_fire_cooldown = 60; asteroid_move_mask = 3; asteroid_base_speed = 1;
    } else if (current_level == 2) {
        enemy_move_mask = 1; enemy_fire_cooldown = 45; asteroid_move_mask = 1; asteroid_base_speed = 1;
    } else if (current_level == 3) {
        enemy_move_mask = 1; enemy_fire_cooldown = 30; asteroid_move_mask = 1; asteroid_base_speed = 1;
    } else if (current_level == 4) {
        enemy_move_mask = 0; enemy_fire_cooldown = 25; asteroid_move_mask = 0; asteroid_base_speed = 1;
    } else {
        enemy_move_mask = 0; enemy_fire_cooldown = 20; asteroid_move_mask = 0; asteroid_base_speed = 2;
    }

    if (game_difficulty == 1) {
        enemy_move_mask |= 1;
        enemy_fire_cooldown += 10;
        asteroid_base_speed = 1;
    }
}

void init_boss(void) {
    if (boss_checkpoint == 1) {
        boss_hp = 50;
        boss_state = 2;
    } else {
        boss_hp = 100;
        boss_state = 4;
    }
    
    stop_music();
    music_ptr = NULL; // Force boss song re-init
    
    teleport_boss();
    
    for (uint8_t i = 0; i < 4; i++) {
        eblife[i] = 0;
    }
    
    // Set Boss Palette
    P6C1 = 0x46; // Red
    P6C2 = 0x96; // Blue
    P6C3 = 0x0A; // Gray
    
    boss_fighter_timer = 60;
    boss_asteroid_cooldown = 120;
}

void teleport_boss(void) {
    do {
        boss_x = (get_random() & 255) | ((get_random() & 1) << 8);
        boss_y = (get_random() & 255) | ((get_random() & 1) << 8);
    } while (boss_x < 100 || boss_y < 100);
    boss_on = 0;
}

void update_render_coords(void) {
    // Player is always at screen center (72, 90)
    // Update enemies
    for (uint8_t i = 0; i < 4; i++) {
        if (elife[i] == 0) {
            e_on[i] = 0;
            continue;
        }
        int16_t dx = (ex[i] - px) & 511;
        if (dx >= 256) dx -= 512;
        ex_scr[i] = dx + 72;
        
        int16_t dy = (ey[i] - py) & 511;
        if (dy >= 256) dy -= 512;
        ey_scr[i] = dy + 90;
        
        if (ex_scr[i] >= -16 && ex_scr[i] <= 176 && ey_scr[i] >= -16 && ey_scr[i] <= 210) {
            e_on[i] = 1;
        } else {
            e_on[i] = 0;
        }
    }
    
    // Update Blue Fighters
    for (uint8_t i = 0; i < 2; i++) {
        if (bflife[i] == 0) {
            bf_on[i] = 0;
            continue;
        }
        int16_t dx = (bfx[i] - px) & 511;
        if (dx >= 256) dx -= 512;
        bfx_scr[i] = dx + 72;
        
        int16_t dy = (bfy[i] - py) & 511;
        if (dy >= 256) dy -= 512;
        bfy_scr[i] = dy + 90;
        
        if (bfx_scr[i] >= -16 && bfx_scr[i] <= 176 && bfy_scr[i] >= -16 && bfy_scr[i] <= 210) {
            bf_on[i] = 1;
        } else {
            bf_on[i] = 0;
        }
    }
    
    // Update Boss
    if (boss_state > 0 && boss_state <= 4) {
        int16_t dx = (boss_x - px) & 511;
        if (dx >= 256) dx -= 512;
        boss_scr_x = dx + 72;
        
        int16_t dy = (boss_y - py) & 511;
        if (dy >= 256) dy -= 512;
        boss_scr_y = dy + 90;
        
        if (boss_scr_x >= -32 && boss_scr_x <= 192 && boss_scr_y >= -32 && boss_scr_y <= 220) {
            boss_on = 1;
        } else {
            boss_on = 0;
        }
    } else {
        boss_on = 0;
    }
    
    // Update Large Asteroid
    if (alife > 0) {
        int16_t dx = (ax - px) & 511;
        if (dx >= 256) dx -= 512;
        ax_scr = dx + 72;
        
        int16_t dy = (ay - py) & 511;
        if (dy >= 256) dy -= 512;
        ay_scr = dy + 90;
    }
    
    // Update Energy Item
    if (energy_on) {
        int16_t dx = (energy_x - px) & 511;
        if (dx >= 256) dx -= 512;
        energy_scr_x = dx + 72;
        
        int16_t dy = (energy_y - py) & 511;
        if (dy >= 256) dy -= 512;
        energy_scr_y = dy + 90;
    }
}

void shift_universe(int8_t scroll_x, int8_t scroll_y) {
    // Shift stars
    for (uint8_t i = 0; i < 4; i++) {
        star_x[i] = (star_x[i] - scroll_x) & 159;
        star_y[i] = (star_y[i] - scroll_y) & 191;
    }
    
    // Shift enemies
    for (uint8_t i = 0; i < 4; i++) {
        if (elife[i] > 0) {
            ex[i] = (ex[i] - scroll_x) & 511;
            ey[i] = (ey[i] - scroll_y) & 511;
        }
    }
    
    // Shift Blue Fighters
    for (uint8_t i = 0; i < 2; i++) {
        if (bflife[i] > 0) {
            bfx[i] = (bfx[i] - scroll_x) & 511;
            bfy[i] = (bfy[i] - scroll_y) & 511;
        }
    }
    
    // Shift Large Asteroid
    if (alife > 0) {
        ax = (ax - scroll_x) & 511;
        ay = (ay - scroll_y) & 511;
    }
    
    // Shift Energy Item
    if (energy_on) {
        energy_x = (energy_x - scroll_x) & 511;
        energy_y = (energy_y - scroll_y) & 511;
    }
    
    // Shift Boss
    if (boss_state > 0) {
        boss_x = (boss_x - scroll_x) & 511;
        boss_y = (boss_y - scroll_y) & 511;
    }
    
    // Shift enemy bullets
    for (uint8_t i = 0; i < 4; i++) {
        if (eblife[i] > 0) {
            ebul_x[i] -= scroll_x;
            ebul_y[i] -= scroll_y;
        }
    }
    
    // Shift Blue Fighter bullet
    if (bf_bul_life > 0) {
        bf_bul_x -= scroll_x;
        bf_bul_y -= scroll_y;
    }
}

void spawn_blue_fighter(void) {
    for (uint8_t i = 0; i < 2; i++) {
        if (bflife[i] == 0) {
            bflife[i] = 1;
            bfx[i] = (px + 120) & 511;
            uint8_t ry_val = get_random();
            if (ry_val < 20) ry_val = 20;
            if (ry_val > 172) ry_val = 172;
            bfy[i] = (py + (ry_val - 90)) & 511;
            return;
        }
    }
}

void update_blue_fighters(void) {
    for (uint8_t i = 0; i < 2; i++) {
        if (bflife[i] == 0) continue;
        
        // Explosion animation
        if (bflife[i] > 1) {
            bflife[i]--;
            if (bflife[i] == 1) bflife[i] = 0;
            continue;
        }
        
        // Firing logic
        if (bf_bul_life == 0 && bf_fire_cooldown == 0 && bf_on[i]) {
            bf_bul_life = 120;
            bf_bul_x = bfx_scr[i] + 4;
            bf_bul_y = bfy_scr[i] + 4;
            play_sfx(sfx_enemyfire);
            bf_fire_cooldown = 90;
        }
        
        // Move towards player vertically, oscillating horizontally
        int16_t dy = bfy_scr[i] - 90;
        if (dy > 2) bfy[i] = (bfy[i] - 1) & 511;
        else if (dy < -2) bfy[i] = (bfy[i] + 1) & 511;
        
        bfx[i] = (bfx[i] - 1) & 511; // Scroll left slowly
    }
    
    if (bf_fire_cooldown > 0) bf_fire_cooldown--;
    
    // Update Blue Fighter Bullet
    if (bf_bul_life > 0) {
        bf_bul_x -= 3; // Moves left
        bf_bul_life--;
    }
}

void update_bullets(void) {
    for (uint8_t i = 0; i < 4; i++) {
        if (blife[i] > 0) {
            bul_y[i] -= 4; // Player bullets move up quickly
            blife[i]--;
        }
    }
    if (bcooldown > 0) bcooldown--;
    
    // Update enemy bullets
    for (uint8_t i = 0; i < 4; i++) {
        if (eblife[i] > 0) {
            ebul_x[i] += ebul_vx[i];
            ebul_y[i] += ebul_vy[i];
            eblife[i]--;
        }
    }
}

void spawn_asteroid(void) {
    alife = 1;
    asteroid_timer = 240;
    ax = (px + 100) & 511;
    ay = (py + (get_random() % 100 - 50)) & 511;
    avx = -1;
    avy = (get_random() & 1) ? 1 : -1;
}

void update_asteroid(void) {
    if (alife == 0) return;
    
    ax = (ax + avx) & 511;
    ay = (ay + avy) & 511;
    
    if (asteroid_timer > 0) {
        asteroid_timer--;
        if (asteroid_timer == 0) alife = 0;
    }
}

void update_enemy(void) {
    for (uint8_t i = 0; i < 4; i++) {
        if (elife[i] == 0) {
            // Spawn chance
            if ((get_random() & 127) <= 5) {
                elife[i] = 1;
                if (get_random() < 128) {
                    ex[i] = (px - 90) & 511;
                } else {
                    ex[i] = (px + 90) & 511;
                }
                uint8_t sy = get_random();
                if (sy < 10) sy = 10;
                if (sy > 180) sy = 180;
                ex[i] = (px + (get_random() < 128 ? -90 : 90)) & 511;
                ey[i] = (py + (sy - 90)) & 511;
                
                uint8_t roll = get_random();
                if (current_level >= 3) {
                    if (roll < 80) etype[i] = 2; // Fighter
                    else if (roll < 160) etype[i] = 1; // Medium Asteroid
                    else etype[i] = 0; // Small Asteroid
                } else if (current_level >= 2) {
                    if (roll < 128) etype[i] = 1;
                    else etype[i] = 0;
                } else {
                    etype[i] = 0;
                }
            }
            continue;
        }
        
        // Handle explosion animation
        if (elife[i] > 1) {
            elife[i]--;
            if (elife[i] == 1) elife[i] = 0;
            continue;
        }
        
        // Movement based on type
        if (etype[i] == 2) { // Enemy Fighter
            // Move towards player
            if (ex_scr[i] < 72) ex[i] = (ex[i] + 1) & 511;
            else if (ex_scr[i] > 72) ex[i] = (ex[i] - 1) & 511;
            
            if (ey_scr[i] < 90) ey[i] = (ey[i] + 1) & 511;
            else if (ey_scr[i] > 90) ey[i] = (ey[i] - 1) & 511;
            
            // Firing logic
            if (ecooldown == 0 && e_on[i]) {
                for (uint8_t b = 0; b < 4; b++) {
                    if (eblife[b] == 0) {
                        eblife[b] = 90;
                        ebul_x[b] = ex_scr[i];
                        ebul_y[b] = ey_scr[i];
                        
                        // Aim at player
                        ebul_vx[b] = (ex_scr[i] < 72) ? 2 : -2;
                        ebul_vy[b] = (ey_scr[i] < 90) ? 2 : -2;
                        
                        play_sfx(sfx_enemyfire);
                        ecooldown = enemy_fire_cooldown;
                        break;
                    }
                }
            }
        } else {
            // Asteroid simple drift
            if (!(frame & asteroid_move_mask)) {
                ex[i] = (ex[i] - asteroid_base_speed) & 511;
            }
        }
    }
    
    if (ecooldown > 0) ecooldown--;
}

void update_boss(void) {
    if (boss_state == 0) return;
    
    if (boss_fighter_timer > 0) boss_fighter_timer--;
    else {
        if (current_level >= 2) {
            // Spawn a boss fighter helper
            for (uint8_t i = 0; i < 4; i++) {
                if (elife[i] == 0) {
                    elife[i] = 1;
                    ex[i] = boss_x;
                    ey[i] = boss_y;
                    etype[i] = 2; // Fighter
                    break;
                }
            }
        }
        boss_fighter_timer = 90;
    }
    
    if (boss_asteroid_cooldown > 0) boss_asteroid_cooldown--;
    else {
        if (current_level == 6 && alife == 0) {
            spawn_asteroid();
        } else if (current_level >= 4) {
            spawn_blue_fighter();
        } else {
            // Fire boss bullets
            for (uint8_t i = 0; i < 4; i++) {
                if (eblife[i] == 0) {
                    eblife[i] = 120;
                    ebul_x[i] = boss_scr_x + 16;
                    ebul_y[i] = boss_scr_y + 8;
                    ebul_vx[i] = (boss_scr_x + 16 < 80) ? 2 : -2;
                    ebul_vy[i] = (boss_scr_y + 8 < 98) ? 2 : -2;
                    play_sfx(sfx_enemyfire);
                    break;
                }
            }
        }
        boss_asteroid_cooldown = 120;
    }
    
    // Boss horizontal movement oscillation
    if (boss_on) {
        boss_move_phase++;
        if (boss_move_phase >= 120) boss_move_phase = 0;
        
        int8_t speed = (current_level >= 5) ? 4 : (current_level >= 3 ? 3 : 2);
        if (boss_move_phase < 60) {
            boss_x = (boss_x + speed) & 511;
        } else {
            boss_x = (boss_x - speed) & 511;
        }
    }
}

void check_collisions(void) {
    // 0. Player vs Energy Item
    if (energy_on) {
        int16_t dx = energy_scr_x - 72;
        int16_t dy = energy_scr_y - 90;
        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;
        if (dx < 12 && dy < 12) {
            energy_on = 0;
            play_sfx(sfx_laser);
            player_shield += 15;
            if (player_shield > 99) player_shield = 99;
        }
    }
    
    // 1. Player Bullets vs Enemies
    for (uint8_t b = 0; b < 4; b++) {
        if (blife[b] == 0) continue;
        
        // Against regular enemies
        for (uint8_t e = 0; e < 4; e++) {
            if (elife[e] != 1) continue;
            
            int16_t dx = bul_x[b] - ex_scr[e];
            int16_t dy = bul_y[b] - ey_scr[e];
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            
            if (dx < 12 && dy < 12) {
                blife[b] = 0;
                elife[e] = 18; // Trigger explosion (18 frames)
                play_sfx(sfx_damage);
                
                if (fighters_remaining > 0) {
                    fighters_remaining--;
                    fighters_bcd[0] = converttobcd(fighters_remaining);
                }
                
                // Add score: 100 points BCD (adds $01, $00 to score0)
                uint8_t add_score[3] = { 0x00, 0x01, 0x00 };
                bcd_add_24(score0, add_score);
                
                // Roll for energy spawn
                if (!energy_on && (get_random() & 7) == 0) {
                    energy_on = 1;
                    energy_x = ex[e];
                    energy_y = ey[e];
                }
                break;
            }
        }
        
        // Against Blue Fighters
        if (blife[b] > 0) {
            for (uint8_t bf = 0; bf < 2; bf++) {
                if (bflife[bf] != 1) continue;
                
                int16_t dx = bul_x[b] - bfx_scr[bf];
                int16_t dy = bul_y[b] - bfy_scr[bf];
                if (dx < 0) dx = -dx;
                if (dy < 0) dy = -dy;
                
                if (dx < 16 && dy < 16) {
                    blife[b] = 0;
                    bflife[bf] = 18; // Trigger explosion
                    play_sfx(sfx_damage);
                    break;
                }
            }
        }
        
        // Against Boss
        if (blife[b] > 0 && boss_state > 0) {
            int16_t dx = bul_x[b] - (boss_scr_x + 16);
            int16_t dy = bul_y[b] - (boss_scr_y + 16);
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            
            if (dx < 24 && dy < 24) {
                blife[b] = 0;
                play_sfx(sfx_damage);
                if (boss_hp > 5) boss_hp -= 5;
                else {
                    boss_hp = 0;
                    boss_state = 0; // Boss defeated!
                    
                    uint8_t add_score[3] = { 0x00, 0x50, 0x00 }; // 5000 points BCD
                    bcd_add_24(score0, add_score);
                }
            }
        }
    }
    
    // 2. Enemy Bullets vs Player
    for (uint8_t b = 0; b < 4; b++) {
        if (eblife[b] == 0) continue;
        int16_t dx = ebul_x[b] - 72;
        int16_t dy = ebul_y[b] - 90;
        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;
        if (dx < 12 && dy < 12) {
            eblife[b] = 0;
            play_sfx(sfx_damage);
            if (player_shield >= 15) player_shield -= 15;
            else player_shield = 0;
        }
    }
    
    // 3. Enemies/Asteroids vs Player
    for (uint8_t e = 0; e < 4; e++) {
        if (elife[e] != 1) continue;
        int16_t dx = ex_scr[e] - 72;
        int16_t dy = ey_scr[e] - 90;
        if (dx < 0) dx = -dx;
        if (dy < 0) dy = -dy;
        if (dx < 12 && dy < 12) {
            elife[e] = 18; // Destruct enemy
            play_sfx(sfx_damage);
            if (player_shield >= 20) player_shield -= 20;
            else player_shield = 0;
        }
    }
}

int main(void) {
    // Select unified font character set
    characterset(unified_font_data);
    charactermode = 0x60; // 160A mode
    
    // Black background
    BACKGRND = 0x00;
    
    // Setup initial palettes
    P0C1=0x26; P0C2=0x24; P0C3=0x04;
    P1C1=0xC2; P1C2=0xC6; P1C3=0xCA;
    P2C1=0x04; P2C2=0x08; P2C3=0x0C;
    P3C1=0xB4; P3C2=0x46; P3C3=0x1C;
    P4C1=0x08; P4C2=0x0C; P4C3=0x0F;
    P5C1=0x34; P5C2=0x86; P5C3=0x0A;
    P6C1=0x42; P6C2=0x46; P6C3=0x4A;
    P7C1=0xC8; P7C2=0x46; P7C3=0x1C;

    // Reset score
    score0[0] = 0; score0[1] = 0; score0[2] = 0;

    // Start title music
    start_title_music();

    // Title Screen State
    while (1) {
        clearscreen();
        
        // Plot Title screen zones
        // Plot Title screen zones (160x96 banner split into two direct-mode columns of 32 and 8 bytes)
        plotsprite(title_screen_zone0, 7, 0, 0, 32);
        plotsprite(title_screen_zone0 + 32, 7, 128, 0, 8);
        plotsprite(title_screen_zone1, 7, 0, 16, 32);
        plotsprite(title_screen_zone1 + 32, 7, 128, 16, 8);
        plotsprite(title_screen_zone2, 7, 0, 32, 32);
        plotsprite(title_screen_zone2 + 32, 7, 128, 32, 8);
        plotsprite(title_screen_zone3, 7, 0, 48, 32);
        plotsprite(title_screen_zone3 + 32, 7, 128, 48, 8);
        plotsprite(title_screen_zone4, 7, 0, 64, 32);
        plotsprite(title_screen_zone4 + 32, 7, 128, 64, 8);
        plotsprite(title_screen_zone5, 7, 0, 80, 32);
        plotsprite(title_screen_zone5 + 32, 7, 128, 80, 8);
        
        // Draw texts
        plotchars("PRESS FIRE TO START", 1, 24, 7, 19);
        plotchars("DIFFICULTY: SELECT TO CHANGE", 3, 4, 9, 28);
        if (game_difficulty == 0) {
            plotchars("MODE: PRO", 5, 44, 10, 9);
        } else {
            plotchars("MODE: EASY", 5, 44, 10, 10);
        }
        
        drawscreen();
        
        if (switchselect) {
            game_difficulty ^= 1;
            while (switchselect) drawscreen(); // Wait release
        }
        
        if (joy0fire) {
            while (joy0fire) drawscreen(); // Wait release
            break;
        }
    }
    
    // Initialize Game Settings
    player_lives = 3;
    player_shield = 99;
    current_level = 1;
    set_level_config();
    init_stars();
    
    // Switch music to Level Theme
    start_level_music(current_level);
    
    // Main Game Loops
    while (player_lives > 0) {
        clearscreen();
        
        // Handle rotation input
        if (joy0left) {
            if (rot_timer == 0) {
                angle = (angle == 0) ? 15 : angle - 1;
                rot_timer = 6;
            }
        } else if (joy0right) {
            if (rot_timer == 0) {
                angle = (angle == 15) ? 0 : angle + 1;
                rot_timer = 6;
            }
        }
        if (rot_timer > 0) rot_timer--;
        
        // Handle thrust
        if (joy0up) {
            apply_thrust();
        } else {
            apply_friction();
        }
        neutralize_forces();
        
        // Scrolling velocity components
        int8_t temp_bx = 0;
        int8_t temp_by = 0;

        int16_t tx = vx_p + rx;
        rx = tx & 63;
        temp_bx = tx / 64;

        int16_t tx_m = vx_m + acc_mx;
        acc_mx = tx_m & 63;
        temp_bx -= tx_m / 64;

        int16_t ty = vy_p + ry;
        ry = ty & 63;
        temp_by = ty / 64;

        int16_t ty_m = vy_m + acc_my;
        acc_my = ty_m & 63;
        temp_by -= ty_m / 64;
        
        // Shift stars and enemies relative to scroll
        shift_universe(temp_bx, temp_by);
        
        // Fire bullet
        if (joy0fire && bcooldown == 0) {
            for (uint8_t i = 0; i < 4; i++) {
                if (blife[i] == 0) {
                    blife[i] = 45; // bullet life
                    // Centered bullet coordinates: Player is at (72, 90)
                    bul_x[i] = 72 + 6;
                    bul_y[i] = 90;
                    play_sfx(sfx_laser);
                    bcooldown = 15;
                    break;
                }
            }
        }
        
        // Updates
        update_bullets();
        update_enemy();
        update_blue_fighters();
        update_asteroid();
        update_boss();
        update_render_coords();
        
        // Check Collisions
        check_collisions();
        
        // Draw background stars
        draw_stars();
        
        // Draw enemies and asteroids
        for (uint8_t i = 0; i < 4; i++) {
            if (elife[i] > 0) {
                if (elife[i] > 1) { // Explosion
                    plotsprite(explosion_data + (18 - elife[i]) * EXPLOSION_WIDTH_BYTES, 3, ex_scr[i], ey_scr[i], EXPLOSION_WIDTH_BYTES);
                } else {
                    if (etype[i] == 2) {
                        plotsprite(fighter_data, FIGHTER_DEFAULT_PALETTE, ex_scr[i], ey_scr[i], FIGHTER_WIDTH_BYTES);
                    } else if (etype[i] == 1) {
                        plotsprite(asteroid_m_data, ASTEROID_M_DEFAULT_PALETTE, ex_scr[i], ey_scr[i], ASTEROID_M_WIDTH_BYTES);
                    } else {
                        plotsprite(asteroid_s_data, ASTEROID_S_DEFAULT_PALETTE, ex_scr[i], ey_scr[i], ASTEROID_S_WIDTH_BYTES);
                    }
                }
            }
        }
        
        // Draw Blue Fighters
        for (uint8_t i = 0; i < 2; i++) {
            if (bflife[i] > 0) {
                if (bflife[i] > 1) {
                    plotsprite(explosion_data + (18 - bflife[i]) * EXPLOSION_WIDTH_BYTES, 3, bfx_scr[i], bfy_scr[i], EXPLOSION_WIDTH_BYTES);
                } else {
                    plotsprite(blue_fighter_data, BLUE_FIGHTER_DEFAULT_PALETTE, bfx_scr[i], bfy_scr[i], BLUE_FIGHTER_WIDTH_BYTES);
                }
            }
        }
        
        // Draw Large Asteroid
        if (alife > 0) {
            plotsprite(asteroid_m_data, ASTEROID_M_DEFAULT_PALETTE, ax_scr, ay_scr, ASTEROID_M_WIDTH_BYTES);
        }
        
        // Draw Energy Item
        if (energy_on) {
            plotsprite(energy_data, ENERGY_DEFAULT_PALETTE, energy_scr_x, energy_scr_y, ENERGY_WIDTH_BYTES);
        }
        
        // Draw player spaceship (rotated based on angle)
        plotsprite(spaceship_data + angle * SPACESHIP_WIDTH_BYTES, SPACESHIP_DEFAULT_PALETTE, 72, 90, SPACESHIP_WIDTH_BYTES);
        
        // Draw bullets
        for (uint8_t i = 0; i < 4; i++) {
            if (blife[i] > 0) {
                plotsprite(bullet_data, BULLET_DEFAULT_PALETTE, bul_x[i], bul_y[i], BULLET_WIDTH_BYTES);
            }
            if (eblife[i] > 0) {
                plotsprite(bullet_data, 6, ebul_x[i], ebul_y[i], BULLET_WIDTH_BYTES);
            }
        }
        
        if (bf_bul_life > 0) {
            plotsprite(bullet_data, 6, bf_bul_x, bf_bul_y, BULLET_WIDTH_BYTES);
        }
        
        // Draw Boss
        if (boss_state > 0 && boss_on) {
            plotsprite(boss_data + (boss_state - 1) * BOSS_WIDTH_BYTES, BOSS_DEFAULT_PALETTE, boss_scr_x, boss_scr_y, BOSS_WIDTH_BYTES);
        }
        
        // Draw Score BCD
        plotvalue(unified_font_data, 7, score0, 6, 2, 0);
        
        // Draw remaining enemies/fighters BCD
        plotchars("REMAIN:", 7, 80, 0, 7);
        plotvalue(unified_font_data, 7, fighters_bcd, 2, 136, 0);
        
        // Draw player shield bar
        plotchars("SHIELD:", 7, 2, 11, 7);
        uint8_t shield_chars = player_shield / 10;
        for (uint8_t s = 0; s < 10; s++) {
            if (s < shield_chars) {
                plotchars("=", 1, 60 + s * 8, 11, 1);
            } else {
                plotchars(".", 4, 60 + s * 8, 11, 1);
            }
        }
        
        // Shield danger warn flashing color
        if (player_shield < 20) {
            if (frame & 16) {
                P5C2 = 0x34; // Red flash
            } else {
                P5C2 = 0x86; // normal
            }
        } else {
            P5C2 = 0x86;
        }
        
        // Check for player death
        if (player_shield == 0) {
            player_lives--;
            if (player_lives > 0) {
                player_shield = 99;
                // Respawn wait
                for (uint8_t wait = 0; wait < 60; wait++) {
                    clearscreen();
                    draw_stars();
                    plotchars("GET READY", 1, 48, 6, 9);
                    drawscreen();
                }
            }
        }
        
        // Check for level complete
        if (fighters_remaining == 0 && (boss_state == 0 || boss_state > 4)) {
            // Level complete!
            current_level++;
            if (current_level > 6) {
                // Victory!
                break;
            }
            
            // Show level complete message
            for (uint16_t wait = 0; wait < 120; wait++) {
                clearscreen();
                draw_stars();
                plotchars("LEVEL COMPLETE", 3, 28, 6, 14);
                drawscreen();
            }
            
            fighters_remaining = 20 + current_level * 5;
            if (current_level == 6) {
                fighters_remaining = 1; // Boss only
                init_boss();
            }
            fighters_bcd[0] = converttobcd(fighters_remaining);
            player_shield = 99;
            set_level_config();
            start_level_music(current_level);
        }
        
        drawscreen();
    }
    
    // End of game
    stop_music();
    clearscreen();
    
    if (player_lives == 0) {
        // Game Over Screen
        while (1) {
            clearscreen();
            plotchars("GAME OVER", 3, 44, 4, 9);
            plotchars("PRESS SELECT TO RESTART", 7, 12, 7, 23);
            plotvalue(unified_font_data, 7, score0, 6, 56, 9);
            drawscreen();
            if (switchselect || switchreset) {
                break; // Restart
            }
        }
    } else {
        // Victory Screen
        while (1) {
            clearscreen();
            plotchars("VICTORY", 1, 52, 3, 7);
            plotchars("THE GALAXY IS SAFE", 7, 12, 5, 18);
            plotchars("PRESS SELECT TO REPLAY", 7, 6, 7, 22);
            plotvalue(unified_font_data, 7, score0, 6, 56, 9);
            drawscreen();
            if (switchselect || switchreset) {
                break;
            }
        }
    }
    
    // Reboot / restart game
    __asm__ volatile ("jmp atari7800_reset_entry");
    
    return 0;
}
