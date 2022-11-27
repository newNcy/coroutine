#pragma once
#include "macros.h"
#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/time.h>
#endif
#include "heap.h"
typedef struct co_timer_t 
{
    int co_id;
	struct timeval expiration_time;
}co_timer_t;

void timer_mgr_init(heap_t * heap);

