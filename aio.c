#include "aio.h"
#include "coroutine.h"
#include "macros.h"
#ifdef WIN32
#include <winsock2.h>
#else 
#include <fcntl.h>
#endif
#include <stdlib.h>

extern  void event_init(io_mgr_t * io_mgr);
extern void event_destroy(io_mgr_t * io_mgr);
extern int event_wait(io_mgr_t * io_mgr, long long timeout);
extern void event_add(io_mgr_t * io_mgr, int fd);
extern void event_del(io_mgr_t * io_mgr, int fd);
void io_init()
{
    env_t * env = thread_env();
    event_init(&env->io_mgr);
    map_init(&thread_env()->io_mgr.wait_map, less, equals);
    env->io_mgr.dead = list_create();
}
void io_destroy()
{
    event_destroy(&thread_env()->io_mgr);
    map_destroy(&thread_env()->io_mgr.wait_map);
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

void io_update(long long timeout)
{
    if (!map_size(&thread_env()->io_mgr.wait_map)) {
        return;
    }

    io_mgr_t * io_mgr = &thread_env()->io_mgr;

    int num_events = event_wait(io_mgr, timeout);
    for (int i = 0; i < num_events; ++ i) {
        int fd = 0;
        int events = 0;
        any_t e = array_get(&io_mgr->fired_events, i);
        decode_event(e, &fd, &events);
        map_iterator_t iter = map_find(&io_mgr->wait_map, (any_t)fd);
        if (map_iterator_valid(&io_mgr->wait_map, iter)) {
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

    while (!list_empty(io_mgr->dead)) {
        wait_info_t * wait = (wait_info_t*)list_pop_front(io_mgr->dead);
        while (!list_empty(wait->reader)) {
            co_t * co = list_pop_front(wait->reader);
            co_resume(co);
        }
        while (!list_empty(wait->writer)) {
            co_t * co = list_pop_front(wait->writer);
            co_resume(co);
        }
        list_destroy(wait->reader);
        list_destroy(wait->writer);
        free(wait);
    }
}

void io_wait(int fd, int events)
{
    map_iterator_t iter = map_find(&thread_env()->io_mgr.wait_map, (any_t)fd);
    if (!map_iterator_valid(&thread_env()->io_mgr.wait_map, iter)) {
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


void io_add(int fd)
{
    io_mgr_t * io_mgr = &thread_env()->io_mgr;
    map_iterator_t iter = map_find(&io_mgr->wait_map, (any_t)fd);
    wait_info_t * wait = nullptr;
    if (map_iterator_valid(&io_mgr->wait_map, iter)) {
        wait = map_iterator_get(iter);
    } else {
        wait = (wait_info_t*)malloc(sizeof(wait_info_t));
        wait->reader = list_create();
        wait->writer = list_create();
        map_set(&io_mgr->wait_map, (any_t)fd, wait);
    }
    event_add(&thread_env()->io_mgr, fd);
    co_info("%d add to io set", fd);
}

void io_del(int fd)
{
    io_mgr_t * io_mgr = &thread_env()->io_mgr;
    co_info("%d remove from io set", fd);
    event_del(io_mgr, fd);
    map_iterator_t iter = map_find(&io_mgr->wait_map, (any_t)fd);
    if (map_iterator_valid(&io_mgr->wait_map, iter)) {
        wait_info_t * wait = map_iterator_get(iter);
        list_push_back(io_mgr->dead, wait);
        map_remove_key(&thread_env()->io_mgr.wait_map, (any_t)fd);
    }
}


