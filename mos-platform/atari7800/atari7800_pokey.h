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

/* XCTRL1: a real Atari 7800 console-side register (not part of POKEY
 * itself) that 7800basic's own POKEY detection/init routine
 * (pokeysound.asm's detectpokeylocation, `if pokeyaddress = $450` branch)
 * always writes alongside POKEY setup for a $450-mounted chip. Meaning
 * undocumented outside 7800basic's own source; included here purely
 * because the reference always does it. */
#define ATARI7800_REG_XCTRL1 0x0470u

/**
 * Real POKEY init sequence, transcribed from 7800basic's own
 * detectpokeylocation routine (includes/pokeysound.asm, the `if
 * pokeyaddress = $450` branch) rather than assumed: this is what every
 * 7800basic game with `set pokeysupport $450` (astrowing.bas included)
 * actually does before touching POKEY, not just an IRQEN mask.
 *
 * Critically, POKEY's internal clock/counters do not run at all while
 * SKCTL's low bits are clear ("in reset") -- confirmed in MAME's own
 * pokey_device::step_one_clock, which only advances its clock counters
 * `if (m_SKCTL & SK_RESET)`. SKCTL resets to 0 (in reset) and nothing
 * else in this platform ever wrote it, so POKEY was never actually
 * running, regardless of IRQEN. The sequence: clear all 16 registers,
 * set the XCTRL1 enable bits, then explicitly bring the chip out of
 * reset via SKCTL before touching AUDCTL.
 */
static inline void atari7800_pokey_init(void) {
	uint8_t reg;
	for (reg = 0; reg < 16u; ++reg) {
		ATARI7800_MMIO8(ATARI7800_POKEY_BASE + reg) = 0x00u;
	}
	ATARI7800_MMIO8(ATARI7800_REG_XCTRL1) |= 0x14u;
	ATARI7800_MMIO8(ATARI7800_REG_SKCTL) = 0x03u;
	ATARI7800_MMIO8(ATARI7800_REG_AUDCTL) = 0x00u;
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
 *   0xFE - end of song; *cursor resets to song_start and returns
 *          immediately, without processing any bytes from the restarted
 *          song this call -- matching astrowing.bas's own PlayMusic
 *          (`.EndSong` resets the pointer and `rts`s; the restarted song's
 *          first frame isn't read until the *next* call). An earlier
 *          version of this function instead fell through to keep
 *          processing the new song in the same call, silently playing an
 *          extra frame's worth of register writes every time a song
 *          looped -- fixed to match the reference exactly.
 * Call once every other frame (the data is authored at 30Hz, half the NTSC
 * frame rate) with *cursor initialized to song_start.
 */
static inline void atari7800_pokey_step(const uint8_t **cursor,
                                        const uint8_t *song_start) {
	const uint8_t *p = *cursor;
	/* Matches PlayMusic's own `cpy #64` safety cap: bail out (leaving
	 * *cursor mid-frame, same as the reference's forced .EndFrame exit)
	 * rather than looping indefinitely on malformed/corrupt song data. */
	uint8_t budget = 64u;

	for (; budget != 0u; budget -= 2u) {
		uint8_t reg = *p;

		if (reg == 0xffu) {
			*cursor = p + 1;
			return;
		}
		if (reg == 0xfeu) {
			*cursor = song_start;
			return;
		}

		ATARI7800_MMIO8(ATARI7800_POKEY_BASE + reg) = p[1];
		p += 2;
	}
	*cursor = p;
}

#endif
