add_executable(palette-demo.a78 palette-demo.c)
target_compile_options(palette-demo.a78 PRIVATE "-Os")
install_example(palette-demo.a78)

add_executable(displaylist-demo.a78 displaylist-demo.c)
target_compile_options(displaylist-demo.a78 PRIVATE "-Os")
install_example(displaylist-demo.a78)

add_executable(asset-bridge-demo.a78 asset-bridge-demo.c)
target_compile_options(asset-bridge-demo.a78 PRIVATE "-Os")
install_example(asset-bridge-demo.a78)

set(CMAKE_C_FLAGS_MINSIZEREL "-Oz -DNDEBUG")

add_executable(astrowing.a78 astrowing/astrowing.c)
target_compile_options(astrowing.a78 PRIVATE "-Oz")
install_example(astrowing.a78)
