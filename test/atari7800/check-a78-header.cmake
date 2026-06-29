if(NOT DEFINED ARTIFACT)
  message(FATAL_ERROR "ARTIFACT is not set")
endif()

if(NOT EXISTS "${ARTIFACT}")
  message(FATAL_ERROR "A78 artifact not found: ${ARTIFACT}")
endif()

file(SIZE "${ARTIFACT}" ARTIFACT_SIZE)
if(ARTIFACT_SIZE LESS 129)
  message(FATAL_ERROR "A78 artifact too small (${ARTIFACT_SIZE} bytes): ${ARTIFACT}")
endif()

# Header bytes 1..9 must be ASCII "ATARI7800".
file(READ "${ARTIFACT}" CONSOLE_HEX HEX OFFSET 1 LIMIT 9)
string(TOUPPER "${CONSOLE_HEX}" CONSOLE_HEX)
if(NOT CONSOLE_HEX STREQUAL "415441524937383030")
  message(FATAL_ERROR "A78 console signature mismatch: ${CONSOLE_HEX}")
endif()
