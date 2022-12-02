
cmake_minimum_required(VERSION 2.8)
include_directories(..)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
add_executable(test_ctx test_ctx.c)
target_link_libraries(test_ctx coroutine)

add_executable(test_container test_container.c)
target_link_libraries(test_container container)

add_executable(test_io test_io.c)
target_link_libraries(test_io coroutine )

add_executable(test_cpp test_cpp.cpp)
target_link_libraries(test_cpp coroutine)

add_executable(test test.c)
target_link_libraries(test coroutine)

add_executable(test_echo_svr test_tcp_echo_svr.c)
target_link_libraries(test_echo_svr coroutine)

add_executable(test_echo_cli test_tcp_echo_cli.c)
target_link_libraries(test_echo_cli coroutine)
