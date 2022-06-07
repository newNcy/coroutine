
project(coroutine)

enable_language(ASM)
add_library(container
        array.c
        heap.c
        list.c
        map.c
        )

add_library(coroutine
	coroutine.c
	context.S
	hook.c
    aio.c
	)

target_link_libraries(coroutine container dl)

#add_definitions("-g -DLOG_INFO -DLOG_DEBUG")
add_definitions("-g")

add_subdirectory(tests)

