#ifndef ATARI7800_H
#define ATARI7800_H

#include <stdint.h>

// RIOT Registers
#define SWCHA  (*(volatile uint8_t *)0x0280)
#define SWCHB  (*(volatile uint8_t *)0x0282)

// TIA Registers
#define INPT0  (*(volatile uint8_t *)0x0008)
#define INPT1  (*(volatile uint8_t *)0x0009)
#define INPT2  (*(volatile uint8_t *)0x000A)
#define INPT3  (*(volatile uint8_t *)0x000B)
#define INPT4  (*(volatile uint8_t *)0x000C)
#define INPT5  (*(volatile uint8_t *)0x000D)
#define AUDC0  (*(volatile uint8_t *)0x0015)
#define AUDC1  (*(volatile uint8_t *)0x0016)
#define AUDF0  (*(volatile uint8_t *)0x0017)
#define AUDF1  (*(volatile uint8_t *)0x0018)
#define AUDV0  (*(volatile uint8_t *)0x0019)
#define AUDV1  (*(volatile uint8_t *)0x001A)

// Maria Registers
#define BACKGRND (*(volatile uint8_t *)0x0020)
#define P0C1     (*(volatile uint8_t *)0x0021)
#define P0C2     (*(volatile uint8_t *)0x0022)
#define P0C3     (*(volatile uint8_t *)0x0023)
#define WSYNC    (*(volatile uint8_t *)0x0024)
#define P1C1     (*(volatile uint8_t *)0x0025)
#define P1C2     (*(volatile uint8_t *)0x0026)
#define P1C3     (*(volatile uint8_t *)0x0027)
#define MSTAT    (*(volatile uint8_t *)0x0028)
#define P2C1     (*(volatile uint8_t *)0x0029)
#define P2C2     (*(volatile uint8_t *)0x002A)
#define P2C3     (*(volatile uint8_t *)0x002B)
#define DPPH     (*(volatile uint8_t *)0x002C)
#define P3C1     (*(volatile uint8_t *)0x002D)
#define P3C2     (*(volatile uint8_t *)0x002E)
#define P3C3     (*(volatile uint8_t *)0x002F)
#define DPPL     (*(volatile uint8_t *)0x0030)
#define P4C1     (*(volatile uint8_t *)0x0031)
#define P4C2     (*(volatile uint8_t *)0x0032)
#define P4C3     (*(volatile uint8_t *)0x0033)
#define CHARBASE (*(volatile uint8_t *)0x0034)
#define P5C1     (*(volatile uint8_t *)0x0035)
#define P5C2     (*(volatile uint8_t *)0x0036)
#define P5C3     (*(volatile uint8_t *)0x0037)
#define OFFSET   (*(volatile uint8_t *)0x0038)
#define P6C1     (*(volatile uint8_t *)0x0039)
#define P6C2     (*(volatile uint8_t *)0x003A)
#define P6C3     (*(volatile uint8_t *)0x003B)
#define CTRL     (*(volatile uint8_t *)0x003C)
#define P7C1     (*(volatile uint8_t *)0x003D)
#define P7C2     (*(volatile uint8_t *)0x003E)
#define P7C3     (*(volatile uint8_t *)0x003F)

// POKEY Registers
#define POKEY_AUDC1 (*(volatile uint8_t *)0x0451)
#define POKEY_AUDC2 (*(volatile uint8_t *)0x0453)
#define POKEY_AUDC3 (*(volatile uint8_t *)0x0455)
#define POKEY_AUDC4 (*(volatile uint8_t *)0x0457)

// 7800basic System variables (mapped to exact memory locations)
#define sSWCHA        (*(volatile uint8_t *)0x2131)
#define sINPT1        (*(volatile uint8_t *)0x2102)
#define sINPT3        (*(volatile uint8_t *)0x2103)
#define visibleover   (*(volatile uint8_t *)0x004D)
#define nmi_enabled   (*(volatile uint8_t *)0x004E)
#define dlend         ((volatile uint8_t *)0x0065)
#define valbufend     (*(volatile uint8_t *)0x01AD)
#define charactermode (*(volatile uint8_t *)0x2106)
#define paldetected   (*(volatile uint8_t *)0x2109)

// Score variables (24-bit BCD, 3 bytes)
#define score0        ((volatile uint8_t *)0x01A6)
#define score1        ((volatile uint8_t *)0x01A9)

// 7800basic user-facing variables (defined as standard globals)
extern volatile uint16_t frame;
extern volatile uint8_t rand_val;
extern volatile uint8_t rand16_val;

// Input helpers (positive logic: 1 = active/pressed, 0 = inactive)
#define joy0up     (!(sSWCHA & 0x10))
#define joy0down   (!(sSWCHA & 0x20))
#define joy0left   (!(sSWCHA & 0x40))
#define joy0right  (!(sSWCHA & 0x80))
#define joy0fire0  (sINPT1 & 0x40) // Button 0 (left)
#define joy0fire1  (sINPT1 & 0x80) // Button 1 (right)
#define joy0fire   joy0fire1       // Default fire is fire1

#define switchreset  (!(SWCHB & 0x01))
#define switchselect (!(SWCHB & 0x02))
#define switchleftb  (SWCHB & 0x40)
#define switchrightb (SWCHB & 0x80)

// 7800basic API functions
void clearscreen(void);
void drawscreen(void);
void plotsprite(const uint8_t *data, uint8_t palette, uint8_t x, uint8_t y, uint8_t width);
void plotchars(const uint8_t *text, uint8_t palette, uint8_t x, uint8_t y, uint8_t num_chars);
void plotvalue(const uint8_t *font, uint8_t palette, const uint8_t *var_ptr, uint8_t num_digits, uint8_t x, uint8_t y);
void characterset(const uint8_t *font);

uint8_t get_random(void);
uint8_t converttobcd(uint8_t val);
void bcd_add_24(volatile uint8_t *dest, const uint8_t *src);

// SFX player
void playsfx(const uint8_t *sfx, uint8_t pitch);
void servicesfxchannels(void);

// POKEY VGM player
extern const uint8_t *volatile music_ptr;
extern const uint8_t *volatile music_loop_start;
extern volatile uint8_t music_active;
extern volatile uint8_t current_song;

void stop_music(void);
void play_music(void);
void play_music_tick(void);

extern const uint8_t Song_01_30hz_bin[];
extern const uint8_t Song_02_30hz_bin[];
extern const uint8_t Song_03_30hz_bin[];
extern const uint8_t Boss_30hz_bin[];

#endif
