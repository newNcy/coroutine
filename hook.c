#define _GNU_SOURCE
#include "coroutine.h"

#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

#include "heap.h"
#include "map.h"
#include "list.h"

typedef struct co_timer_t 
{
    int co_id;
	struct timeval expiration_time;
}co_timer_t;

typedef struct
{
    int read_co;
    int write_co;
} wait_info_t;

typedef struct co_io_mgr_t
{
    int epoll_id;
    int epoll_max;
    struct epoll_event * epoll_events;
    map_t wait_map;
} co_io_mgr_t;

heap_t timer_heap = {0};

co_io_mgr_t co_io_mgr = {0};

int timeval_less(struct timeval * lhs, struct timeval * rhs)
{
    return lhs->tv_sec < rhs->tv_sec || lhs->tv_sec == rhs->tv_sec && lhs->tv_usec < rhs->tv_usec;
}

int timer_compare(co_timer_t * lhs, co_timer_t * rhs)
{
    return timeval_less(&lhs->expiration_time, &rhs->expiration_time);
}

int usleep(useconds_t us)
{
    if (us <= 0) {
        return 0;
    }
    co_timer_t * timer = (co_timer_t*)malloc(sizeof(co_timer_t));
    timer->co_id = co_running();
    gettimeofday(&timer->expiration_time, NULL);
    timer->expiration_time.tv_sec += us/1000000;
    timer->expiration_time.tv_usec += us%1000000;
    heap_push(&timer_heap, timer);
    co_yield();
    return 1;
}

unsigned int sleep(unsigned int s)
{
    return usleep(s * 1000 * 1000);
}

suseconds_t process_timer()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    while (!heap_empty(&timer_heap)) {
        co_timer_t * timer = (co_timer_t*)heap_top(&timer_heap);
        if (timeval_less(&now, &timer->expiration_time)) {
            return (timer->expiration_time.tv_sec - now.tv_sec) * 1000 + (timer->expiration_time.tv_usec - now.tv_usec)/1000;
        } else {
            int id = timer->co_id;
            free(timer);
            heap_pop(&timer_heap);

            co_resume(id);
        }
    }
    return -1;
}



/////////////hook///////////////

typedef int (*socket_func_t)(int domain, int type, int protocol);
typedef int (*accept_func_t)(int fd, struct sockaddr *addr, socklen_t *len);
typedef int (*connect_func_t)(int fd, const struct sockaddr *addr, socklen_t len);
typedef int (*recv_func_t)(int fd, void * buf, size_t len, int flags);
typedef int (*send_func_t)(int fd, void * buf, size_t len, int flags);
typedef int (*close_func_t)(int fd);

//todo open read write ...


int setnoblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

#define HOOK_FUNC( x ) static x##_func_t _##x = NULL; if (!_##x)  _##x = (x##_func_t)dlsym(RTLD_NEXT, #x)

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
}

int socket(int domain, int type, int protocal)
{
    HOOK_FUNC(socket);
    int sock = _socket(domain, type, protocal); 
	setnoblocking(sock);
    co_io_add(sock);
    return sock;
}

int accept(int fd, struct sockaddr * addr, socklen_t * len)
{
    HOOK_FUNC(accept);
    co_debug("async accept");
    co_io_wait(fd, EPOLLIN);
    int sock = _accept(fd, addr, len);
    co_debug("async accept finish");
    co_io_add(sock);
    return sock;
}

int connect(int fd, const struct sockaddr * addr, socklen_t len)
{
	HOOK_FUNC(connect);
	int ret = _connect(fd, addr, len);
	if (ret == 0) {
		co_io_add(fd);
		return ret;
	} else if (errno !=  EINPROGRESS) {
		return ret;
	}
	co_io_wait(fd, EPOLLOUT);
	int error = 0;
	len = sizeof(error);
	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0)  {
		return -1;
	}
	return 0;
}


ssize_t recv(int fd, void * buff, size_t len, int flags) 
{
    HOOK_FUNC(recv);
    co_debug("async recv");
    co_io_wait(fd, EPOLLIN);
    co_debug("async recv finish");
    return _recv(fd, buff, len, flags);
}

ssize_t send(int fd, const void * buff, size_t len, int flags)
{
    HOOK_FUNC(send);
    co_debug("async send");
    co_io_wait(fd, EPOLLOUT);
    co_debug("async send finish");
    return _send(fd, buff, len, flags);
}

int close(int fd)
{
    HOOK_FUNC(close);
    co_io_del(fd);
    return _close(fd);
}


void co_event_init()
{
    co_io_mgr.epoll_max = 1024;
    co_io_mgr.epoll_id = epoll_create(1);
    co_io_mgr.epoll_events = (struct epoll_event*)malloc(co_io_mgr.epoll_max * sizeof(struct epoll_event));
    memset(co_io_mgr.epoll_events, 0, co_io_mgr.epoll_max * sizeof(struct epoll_event));

    heap_init(&timer_heap, timer_compare);
    map_init(&co_io_mgr.wait_map, less, equals);
}

void co_event_loop()
{
    while (1) {
        suseconds_t next_wake = process_timer();
        int ready = epoll_wait(co_io_mgr.epoll_id, co_io_mgr.epoll_events, co_io_mgr.epoll_max, next_wake);
        if (ready > 0) {
            for (int i = 0; i < ready; ++ i) {
                int events = co_io_mgr.epoll_events[i].events; 
                wait_info_t * wait = (wait_info_t*)co_io_mgr.epoll_events[i].data.ptr; 
                if (events & EPOLLIN) {
                    co_resume(wait->read_co);
                }
                if (events & EPOLLOUT) {
                    co_resume(wait->write_co);
                }
            }
        }
        if (co_is_all_finish()) {
            break;
        }
    }
    heap_destroy(&timer_heap);
    map_destroy(&co_io_mgr.wait_map);
}




