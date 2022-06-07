
include_directories(..)

add_executable(test_ctx test_ctx.c)
target_link_libraries(test_ctx coroutine dl)

add_executable(test_container test_container.c)
target_link_libraries(test_container container)

add_executable(test_io test_io.c)
target_link_libraries(test_io coroutine )

add_executable(test_common test_cpp.cpp)
target_link_libraries(test_common coroutine)

add_executable(test_common2 test.c)
target_link_libraries(test_common2 coroutine)
