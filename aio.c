#include "aio.h"
#include "coroutine.h"
#ifdef WIN32
#include <winsock.h>
#else 
#include <fcntl.h>
#endif

void io_init()
{
    env_t * env = thread_env();
    event_init(&env->io_mgr);
    map_init(&thread_env()->io_mgr.wait_map, less, equals);
}
void io_destroy()
{
    event_destroy(&thread_env()->io_mgr);
    map_destroy(&thread_env()->io_mgr.wait_map);
}

any_t encode_event(int fd, int events) 
{ 
    return fd << 3 | events;
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

    int num_events = event_wait(io_mgr);
    for (int i = 0; i < num_events; ++ i) {
        int fd = 0;
        int events = 0;
        any_t e = array_get(&io_mgr->fired_events, i);
        decode_event(e, &fd, &events);
        map_iterator_t iter = map_find(&io_mgr->wait_map, fd);
        if (map_iterator_valid(&io_mgr->wait_map, iter)) {
            wait_info_t * wait = (wait_info_t*)map_iterator_get(iter);
            if (events & IO_READ) {
                co_resume(wait->read_co);
            }
            if (events & IO_WRITE) {
                co_resume(wait->write_co);
            }
        }
    }
}

void io_wait(int fd, int events)
{
    map_iterator_t iter = map_find(&thread_env()->io_mgr.wait_map, fd);
    wait_info_t * wait = (wait_info_t*)map_iterator_get(iter);
    int cur = co_running();
    if (events & IO_READ) {
        wait->read_co = cur;
        if (wait->write_co == cur) {
            wait->write_co = CO_ID_INVALID;
        }
    } else if (events & IO_WRITE) {
        wait->write_co = cur;
        if (wait->read_co == cur) {
            wait->read_co = CO_ID_INVALID;
        }
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


void io_add(fd)
{
    map_iterator_t iter = map_find(&thread_env()->io_mgr.wait_map, fd);
    wait_info_t * wait = nullptr;
    if (map_iterator_valid(&thread_env()->io_mgr.wait_map, iter)) {
        wait = map_iterator_get(iter);
    } else {
        wait = (wait_info_t*)malloc(sizeof(wait_info_t));
        wait->read_co = CO_ID_INVALID;
        wait->write_co = CO_ID_INVALID;
        map_set(&thread_env()->io_mgr.wait_map, fd, wait);
    }
    event_add(&thread_env()->io_mgr, fd);
    co_debug("%d add to io set", fd);
}

void io_del(fd)
{
    co_debug("%d remove from io set", fd);
    event_del(&thread_env()->io_mgr, fd);
    map_iterator_t iter = map_find(&thread_env()->io_mgr.wait_map, fd);
    if (map_iterator_valid(&thread_env()->io_mgr.wait_map, iter)) {
        wait_info_t * wait = map_iterator_get(iter);
        free(wait);
        map_erase_iter(&thread_env()->io_mgr.wait_map, iter);
    }
}


