; Atari 7800 startup and NMI/IRQ handlers

.section .cart_rom_reset_entry,"axR",@progbits
.global atari7800_reset_entry
atari7800_reset_entry:
  jmp _start

.section .init.50,"ax",@progbits
.global __do_7800_reset
__do_7800_reset:
  sei
  cld

  ; Disable NMIs immediately
  lda #$00
  sta $4E

  ; Lock console in 7800 mode
  lda #$07
  sta $01     ; INPTCTRL

  ; Disable DMA
  lda #$7F
  sta $3C     ; CTRL

  ; Set OFFSET to 0
  lda #$00
  sta $38     ; OFFSET
  sta $01     ; Release joystick freeze
  sta $20     ; BACKGRND = black

  ; Set stack pointer
  ldx #$FF
  txs

  ; Clear RAM from 0x1800 to 0x27FF
  lda #0
  tay
  sta $80
  ldx #$18
mem_clear_loop:
  stx $81
  sta ($80),y
  iny
  bne mem_clear_loop
  inx
  cpx #$28
  bne mem_clear_loop
  sta $81     ; Restore ZP high byte

  ; Set Maria Display List List pointer to 0x1800
  lda #$00
  sta $30     ; DPPL
  lda #$18
  sta $2C     ; DPPH

  ; Initialize interruptindex
  lda #3
  sta 0x01B2  ; interruptindex

  ; Initialize random seed
  lda #1
  ora $0284   ; INTIM
  sta 0x40    ; rand

; Default IRQ handler
.section .text._atari7800_irq_default,"axR",@progbits
.global _atari7800_irq_default
_atari7800_irq_default:
  rti

; NMI Handler
.section .text.nmi,"axR",@progbits
.global nmi
nmi:
  bit $4E     ; Check if NMIs are enabled
  bpl nmi_dummy_exit

  pha
  txa
  pha
  tya
  pha
  cld

  ; Toggle visibleover
  lda 0x4D    ; visibleover
  eor #255
  sta 0x4D    ; visibleover

  ; Decrement interruptindex
  dec 0x01B2  ; interruptindex
  bne nmi_exit

  ; reallyoffvisible:
  lda #0
  sta 0x4D    ; visibleover
  lda #3
  sta 0x01B2  ; interruptindex

  ; Call sound update (servicesfxchannels)
  jsr servicesfxchannels

  ; Call music update (play_music_tick)
  jsr play_music_tick

nmi_exit:
  pla
  tay
  pla
  tax
  pla
nmi_dummy_exit:
  rti
