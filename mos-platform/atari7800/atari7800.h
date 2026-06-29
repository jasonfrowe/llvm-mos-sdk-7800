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

/* Frequently used register aliases. */
#define ATARI7800_INPTCTRL ATARI7800_MMIO8(ATARI7800_REG_INPTCTRL)
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

/* Stable NTSC blank-frame layout (1 top + 28 display + 4 bottom). */
#define ATARI7800_MARIA_NTSC_DLL_ENTRIES 33u

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

void atari7800_init_system(void);
void atari7800_wait_vblank(void);
void atari7800_configure_video(uint16_t display_list_addr, uint8_t ctrl, uint8_t bgcolor);
void atari7800_maria_build_blank_ntsc(atari7800_maria_dll_entry_t *display_list,
		const atari7800_maria_null_header_t *zone_header);

#endif
