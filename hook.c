#define _GNU_SOURCE
#include "coroutine.h"

#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
//#include <ws2tcpip.h>
#else
#include <dlfcn.h>
#endif
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

typedef unsigned socklen_t;
/////////////hook///////////////

typedef int (*socket_func_t)(int domain, int type, int protocol);
typedef int (*accept_func_t)(int fd, struct sockaddr *addr, socklen_t *len);
typedef int (*connect_func_t)(int fd, const struct sockaddr *addr, socklen_t len);
typedef int (*recv_func_t)(int fd, void * buf, size_t len, int flags);
typedef int (*send_func_t)(int fd, void * buf, size_t len, int flags);
typedef int (*close_func_t)(int fd);

static socket_func_t __socket = NULL;
static accept_func_t __accept = NULL;
static connect_func_t __connect = NULL;
static recv_func_t __recv = NULL;
static send_func_t __send = NULL;
static close_func_t __close = NULL;

//todo open read write ...


int asocket(int domain, int type, int protocal)
{
    int sock = __socket(domain, type, protocal); 
	io_setnoblocking(sock);
    io_add(sock);
    return sock;
}

int aaccept(int fd, struct sockaddr * addr, socklen_t * len)
{
    co_debug("async accept");
    io_wait(fd, IO_READ);
    int sock = __accept(fd, addr, len);
    co_debug("async accept finish");
    io_add(sock);
    return sock;
}

int aconnect(int fd, const struct sockaddr * addr, socklen_t len)
{
	int ret = __connect(fd, addr, len);
	if (ret == 0) {
		io_add(fd);
		return ret;
	} else if (errno !=  EINPROGRESS) {
		return ret;
	}
	io_wait(fd, IO_WRITE);
	int error = 0;
	len = sizeof(error);
#ifndef WIN32
	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0)  {
		return -1;
	}
#endif
	return 0;
}


ssize_t arecv(int fd, void * buff, size_t len, int flags) 
{
    co_debug("async recv");
    io_wait(fd, IO_READ);
    co_debug("async recv finish");
    return __recv(fd, buff, len, flags);
}

ssize_t asend(int fd, const void * buff, size_t len, int flags)
{
    co_debug("async send");
    io_wait(fd, IO_WRITE);
    co_debug("async send finish");
    return __send(fd, buff, len, flags);
}

int aclose(int fd)
{
    io_del(fd);
    return __close(fd);
}


#ifdef WIN32

void hook_api(char * name, void * impl)
{
   // HookFunction("ws2_32", "socket", asocket
}
#define HOOK_FUNC( x ) __##x = x;
#else
#define HOOK_FUNC( x ) if (!__##x) __##x = (x##_func_t)dlsym(RTLD_NEXT, #x)
#endif

void hook_sys_call()
{
    HOOK_FUNC(socket);
    HOOK_FUNC(accept);
	HOOK_FUNC(connect);
    HOOK_FUNC(recv);
    HOOK_FUNC(send);
    HOOK_FUNC(close);
}




