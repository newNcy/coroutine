#include "aio.h"
#include "coroutine.h"

aio_mgr_t aio_mgr = {0};

void aio_init()
{
    aio_mgr.epoll_max = 1024;
    aio_mgr.epoll_id = epoll_create(1);
    aio_mgr.epoll_events = (struct epoll_event*)malloc(aio_mgr.epoll_max * sizeof(struct epoll_event));
    memset(aio_mgr.epoll_events, 0, aio_mgr.epoll_max * sizeof(struct epoll_event));
    map_init(&aio_mgr.wait_map, less, equals);
}
void aio_destroy()
{
    map_destroy(&aio_mgr.wait_map);
}

void aio_update(suseconds_t timeout)
{
    if (!map_size(&aio_mgr.wait_map)) {
        return;
    }
    int ready = epoll_wait(aio_mgr.epoll_id, aio_mgr.epoll_events, aio_mgr.epoll_max, timeout);
    if (ready > 0) {
        for (int i = 0; i < ready; ++ i) {
            int events = aio_mgr.epoll_events[i].events; 
            wait_info_t * wait = (wait_info_t*)aio_mgr.epoll_events[i].data.ptr; 
            if (events & EPOLLIN) {
                co_resume(wait->read_co);
            }
            if (events & EPOLLOUT) {
                co_resume(wait->write_co);
            }
        }
    }
}

void co_io_wait(int fd, int events)
{
    map_iterator_t iter = map_find(&aio_mgr.wait_map, fd);
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

void co_io_add(fd)
{
    map_iterator_t iter = map_find(&aio_mgr.wait_map, fd);
    wait_info_t * wait = nullptr;
    if (map_iterator_valid(&aio_mgr.wait_map, iter)) {
        wait = map_iterator_get(iter);
    } else {
        wait = (wait_info_t*)malloc(sizeof(wait_info_t));
        wait->read_co = CO_ID_INVALID;
        wait->write_co = CO_ID_INVALID;
        map_set(&aio_mgr.wait_map, fd, wait);
    }

    struct epoll_event event;
    event.data.ptr = wait;
    event.events = EPOLLIN | EPOLLOUT;
    epoll_ctl(aio_mgr.epoll_id, EPOLL_CTL_ADD, fd, &event);
    co_debug("%d add to io set", fd);
}

void co_io_del(fd)
{
    co_debug("%d remove from io set", fd);
    epoll_ctl(aio_mgr.epoll_id, EPOLL_CTL_DEL, fd, NULL);
    
    map_iterator_t iter = map_find(&aio_mgr.wait_map, fd);
    wait_info_t * wait = map_iterator_get(iter);
    free(wait);
    map_erase_iter(&aio_mgr.wait_map, iter);
}


