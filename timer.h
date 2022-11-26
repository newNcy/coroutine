#pragma once

#include <sys/time.h>
#include "heap.h"
typedef struct co_timer_t 
{
    int co_id;
	struct timeval expiration_time;
}co_timer_t;

void timer_mgr_init(heap_t * heap);

