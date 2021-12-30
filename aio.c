#include "aio.h"
#include "coroutine.h"

co_io_mgr_t co_io_mgr = {0};

void co_io_wait(int fd, int events)
{
    map_iterator_t iter = map_find(&co_io_mgr.wait_map, fd);
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
    map_iterator_t iter = map_find(&co_io_mgr.wait_map, fd);
    wait_info_t * wait = nullptr;
    if (map_iterator_valid(&co_io_mgr.wait_map, iter)) {
        wait = map_iterator_get(iter);
    } else {
        wait = (wait_info_t*)malloc(sizeof(wait_info_t));
        wait->read_co = CO_ID_INVALID;
        wait->write_co = CO_ID_INVALID;
        map_set(&co_io_mgr.wait_map, fd, wait);
    }

    struct epoll_event event;
    event.data.ptr = wait;
    event.events = EPOLLIN | EPOLLOUT;
    epoll_ctl(co_io_mgr.epoll_id, EPOLL_CTL_ADD, fd, &event);
    co_debug("%d add to io set", fd);
}

void co_io_del(fd)
{
    co_debug("%d remove from io set", fd);
    epoll_ctl(co_io_mgr.epoll_id, EPOLL_CTL_DEL, fd, NULL);
    
    map_iterator_t iter = map_find(&co_io_mgr.wait_map, fd);
    wait_info_t * wait = map_iterator_get(iter);
    free(wait);

}


