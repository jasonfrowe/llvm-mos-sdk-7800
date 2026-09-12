/* Atari 7800 "SuperGame" (Type 2) bankswitched linker script: 8x16KiB banks
 * (128KiB total). This is the link.ld for the separate "atari7800-supergame"
 * platform (mos-platform/atari7800-supergame/CMakeLists.txt, PARENT
 * atari7800) -- a program opts in simply by being built with
 * `--target=atari7800-supergame` instead of `--target=atari7800`, the same
 * way NES mapper variants (nes-mmc3, nes-nrom, ...) are separate platforms
 * rather than a single platform with a linker-script toggle. This had to be
 * a separate platform, not a per-program `-Wl,-T` override within the flat
 * atari7800 platform: the mos toolchain always applies the current
 * platform's own link.ld unconditionally, so an additional explicit `-T`
 * merges with it (and conflicts) rather than replacing it -- confirmed
 * empirically (`region 'zp' already defined`) when a per-target override
 * was tried first. See mapper.h for the hardware summary and
 * examples/atari7800-supergame/bank-min/ for a minimal smoke test.
 *
 * Layout mirrors mos-platform/nes-mmc3's proven banked-linker-script pattern
 * (prg-rom-banked-8.ld / _prg-rom-banked.ld), simplified because SuperGame's
 * two "fixed" halves are NOT a single contiguous 32KiB block: bank 6 sits at
 * $4000-$7FFF and bank 7 sits at $C000-$FFFF, with the switchable window
 * ($8000-$BFFF, banks 0-5) in between. Both fixed halves are mapped
 * simultaneously at all times, so code in one can call code in the other
 * directly with zero wrapper overhead -- only crossing into/out of the
 * switchable window needs banked_call_8000 (mapper.h).
 *
 * Bank-switching itself is a single byte write to any address in
 * $8000-$BFFF (see mapper.s) -- unlike MMC3, there is no separate
 * register-select step, so unlike nes-mmc3's per-bank VMA symbols
 * (__prg_rom_N, overridable per bank), SuperGame's switchable banks all
 * uniformly target $8000.
 */

PROVIDE(__cart_pokey_flags = 2);

PROVIDE(__stack = 0x2800);

/* Each switchable bank (0-5) shares CPU address $8000 but needs a distinct
 * linker-global address so the regions don't overlap; only the low 16 bits
 * (truncated into the actual 6502 opcode operands) matter at runtime, and
 * those are 0x8000 for every one of them -- same trick as
 * mos-platform/nes-mmc3/_prg-rom-banked.ld's __prg_rom_N_lma. */
__cart_rom_bank_0_lma = 0x000000 + 0x8000;
__cart_rom_bank_1_lma = 0x010000 + 0x8000;
__cart_rom_bank_2_lma = 0x020000 + 0x8000;
__cart_rom_bank_3_lma = 0x030000 + 0x8000;
__cart_rom_bank_4_lma = 0x040000 + 0x8000;
__cart_rom_bank_5_lma = 0x050000 + 0x8000;

MEMORY {
  zp : ORIGIN = __rc31 + 1, LENGTH = 0x100 - (__rc31 + 1)

  header (r) : ORIGIN = 0x0000, LENGTH = 128

  /* Switchable window ($8000-$BFFF), banks 0-5. */
  cart_rom_bank_0 (rx) : ORIGIN = __cart_rom_bank_0_lma, LENGTH = 0x4000
  cart_rom_bank_1 (rx) : ORIGIN = __cart_rom_bank_1_lma, LENGTH = 0x4000
  cart_rom_bank_2 (rx) : ORIGIN = __cart_rom_bank_2_lma, LENGTH = 0x4000
  cart_rom_bank_3 (rx) : ORIGIN = __cart_rom_bank_3_lma, LENGTH = 0x4000
  cart_rom_bank_4 (rx) : ORIGIN = __cart_rom_bank_4_lma, LENGTH = 0x4000
  cart_rom_bank_5 (rx) : ORIGIN = __cart_rom_bank_5_lma, LENGTH = 0x4000

  /* Fixed bank 6 ($4000-$7FFF, "second-to-last"): freely-calling hot-path
   * code lives here by default (c_readonly alias below). */
  cart_rom_fixed_lo (rx) : ORIGIN = 0x4000, LENGTH = 0x4000

  /* Fixed bank 7 ($C000-$FFFF, "last"): the only bank guaranteed present at
   * reset, so vectors/footer/reset trampoline live here. Two overlapping
   * regions, same technique as the flat link.ld's cart_rom/cart_rom_code:
   * the full 16KiB region for the footer at its fixed address, and a
   * bounded sub-region (ending where the footer begins) for the reset
   * trampoline, so overflow into the footer fails to link instead of
   * silently colliding with it. */
  cart_rom_fixed_hi (rx) : ORIGIN = 0xc000, LENGTH = 0x4000
  cart_rom_fixed_hi_code (rx) : ORIGIN = 0xc000, LENGTH = 0xff80 - 0xc000

  ram_low (w) : ORIGIN = 0x1800, LENGTH = 0x0840
  ram (w)     : ORIGIN = 0x2200, LENGTH = 0x0600
}

/* Provide imaginary (zero page) registers. */
__rc0 = 0x80;
INCLUDE imag-regs.ld
ASSERT(__rc31 == 0x9f, "Inconsistent zero page map.")

/* Provide default interrupt handlers if the program does not define them. */
PROVIDE(nmi = _atari7800_irq_default);
PROVIDE(irq = _atari7800_irq_default);
PROVIDE(atari7800_reset_entry = _start);

REGION_ALIAS("c_readonly", cart_rom_fixed_lo)
REGION_ALIAS("c_writeable", ram)

SECTIONS {
  .scene_zones_low 0x1a00 (NOLOAD) : {
    *(.scene_zones_low .scene_zones_low.*)
  } >ram_low

  .scene_zones_high 0x2200 (NOLOAD) : {
    *(.scene_zones_high .scene_zones_high.*)
  } >ram

  /* mapper.s's .zeropage __prg_8000 needs an explicit address -- unlike the
   * __rcN imaginary registers (imag-regs.ld), nothing in this SDK's linker
   * scripts auto-allocates .zeropage-declared symbols (confirmed by
   * building this platform: __prg_8000 came back undefined at link time).
   * Reserve its one byte here, before the generic .zp.bss catch-all in
   * c.ld/zp.ld claims the rest of the zp region, so the two can't alias. */
  .atari7800_supergame_zp (NOLOAD) : {
    __prg_8000 = .;
    . = . + 1;
  } >zp

  INCLUDE c.ld

  /* 7800basic-compatible A78 header skeleton -- same field layout as the
   * flat link.ld (see that file's comments for the full field-by-field
   * derivation), except the ROM size field and the v3 cart-type byte, which
   * are fixed here for the SuperGame+POKEY@450 combination: v3 byte 53=0x00,
   * byte 54 = 0x02 (Type 2 / SuperGame select bit) | 0x40 (POKEY @ $450,
   * when __cart_pokey_flags=2) -- derived directly from this SDK's test
   * emulator's header-decode switch (third_party/a7800/src/devices/bus/
   * a7800/a78_slot.cpp), which selects a78_rom_p450_sg_device for exactly
   * this bit combination. That device only overrides read_04xx/write_04xx
   * on top of the plain SuperGame device, so no further emulator changes
   * are needed -- it's covered by the POKEY $0400-$047F fix already applied
   * for the non-banked POKEY@450 header (7800port.md #19). */
  .a78_header 0 : {
    BYTE(4) /* Header version */

    /* "ATARI7800        " console string (16 bytes). */
    BYTE(0x41) BYTE(0x54) BYTE(0x41) BYTE(0x52)
    BYTE(0x49) BYTE(0x37) BYTE(0x38) BYTE(0x30)
    BYTE(0x30)
    BYTE(0x20) BYTE(0x20) BYTE(0x20) BYTE(0x20)
    BYTE(0x20) BYTE(0x20) BYTE(0x20)

    /* Game name (32 bytes), currently zeroed. */
    . = . + 32;

    /* ROM size, big-endian 32-bit value: 128 KiB (8 x 16 KiB banks). */
    BYTE(0x00) BYTE(0x02) BYTE(0x00) BYTE(0x00)

    /* v3 carttype bytes. */
    BYTE(0)
    BYTE(0x02 | ((__cart_pokey_flags << 5) & 0x40))

    /* Controllers 1 and 2: standard 7800 joypad. */
    BYTE(1)
    BYTE(1)

    /* TV format and save peripheral. */
    BYTE(0)
    BYTE(0)

    /* Unused1[3] + IRQ. */
    . = . + 4;

    /* XM expansion. */
    BYTE(0)

    /* v4 mapper/audio/interrupt fields (see flat link.ld's comment on byte
     * order -- low-order byte of each 2-byte field goes second). Mapper
     * field left at 0: this SDK's test emulator reads only the v3 byte
     * above for cart-type detection, same as the flat/non-banked case. */
    BYTE(0) BYTE(0)
    BYTE(0) BYTE(__cart_pokey_flags)
    BYTE(0) BYTE(0)

    /* Unused2[30]. */
    . = . + 30;

    /* "ACTUAL CART DATA STARTS HERE" (28 bytes). */
    BYTE(0x41) BYTE(0x43) BYTE(0x54) BYTE(0x55) BYTE(0x41) BYTE(0x4c)
    BYTE(0x20) BYTE(0x43) BYTE(0x41) BYTE(0x52) BYTE(0x54) BYTE(0x20)
    BYTE(0x44) BYTE(0x41) BYTE(0x54) BYTE(0x41) BYTE(0x20) BYTE(0x53)
    BYTE(0x54) BYTE(0x41) BYTE(0x52) BYTE(0x54) BYTE(0x53) BYTE(0x20)
    BYTE(0x48) BYTE(0x45) BYTE(0x52) BYTE(0x45)
  } >header
  ASSERT(SIZEOF(.a78_header) == 128, "ATARI 7800: A78 header must be 128 bytes")

  .cart_rom_bank_0 : { *(.cart_rom_bank_0 .cart_rom_bank_0.*) } >cart_rom_bank_0
  .cart_rom_bank_1 : { *(.cart_rom_bank_1 .cart_rom_bank_1.*) } >cart_rom_bank_1
  .cart_rom_bank_2 : { *(.cart_rom_bank_2 .cart_rom_bank_2.*) } >cart_rom_bank_2
  .cart_rom_bank_3 : { *(.cart_rom_bank_3 .cart_rom_bank_3.*) } >cart_rom_bank_3
  .cart_rom_bank_4 : { *(.cart_rom_bank_4 .cart_rom_bank_4.*) } >cart_rom_bank_4
  .cart_rom_bank_5 : { *(.cart_rom_bank_5 .cart_rom_bank_5.*) } >cart_rom_bank_5

  .cart_rom_fixed : { *(.cart_rom_fixed .cart_rom_fixed.*) } >cart_rom_fixed_lo

  /* Overflow room for generic code/rodata that doesn't fit in
   * cart_rom_fixed_lo (the default c_readonly target) alongside everything
   * else living there -- a program tags specific large, self-contained
   * assets with __attribute__((section(".cart_rom_fixed_hi_extra.rodata")))
   * to move them here explicitly (e.g. examples/atari7800-supergame/
   * astrowing/astrowing.c's misc_sprite_data/spaceship_data). Bounded to
   * cart_rom_fixed_hi_code (not the full cart_rom_fixed_hi), so overflow
   * into the footer fails to link instead of silently colliding with it,
   * same rationale as .cart_rom_reset_entry below. */
  .cart_rom_fixed_hi_extra : {
    *(.cart_rom_fixed_hi_extra .cart_rom_fixed_hi_extra.*)
  } >cart_rom_fixed_hi_code

  /* Reset entry trampoline -- placed in the fixed-hi bank alongside the
   * vectors/footer it's addressed by, same KEEP() rationale as flat link.ld. */
  .cart_rom_reset_entry : {
    KEEP(*(.cart_rom_reset_entry .cart_rom_reset_entry.*))
  } >cart_rom_fixed_hi_code

  /* Reserve final 128 bytes for signature/verification data and vectors,
   * same layout as flat link.ld. */
  .cart_footer 0xff80 : {
    . = . + 120;
    BYTE(0xff)
    BYTE(0xf7)
    SHORT(nmi)
    SHORT(atari7800_reset_entry)
    SHORT(irq)
  } >cart_rom_fixed_hi
}

OUTPUT_FORMAT {
  FULL(header)
  FULL(cart_rom_bank_0)
  FULL(cart_rom_bank_1)
  FULL(cart_rom_bank_2)
  FULL(cart_rom_bank_3)
  FULL(cart_rom_bank_4)
  FULL(cart_rom_bank_5)
  FULL(cart_rom_fixed_lo)
  FULL(cart_rom_fixed_hi)
}
