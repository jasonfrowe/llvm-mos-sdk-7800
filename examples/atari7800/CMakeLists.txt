add_executable(astrowing.a78
  astrowing/astrowing.c
  astrowing/music_data.S
  astrowing/assets_blocks.c
)
target_compile_options(astrowing.a78 PRIVATE "-Os")
install_example(astrowing.a78)

add_custom_command(TARGET astrowing.a78 POST_BUILD
  COMMAND /Users/jasonrowe/Software/Atari7800/GitHub/7800basic/7800sign -w $<TARGET_FILE:astrowing.a78>
  COMMENT "Signing Atari 7800 cartridge"
)
