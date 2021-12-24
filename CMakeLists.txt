
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
	)

add_library(coroutine_hook 
	hook.c
	)
target_link_libraries(coroutine container )

add_definitions("-g -DLOG_INFO -DLOG_DEBUG")

add_executable(test_io main.c)
target_link_libraries(test_io coroutine dl coroutine_hook)

add_subdirectory(tests)

