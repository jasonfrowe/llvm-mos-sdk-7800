platform(atari7800-supergame COMPLETE PARENT atari7800)

if(NOT CMAKE_CROSSCOMPILING)
  return()
endif()

install(FILES mapper.h TYPE INCLUDE)

add_platform_library(atari7800-supergame-c
  mapper.s
)
target_link_libraries(atari7800-supergame-c PRIVATE common-asminc)
