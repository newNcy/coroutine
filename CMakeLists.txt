
cmake_minimum_required(VERSION 2.8)
project(coroutine)

enable_language(C ASM)


set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-g -fno-strict-aliasing -Wall -pipe  -D_GNU_SOURCE -D_REENTRANT -fPIC -Wno-deprecated -m64 -DLOG_INFO -DLOG_DEBUG")

include_directories(container)
add_library(container
        container/array.c
        container/heap.c
        container/list.c
        container/map.c)

add_library(coroutine
	hook.c
	coroutine.c
	context.S
    timer.c
    aio.c
	)

add_library(aio aio.c)

if(UNIX) 
    target_sources(aio PRIVATE aio_epoll.c)
else()
    target_sources(aio PRIVATE aio_select.c)
endif()


target_link_libraries(coroutine container aio)

if(UNIX)
    target_link_libraries(coroutine dl)
elseif (WIN32)
    target_link_libraries(coroutine ws2_32)
endif()

#add_definitions("-g -DLOG_INFO -DLOG_DEBUG")

add_subdirectory(tests)

