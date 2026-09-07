// Copyright 2026 LLVM-MOS Project
// Licensed under the Apache License, Version 2.0 with LLVM Exceptions.

#ifndef _ATARI7800_POKEY_H
#define _ATARI7800_POKEY_H

#include <stdint.h>
#include <atari7800.h>

/* POKEY sound chip, assumed present at $0450 (a cartridge-mounted POKEY,
 * matching 7800basic's "set pokeysupport $450" -- the most common location
 * for a POKEY-equipped 7800 cart). Declaring this in the .a78 header (see
 * __cart_pokey_flags in link.ld) is a separate, required step: without it,
 * neither the emulator nor real hardware routes this address range to a
 * POKEY chip at all. */
#define ATARI7800_REG_AUDF1  0x0450u
#define ATARI7800_REG_AUDC1  0x0451u
#define ATARI7800_REG_AUDF2  0x0452u
#define ATARI7800_REG_AUDC2  0x0453u
#define ATARI7800_REG_AUDF3  0x0454u
#define ATARI7800_REG_AUDC3  0x0455u
#define ATARI7800_REG_AUDF4  0x0456u
#define ATARI7800_REG_AUDC4  0x0457u
#define ATARI7800_REG_AUDCTL 0x0458u
#define ATARI7800_REG_STIMER 0x0459u
#define ATARI7800_REG_SKREST 0x045au
#define ATARI7800_REG_SEROUT 0x045du
#define ATARI7800_REG_IRQEN  0x045eu
#define ATARI7800_REG_SKCTL  0x045fu

#define ATARI7800_POKEY_BASE 0x0450u

/**
 * POKEY's IRQ output is wired to the 6502's IRQ line on this cart type.
 * IRQEN's reset state is not guaranteed clear, and at least one source
 * (serial/keyboard-related bits, meaningless on a cart-mounted POKEY with
 * nothing wired to those pins) can come up already asserted -- observed
 * hands-on as a total lockup (an IRQ storm: the CPU re-enters the default
 * `rti`-only IRQ handler as fast as it can return from it, starving the
 * rest of the program of any real execution time) the instant POKEY
 * hardware is declared present, before this function existed. Call once,
 * before any other POKEY register write, to mask every POKEY IRQ source.
 */
static inline void atari7800_pokey_init(void) {
	ATARI7800_MMIO8(ATARI7800_REG_IRQEN) = 0x00u;
}

/**
 * Advances one frame of a 30Hz POKEY register-stream song (the format used
 * by astrowing.bas's hand-written PlayMusic routine and its vgm2pokey_30hz
 * converter): a sequence of (register_offset, value) byte pairs, each pair
 * writing `value` to `ATARI7800_POKEY_BASE + register_offset`, terminated
 * by one of two marker bytes in the register_offset position instead of a
 * real offset:
 *   0xFF - end of this frame; *cursor is advanced past it, ready for the
 *          next call.
 *   0xFE - end of song; *cursor resets to song_start and playback loops
 *          (does not return early -- continues processing the new frame
 *          from the top of the song in the same call).
 * Call once every other frame (the data is authored at 30Hz, half the NTSC
 * frame rate) with *cursor initialized to song_start.
 */
static inline void atari7800_pokey_step(const uint8_t **cursor,
                                        const uint8_t *song_start) {
	const uint8_t *p = *cursor;

	for (;;) {
		uint8_t reg = *p;

		if (reg == 0xffu) {
			*cursor = p + 1;
			return;
		}
		if (reg == 0xfeu) {
			p = song_start;
			continue;
		}

		ATARI7800_MMIO8(ATARI7800_POKEY_BASE + reg) = p[1];
		p += 2;
	}
}

#endif
