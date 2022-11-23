#pragma once

#include <sys/time.h>
typedef struct co_timer_t 
{
    int co_id;
	struct timeval expiration_time;
}co_timer_t;

void co_event_init();
void co_event_loop();

