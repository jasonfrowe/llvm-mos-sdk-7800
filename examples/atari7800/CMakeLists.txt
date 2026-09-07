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

add_executable(title-demo.a78 title-demo/title-demo.c)
target_compile_options(title-demo.a78 PRIVATE "-Os")
# 48 KiB, non-bankswitched (just 16KiB more flat ROM than the 32K default,
# mapped at 0x4000-0x7FFF -- see mos-platform/atari7800/link.ld) + POKEY @
# $450 are NOT enabled here: confirmed both are required together to match
# every real POKEY-equipped astrowing.bas build, but the combination
# crashes outright in this SDK's test emulator (a full black screen, CPU
# executing garbage) even at 48K -- ruling out ROM size as the actual
# blocker, and a follow-up IRQ-masking fix (atari7800_pokey_init) didn't
# resolve it either (confirmed via direct flag inspection that the crash
# isn't even a maskable-IRQ storm). 48K alone (no POKEY) was confirmed
# to work correctly. See 7800port.md for the full investigation; this
# needs more study of the emulator's cart-slot implementation, or real
# hardware, before enabling POKEY here again.
install_example(title-demo.a78)

option(ATARI7800_ASTROWING_DEBUG_FRAME_BUDGET
  "Flash the background color during astrowing's per-frame CPU work, to visually check it fits inside vblank"
  OFF)
if(ATARI7800_ASTROWING_DEBUG_FRAME_BUDGET)
  target_compile_definitions(astrowing.a78 PRIVATE ATARI7800_DEBUG_FRAME_BUDGET=1)
endif()
