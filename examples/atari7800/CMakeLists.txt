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
# 48K + POKEY@450 NOT enabled here: re-investigated this session (7800port.md
# #16) with a header now confirmed byte-for-byte identical to the reference
# astrowing.bas build (a real v4 audio-field byte-order bug was found and
# fixed along the way) -- the crash persists regardless, including in a
# minimal 3-line test program (pokey-min.a78, above) with no scene code at
# all. This rules out both ROM size and the header as the cause. Left
# disabled pending either real-hardware testing or deeper study of this
# MAME fork's a78_p450_t0 cart-slot memory-map implementation.
install_example(title-demo.a78)

option(ATARI7800_ASTROWING_DEBUG_FRAME_BUDGET
  "Flash the background color during astrowing's per-frame CPU work, to visually check it fits inside vblank"
  OFF)
if(ATARI7800_ASTROWING_DEBUG_FRAME_BUDGET)
  target_compile_definitions(astrowing.a78 PRIVATE ATARI7800_DEBUG_FRAME_BUDGET=1)
endif()
