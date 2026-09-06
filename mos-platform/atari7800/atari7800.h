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
void atari7800_maria_build_blank_ntsc(atari7800_maria_dll_entry_t *display_list,
		const atari7800_maria_null_header_t *zone_header);
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
void atari7800_scene_init_160a(atari7800_scene_t *scene, uint8_t bgcolor);
/* Opt-in: flags the DLL's top and bottom blank template entries (never
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
void atari7800_scene_enable_nmi_sync(atari7800_scene_t *scene);
/* Blocks until MARIA has just started scanning the visible display area
 * for a new frame (see atari7800_scene_enable_nmi_sync and the loop shape
 * in its doc comment). Call this once per loop, before any non-drawing
 * game logic. */
void atari7800_scene_wait_active_start(void);
/* Blocks until MARIA has finished scanning the current frame's visible
 * area -- call this right before any draw calls that touch zone buffers
 * or the DLL, after atari7800_scene_wait_active_start() and any logic. */
void atari7800_scene_wait_display_safe(void);
void atari7800_scene_set_palette(atari7800_scene_t *scene,
		uint8_t palette_index, atari7800_palette3_t colors);
void atari7800_scene_begin_frame(atari7800_scene_t *scene);
void atari7800_scene_end_frame(atari7800_scene_t *scene);
/* Pins/unpins the zone containing y_pos as static residency: while pinned,
 * begin_frame/end_frame leave it completely alone instead of resetting or
 * wiping it every frame. Draw into the zone, then pin it once done; use
 * this for HUD elements that only change occasionally instead of
 * redrawing them every frame. See the definition in atari7800.c for the
 * full pin/unpin contract. */
void atari7800_scene_set_zone_static(atari7800_scene_t *scene, uint8_t y_pos,
		uint8_t is_static);
/* Returns ATARI7800_OK, ATARI7800_ERR_INVALID (null scene/asset), or
 * ATARI7800_ERR_BUDGET_FULL if the sprite's zone has no room left for it. */
uint8_t atari7800_scene_draw_sprite(atari7800_scene_t *scene,
		const atari7800_sprite_asset_t *asset, uint8_t x_pos,
		uint8_t y_pos);
/* Draws/updates a persistent sprite object at (x, y). The first call for a
 * given object appends it to the target zone's persistent object list and
 * places it; later calls patch its existing header in place if y_pos
 * still maps to the same zone (cheap -- no zone bookkeeping touched at
 * all), or append it to a new zone's list if it crossed a zone boundary
 * (see atari7800_scene_object_t for what happens to its old slot).
 * Cheaper than atari7800_scene_draw_sprite for anything redrawn every
 * frame, since it never needs atari7800_scene_begin_frame/end_frame.
 * Returns ATARI7800_OK, ATARI7800_ERR_INVALID (null scene/object/asset),
 * or ATARI7800_ERR_BUDGET_FULL if the target zone's object list is full. */
uint8_t atari7800_scene_sprite(atari7800_scene_t *scene,
		atari7800_scene_object_t *object,
		const atari7800_sprite_asset_t *asset, uint8_t x_pos,
		uint8_t y_pos);
/* Hides a placed persistent object by parking it off-screen, without
 * releasing its slot. Call atari7800_scene_sprite again later to show it
 * at a new position -- if that lands back in the same zone it was hidden
 * in, it's just another in-place patch. A no-op if never placed. */
void atari7800_scene_hide_sprite(atari7800_scene_object_t *object);
/* Returns ATARI7800_OK, ATARI7800_ERR_INVALID (bad args/empty font), or
 * ATARI7800_ERR_BUDGET_FULL if a zone ran out of room mid-string (any
 * glyphs already written before that point remain drawn). */
uint8_t atari7800_scene_draw_text(atari7800_scene_t *scene,
		const atari7800_font_descriptor_t *font,
		uint8_t x_pos, uint8_t y_pos, const char *text);
/* Returns ATARI7800_OK, ATARI7800_ERR_INVALID (bad args/empty font), or
 * ATARI7800_ERR_BUDGET_FULL if the string needs more than max_entries
 * glyphs (out_run is left untouched in that case). */
uint8_t atari7800_build_glyph_run(const atari7800_font_descriptor_t *font,
		const char *text, atari7800_glyph_run_entry_t *entries,
		uint8_t max_entries, atari7800_glyph_run_t *out_run);
/* Returns ATARI7800_OK, ATARI7800_ERR_INVALID (bad args), or
 * ATARI7800_ERR_BUDGET_FULL if a zone ran out of room mid-run (any
 * glyphs already written before that point remain drawn). */
uint8_t atari7800_scene_draw_glyph_run(atari7800_scene_t *scene,
		const atari7800_font_descriptor_t *font, uint8_t x_pos,
		uint8_t y_pos, const atari7800_glyph_run_t *run);

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

#endif
