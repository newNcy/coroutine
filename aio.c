#include "aio.h"
#include "coroutine.h"
#include <assert.h>
#include "macros.h"
#ifdef WIN32
#include <winsock2.h>
#else 
#include <fcntl.h>
#endif
#include <stdlib.h>

extern void *event_create();
extern void event_destroy(void * ctx);
extern int event_wait(void * ctx, array_t * fired, long long timeout);
extern void event_add(void * ctx, int fd);
extern void event_del(void * ctx, int fd);

aio_t * aio_create()
{
    aio_t * aio = (aio_t*)malloc(sizeof(aio_t));
    aio->wait_map = map_create(nullptr, nullptr);
    aio->fired_events = array_create();
    aio->event_ctx = event_create();
    aio->dead = list_create();

    return aio;
}
void aio_destroy(aio_t * aio)
{
    event_destroy(aio->event_ctx);
    map_destroy(aio->wait_map);
}

any_t encode_event(int fd, int events) 
{ 
    return (any_t)(fd << 3 | events);
}

void decode_event(any_t event, int * fd, int * events) 
{ 
    *fd = (long long )event>>3; 
    *events = (long long)event & 0x7; 
}

void aio_update(aio_t * aio, long long timeout)
{
    if (!map_size(aio->wait_map)) {
        return;
    }

    int num_events = event_wait(aio->event_ctx, aio->fired_events, timeout);
    for (int i = 0; i < num_events; ++ i) {
        int fd = 0;
        int events = 0;
        any_t e = array_get(aio->fired_events, i);
        decode_event(e, &fd, &events);
        map_iterator_t iter = map_get(aio->wait_map, (any_t)fd);
        if (map_iterator_valid(aio->wait_map, iter)) {
            wait_info_t * wait = (wait_info_t*)map_iterator_get(iter);
            if ((events & IO_READ) && !list_empty(wait->reader)) {
                co_t * co = list_pop_front(wait->reader);
                co_resume(co);
            }
            if ((events & IO_WRITE) && !list_empty(wait->writer)) {
                co_t * co = list_pop_front(wait->writer);
                co_resume(co);
            }
        }
    }

    while (!list_empty(aio->dead)) {
        wait_info_t * wait = (wait_info_t*)list_pop_front(aio->dead);
        while (!list_empty(wait->reader)) {
            co_t * co = list_pop_front(wait->reader);
            co_resume(co);
        }
        list_destroy(wait->reader);

        while (!list_empty(wait->writer)) {
            co_t * co = list_pop_front(wait->writer);
            co_resume(co);
        }
        list_destroy(wait->writer);
        free(wait);
    }
}

void aio_wait(aio_t * aio, int fd, int events)
{
    map_iterator_t iter = map_get(aio->wait_map, (any_t)fd);
    if (!map_iterator_valid(aio->wait_map, iter)) {
        return;
    }
    wait_info_t * wait = (wait_info_t*)map_iterator_get(iter);
    if (events & IO_READ) {
        list_push_back(wait->reader, co_running());
    } 
    if (events & IO_WRITE) {
        list_push_back(wait->writer, co_running());
    }
    
    co_yield();
}

int io_setnoblocking(int fd)
{
#ifdef WIN32
    int mode = 1;
    ioctlsocket(fd, FIONBIO, &mode);
#else
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
#endif
}


void aio_add(aio_t * aio, int fd)
{
    map_iterator_t iter = map_get(aio->wait_map, (any_t)fd);
    wait_info_t * wait = nullptr;
    if (!map_iterator_valid(aio->wait_map, iter)) {
        wait = (wait_info_t*)malloc(sizeof(wait_info_t));
        wait->reader = list_create();
        wait->writer = list_create();
        map_set(aio->wait_map, (any_t)fd, wait);
    }
    event_add(aio->event_ctx, fd);
    co_info("%d add to io set", fd);
}

void aio_del(aio_t * aio, int fd)
{
    co_info("%d remove from io set", fd);
    event_del(aio->event_ctx, fd);
    map_iterator_t iter = map_get(aio->wait_map, (any_t)fd);
    assert(iter);
    if (map_iterator_valid(aio->wait_map, iter)) {
        wait_info_t * wait = map_iterator_get(iter);
        list_push_back(aio->dead, wait);
        map_remove_key(aio->wait_map, (any_t)fd);
    }
}

void aio_debug_print_info()
{
    aio_t * aio = thread_env()->aio;
    map_t  * wait_map = aio->wait_map;

    for (map_iterator_t it = map_begin(wait_map); it != map_end(wait_map); it = map_next(it)) {
        wait_info_t * wait = (wait_info_t*)map_iterator_get(it);
        list_t * rl = wait->reader;
        list_t * wl = wait->writer;
        printf("r/w of %d count:%d/%d\n", (int)it->key, list_size(rl), list_size(wl));
    }
}
