
cmake_minimum_required(VERSION 2.8)
project(coroutine)

enable_language(C ASM)
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-g -fno-strict-aliasing -Wall -export-dynamic -Wall -pipe  -D_GNU_SOURCE -D_REENTRANT -fPIC -Wno-deprecated -m64 -DLOG_DEBUG -DLOG_INFO")

add_library(container
        container/array.c
        container/heap.c
        container/list.c
        container/map.c)
include_directories(container)
add_library(coroutine
	hook.c
	coroutine.c
	context.S
    timer.c
    aio.c
	)

target_link_libraries(coroutine container dl)

#add_definitions("-g -DLOG_INFO -DLOG_DEBUG")

add_subdirectory(tests)

