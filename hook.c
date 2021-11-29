#define _GNU_SOURCE
#include "coroutine.h"

#include <time.h>
#include <malloc.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/epoll.h>
#include <unistd.h>

typedef struct co_timer_t 
{
    int co_id;
    time_t expiration_time;
    struct co_timer_t * next;
}co_timer_t;

typedef struct co_timer_mgr_t
{
    co_timer_t * head;
}co_timer_mgr_t;



typedef struct co_io_mgr_t
{
	int epoll_id;
	int epoll_max;
	struct epoll_event * epoll_events;
} co_io_mgr_t;

co_timer_mgr_t co_timer_mgr = {0};
co_io_mgr_t co_io_mgr = {0};

void print_tmgr()
{
    co_timer_t * cur = co_timer_mgr.head;
    int max = 20;
    int i = 0; 
    while (cur) {
        ++ i;
        if (i >= max) {
            printf("xxx\n");
            break;
        }
        printf("[%d:%d]\n", cur->co_id, cur->expiration_time);
        cur = cur->next;
    }
}

unsigned int sleep(unsigned int s)
{
    if (s <= 0) {
        return;
    }
    co_timer_t * timer = (co_timer_t*)malloc(sizeof(co_timer_t));
    timer->co_id = co_running();
    timer->expiration_time = time(0) + s;
    timer->next = NULL;

    //printf("%d sleep %d\n", timer->co_id, s);
    //print_tmgr();
    if (!co_timer_mgr.head) {
        //printf("first one\n");
        co_timer_mgr.head = timer;
    } else {
        // todo 换成小根堆
        if (co_timer_mgr.head->expiration_time > timer->expiration_time) {
            //printf("insert as head\n");
            timer->next = co_timer_mgr.head;
            co_timer_mgr.head = timer;
        }else {
            //printf("insert inside\n");
            co_timer_t * cur = co_timer_mgr.head;
            while (cur->next && cur->next->expiration_time < timer->expiration_time) {
                cur = cur->next;
            }
            timer->next = cur->next;
            cur->next = timer;
        }
    }
    //print_tmgr();
    co_yield();
}

int has_timer()
{
    return co_timer_mgr.head != NULL;
}

void process_timer()
{
    co_timer_t * cur = co_timer_mgr.head;
    time_t now = time(0);
    while (cur) {
        if (cur->expiration_time <= now) {
            int id = cur->co_id;
            co_timer_t * next = cur->next;
            free(cur);
            co_timer_mgr.head = cur = next;

            co_resume(id);
        } else {
            break;
        }
    }
}



/////////////hook///////////////

typedef int (*socket_func_t)(int domain, int type, int protocol);
typedef int (*accept_func_t)(int fd, struct sockaddr *addr, socklen_t *len);
typedef int (*recv_func_t)(int fd, char * buf, int len, int flags);
typedef int (*send_func_t)(int fd, char * buf, int len, int flags);
typedef int (*close_func_t)(int fd);



#define HOOK_FUNC( x ) static x##_func_t _##x = NULL; if (!_##x)  _##x = (x##_func_t)dlsym(RTLD_NEXT, #x)

void co_io_wait(int fd, int events)
{
	struct epoll_event event;
	event.data.fd = co_running();
	event.events = events;
	epoll_ctl(co_io_mgr.epoll_id, EPOLL_CTL_MOD, fd, &event);
	co_yield();
}

void co_io_add(fd)
{
	struct epoll_event event;
	event.data.fd = co_running();
	event.events = EPOLLIN | EPOLLOUT;
	epoll_ctl(co_io_mgr.epoll_id, EPOLL_CTL_ADD, fd, &event);
	debug("%d add to io set", fd);
}

void co_io_del(fd)
{
	debug("%d remove from io set", fd);
	epoll_ctl(co_io_mgr.epoll_id, EPOLL_CTL_DEL, fd, NULL);
}

int socket(int domain, int type, int protocal)
{
	HOOK_FUNC(socket);
	int sock = _socket(domain, type, protocal); 
	co_io_add(sock);
	return sock;
}

int accept(int fd, struct sockaddr * addr, socklen_t * len)
{
	HOOK_FUNC(accept);
	debug("async accept");
	co_io_wait(fd, EPOLLIN);
	int sock = _accept(fd, addr, len);
	debug("async accept finish");
	co_io_add(sock);
	return sock;
}

int recv(int fd,char * buff, int len, int flags) 
{
	HOOK_FUNC(recv);
	debug("async recv");
	co_io_wait(fd, EPOLLIN);
	debug("async recv finish");
	return _recv(fd, buff, len, flags);
}

int send(int fd, char * buff, int len, int flags)
{
	HOOK_FUNC(send);
	debug("async send");
	co_io_wait(fd, EPOLLOUT);
	debug("async send finish");
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
	co_io_mgr.epoll_max = 100;
	co_io_mgr.epoll_id = epoll_create(1);
	co_io_mgr.epoll_events = (struct epoll_event*)malloc(co_io_mgr.epoll_max * sizeof(struct epoll_event));
	memset(co_io_mgr.epoll_events, 0, co_io_mgr.epoll_max * sizeof(struct epoll_event));
}

void co_event_loop()
{
    while (1) {
		if ( has_timer()) {
			process_timer();
		}
		int ready = epoll_wait(co_io_mgr.epoll_id, co_io_mgr.epoll_events, co_io_mgr.epoll_max, 0);
		if (ready > 0) {
			for (int i = 0; i < ready; ++ i) {
				int co = co_io_mgr.epoll_events[i].data.fd;
				co_resume(co);
			}
		}
		if (co_is_all_finish()) {
			break;
		}
    }
}




