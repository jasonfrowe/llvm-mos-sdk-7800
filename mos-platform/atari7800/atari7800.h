// Copyright 2026 LLVM-MOS Project
// Licensed under the Apache License, Version 2.0 with LLVM Exceptions.

#ifndef _ATARI7800_H
#define _ATARI7800_H

#include <stdint.h>

#define ATARI7800_RAM_START 0x1800u
#define ATARI7800_RAM_END   0x27ffu

#define ATARI7800_MMIO8(addr) (*(volatile uint8_t *)(uintptr_t)(addr))

/* TIA / input control region. */
#define ATARI7800_REG_INPTCTRL 0x0001u
#define ATARI7800_REG_INPT0    0x0008u
#define ATARI7800_REG_INPT1    0x0009u
#define ATARI7800_REG_INPT2    0x000au
#define ATARI7800_REG_INPT3    0x000bu
#define ATARI7800_REG_INPT4    0x000cu
#define ATARI7800_REG_INPT5    0x000du

/* MARIA region. */
#define ATARI7800_REG_BACKGRND 0x0020u
#define ATARI7800_REG_P0C1     0x0021u
#define ATARI7800_REG_P0C2     0x0022u
#define ATARI7800_REG_P0C3     0x0023u
#define ATARI7800_REG_WSYNC    0x0024u
#define ATARI7800_REG_P1C1     0x0025u
#define ATARI7800_REG_P1C2     0x0026u
#define ATARI7800_REG_P1C3     0x0027u
#define ATARI7800_REG_MSTAT    0x0028u
#define ATARI7800_REG_DPPH     0x002cu
#define ATARI7800_REG_DPPL     0x0030u
#define ATARI7800_REG_CHARBASE 0x0034u
#define ATARI7800_REG_OFFSET   0x0038u
#define ATARI7800_REG_CTRL     0x003cu

/* RIOT I/O region. */
#define ATARI7800_REG_SWCHA    0x0280u
#define ATARI7800_REG_SWCHB    0x0282u

#define ATARI7800_INPTCTRL ATARI7800_MMIO8(ATARI7800_REG_INPTCTRL)
#define ATARI7800_INPT0    ATARI7800_MMIO8(ATARI7800_REG_INPT0)
#define ATARI7800_INPT1    ATARI7800_MMIO8(ATARI7800_REG_INPT1)
#define ATARI7800_INPT2    ATARI7800_MMIO8(ATARI7800_REG_INPT2)
#define ATARI7800_INPT3    ATARI7800_MMIO8(ATARI7800_REG_INPT3)
#define ATARI7800_INPT4    ATARI7800_MMIO8(ATARI7800_REG_INPT4)
#define ATARI7800_INPT5    ATARI7800_MMIO8(ATARI7800_REG_INPT5)
#define ATARI7800_BACKGRND ATARI7800_MMIO8(ATARI7800_REG_BACKGRND)
#define ATARI7800_P0C1     ATARI7800_MMIO8(ATARI7800_REG_P0C1)
#define ATARI7800_P0C2     ATARI7800_MMIO8(ATARI7800_REG_P0C2)
#define ATARI7800_P0C3     ATARI7800_MMIO8(ATARI7800_REG_P0C3)
#define ATARI7800_WSYNC    ATARI7800_MMIO8(ATARI7800_REG_WSYNC)
#define ATARI7800_MSTAT    ATARI7800_MMIO8(ATARI7800_REG_MSTAT)
#define ATARI7800_DPPH     ATARI7800_MMIO8(ATARI7800_REG_DPPH)
#define ATARI7800_DPPL     ATARI7800_MMIO8(ATARI7800_REG_DPPL)
#define ATARI7800_CHARBASE ATARI7800_MMIO8(ATARI7800_REG_CHARBASE)
#define ATARI7800_OFFSET   ATARI7800_MMIO8(ATARI7800_REG_OFFSET)
#define ATARI7800_CTRL     ATARI7800_MMIO8(ATARI7800_REG_CTRL)
#define ATARI7800_SWCHA    ATARI7800_MMIO8(ATARI7800_REG_SWCHA)
#define ATARI7800_SWCHB    ATARI7800_MMIO8(ATARI7800_REG_SWCHB)

/* SWCHA direction bits are active-low. */
#define ATARI7800_SWCHA_RIGHT 0x80u
#define ATARI7800_SWCHA_LEFT  0x40u
#define ATARI7800_SWCHA_DOWN  0x20u
#define ATARI7800_SWCHA_UP    0x10u

/* CTRL register bits/modes. */
#define ATARI7800_CTRL_MODE_160A 0x00u
#define ATARI7800_CTRL_MODE_160B 0x80u
#define ATARI7800_CTRL_MODE_320A 0x01u
#define ATARI7800_CTRL_MODE_320B 0x02u
#define ATARI7800_CTRL_MODE_320C 0x03u
#define ATARI7800_CTRL_DMA_ENABLE 0x40u
#define ATARI7800_BG_DARKGRAY 0x02u

#ifndef ATARI7800_ZONE_HEIGHT
#define ATARI7800_ZONE_HEIGHT 8
#endif

#if ATARI7800_ZONE_HEIGHT == 16
#define ATARI7800_ZONE_SHIFT 4
#define ATARI7800_ZONE_OFFSET 15u
#define ATARI7800_SCENE_VISIBLE_ZONES 14u
#define ATARI7800_MARIA_NTSC_DLL_ENTRIES 19u
#elif ATARI7800_ZONE_HEIGHT == 8
#define ATARI7800_ZONE_SHIFT 3
#define ATARI7800_ZONE_OFFSET 7u
#define ATARI7800_SCENE_VISIBLE_ZONES 28u
#define ATARI7800_MARIA_NTSC_DLL_ENTRIES 33u
#else
#error "Unsupported zone height"
#endif

/* Display list entry flags. */
#define ATARI7800_DLL_FLAG_NMI 0x80u

typedef struct __attribute__((packed)) atari7800_maria_dll_entry {
	uint8_t offset;
	uint8_t zone_addr_hi;
	uint8_t zone_addr_lo;
} atari7800_maria_dll_entry_t;

typedef struct __attribute__((packed)) atari7800_maria_null_header {
	uint8_t offset;
	uint8_t zero;
} atari7800_maria_null_header_t;

typedef struct __attribute__((packed)) atari7800_palette3 {
	uint8_t c1;
	uint8_t c2;
	uint8_t c3;
} atari7800_palette3_t;

#define ATARI7800_SPRITE_LAYOUT_MARIA_STRIDED 0u
#define ATARI7800_SPRITE_LAYOUT_CONTIGUOUS_160A 1u

/* Explicit status codes returned by the sprite/text/glyph-run draw calls
 * below, in place of a plain success/fail boolean (note the flipped
 * convention vs. that older boolean: 0 now means success). Callers that
 * need to react differently to an over-budget zone/buffer (e.g. stop
 * trying to add more objects to it this frame) versus a programming
 * error (null pointers, empty font) can compare against these directly
 * instead of only knowing that "something" failed. */
#define ATARI7800_OK              0u
#define ATARI7800_ERR_INVALID     1u
#define ATARI7800_ERR_BUDGET_FULL 2u

typedef struct atari7800_sprite_asset {
	const uint8_t *data;
	uint8_t width_bytes;
	uint8_t height_lines;
	uint8_t mode;
	uint8_t palette;
	uint8_t width_twos_comp;
	uint8_t data_layout;
} atari7800_sprite_asset_t;

typedef struct atari7800_font_descriptor {
	const uint8_t *data;
	const uint8_t *char_to_glyph;
	uint8_t char_to_glyph_len;
	uint8_t first_char;
	uint8_t glyph_count;
	uint8_t glyph_width_bytes;
	uint8_t glyph_height_lines;
	uint8_t glyph_mode;
	uint8_t glyph_palette;
	uint8_t glyph_width_twos_comp;
	uint8_t glyph_advance;
	uint8_t space_advance;
	uint8_t line_advance;
} atari7800_font_descriptor_t;

typedef struct atari7800_scene {
	uint8_t *zone;
	uint16_t zone_size;
	uint8_t next_object;
	uint8_t initialized;
} atari7800_scene_t;

/* A persistent sprite object: retains its assigned zone and slot across
 * frames so re-drawing it (typically every frame, at a new position)
 * patches bytes in an existing header in place instead of re-deriving
 * zone membership and appending a fresh one every time, the way
 * atari7800_scene_draw_sprite does. Loosely adapted from 7800basic's
 * object model (each sprite instance owns a fixed slot), but the slot
 * number here is local to whichever zone the object currently occupies,
 * not a single number reserved across every zone: our display lists are
 * compact-plus-terminator rather than always-fully-populated, so a
 * cross-zone "reserved everywhere" slot would leave stale bytes in the
 * gap between non-adjacent occupied slots in a shared zone. Moving to a
 * new zone parks the old slot off screen rather than reclaiming it (see
 * atari7800_scene_sprite), so a zone's slot usage only ever grows, even
 * if live occupancy shrinks -- fine for a modest, mostly-stable object
 * count, worth knowing about for a scene with heavy zone-crossing churn.
 *
 * Declare one per logical sprite instance (a loop variable/array element
 * is fine for interchangeable sprites, e.g. a star field), initialized
 * with ATARI7800_SCENE_OBJECT_INIT, and pass it to
 * atari7800_scene_sprite() every frame. */
typedef struct atari7800_scene_object {
	uint8_t zone_index;
	uint8_t slot;
} atari7800_scene_object_t;

#define ATARI7800_SCENE_OBJECT_UNPLACED 0xffu
#define ATARI7800_SCENE_OBJECT_INIT { ATARI7800_SCENE_OBJECT_UNPLACED, 0u }

/* One pre-resolved glyph in a glyph run: which glyph to draw and its pen_x
 * offset from the run's draw-time x_pos. glyph_index of
 * ATARI7800_GLYPH_RUN_BLANK marks a skipped cell (originally whitespace),
 * which costs zero object slots when drawn. */
#define ATARI7800_GLYPH_RUN_BLANK 0xffu

typedef struct atari7800_glyph_run_entry {
	uint8_t glyph_index;
	uint8_t x_offset;
} atari7800_glyph_run_entry_t;

/* A single-line run of glyphs pre-resolved once (e.g. at startup or whenever
 * the text changes), so per-frame HUD redraws skip character-to-glyph lookup
 * and whitespace/newline branching entirely. Build with
 * atari7800_build_glyph_run(); draw every frame with
 * atari7800_scene_draw_glyph_run(). */
typedef struct atari7800_glyph_run {
	const atari7800_glyph_run_entry_t *entries;
	uint8_t count;
} atari7800_glyph_run_t;

#define ATARI7800_MARIA_ZONE5_OBJECT_BYTES 5u
#define ATARI7800_MARIA_ZONE5_TERMINATOR_BYTES 2u
#define ATARI7800_SCENE_ZONE_BYTES 64u

static inline uint16_t atari7800_ptr16(const void *ptr) {
	return (uint16_t)(uintptr_t)ptr;
}

static inline void atari7800_maria_init_null_header(
		atari7800_maria_null_header_t *header, uint8_t offset) {
	header->offset = offset;
	header->zero = 0;
}

static inline void atari7800_maria_init_dll_entry(
		atari7800_maria_dll_entry_t *entry, uint8_t line_offset,
		const void *zone_header, uint8_t flags) {
	const uint16_t zone = atari7800_ptr16(zone_header);
	entry->offset = (uint8_t)((line_offset & 0x7fu) | (flags & ATARI7800_DLL_FLAG_NMI));
	entry->zone_addr_hi = (uint8_t)(zone >> 8);
	entry->zone_addr_lo = (uint8_t)(zone & 0xffu);
}

static inline void atari7800_set_display_list(uint16_t addr) {
	ATARI7800_DPPL = (uint8_t)(addr & 0xffu);
	ATARI7800_DPPH = (uint8_t)(addr >> 8);
}

static inline void atari7800_wait_hsync(void) {
	ATARI7800_WSYNC = 0;
}

/* Equivalent to the palette bits used by 7800basic plotsprite pal|width byte. */
static inline uint8_t atari7800_maria_pal_width(uint8_t palette,
		uint8_t width_twos_comp) {
	return (uint8_t)(((palette & 0x07u) << 5) | (width_twos_comp & 0x1fu));
}

void atari7800_init_system(void);
void atari7800_wait_vblank(void);
void atari7800_configure_video(uint16_t display_list_addr, uint8_t ctrl, uint8_t bgcolor);
void atari7800_init_160a(uint16_t display_list_addr, uint8_t bgcolor);
void atari7800_set_palette3(uint8_t palette_index, atari7800_palette3_t colors);
void atari7800_maria_clear_zone(uint8_t *zone, uint16_t zone_size);
/* Returns ATARI7800_OK, or ATARI7800_ERR_BUDGET_FULL if the object plus its
 * terminator bytes would not fit in zone_size. */
uint8_t atari7800_maria_plot_sprite_zone5(uint8_t *zone, uint16_t zone_size,
		uint8_t object_index, uint16_t sprite_addr, uint8_t mode,
		uint8_t palette, uint8_t width_twos_comp, uint8_t x_pos);
/* Returns ATARI7800_OK, ATARI7800_ERR_INVALID (null/empty asset), or
 * ATARI7800_ERR_BUDGET_FULL (see atari7800_maria_plot_sprite_zone5). */
uint8_t atari7800_maria_plot_sprite_asset_zone5(uint8_t *zone,
		uint16_t zone_size, uint8_t object_index,
		const atari7800_sprite_asset_t *asset, uint8_t x_pos);
/* Rewrites an already-placed object header in place (5 bytes only -- does
 * NOT zero the 2 bytes after it, unlike atari7800_maria_plot_sprite_zone5).
 * Use only for a slot that was already placed with the plot function above
 * (so its terminator/next-slot bytes are already correct); using this for
 * a slot's first placement would leave the list unterminated. Returns
 * ATARI7800_OK or ATARI7800_ERR_BUDGET_FULL. */
uint8_t atari7800_maria_patch_sprite_zone5(uint8_t *zone, uint16_t zone_size,
		uint8_t object_index, uint16_t sprite_addr, uint8_t mode,
		uint8_t palette, uint8_t width_twos_comp, uint8_t x_pos);
/* Resolves a sprite asset descriptor and invokes
 * atari7800_maria_patch_sprite_zone5. Returns ATARI7800_OK,
 * ATARI7800_ERR_INVALID (null/empty asset), or ATARI7800_ERR_BUDGET_FULL. */
uint8_t atari7800_maria_patch_sprite_asset_zone5(uint8_t *zone,
		uint16_t zone_size, uint8_t object_index,
		const atari7800_sprite_asset_t *asset, uint8_t x_pos);

/* 7800basic-style convenience wrappers. */
static inline void atari7800_clearscreen(uint8_t *zone, uint16_t zone_size) {
	atari7800_maria_clear_zone(zone, zone_size);
}

static inline uint8_t atari7800_plotsprite_asset(
		uint8_t *zone, uint16_t zone_size, uint8_t object_index,
		const atari7800_sprite_asset_t *asset, uint8_t x_pos) {
	return atari7800_maria_plot_sprite_asset_zone5(zone, zone_size,
			object_index, asset, x_pos);
}

/* atari7800.c defines ATARI7800_NO_SCENE_IMPL before including this header:
 * it implements a few zone-height-INDEPENDENT functions declared above (and
 * needs the types/macros above for that), but must not also pull in the
 * zone-height-dependent block below -- every one of its own callers already
 * gets a private copy of that block from including this header themselves,
 * and atari7800.c is unconditionally linked into every program via
 * libc.a, so if it also defined `nmi` (a real global symbol, see below) that
 * would collide with the copy in every single program that links it. */
#ifndef ATARI7800_NO_SCENE_IMPL

/* ===========================================================================
 * Zone-height-dependent scene state and functions.
 *
 * WHY THESE ARE DEFINED HERE (in the header) INSTEAD OF atari7800.c:
 * ATARI7800_ZONE_HEIGHT is meant to be set per-program, by #define-ing it
 * before #include <atari7800.h> (see astrowing.c). That only works for code
 * that is compiled fresh into EACH program's own translation unit. It does
 * NOT work for code compiled once into the prebuilt mos-platform static
 * library (what atari7800.c used to be, and what a plain `void foo(...);`
 * declaration here + a definition in atari7800.c produces): that library is
 * built exactly once, during SDK bootstrap, using whatever ATARI7800_ZONE_HEIGHT
 * the header defaults to (8) -- long before any program's own #define could
 * possibly affect it. Every function below that touches zone geometry (the
 * zone buffers, the DLL, ATARI7800_ZONE_SHIFT/ATARI7800_SCENE_VISIBLE_ZONES)
 * was silently always built for 8-line zones this way, regardless of what
 * astrowing.c defined, for as long as ATARI7800_ZONE_HEIGHT has existed --
 * two halves of the same program disagreeing about where zone boundaries
 * are, which is what actually caused this session's "objects on the same
 * row corrupt/disappear" bugs (not a MARIA hardware limitation).
 *
 * Making these `static` (data) / `static inline` (functions), defined
 * directly in the header, fixes this: each translation unit that includes
 * this header gets its own private copy, compiled with THAT unit's own
 * ATARI7800_ZONE_HEIGHT. The tradeoff: use the scene API (any function or
 * variable whose name starts with atari7800_scene_, plus
 * atari7800_maria_build_blank_ntsc) from exactly ONE .c file per program.
 * Two .c files in the same program both including
 * this header would each get a separate, disconnected copy of the zone
 * state, and defining `nmi` (a real global symbol, needed so the linker's
 * weak-vector-override mechanism in link.ld can find it) in more than one
 * of them would be a duplicate-symbol link error. Every atari7800 example
 * in this SDK is a single .c file, so this is not a new constraint in
 * practice, just one worth knowing about.
 * ===========================================================================
 */

/* Null zone display list header used to terminate empty zones. */
static atari7800_maria_null_header_t atari7800_scene_null_zone;

/* The Display List List (DLL) pointing to all zones on the screen. */
static atari7800_maria_dll_entry_t
    atari7800_scene_display_list[ATARI7800_MARIA_NTSC_DLL_ENTRIES];

/* Low-RAM and High-RAM display list zone buffers split to bypass physical memory shadow pages. */
static uint8_t atari7800_scene_zones_low[ATARI7800_SCENE_VISIBLE_ZONES / 2][ATARI7800_SCENE_ZONE_BYTES]
    __attribute__((section(".scene_zones_low")));
static uint8_t atari7800_scene_zones_high[ATARI7800_SCENE_VISIBLE_ZONES / 2][ATARI7800_SCENE_ZONE_BYTES]
    __attribute__((section(".scene_zones_high")));

/* Read-only lookup table to access zone pointers by index. */
static uint8_t * const atari7800_scene_zones[ATARI7800_SCENE_VISIBLE_ZONES] = {
	&atari7800_scene_zones_low[0][0],
	&atari7800_scene_zones_low[1][0],
	&atari7800_scene_zones_low[2][0],
	&atari7800_scene_zones_low[3][0],
	&atari7800_scene_zones_low[4][0],
	&atari7800_scene_zones_low[5][0],
	&atari7800_scene_zones_low[6][0],
#if ATARI7800_ZONE_HEIGHT == 8
	&atari7800_scene_zones_low[7][0],
	&atari7800_scene_zones_low[8][0],
	&atari7800_scene_zones_low[9][0],
	&atari7800_scene_zones_low[10][0],
	&atari7800_scene_zones_low[11][0],
	&atari7800_scene_zones_low[12][0],
	&atari7800_scene_zones_low[13][0],
#endif
	&atari7800_scene_zones_high[0][0],
	&atari7800_scene_zones_high[1][0],
	&atari7800_scene_zones_high[2][0],
	&atari7800_scene_zones_high[3][0],
	&atari7800_scene_zones_high[4][0],
	&atari7800_scene_zones_high[5][0],
	&atari7800_scene_zones_high[6][0],
#if ATARI7800_ZONE_HEIGHT == 8
	&atari7800_scene_zones_high[7][0],
	&atari7800_scene_zones_high[8][0],
	&atari7800_scene_zones_high[9][0],
	&atari7800_scene_zones_high[10][0],
	&atari7800_scene_zones_high[11][0],
	&atari7800_scene_zones_high[12][0],
	&atari7800_scene_zones_high[13][0],
#endif
};

/* Tracker for the next available object slot index inside each zone display list. */
static uint8_t atari7800_scene_zone_next_object[ATARI7800_SCENE_VISIBLE_ZONES];

/* Tracker for active zones in the previous frame, used to identify and clean up inactive zones. */
static uint8_t atari7800_scene_active_zones_prev[ATARI7800_SCENE_VISIBLE_ZONES];

/* Tracker for active zones in the current frame. */
static uint8_t atari7800_scene_active_zones_curr[ATARI7800_SCENE_VISIBLE_ZONES];

/* Per-zone "static residency" flag (see atari7800_scene_set_zone_static):
 * begin_frame/end_frame skip these zones entirely instead of
 * resetting/wiping them every frame, so content that rarely changes (e.g.
 * HUD labels) costs nothing once drawn. One byte per zone rather than a
 * packed bitmask: this is read in the per-frame begin_frame loop, and a
 * runtime-variable bit shift (needed to test/set an arbitrary bit in a
 * mask) has no single-instruction form on 6502, so it was actually costing
 * far more in CPU cycles than the RAM it saved -- measured at ~3,955
 * cycles/frame in begin_frame alone via a frame-budget debug marker, more
 * than the sprite draw calls it was gating. */
static uint8_t atari7800_scene_zone_static[ATARI7800_SCENE_VISIBLE_ZONES];

/* Count of persistent objects (see atari7800_scene_object_t) currently
 * packed, compactly from slot 0, in each zone's object list. Separate
 * from atari7800_scene_zone_next_object, which is reset every frame by
 * atari7800_scene_begin_frame for the older append-and-diff API -- mixing
 * the two APIs for objects that could land in the same zone is not
 * supported, since both would think they own the zone's byte offset 0. */
static uint8_t atari7800_scene_zone_persist_count[ATARI7800_SCENE_VISIBLE_ZONES];

/* Toggled by the NMI handler installed by atari7800_scene_enable_nmi_sync:
 * nonzero while MARIA is scanning the visible display area, zero once it
 * reaches the bottom blank entries. Only meaningful once that function
 * has been called; otherwise NMI never fires (no DLL entry requests it)
 * and this just stays zero forever. */
static volatile uint8_t atari7800_scene_display_busy;

/**
 * Populates a blank Display List List (DLL) for NTSC display timing.
 * Reserves top, visible screen zones, and bottom blank boundaries.
 */
static inline void atari7800_maria_build_blank_ntsc(
    atari7800_maria_dll_entry_t *display_list,
    const atari7800_maria_null_header_t *zone_header) {
	uint8_t i;

	atari7800_maria_init_dll_entry(&display_list[0], 8, zone_header, 0);

#if ATARI7800_ZONE_HEIGHT == 16
	for (i = 0; i < 14; ++i) {
		atari7800_maria_init_dll_entry(&display_list[1u + i], ATARI7800_ZONE_OFFSET,
		                               zone_header, 0);
	}
	atari7800_maria_init_dll_entry(&display_list[15], 15, zone_header, 0);
	atari7800_maria_init_dll_entry(&display_list[16], 9, zone_header, 0);
	atari7800_maria_init_dll_entry(&display_list[17], 15, zone_header, 0);
	atari7800_maria_init_dll_entry(&display_list[18], 8, zone_header, 0);
#else
	for (i = 0; i < 28; ++i) {
		atari7800_maria_init_dll_entry(&display_list[1u + i], ATARI7800_ZONE_OFFSET,
		                               zone_header, 0);
	}
	atari7800_maria_init_dll_entry(&display_list[29], 15, zone_header, 0);
	atari7800_maria_init_dll_entry(&display_list[30], 9, zone_header, 0);
	atari7800_maria_init_dll_entry(&display_list[31], 15, zone_header, 0);
	atari7800_maria_init_dll_entry(&display_list[32], 8, zone_header, 0);
#endif
}

/**
 * Resets per-zone object cursors at the beginning of a frame. Zones pinned
 * static (see atari7800_scene_set_zone_static) are skipped entirely, so
 * their object cursor and display-list entry carry over untouched.
 */
static inline void atari7800_scene_begin_frame(atari7800_scene_t *scene) {
	(void)scene;

	for (uint8_t z_idx = 0; z_idx < ATARI7800_SCENE_VISIBLE_ZONES; ++z_idx) {
		if (atari7800_scene_zone_static[z_idx] != 0u) {
			continue;
		}
		atari7800_scene_active_zones_curr[z_idx] = 0u;
		atari7800_scene_zone_next_object[z_idx] = 0u;
	}
}

/**
 * Clears display lists of zones that were active in the previous frame
 * but went unused in the current frame, preventing flickering or trail artifacts.
 */
static inline void atari7800_scene_end_frame(atari7800_scene_t *scene) {
	uint8_t zone_index;
	(void)scene;

	for (zone_index = 0; zone_index < ATARI7800_SCENE_VISIBLE_ZONES;
	     ++zone_index) {
		if (atari7800_scene_active_zones_prev[zone_index] != 0u &&
		    atari7800_scene_active_zones_curr[zone_index] == 0u) {
			atari7800_maria_clear_zone(atari7800_scene_zones[zone_index],
			                           ATARI7800_SCENE_ZONE_BYTES);
			atari7800_maria_init_dll_entry(
			    &atari7800_scene_display_list[(uint8_t)(1u + zone_index)], ATARI7800_ZONE_OFFSET,
			    &atari7800_scene_null_zone, 0u);
		}
		atari7800_scene_active_zones_prev[zone_index] =
		    atari7800_scene_active_zones_curr[zone_index];
	}
}

/**
 * Pins (is_static != 0) or unpins (is_static == 0) the zone containing
 * y_pos as static residency. While pinned, atari7800_scene_begin_frame
 * leaves the zone's object cursor and display-list entry untouched and
 * atari7800_scene_end_frame never wipes it for going unused, so its
 * last-drawn content persists every frame at zero per-frame cost. Draw
 * into the zone as normal, then pin it once drawing is done; unpinning
 * does not redraw or clear anything by itself, it just lets the next
 * atari7800_scene_begin_frame resume resetting the zone as normal so new
 * content can be drawn into it.
 */
static inline void atari7800_scene_set_zone_static(atari7800_scene_t *scene, uint8_t y_pos,
                                     uint8_t is_static) {
	uint8_t zone_index;
	(void)scene;

	zone_index = (uint8_t)(y_pos >> ATARI7800_ZONE_SHIFT);
	if (zone_index >= ATARI7800_SCENE_VISIBLE_ZONES) {
		zone_index = (uint8_t)(ATARI7800_SCENE_VISIBLE_ZONES - 1u);
	}

	atari7800_scene_zone_static[zone_index] = is_static;

	if (is_static != 0u) {
		atari7800_scene_active_zones_curr[zone_index] = 1u;
		atari7800_scene_active_zones_prev[zone_index] = 1u;
	}
}

/**
 * Initializes the stateful scene manager, building the DLL structure
 * and clearing the low/high zone buffers.
 */
static inline void atari7800_scene_init_160a(atari7800_scene_t *scene, uint8_t bgcolor) {
	uint8_t zone_index;

	atari7800_maria_init_null_header(&atari7800_scene_null_zone, 0u);
	atari7800_maria_build_blank_ntsc(atari7800_scene_display_list,
	                                 &atari7800_scene_null_zone);
	for (zone_index = 0; zone_index < ATARI7800_SCENE_VISIBLE_ZONES; ++zone_index) {
		atari7800_scene_active_zones_prev[zone_index] = 0u;
		atari7800_scene_active_zones_curr[zone_index] = 0u;
		atari7800_scene_zone_static[zone_index] = 0u;
		atari7800_scene_zone_persist_count[zone_index] = 0u;
	}

	for (zone_index = 0; zone_index < ATARI7800_SCENE_VISIBLE_ZONES;
	     ++zone_index) {
		atari7800_scene_zone_next_object[zone_index] = 0u;
		atari7800_maria_clear_zone(atari7800_scene_zones[zone_index],
		                           ATARI7800_SCENE_ZONE_BYTES);
	}

	atari7800_scene_begin_frame(scene);
	scene->initialized = 1u;
	atari7800_init_160a(atari7800_ptr16(atari7800_scene_display_list), bgcolor);
}

/**
 * NMI handler for atari7800_scene_enable_nmi_sync: fires once MARIA
 * reaches the top blank template entry (start of visible area) and once
 * more at the bottom blank template entry (end of visible area). Since
 * the CPU can't tell which of the two fired, a plain toggle works because
 * they're the only two NMI-flagged entries and always fire in that fixed
 * order every frame. Kept to a single instruction's worth of real work so
 * it costs as little as possible wherever it preempts.
 *
 * Deliberately NOT static: link.ld overrides the weak default NMI vector
 * (PROVIDE(nmi = _atari7800_irq_default)) with whatever global symbol
 * named `nmi` the program defines, so this needs real external linkage --
 * see the file-level comment above for why that's safe given this header's
 * one-.c-file-per-program usage pattern. A program that never calls
 * atari7800_scene_enable_nmi_sync still gets this handler installed
 * (harmless: it only fires if some DLL entry sets ATARI7800_DLL_FLAG_NMI,
 * which only that function ever does).
 */
__attribute__((interrupt)) void nmi(void) {
	atari7800_scene_display_busy ^= 1u;
}

/**
 * Opt-in: flags the DLL's top and bottom blank template entries (never
 * touched by per-frame zone activity, unlike the visible-zone entries)
 * with ATARI7800_DLL_FLAG_NMI and installs an NMI handler that toggles a
 * "display busy" flag on each one. This is what makes
 * atari7800_scene_wait_active_start()/wait_display_safe() meaningful --
 * without calling this first, the busy flag never changes and those two
 * calls will hang forever. Call once after atari7800_scene_init_160a.
 *
 * Intended loop shape, so non-drawing logic overlaps active-display DMA
 * time instead of being serialized before it the way a fully-polled loop
 * (atari7800_wait_vblank() at the top, then everything) forces:
 *
 *   atari7800_scene_enable_nmi_sync(&scene);  // once
 *   for (;;) {
 *     atari7800_scene_wait_active_start();    // sync to a fresh frame
 *     update_input(); update_physics(); ...   // overlaps active display
 *     atari7800_scene_wait_display_safe();    // wait for it to finish
 *     draw_sprite(...); ...                   // only now touch the DLL
 *   }
 */
static inline void atari7800_scene_enable_nmi_sync(atari7800_scene_t *scene) {
	(void)scene;

	atari7800_scene_display_busy = 0u;

	/* Re-flag the fixed top/bottom blank entries that
	 * atari7800_maria_build_blank_ntsc already set up (same line_offset
	 * values, same index formula, for both zone heights), adding
	 * ATARI7800_DLL_FLAG_NMI without disturbing their timing. */
	atari7800_maria_init_dll_entry(&atari7800_scene_display_list[0], 8u,
	    &atari7800_scene_null_zone, ATARI7800_DLL_FLAG_NMI);
	atari7800_maria_init_dll_entry(
	    &atari7800_scene_display_list[(uint8_t)(ATARI7800_SCENE_VISIBLE_ZONES + 1u)],
	    15u, &atari7800_scene_null_zone, ATARI7800_DLL_FLAG_NMI);

	atari7800_wait_vblank();
}

/**
 * Blocks until MARIA has just started scanning the visible display area
 * for a new frame (see atari7800_scene_enable_nmi_sync and the loop shape
 * in its doc comment). Call this once per loop, before any non-drawing
 * game logic. Edge-triggered like atari7800_wait_vblank (waits for busy to
 * clear if currently set, then waits for a *fresh* transition to busy)
 * rather than "return if already busy": that guarantees whatever runs
 * right after this call gets the (near-)full active-display window to
 * overlap with, instead of only whatever happened to be left of a window
 * that was already partway through.
 */
static inline void atari7800_scene_wait_active_start(void) {
	while (atari7800_scene_display_busy != 0u) {
	}
	while (atari7800_scene_display_busy == 0u) {
	}
}

/**
 * Blocks until MARIA has finished scanning the current frame's visible
 * area -- call this right before any draw calls that touch zone buffers
 * or the DLL, after atari7800_scene_wait_active_start() and any logic.
 * Level-triggered (just waits while busy) is correct here, unlike
 * atari7800_scene_wait_active_start: this is meant to be called once per
 * loop, always after that function (directly or via logic that ran in
 * between), at which point busy is known to be currently set, so there's
 * no risk of returning based on a stale already-safe window.
 */
static inline void atari7800_scene_wait_display_safe(void) {
	while (atari7800_scene_display_busy != 0u) {
	}
}

void atari7800_scene_set_palette(atari7800_scene_t *scene,
		uint8_t palette_index, atari7800_palette3_t colors);

/**
 * Hides a placed persistent object by parking it off-screen (x_pos beyond
 * the visible 160px width), without releasing its slot. Call
 * atari7800_scene_sprite again later to show it at a new position -- if
 * that lands back in the same zone it was hidden in, it's just another
 * in-place patch. A no-op if the object was never placed.
 */
static inline void atari7800_scene_hide_sprite(atari7800_scene_object_t *object) {
	uint8_t *zone;

	if (object == 0 || object->zone_index == ATARI7800_SCENE_OBJECT_UNPLACED) {
		return;
	}

	zone = atari7800_scene_zones[object->zone_index];
	zone[(uint16_t)object->slot * ATARI7800_MARIA_ZONE5_OBJECT_BYTES + 4u] =
	    200u;
}

/**
 * Draws/updates a persistent sprite object (see atari7800_scene_object_t) at
 * (x, y). The first call for a given object appends it to the target zone's
 * persistent object list and places it; later calls patch its existing
 * header in place if y_pos still maps to the same zone (cheap -- no zone
 * bookkeeping touched at all), or append it to a new zone's list if it
 * crossed a zone boundary (see atari7800_scene_object_t for what happens to
 * its old slot). Cheaper than atari7800_scene_draw_sprite for anything
 * redrawn every frame, since it never needs
 * atari7800_scene_begin_frame/end_frame.
 * Returns ATARI7800_OK, ATARI7800_ERR_INVALID (null scene/object/asset),
 * or ATARI7800_ERR_BUDGET_FULL if the target zone's object list is full.
 *
 * CAVEAT: a zone's slot usage only grows (see atari7800_scene_object_t) --
 * an object that repeatedly crosses in and out of the same zone (e.g. one
 * that oscillates across a zone boundary every frame) burns a fresh slot
 * every time it re-enters, and will eventually exhaust that zone's ~12-slot
 * budget even though its live occupancy never grows. Safe for objects with
 * a fixed or rarely-changing zone (HUD elements, an enemy at a fixed Y);
 * avoid for objects that cross zone boundaries often (e.g. a scrolling
 * starfield) -- use atari7800_scene_draw_sprite with
 * atari7800_scene_begin_frame/end_frame instead, which re-derives each
 * zone's object list from scratch every frame and has no such growth.
 */
static inline uint8_t atari7800_scene_sprite(atari7800_scene_t *scene,
                               atari7800_scene_object_t *object,
                               const atari7800_sprite_asset_t *asset,
                               uint8_t x_pos, uint8_t y_pos) {
	uint8_t target_zone;
	uint8_t slot;
	uint8_t *zone;
	uint8_t status;

	if (scene == 0 || object == 0 || asset == 0 || scene->initialized == 0u) {
		return ATARI7800_ERR_INVALID;
	}

	target_zone = (uint8_t)(y_pos >> ATARI7800_ZONE_SHIFT);
	if (target_zone >= ATARI7800_SCENE_VISIBLE_ZONES) {
		target_zone = (uint8_t)(ATARI7800_SCENE_VISIBLE_ZONES - 1u);
	}

	if (object->zone_index == target_zone) {
		/* Common case: already placed here, just patch the header in place.
		 * Must not touch the 2 bytes after it (see
		 * atari7800_maria_patch_sprite_zone5) -- they belong to whatever
		 * object occupies the next slot in this zone, if any. */
		zone = atari7800_scene_zones[target_zone];
		return atari7800_maria_patch_sprite_asset_zone5(
		    zone, ATARI7800_SCENE_ZONE_BYTES, object->slot, asset, x_pos);
	}

	if (object->zone_index != ATARI7800_SCENE_OBJECT_UNPLACED) {
		/* Crossed a zone boundary: park the old slot off-screen and leave it
		 * reserved there (see atari7800_scene_object_t for why it isn't
		 * reclaimed), then fall through to place fresh in the new zone. */
		atari7800_scene_hide_sprite(object);
	}

	zone = atari7800_scene_zones[target_zone];
	slot = atari7800_scene_zone_persist_count[target_zone];

	status = atari7800_maria_plot_sprite_asset_zone5(
	    zone, ATARI7800_SCENE_ZONE_BYTES, slot, asset, x_pos);
	if (status != ATARI7800_OK) {
		return status;
	}

	if (slot == 0u) {
		atari7800_maria_init_dll_entry(
		    &atari7800_scene_display_list[(uint8_t)(1u + target_zone)],
		    ATARI7800_ZONE_OFFSET, zone, 0u);
	}

	atari7800_scene_zone_persist_count[target_zone] = (uint8_t)(slot + 1u);
	object->zone_index = target_zone;
	object->slot = slot;
	return ATARI7800_OK;
}

/**
 * Draws a sprite asset at coordinates (x, y). Matches the vertical coordinate
 * to the appropriate zone, checks for buffer overflows, plots the 5-byte
 * header, and links the zone to the active display list.
 * Returns ATARI7800_OK, ATARI7800_ERR_INVALID (null scene/asset), or
 * ATARI7800_ERR_BUDGET_FULL if the sprite's zone has no room left for it.
 */
static inline uint8_t atari7800_scene_draw_sprite(atari7800_scene_t *scene,
                                    const atari7800_sprite_asset_t *asset,
                                    uint8_t x_pos, uint8_t y_pos) {
	uint8_t zone_index;
	uint8_t object_index;
	uint8_t *zone;
	uint8_t status;

	if (scene == 0 || asset == 0) {
		return ATARI7800_ERR_INVALID;
	}

	if (scene->initialized == 0u) {
		object_index = scene->next_object;
		status = atari7800_plotsprite_asset(scene->zone, scene->zone_size,
		                                    object_index, asset, x_pos);
		if (status != ATARI7800_OK) {
			return status;
		}
		scene->next_object = (uint8_t)(scene->next_object + 1u);
		return ATARI7800_OK;
	}

	zone_index = (uint8_t)(y_pos >> ATARI7800_ZONE_SHIFT);
	if (zone_index >= ATARI7800_SCENE_VISIBLE_ZONES) {
		zone_index = (uint8_t)(ATARI7800_SCENE_VISIBLE_ZONES - 1u);
	}

	zone = atari7800_scene_zones[zone_index];
	object_index = atari7800_scene_zone_next_object[zone_index];

	status = atari7800_plotsprite_asset(zone, ATARI7800_SCENE_ZONE_BYTES,
	                                    object_index, asset, x_pos);
	if (status != ATARI7800_OK) {
		return status;
	}

	if (atari7800_scene_active_zones_curr[zone_index] == 0u) {
		atari7800_maria_init_dll_entry(
		    &atari7800_scene_display_list[(uint8_t)(1u + zone_index)], ATARI7800_ZONE_OFFSET, zone,
		    0u);
	}

	atari7800_scene_zone_next_object[zone_index] = (uint8_t)(object_index + 1u);
	atari7800_scene_active_zones_curr[zone_index] = 1u;
	return ATARI7800_OK;
}

/**
 * Draws a string of characters at coordinates (x, y). Highly optimized to fit
 * within the strict NTSC VBLANK window: caches font parameters in registers,
 * uses 8-bit offset indexing, loops cleanly over character glyphs, and plots
 * the 5-byte headers directly.
 * Returns ATARI7800_OK, ATARI7800_ERR_INVALID (bad args/empty font), or
 * ATARI7800_ERR_BUDGET_FULL if a zone ran out of room mid-string (any
 * glyphs already written before that point remain drawn).
 */
static inline uint8_t atari7800_scene_draw_text(atari7800_scene_t *scene,
                                  const atari7800_font_descriptor_t *font,
                                  uint8_t x_pos, uint8_t y_pos,
                                  const char *text) {
	uint8_t pen_x = x_pos;
	uint8_t pen_y = y_pos;
	const uint8_t start_x = x_pos;

	if (scene == 0 || font == 0 || text == 0 || font->data == 0 ||
	    font->glyph_count == 0u || font->glyph_advance == 0u) {
		return ATARI7800_ERR_INVALID;
	}

	/* Cache font properties into local variables (fast register allocation) */
	const uint16_t font_data_addr = atari7800_ptr16(font->data);
	const uint8_t glyph_mode = font->glyph_mode;
	const uint8_t pal_width = atari7800_maria_pal_width(font->glyph_palette, font->glyph_width_twos_comp);
	const uint8_t glyph_advance = font->glyph_advance;
	const uint8_t space_advance = (font->space_advance != 0u) ? font->space_advance : font->glyph_advance;
	const uint8_t line_advance = font->line_advance;
	const uint8_t * const char_to_glyph = font->char_to_glyph;
	const uint8_t char_to_glyph_len = font->char_to_glyph_len;
	const uint8_t first_char = font->first_char;
	const uint8_t glyph_count = font->glyph_count;

	if (scene->initialized == 0u) {
		/* If scene not initialized, use the simple slow path (rarely executed) */
		while (*text != '\0') {
			const uint8_t ch = (uint8_t)*text;
			if (ch == (uint8_t)'\n') {
				pen_x = start_x;
				pen_y = (uint8_t)(pen_y + line_advance);
				++text;
				continue;
			}
			if (ch == (uint8_t)' ') {
				pen_x += space_advance;
				++text;
				continue;
			}
			uint8_t glyph_index = 0xffu;
			if (char_to_glyph != 0 && ch < char_to_glyph_len) {
				glyph_index = char_to_glyph[ch];
			} else if (ch >= first_char) {
				glyph_index = (uint8_t)(ch - first_char);
			}
			if (glyph_index < glyph_count) {
				uint16_t sprite_addr = font_data_addr + ((uint16_t)glyph_index << 1);
				const uint8_t obj_idx = scene->next_object;
				const uint8_t status = atari7800_maria_plot_sprite_zone5(
				    scene->zone, scene->zone_size, obj_idx, sprite_addr, glyph_mode,
				    font->glyph_palette, font->glyph_width_twos_comp, pen_x);
				if (status != ATARI7800_OK) {
					return status;
				}
				scene->next_object = (uint8_t)(scene->next_object + 1u);
			}
			pen_x = (uint8_t)(pen_x + glyph_advance);
			++text;
		}
		return ATARI7800_OK;
	}

	/* Main optimized drawing loop for initialized scenes */
	uint8_t zone_index = (uint8_t)(pen_y >> ATARI7800_ZONE_SHIFT);
	if (zone_index >= ATARI7800_SCENE_VISIBLE_ZONES) {
		zone_index = (uint8_t)(ATARI7800_SCENE_VISIBLE_ZONES - 1u);
	}
	uint8_t *zone = atari7800_scene_zones[zone_index];
	uint8_t object_index = atari7800_scene_zone_next_object[zone_index];
	uint8_t start_offset = (uint8_t)(object_index * 5u);

	if (atari7800_scene_active_zones_curr[zone_index] == 0u) {
		atari7800_maria_init_dll_entry(
		    &atari7800_scene_display_list[(uint8_t)(1u + zone_index)], ATARI7800_ZONE_OFFSET,
		    zone, 0u);
		atari7800_scene_active_zones_curr[zone_index] = 1u;
	}

	while (*text != '\0') {
		const uint8_t ch = (uint8_t)*text;

		if (ch == (uint8_t)'\n') {
			/* Write back previous zone's object cursor */
			atari7800_scene_zone_next_object[zone_index] = object_index;

			pen_x = start_x;
			pen_y = (uint8_t)(pen_y + line_advance);
			zone_index = (uint8_t)(pen_y >> ATARI7800_ZONE_SHIFT);
			if (zone_index >= ATARI7800_SCENE_VISIBLE_ZONES) {
				zone_index = (uint8_t)(ATARI7800_SCENE_VISIBLE_ZONES - 1u);
			}
			zone = atari7800_scene_zones[zone_index];
			object_index = atari7800_scene_zone_next_object[zone_index];
			start_offset = (uint8_t)(object_index * 5u);

			if (atari7800_scene_active_zones_curr[zone_index] == 0u) {
				atari7800_maria_init_dll_entry(
				    &atari7800_scene_display_list[(uint8_t)(1u + zone_index)], ATARI7800_ZONE_OFFSET,
				    zone, 0u);
				atari7800_scene_active_zones_curr[zone_index] = 1u;
			}

			++text;
			continue;
		}

		if (ch == (uint8_t)' ') {
			pen_x += space_advance;
			++text;
			continue;
		}

		{
			uint8_t glyph_index = 0xffu;

			if (char_to_glyph != 0 && ch < char_to_glyph_len) {
				glyph_index = char_to_glyph[ch];
			} else if (ch >= first_char) {
				glyph_index = (uint8_t)(ch - first_char);
			}

			if (glyph_index < glyph_count) {
				uint16_t sprite_addr = font_data_addr + ((uint16_t)glyph_index << 1);

				const uint8_t end = (uint8_t)(start_offset + 7u);
				if (end > ATARI7800_SCENE_ZONE_BYTES) {
					atari7800_scene_zone_next_object[zone_index] = object_index;
					return ATARI7800_ERR_BUDGET_FULL;
				}

				zone[start_offset] = (uint8_t)(sprite_addr & 0xffu);
				zone[start_offset + 1] = glyph_mode;
				zone[start_offset + 2] = (uint8_t)(sprite_addr >> 8);
				zone[start_offset + 3] = pal_width;
				zone[start_offset + 4] = pen_x;
				zone[start_offset + 5] = 0x00u;
				zone[start_offset + 6] = 0x00u;

				object_index = (uint8_t)(object_index + 1u);
				start_offset = (uint8_t)(start_offset + 5u);
			}
		}

		pen_x = (uint8_t)(pen_x + glyph_advance);
		++text;
	}

	/* Write back final zone's object cursor */
	atari7800_scene_zone_next_object[zone_index] = object_index;
	return ATARI7800_OK;
}

/* Returns ATARI7800_OK, ATARI7800_ERR_INVALID (bad args/empty font), or
 * ATARI7800_ERR_BUDGET_FULL if the string needs more than max_entries
 * glyphs (out_run is left untouched in that case). */
uint8_t atari7800_build_glyph_run(const atari7800_font_descriptor_t *font,
		const char *text, atari7800_glyph_run_entry_t *entries,
		uint8_t max_entries, atari7800_glyph_run_t *out_run);

/**
 * Draws a pre-resolved glyph run at coordinates (x, y). This is the hot-loop
 * counterpart to atari7800_build_glyph_run(): no character lookup, no
 * whitespace/newline handling, just direct 5-byte object header writes,
 * matching the optimized inner loop of atari7800_scene_draw_text().
 * Returns ATARI7800_OK, ATARI7800_ERR_INVALID (bad args), or
 * ATARI7800_ERR_BUDGET_FULL if a zone ran out of room mid-run (any glyphs
 * already written before that point remain drawn).
 */
static inline uint8_t atari7800_scene_draw_glyph_run(atari7800_scene_t *scene,
                                       const atari7800_font_descriptor_t *font,
                                       uint8_t x_pos, uint8_t y_pos,
                                       const atari7800_glyph_run_t *run) {
	uint8_t i;

	if (scene == 0 || font == 0 || run == 0 || font->data == 0 ||
	    run->entries == 0) {
		return ATARI7800_ERR_INVALID;
	}

	const uint16_t font_data_addr = atari7800_ptr16(font->data);
	const uint8_t glyph_mode = font->glyph_mode;
	const uint8_t pal_width =
	    atari7800_maria_pal_width(font->glyph_palette, font->glyph_width_twos_comp);
	const uint8_t count = run->count;
	const atari7800_glyph_run_entry_t * const entries = run->entries;

	if (scene->initialized == 0u) {
		for (i = 0; i < count; ++i) {
			uint16_t sprite_addr =
			    font_data_addr + ((uint16_t)entries[i].glyph_index << 1);
			const uint8_t obj_idx = scene->next_object;
			const uint8_t status = atari7800_maria_plot_sprite_zone5(
			    scene->zone, scene->zone_size, obj_idx, sprite_addr, glyph_mode,
			    font->glyph_palette, font->glyph_width_twos_comp,
			    (uint8_t)(x_pos + entries[i].x_offset));
			if (status != ATARI7800_OK) {
				return status;
			}
			scene->next_object = (uint8_t)(scene->next_object + 1u);
		}
		return ATARI7800_OK;
	}

	uint8_t zone_index = (uint8_t)(y_pos >> ATARI7800_ZONE_SHIFT);
	if (zone_index >= ATARI7800_SCENE_VISIBLE_ZONES) {
		zone_index = (uint8_t)(ATARI7800_SCENE_VISIBLE_ZONES - 1u);
	}
	uint8_t *zone = atari7800_scene_zones[zone_index];
	uint8_t object_index = atari7800_scene_zone_next_object[zone_index];
	uint8_t start_offset = (uint8_t)(object_index * 5u);

	if (atari7800_scene_active_zones_curr[zone_index] == 0u) {
		atari7800_maria_init_dll_entry(
		    &atari7800_scene_display_list[(uint8_t)(1u + zone_index)],
		    ATARI7800_ZONE_OFFSET, zone, 0u);
		atari7800_scene_active_zones_curr[zone_index] = 1u;
	}

	for (i = 0; i < count; ++i) {
		uint16_t sprite_addr =
		    font_data_addr + ((uint16_t)entries[i].glyph_index << 1);
		const uint8_t end = (uint8_t)(start_offset + 7u);

		if (end > ATARI7800_SCENE_ZONE_BYTES) {
			atari7800_scene_zone_next_object[zone_index] = object_index;
			return ATARI7800_ERR_BUDGET_FULL;
		}

		zone[start_offset] = (uint8_t)(sprite_addr & 0xffu);
		zone[start_offset + 1] = glyph_mode;
		zone[start_offset + 2] = (uint8_t)(sprite_addr >> 8);
		zone[start_offset + 3] = pal_width;
		zone[start_offset + 4] = (uint8_t)(x_pos + entries[i].x_offset);
		zone[start_offset + 5] = 0x00u;
		zone[start_offset + 6] = 0x00u;

		object_index = (uint8_t)(object_index + 1u);
		start_offset = (uint8_t)(start_offset + 5u);
	}

	atari7800_scene_zone_next_object[zone_index] = object_index;
	return ATARI7800_OK;
}

#endif /* !ATARI7800_NO_SCENE_IMPL */

#endif
