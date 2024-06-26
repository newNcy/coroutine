#include "coroutine.h"
#include <errno.h>
#include <assert.h>
#ifdef WIN32
typedef unsigned socklen_t;
#else
#include <netinet/in.h>
#endif


volatile int connection_count = 0;
map_t * fd_used = NULL;

void serve(int conn)
{
    connection_count ++;
    while(true) {
        char buff[1024] = {0};
        int rc = co_recv(conn, buff, 1024, 0);
        //printf("%d->%d:%s\n", conn, rc, buff);
        if (rc <= 0) {
            printf("connection[%d] closed\n", conn);
#ifdef WIN32
            int e = WSAGetLastError();
            printf("wsa error %d, errno:%d\n", e, errno);
#endif
            break;
        }
        //int sc = co_send(conn, buff, rc, 0);
        //printf("send %d bytes\n", sc);
    }
    connection_count --;
    co_close(conn);
    //map_remove_key(fd_used, (any_t)conn);
}

void show_connection_count()
{
    for (;;) {
        co_sleep_ms(1000);
        printf("connection_count:%d coroutine count:%d fd_used size %d\n", connection_count, co_count(), map_size(fd_used));
        //aio_debug_print_info();
        fflush(stdout);
    }
}

void accpetor(int port)
{
    fd_used = map_create(NULL, NULL);
#ifdef WIN32
    WORD word = MAKEWORD(2, 2);
    WSADATA wdata;
    int serr = WSAStartup(word, &wdata);
    if (serr != 0) {
        return -1;
    }
#endif

    int sock = co_socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in bind_info; 
    bind_info.sin_family = AF_INET;
    bind_info.sin_addr.s_addr = htonl(INADDR_ANY);
    bind_info.sin_port = htons(port);

    int err = bind(sock, (struct sockaddr*)&bind_info, sizeof(bind_info));

    int dummy;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &dummy, sizeof(dummy));
    if (err) {
        printf("bind failed\n");
        co_close(sock);
        return;
    }

    err = listen(sock, 1024);
    
    printf("%d listen on %d\n", sock, port);
    while(true) {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int conn = co_accept(sock, (struct sockaddr *)&client, &len);
        if (conn< 0) {
            perror("accept");
            continue;
        }
        unsigned char * ip = (char*)&client.sin_addr.s_addr;
        printf("[%d][%d.%d.%d.%d:%d]\n", conn, ip[0], ip[1], ip[2], ip[3], htons(client.sin_port));
        assert(map_get(fd_used, (any_t)conn) == nullptr);
        //map_set(fd_used, (any_t)conn, (any_t)1);
        co_start(serve, (void*)conn);
    }
    co_close(sock);
}

int main()
{
    co_start(show_connection_count, NULL);
    co_main(accpetor, (void*)12240);
}
