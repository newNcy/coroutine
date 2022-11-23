#pragma once
#define CO_STACK_SIZE 1024 * 1024 * 2
#define true 1

#ifdef LOG_DEBUG
#define co_debug(fmt,...) printf("[debug] [%d] "fmt"\n", co_running(),  ##__VA_ARGS__)
#else
#define co_debug
#endif

#ifdef LOG_INFO
#define co_info(fmt,...) printf("[info] [%d] "fmt"\n", co_running(),  ##__VA_ARGS__)
#else
#define co_info
#endif

#define async (void*)
#ifndef __cplusplus
#define thread_local __thread
#endif


