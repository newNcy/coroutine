
cmake_minimum_required(VERSION 2.8)
project(coroutine)

enable_language(C)
add_compile_options(-Wreturn)

if (UNIX OR MINGW)
    enable_language(ASM)
    set(ASM_OPTIONS "-x assembler-with-cpp")
    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-g -D_GNU_SOURCE -fno-strict-aliasing -D_GNU_SOURCE -Wall -pipe -fPIC -Wno-deprecated -Wreturn-type" )
else()
    enable_language(ASM_MASM)
endif()


add_library(ctx ctx.h)
message("${CMAKE_SYSTEM_PROCESSOR}" )
if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    target_sources(ctx PRIVATE ctx.arm64.asm)
elseif (UNIX OR MINGW)
    target_sources(ctx PRIVATE ctx.S)
else()
    target_sources(ctx PRIVATE ctx.asm)
endif()

include_directories(container)
add_library(container
        container/array.c
        container/heap.c
        container/list.c
        container/map.c
        )

add_library(coroutine
	coroutine.c
    timer.c
	)

add_library(aio aio.c hook.c)
target_link_libraries(aio container)

if (APPLE)
    target_sources(aio PRIVATE event_kquene.c)
elseif(UNIX) 
    target_sources(aio PRIVATE event_epoll.c)
else()
    target_sources(aio PRIVATE event_select.c)
endif()


target_link_libraries(coroutine ctx container aio )

if(UNIX)
    target_link_libraries(coroutine dl)
elseif (WIN32)
    target_link_libraries(coroutine ws2_32)
endif()

#add_definitions("-g -DLOG_INFO -DLOG_DEBUG")

add_subdirectory(tests)

