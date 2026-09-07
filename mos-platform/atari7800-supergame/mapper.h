// Atari 7800 "SuperGame" (Type 2, 8x16KiB banks) bankswitching support.
//
// Hardware (confirmed against this SDK's own test emulator,
// third_party/a7800/src/devices/bus/a7800/rom.cpp, a78_rom_sg_device):
//   $4000-$7FFF: fixed to bank 6 (second-to-last).
//   $8000-$BFFF: switchable among banks 0-7. A write of any byte value to
//                ANY address in this range selects the bank shown here.
//   $C000-$FFFF: fixed to bank 7 (last) -- vectors/reset live here.
// Both fixed halves are mapped simultaneously at all times, so code in one
// can call code in the other directly, with no wrapper needed. Only a call
// that crosses into/out of the switchable $8000-$BFFF window needs
// banked_call_8000.
//
// API modeled on mos-platform/nes-mmc3/mapper.h's banked_call_8000, but
// simplified: SuperGame's bank switch is a single byte write (no MMC3-style
// two-register select protocol), so there is no retry loop.

#ifndef _ATARI7800_MAPPER_H_
#define _ATARI7800_MAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

// Switch to the given bank at $8000-$BFFF. Your prior bank is not saved.
__attribute__((leaf)) void set_prg_8000(unsigned char bank_id);

// Get the current bank at $8000-$BFFF.
__attribute__((leaf)) unsigned char get_prg_8000(void);

// Switch to bank_id, call method with no arguments, then switch back to
// whatever bank was active at $8000-$BFFF before the call. Safe to nest:
// calling banked_call_8000 again from within method works correctly.
__attribute__((leaf, callback(2))) void banked_call_8000(unsigned char bank_id,
                                                         void (*method)(void));

#ifdef __cplusplus
}
#endif

#endif // _ATARI7800_MAPPER_H_
