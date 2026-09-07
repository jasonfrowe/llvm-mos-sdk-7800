; Atari 7800 SuperGame bankswitching. See mapper.h for the hardware summary.
; banked_call_8000's save/switch/call/restore shape mirrors
; mos-platform/nes-mmc3/mapper.s, reusing the same shared __call_indir helper
; (mos-platform/common/crt/call-indir.S); set_prg_8000 is simpler than MMC3's
; since SuperGame selects a bank with a single byte write to any address in
; $8000-$BFFF, with no register-select step and no retry-on-interrupt needed.

.include "imag.inc"

.zeropage __prg_8000

.section .text.set_prg_8000,"ax",@progbits
.globl __set_prg_8000
.weak set_prg_8000
__set_prg_8000:
set_prg_8000:
	sta __prg_8000
	sta $8000
	rts

.section .text.get_prg_8000,"ax",@progbits
.globl __get_prg_8000
.weak get_prg_8000
__get_prg_8000:
get_prg_8000:
	lda __prg_8000
	rts

.section .text.banked_call_8000,"ax",@progbits
.weak banked_call_8000
__banked_call_8000:
banked_call_8000:
	tay
	lda __prg_8000
	pha
	tya
	jsr __set_prg_8000
	lda __rc2
	sta __rc18
	lda __rc3
	sta __rc19
	jsr __call_indir
	pla
	jsr __set_prg_8000
	rts
