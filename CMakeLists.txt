
project(coroutine)

enable_language(ASM)
add_library(container
        array.c
        heap.c
        map.c
        )

add_library(coroutine
        coroutine.c
        hook.c
        context.S
        )
target_link_libraries(coroutine container )

add_definitions("-g -DLOG_INFO")

add_executable(test_io main.c)

target_link_libraries(test_io coroutine dl)
