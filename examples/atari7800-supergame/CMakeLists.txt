add_executable(bank-min.a78 bank-min/bank-min.c)
target_compile_options(bank-min.a78 PRIVATE "-Os")
install_example(bank-min.a78)

# Migrated from examples/atari7800/astrowing/ (flat 48K platform, now
# retired) once it hit that platform's practical ROM ceiling -- see
# 7800port.md #25. No __cart_pokey_flags/__cart_rom_size --defsym overrides
# needed here: mos-platform/atari7800-supergame/link.ld already hardcodes
# the SuperGame+POKEY@450 header and 128K ROM size unconditionally.
add_executable(astrowing.a78 astrowing/astrowing.c)
target_compile_options(astrowing.a78 PRIVATE "-Os")
# atari7800/clang.cfg's -mlto-zp=96 (inherited via cfg chaining) leaves this
# program's "zp" linker region (mos-platform/atari7800-supergame/link.ld)
# exactly full -- confirmed the flat, non-banked predecessor of this program
# built at that same ceiling with zero slack. banked_call_8000's one extra
# persistent byte (mapper.h's __prg_8000) then overflows it by exactly 1.
# LTO's zp promotion turned out to be adaptive enough that trimming a named
# global to compensate didn't help (it just promoted a different temporary
# into the freed byte instead, net zero) -- the real fix is constraining
# the compiler's own budget by the same 1 byte, forcing it to leave that
# byte free rather than opportunistically filling it.
target_link_options(astrowing.a78 PRIVATE "-mlto-zp=95")
install_example(astrowing.a78)

option(ATARI7800_ASTROWING_DEBUG_FRAME_BUDGET
  "Flash the background color during astrowing's per-frame CPU work, to visually check it fits inside vblank"
  OFF)
if(ATARI7800_ASTROWING_DEBUG_FRAME_BUDGET)
  target_compile_definitions(astrowing.a78 PRIVATE ATARI7800_DEBUG_FRAME_BUDGET=1)
endif()
