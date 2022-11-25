#include "aio.h"
#include "coroutine.h"

void io_init()
{
    thread_env()->io_mgr.epoll_max = 1024;
    thread_env()->io_mgr.epoll_id = epoll_create(1);
    thread_env()->io_mgr.epoll_events = (struct epoll_event*)malloc(thread_env()->io_mgr.epoll_max * sizeof(struct epoll_event));
    memset(thread_env()->io_mgr.epoll_events, 0, thread_env()->io_mgr.epoll_max * sizeof(struct epoll_event));
    map_init(&thread_env()->io_mgr.wait_map, less, equals);
}
void io_destroy()
{
    map_destroy(&thread_env()->io_mgr.wait_map);
}

void io_update(suseconds_t timeout)
{
    if (!map_size(&thread_env()->io_mgr.wait_map)) {
        return;
    }
    int ready = epoll_wait(thread_env()->io_mgr.epoll_id, thread_env()->io_mgr.epoll_events, thread_env()->io_mgr.epoll_max, timeout);
    if (ready > 0) {
        for (int i = 0; i < ready; ++ i) {
            int events = thread_env()->io_mgr.epoll_events[i].events; 
            wait_info_t * wait = (wait_info_t*)thread_env()->io_mgr.epoll_events[i].data.ptr; 
            if (events & EPOLLIN) {
                co_resume(wait->read_co);
            }
            if (events & EPOLLOUT) {
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
    if (events & EPOLLIN) {
        wait->read_co = cur;
        if (wait->write_co == cur) {
            wait->write_co = CO_ID_INVALID;
        }
    } else if (events & EPOLLOUT) {
        wait->write_co = cur;
        if (wait->read_co == cur) {
            wait->read_co = CO_ID_INVALID;
        }
    }
    co_yield();
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

    struct epoll_event event;
    event.data.ptr = wait;
    event.events = EPOLLIN | EPOLLOUT;
    epoll_ctl(thread_env()->io_mgr.epoll_id, EPOLL_CTL_ADD, fd, &event);
    co_debug("%d add to io set", fd);
}

void io_del(fd)
{
    co_debug("%d remove from io set", fd);
    epoll_ctl(thread_env()->io_mgr.epoll_id, EPOLL_CTL_DEL, fd, NULL);
    
    map_iterator_t iter = map_find(&thread_env()->io_mgr.wait_map, fd);
    wait_info_t * wait = map_iterator_get(iter);
    free(wait);
    map_erase_iter(&thread_env()->io_mgr.wait_map, iter);
}


