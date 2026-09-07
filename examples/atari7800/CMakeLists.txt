add_executable(palette-demo.a78 palette-demo.c)
target_compile_options(palette-demo.a78 PRIVATE "-Os")
install_example(palette-demo.a78)

add_executable(displaylist-demo.a78 displaylist-demo.c)
target_compile_options(displaylist-demo.a78 PRIVATE "-Os")
install_example(displaylist-demo.a78)

add_executable(asset-bridge-demo.a78 asset-bridge-demo.c)
target_compile_options(asset-bridge-demo.a78 PRIVATE "-Os")
install_example(asset-bridge-demo.a78)

add_executable(astrowing.a78 astrowing/astrowing.c)
target_compile_options(astrowing.a78 PRIVATE "-Os")
install_example(astrowing.a78)

# Minimal POKEY crash repro: 48K + POKEY@450, byte-for-byte identical header
# to the reference astrowing.bas build (see 7800port.md #16), yet the CPU
# still crashes within ~1 second even with NO scene/graphics code at all.
# Kept permanently (not reverted like title-demo/astrowing below) as the
# smallest possible test case for whoever next investigates this --
# real hardware, or deeper emulator-side study.
add_executable(pokey-min.a78 pokey-min/pokey-min.c)
target_link_options(pokey-min.a78 PRIVATE "-Wl,--defsym=__cart_pokey_flags=2" "-Wl,--defsym=__cart_rom_size=48")
install_example(pokey-min.a78)

add_executable(title-demo.a78 title-demo/title-demo.c)
target_compile_options(title-demo.a78 PRIVATE "-Os")
# 48K + POKEY@450: re-enabled (7800port.md #19) after finding and fixing the
# actual root cause in the test emulator itself (third_party/a7800, a fork
# of 7800-devtools/a7800 this SDK now owns and builds from source) -- its
# a78_p450_t0 cart-slot device was claiming the full 0x0400-0x04FF for
# POKEY, when only 0x0400-0x047F should be (0x0480-0x04FF is real 6532 RIOT
# RAM per the 7800 Software Guide's memory map); the real BIOS's own boot
# sequence walks its PC through that range and crashes on the resulting
# constant 0xFF stub. Confirmed fixed against pokey-min.a78 and this target
# alike.
target_link_options(title-demo.a78 PRIVATE "-Wl,--defsym=__cart_pokey_flags=2" "-Wl,--defsym=__cart_rom_size=48")
install_example(title-demo.a78)

option(ATARI7800_ASTROWING_DEBUG_FRAME_BUDGET
  "Flash the background color during astrowing's per-frame CPU work, to visually check it fits inside vblank"
  OFF)
if(ATARI7800_ASTROWING_DEBUG_FRAME_BUDGET)
  target_compile_definitions(astrowing.a78 PRIVATE ATARI7800_DEBUG_FRAME_BUDGET=1)
endif()
