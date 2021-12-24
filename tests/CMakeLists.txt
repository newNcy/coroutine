
include_directories(..)

add_executable(test_ctx test_ctx.c)
target_link_libraries(test_ctx coroutine dl)

add_executable(test_container test_container.c)
target_link_libraries(test_container container)

