; Atari 7800 startup and default interrupt handlers.

; Reset trampoline located in high ROM so 7800sign can hash/sign carts.
.global atari7800_reset_entry
.section .cart_rom_reset_entry,"axR",@progbits
atari7800_reset_entry:
  jmp _start

; Run very early during _start.
.global __do_7800_reset
.section .init.50,"ax",@progbits
__do_7800_reset:
  sei                 ; Disable IRQs during startup.
  cld                 ; Ensure decimal mode is clear.
  lda #$07
  sta $01             ; INPTCTRL: lock machine in 7800 mode.
  lda #$7f
  sta $3c             ; CTRL: DMA off during init.
  ldx #$00
  stx $38             ; OFFSET must be zero.
  stx $01             ; Release joystick freeze condition.
  dex
  txs                 ; Hardware stack at $01ff.

; Default IRQ/NMI handlers for platforms without user handlers.
.global _atari7800_irq_default
.section .text._atari7800_irq_default,"axR",@progbits
_atari7800_irq_default:
  rti
