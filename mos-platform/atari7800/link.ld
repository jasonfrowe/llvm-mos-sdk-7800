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

/* Keep this in writable RAM; stack grows downward. */
PROVIDE(__stack = 0x2800);

MEMORY {
  zp : ORIGIN = __rc31 + 1, LENGTH = 0x100 - (__rc31 + 1)

  header   (r) : ORIGIN = 0x0000, LENGTH = 128
  cart_rom (rx) : ORIGIN = 0x8000, LENGTH = __cart_rom_size * 1024
  ram (w)       : ORIGIN = 0x1800, LENGTH = 0x1000
}

/* Provide imaginary (zero page) registers. */
__rc0 = 0x80;
INCLUDE imag-regs.ld
ASSERT(__rc31 == 0x9f, "Inconsistent zero page map.")

/* Provide default interrupt handlers if the program does not define them. */
PROVIDE(nmi = _atari7800_irq_default);
PROVIDE(irq = _atari7800_irq_default);

REGION_ALIAS("c_readonly", cart_rom)
REGION_ALIAS("c_writeable", ram)

SECTIONS {
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

    /* v3 carttype bytes. */
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

    /* v4 mapper/audio/interrupt fields. */
    BYTE(0) BYTE(0) BYTE(0) BYTE(0) BYTE(0) BYTE(0)

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
    SHORT(_start)
    SHORT(irq)
  } >cart_rom
}

OUTPUT_FORMAT {
  FULL(header)
  FULL(cart_rom)
}
