/* Atari 7800 linker script (initial baseline).
 *
 * This first implementation targets a flat 32 KiB ROM image mapped at
 * 0x8000-0xFFFF with a 4 KiB RAM window at 0x1800-0x27FF.
 *
 * Output is emitted as an A78 file: 128-byte header + ROM payload.
 */

PROVIDE(__cart_rom_size = 32);
ASSERT(__cart_rom_size == 32,
       "ATARI 7800: Initial target currently supports only 32 KiB ROM");

/* POKEY declaration for the .a78 header's v4 "audio" field (low byte,
 * bits 0-2): 0 = none (default, every program that doesn't override this),
 * 2 = POKEY @ $450. Override per-program with
 * `-Wl,--defsym=__cart_pokey_flags=2` rather than changing this default --
 * see the ATARI7800_ZONE_HEIGHT lesson in 7800port.md for why a hardware
 * assumption like this must not be baked into the shared header skeleton
 * every example inherits. Legacy v3 cart-type POKEY bits are deliberately
 * left at 0: this SDK's header declares version 4, and the A78 header spec
 * says v4 fields take precedence on any v4-aware emulator/flashcart. */
PROVIDE(__cart_pokey_flags = 0);

/* Keep this in writable RAM; stack grows downward. */
PROVIDE(__stack = 0x2800);

MEMORY {
  zp : ORIGIN = __rc31 + 1, LENGTH = 0x100 - (__rc31 + 1)

  header   (r) : ORIGIN = 0x0000, LENGTH = 128
  cart_rom (rx) : ORIGIN = 0x8000, LENGTH = __cart_rom_size * 1024
  ram_low (w)   : ORIGIN = 0x1800, LENGTH = 0x0840
  ram (w)       : ORIGIN = 0x2200, LENGTH = 0x0600
}

/* Provide imaginary (zero page) registers. */
__rc0 = 0x80;
INCLUDE imag-regs.ld
ASSERT(__rc31 == 0x9f, "Inconsistent zero page map.")

/* Provide default interrupt handlers if the program does not define them. */
PROVIDE(nmi = _atari7800_irq_default);
PROVIDE(irq = _atari7800_irq_default);
PROVIDE(atari7800_reset_entry = _start);

REGION_ALIAS("c_readonly", cart_rom)
REGION_ALIAS("c_writeable", ram)

SECTIONS {
  .scene_zones_low 0x1a00 (NOLOAD) : {
    *(.scene_zones_low .scene_zones_low.*)
  } >ram_low

  .scene_zones_high 0x2200 (NOLOAD) : {
    *(.scene_zones_high .scene_zones_high.*)
  } >ram

  INCLUDE c.ld

  /* 7800basic-compatible A78 header skeleton. */
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

    /* ROM size, little-endian 32-bit value. */
    BYTE((__cart_rom_size * 1024 >> 24) & 0xff)
    BYTE((__cart_rom_size * 1024 >> 16) & 0xff)
    BYTE((__cart_rom_size * 1024 >> 8) & 0xff)
    BYTE((__cart_rom_size * 1024 >> 0) & 0xff)

    /* v3 carttype bytes. Byte 54 bit 6 [0x40] is the legacy "POKEY at
     * $0450" flag, and it's the ONE this SDK's actual test emulator (a7800,
     * a 7800-devtools/a7800 fork) reads for cart-type detection -- it does
     * NOT look at the v4 "audio" field below at all, despite this header
     * declaring version 4. Deliberately left at 0 (not derived from
     * __cart_pokey_flags) anyway: setting that bit switches this
     * emulator's cart-slot device to its "a78_p450_t0" class
     * (a78_slot.cpp), which broke this platform's basic ROM/RAM layout
     * outright (confirmed hands-on: title-demo rendered a fully black
     * screen with the bit set, and rendered correctly again the instant it
     * was cleared, v4 field unchanged) -- not investigated further since
     * correct rendering matters more than an audible POKEY in this one
     * emulator fork. The v4 field below is still set correctly per spec,
     * for real hardware or any v4-aware tool; POKEY code written against
     * it (atari7800_pokey.h) is untested for actual sound in this
     * environment as a result -- see 7800port.md. */
    BYTE(0)
    BYTE(0)

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

    /* v4 mapper/audio/interrupt fields: mapper (2 bytes), audio (2 bytes,
     * low byte bits 0-2 select POKEY location -- see __cart_pokey_flags
     * above), interrupt (2 bytes). */
    BYTE(0) BYTE(0)
    BYTE(__cart_pokey_flags) BYTE(0)
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

  .cart_rom_fixed : { *(.cart_rom_fixed .cart_rom_fixed.*) } >cart_rom

  /*
   * Place reset entry trampoline in high ROM so 7800sign can hash/sign carts.
   * The trampoline jumps to the regular C runtime _start.
   */
  .cart_rom_reset_entry 0xf000 : {
    KEEP(*(.cart_rom_reset_entry .cart_rom_reset_entry.*))
  } >cart_rom

  /*
   * Reserve final 128 bytes for signature/verification data and vectors.
   * This layout is compatible with 7800sign/sign7800 expectations.
   */
  .cart_footer 0xff80 : {
    /* Signature area at 0xff80-0xfff7 (120 bytes). */
    . = . + 120;

    /* Region verification and encryption/hash config bytes. */
    BYTE(0xff)
    BYTE(0xf7)

    /* CPU vectors. */
    SHORT(nmi)
    SHORT(atari7800_reset_entry)
    SHORT(irq)
  } >cart_rom
}

OUTPUT_FORMAT {
  FULL(header)
  FULL(cart_rom)
}
