
project(coroutine)

enable_language(ASM)
add_library(container
        array.c
        heap.c
        )

add_library(coroutine
        coroutine.c
        hook.c
        context.S
        )

add_definitions("-g")

add_executable(test_io main.c)

target_link_libraries(test_io container coroutine dl)
