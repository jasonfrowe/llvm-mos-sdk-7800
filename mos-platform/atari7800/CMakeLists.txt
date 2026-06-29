platform(atari7800 COMPLETE HOSTED PARENT common)

if(NOT CMAKE_CROSSCOMPILING)
  return()
endif()

install(FILES atari7800.h TYPE INCLUDE)

add_platform_library(atari7800-crt0
  reset.S
)
merge_libraries(atari7800-crt0
  common-init-stack
  common-copy-data
  common-zero-bss
  common-exit-loop
)

add_platform_library(atari7800-c
  atari7800.c
  getchar.c
  putchar.c
)
target_include_directories(atari7800-c BEFORE PUBLIC .)
