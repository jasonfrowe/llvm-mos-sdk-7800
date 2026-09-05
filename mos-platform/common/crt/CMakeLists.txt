# libcrt is the libcalls needed by the compiler. All libcalls emitted by the
# compiler (except C standard library functions) should be here.
add_platform_library(common-crt
  const.S

  call-indir.S

  divmod.cc
  divmod-large.cc
  mul.cc
  shift.cc
  rotate.cc
)

# Merge the builtins library from llvm-mos into libcrt.
# Query clang for its resource directory
execute_process(
  COMMAND ${CMAKE_C_COMPILER} -print-resource-dir
  RESULT_VARIABLE CLANG_RESOURCE_RESULT
  OUTPUT_VARIABLE CLANG_RESOURCE_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT CLANG_RESOURCE_RESULT EQUAL 0)
  message(FATAL_ERROR
    "Failed to run ${CMAKE_C_COMPILER} -print-resource-dir")
endif()

set(BUILTINS_DIR
  ${CLANG_RESOURCE_DIR}/lib/mos-unknown-unknown)

if(NOT EXISTS "${BUILTINS_DIR}")
  message(FATAL_ERROR
    "Builtins directory not found: ${BUILTINS_DIR}")
endif()

# A plain (non-cache) variable is used here instead of find_library(), since
# find_library() caches its result in CMakeCache.txt forever. The toolchain is
# fetched from a mutable "main" release (see cmake/bootstrap-compiler.cmake),
# so its clang version/resource-dir can change across re-downloads without a
# fresh configure being forced, which would otherwise leave a stale cached path.
set(builtins ${BUILTINS_DIR}/libclang_rt.builtins.a)
if(NOT EXISTS "${builtins}")
  message(FATAL_ERROR "builtins library not found: ${builtins}")
endif()

merge_libraries(common-crt ${builtins})
