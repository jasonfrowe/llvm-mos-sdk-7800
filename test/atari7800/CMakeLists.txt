cmake_minimum_required(VERSION 3.18)

project(test-atari7800 LANGUAGES C)

include(CTest)

add_executable(smoke.a78 smoke.c)

add_test(NAME atari7800-smoke-a78-header
  COMMAND ${CMAKE_COMMAND}
    -DARTIFACT=$<TARGET_FILE:smoke.a78>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/check-a78-header.cmake)
