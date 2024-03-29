#pragma once
#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <dlfcn.h>
#include <unistd.h>
#endif
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
void hook_sys_call();
int co_socket(int domain, int type, int protocal);
int co_accept(int fd, struct sockaddr * addr, socklen_t * len);
int co_connect(int fd, const struct sockaddr * addr, socklen_t len);
size_t co_recv(int fd, void * buff, size_t len, int flags) ;
size_t co_send(int fd, const void * buff, size_t len, int flags);
int co_close(int fd);
