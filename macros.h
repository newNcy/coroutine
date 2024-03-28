#pragma once
#define CO_STACK_SIZE 1024 * 1024 * 2
#define true 1

#ifdef LOG_DEBUG
#define co_debug(fmt,...) printf("[debug] [%d] "fmt"\n", co_running()? co_running()->id : -1,  ##__VA_ARGS__)
#else
#define co_debug
#endif

#ifdef LOG_INFO
#define co_info(fmt,...) printf("[info] [%d] "fmt"\n", co_running()? co_running()->id : -1,  ##__VA_ARGS__)
#else
#define co_info
#endif

#define async (void*)
#ifndef __cplusplus
    #ifndef WIN32
        #define thread_local __thread
    #else
        #define thread_local __declspec(thread)
    #endif
#endif
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#endif


