#define _GNU_SOURCE
#include "coroutine.h"

#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

/////////////hook///////////////

typedef int (*socket_func_t)(int domain, int type, int protocol);
typedef int (*accept_func_t)(int fd, struct sockaddr *addr, socklen_t *len);
typedef int (*connect_func_t)(int fd, const struct sockaddr *addr, socklen_t len);
typedef int (*recv_func_t)(int fd, void * buf, size_t len, int flags);
typedef int (*send_func_t)(int fd, void * buf, size_t len, int flags);
typedef int (*close_func_t)(int fd);

static socket_func_t _socket = NULL;
static accept_func_t _accept = NULL;
static connect_func_t _connect = NULL;
static recv_func_t _recv = NULL;
static send_func_t _send = NULL;
static close_func_t _close = NULL;

//todo open read write ...


int setnoblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

#define HOOK_FUNC( x ) if (!_##x) _##x = (x##_func_t)dlsym(RTLD_NEXT, #x)
int socket(int domain, int type, int protocal)
{
    int sock = _socket(domain, type, protocal); 
	setnoblocking(sock);
    io_add(sock);
    return sock;
}

int accept(int fd, struct sockaddr * addr, socklen_t * len)
{
    co_debug("async accept");
    io_wait(fd, EPOLLIN);
    int sock = _accept(fd, addr, len);
    co_debug("async accept finish");
    io_add(sock);
    return sock;
}

int connect(int fd, const struct sockaddr * addr, socklen_t len)
{
	int ret = _connect(fd, addr, len);
	if (ret == 0) {
		io_add(fd);
		return ret;
	} else if (errno !=  EINPROGRESS) {
		return ret;
	}
	io_wait(fd, EPOLLOUT);
	int error = 0;
	len = sizeof(error);
	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0)  {
		return -1;
	}
	return 0;
}


ssize_t recv(int fd, void * buff, size_t len, int flags) 
{
    co_debug("async recv");
    io_wait(fd, EPOLLIN);
    co_debug("async recv finish");
    return _recv(fd, buff, len, flags);
}

ssize_t send(int fd, const void * buff, size_t len, int flags)
{
    co_debug("async send");
    io_wait(fd, EPOLLOUT);
    co_debug("async send finish");
    return _send(fd, buff, len, flags);
}

int close(int fd)
{
    io_del(fd);
    return _close(fd);
}

void hook_sys_call()
{
    HOOK_FUNC(socket);
    HOOK_FUNC(accept);
	HOOK_FUNC(connect);
    HOOK_FUNC(recv);
    HOOK_FUNC(send);
    HOOK_FUNC(close);
}




