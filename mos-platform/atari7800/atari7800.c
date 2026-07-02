#include "atari7800.h"
#include <stddef.h>

// Define 7800basic user variables
volatile uint16_t frame = 0;
volatile uint8_t rand_val = 1;
volatile uint8_t rand16_val = 0;

// Constants for display layout
#define ZONE_HEIGHT 16
#define ZONE_COUNT 12
#define ZONE0_ADDRESS 0x1880

// SFX player channels
struct sfx_channel {
    const uint8_t *ptr;
    uint8_t priority;
    uint8_t frames;
    uint8_t tick;
    uint8_t pitch_offset;
};

static struct sfx_channel sfx_channels[2];
static volatile uint8_t sfx_schedule_lock = 0;

// Music player state
volatile uint8_t music_active = 0;
const uint8_t *volatile music_ptr = NULL;
const uint8_t *volatile music_loop_start = NULL;
volatile uint8_t current_song = 1;
extern volatile uint8_t boss_state;

// Pre-defined DLL lookup table mapped in ROM, copied to 0x1800 at startup/clearscreen
static const uint8_t dll_lut[] = {
  // 25 blank lines before visible screen (for WSCREENHEIGHT = 192, WZONEHEIGHT = 16)
  0x4F, 0x21, 0x00, // 16 blank lines
  0x27, 0x21, 0x00, // 8 blank lines
  0x07, 0x21, 0x00, // 1 blank line

  // Visible zones (12 zones of 16 lines each)
  0xCF, 0x18, 0x80, // Zone 0 (NMI entry)
  0x4F, 0x19, 0x20, // Zone 1
  0x4F, 0x19, 0xC0, // Zone 2
  0x4F, 0x1A, 0x60, // Zone 3
  0x4F, 0x1B, 0x00, // Zone 4
  0x4F, 0x1B, 0xA0, // Zone 5
  0x4F, 0x1C, 0x40, // Zone 6
  0x4F, 0x1C, 0xCE, // Zone 7
  0x4F, 0x1D, 0x80, // Zone 8
  0x4F, 0x1E, 0x20, // Zone 9
  0x4F, 0x1E, 0xC0, // Zone 10
  0x4F, 0x1F, 0x60, // Zone 11

  // Overscan / non-visible lines (38 blank lines)
  0xC3, 0x21, 0x00, // 4 blank lines (NMI)
  0x8F, 0x21, 0x00, // 16 blank lines (NMI)
  0x0F, 0x21, 0x00, // 16 blank lines
  0x0F, 0x21, 0x00, // 16 blank lines
  0x0F, 0x21, 0x00  // 16 blank lines
};

void clearscreen(void) {
    // Copy DLL lookup table to 0x1800
    uint8_t *dll_dest = (uint8_t *)0x1800;
    for (uint8_t i = 0; i < sizeof(dll_lut); i++) {
        dll_dest[i] = dll_lut[i];
    }

    for (uint8_t i = 0; i < ZONE_COUNT; i++) {
        dlend[i] = 0;
    }
    valbufend = 0;
    
    // Enable Maria DMA (160A mode) and enable NMIs
    CTRL = 0x40;
    nmi_enabled = 0x80;
}

static void read_controllers(void) {
    // Save directional inputs from SWCHA
    sSWCHA = SWCHA;

    // Normalised fire button inputs: 1 = pressed, 0 = unpressed
    sINPT1 = 0;
    if (!(INPT4 & 0x80)) sINPT1 |= 0x80; // Button 1 (Left / single button)
    if (!(INPT0 & 0x80)) sINPT1 |= 0x40; // Button 0 (Right)

    sINPT3 = 0;
    if (!(INPT5 & 0x80)) sINPT3 |= 0x80; // Player 1 Button 1
    if (!(INPT2 & 0x80)) sINPT3 |= 0x40; // Player 1 Button 0
}

void drawscreen(void) {
    // Wait for the visible screen drawing to finish
    while (visibleover != 0);

    // Terminate display lists for all zones
    for (uint8_t i = 0; i < ZONE_COUNT; i++) {
        uint8_t *dl = (uint8_t *)(ZONE0_ADDRESS + i * 160);
        dl[dlend[i]] = 0x00; // terminating byte
    }

    // Enable interrupts
    __asm__ volatile ("cli");

    // Wait for the visible screen to start rendering (NMI will toggle visibleover to 255)
    while (visibleover == 0);

    // Update game frame tick
    frame++;

    // Update input state registers
    read_controllers();
}

void plotsprite(const uint8_t *data, uint8_t palette, uint8_t x, uint8_t y, uint8_t width) {
    uint8_t zone = y / ZONE_HEIGHT;
    if (zone >= ZONE_COUNT) return;

    uint8_t y_offset = y & (ZONE_HEIGHT - 1);
    uint16_t addr1 = (uint16_t)(uintptr_t)data + (y_offset * 256);
    uint8_t mode = 0x40; // 160A mode sprite

    // First zone entry
    uint8_t *dl1 = (uint8_t *)(ZONE0_ADDRESS + zone * 160);
    uint8_t idx1 = dlend[zone];
    if (idx1 < 155) {
        dl1[idx1++] = addr1 & 0xFF;
        dl1[idx1++] = mode;
        dl1[idx1++] = addr1 >> 8;
        dl1[idx1++] = (palette << 5) | ((0 - width) & 31);
        dl1[idx1++] = x;
        dlend[zone] = idx1;
    }

    // Split across zone boundary
    if (y_offset > 0 && (zone + 1) < ZONE_COUNT) {
        uint16_t addr2 = addr1 - (ZONE_HEIGHT * 256);
        uint8_t *dl2 = (uint8_t *)(ZONE0_ADDRESS + (zone + 1) * 160);
        uint8_t idx2 = dlend[zone + 1];
        if (idx2 < 155) {
            dl2[idx2++] = addr2 & 0xFF;
            dl2[idx2++] = mode;
            dl2[idx2++] = addr2 >> 8;
            dl2[idx2++] = (palette << 5) | ((0 - width) & 31);
            dl2[idx2++] = x;
            dlend[zone + 1] = idx2;
        }
    }
}

void plotchars(const uint8_t *text, uint8_t palette, uint8_t x, uint8_t y, uint8_t num_chars) {
    uint8_t zone = y;
    if (zone >= ZONE_COUNT) return;

    uint16_t addr = (uint16_t)(uintptr_t)text;
    uint8_t *dl = (uint8_t *)(ZONE0_ADDRESS + zone * 160);
    uint8_t idx = dlend[zone];
    if (idx < 155) {
        dl[idx++] = addr & 0xFF;
        dl[idx++] = charactermode;
        dl[idx++] = addr >> 8;
        dl[idx++] = (palette << 5) | ((0 - num_chars) & 31);
        dl[idx++] = x;
        dlend[zone] = idx;
    }
}

void plotvalue(const uint8_t *font, uint8_t palette, const uint8_t *var_ptr, uint8_t num_digits, uint8_t x, uint8_t y) {
    uint8_t *val_buf = (uint8_t *)(0x2000 + valbufend);
    uint8_t digits_left = num_digits;
    uint8_t *dest = val_buf;
    uint8_t src_idx = 0;

    // Odd digit count BCD skips the high nibble of first byte
    if (digits_left & 1) {
        uint8_t val = var_ptr[src_idx];
        uint8_t low = val & 0x0F;
        *dest++ = (uint8_t)(uintptr_t)font + low * 2;
        digits_left--;
        src_idx++;
    }

    while (digits_left > 0) {
        uint8_t val = var_ptr[src_idx++];
        
        uint8_t high = val >> 4;
        *dest++ = (uint8_t)(uintptr_t)font + high * 2;
        digits_left--;
        if (digits_left == 0) break;

        uint8_t low = val & 0x0F;
        *dest++ = (uint8_t)(uintptr_t)font + low * 2;
        digits_left--;
    }

    plotchars(val_buf, palette, x, y, num_digits);
    valbufend += num_digits;
}

void characterset(const uint8_t *font) {
    CHARBASE = (uint16_t)(uintptr_t)font >> 8;
}

uint8_t get_random(void) {
    uint8_t carry = rand_val & 1;
    rand_val >>= 1;
    if (carry) {
        rand16_val = (rand16_val << 1) | 1;
        rand_val ^= 0xB4;
    } else {
        rand16_val = (rand16_val << 1);
    }
    return rand_val ^ rand16_val;
}

uint8_t converttobcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

void bcd_add_24(volatile uint8_t *dest, const uint8_t *src) {
    __asm__ volatile (
        "sed\n"
        "clc\n"
        "lda %0\n"
        "adc %3\n"
        "sta %0\n"
        "lda %1\n"
        "adc %4\n"
        "sta %1\n"
        "lda %2\n"
        "adc %5\n"
        "sta %2\n"
        "cld\n"
        : "+r"(dest[0]), "+r"(dest[1]), "+r"(dest[2])
        : "r"(src[0]), "r"(src[1]), "r"(src[2])
    );
}

void playsfx(const uint8_t *sfx, uint8_t pitch) {
    if (!sfx) return;
    uint8_t priority = sfx[1];

    sfx_schedule_lock = 1;

    uint8_t chan = 0;
    if (sfx_channels[0].ptr == NULL) {
        chan = 0;
    } else if (sfx_channels[1].ptr == NULL) {
        chan = 1;
    } else if (sfx_channels[0].priority < sfx_channels[1].priority) {
        chan = 0;
    } else if (sfx_channels[1].priority < sfx_channels[0].priority) {
        chan = 1;
    } else {
        chan = 0;
    }

    if (sfx_channels[chan].ptr == NULL || priority >= sfx_channels[chan].priority) {
        sfx_channels[chan].ptr = sfx + 3; // skip version, priority, frames_per_chunk
        sfx_channels[chan].priority = priority;
        sfx_channels[chan].frames = sfx[2];
        sfx_channels[chan].tick = 0;
        sfx_channels[chan].pitch_offset = pitch;
    }

    sfx_schedule_lock = 0;
}

void servicesfxchannels(void) {
    if (sfx_schedule_lock) return;

    for (uint8_t chan = 0; chan < 2; chan++) {
        struct sfx_channel *c = &sfx_channels[chan];
        if (c->ptr == NULL) continue;

        if (c->tick > 0) {
            c->tick--;
            continue;
        }

        uint8_t freq = c->ptr[0];
        uint8_t ctrl = c->ptr[1];
        uint8_t vol  = c->ptr[2];

        if (freq == 0 && ctrl == 0 && vol == 0) {
            c->ptr = NULL;
            c->priority = 0;
            if (chan == 0) {
                AUDV0 = 0;
            } else {
                AUDV1 = 0;
            }
            continue;
        }

        if (chan == 0) {
            AUDF0 = freq + c->pitch_offset;
            AUDC0 = ctrl;
            AUDV0 = vol;
        } else {
            AUDF1 = freq + c->pitch_offset;
            AUDC1 = ctrl;
            AUDV1 = vol;
        }

        c->tick = c->frames - 1;
        c->ptr += 3;
        if (c->priority > 0) c->priority--;
    }
}

void stop_music(void) {
    music_active = 0;
    POKEY_AUDC1 = 0;
    POKEY_AUDC2 = 0;
    POKEY_AUDC3 = 0;
    POKEY_AUDC4 = 0;
}

void play_music(void) {
    if (music_ptr == NULL) return;

    uint8_t idx = 0;
    while (1) {
        uint8_t reg = music_ptr[idx++];
        if (reg == 0xFF) {
            // End of frame
            music_ptr += idx;
            break;
        }
        if (reg == 0xFE) {
            // End of song -> Loop!
            music_ptr = music_loop_start;
            break;
        }

        uint8_t val = music_ptr[idx++];
        volatile uint8_t *pokey = (volatile uint8_t *)0x0450;
        pokey[reg] = val;

        if (idx >= 64) {
            // Safety limit
            music_ptr += idx;
            break;
        }
    }
}

void play_music_tick(void) {
    static uint8_t music_divider = 0;
    music_divider ^= 1;
    if (music_divider == 0 && music_active) {
        play_music();
    }
}
