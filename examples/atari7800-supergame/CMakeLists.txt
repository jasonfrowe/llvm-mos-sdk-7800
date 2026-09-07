add_executable(bank-min.a78 bank-min/bank-min.c)
target_compile_options(bank-min.a78 PRIVATE "-Os")
install_example(bank-min.a78)
