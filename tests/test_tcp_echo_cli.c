
#include "coroutine.h"
#include <errno.h>
#ifdef WIN32
typedef unsigned socklen_t;
#endif

typedef struct connection_t
{
    int fd;
    int error;
}connection_t;


connection_t * make_connection(int fd)
{
    connection_t * conn = (connection_t*)malloc(sizeof(connection_t));
    conn->fd = fd;
    conn->error = 0;
    return conn;
}


void connection_heartbeat(connection_t * conn)
{
}

void connection_task(connection_t * conn)
{
    while (true) {
        char buff[] = "hello";
        int sc = asend(conn->fd, buff, sizeof(buff), 0); 
        printf("send %d bytes\n", sc);
        sleep(5);
    }
}

void async_connect_to(struct sockaddr * peer)
{
    int sock = asocket(AF_INET, SOCK_STREAM, 0);    
    socklen_t len = sizeof(struct sockaddr_in);
    int err = aconnect(sock, peer, len);
    if (err != 0) {
#ifdef WIN32
        int e = WSAGetLastError();
        printf("wsa error %d\n", e);
#endif
        perror("connect");
    } else {
        printf("connected %d\n", sock);
        connection_t * conn = make_connection(sock);
        co_start(connection_task, conn);
    }
}



void connector(int count)
{
#ifdef WIN32
    WORD word = MAKEWORD(2, 2);
    WSADATA wdata;
    int serr = WSAStartup(word, &wdata);
    if (serr != 0) {
        return -1;
    }
#endif


    struct sockaddr_in bind_info; 
    bind_info.sin_family = AF_INET;
    bind_info.sin_addr.s_addr = inet_addr("127.0.0.1");
    bind_info.sin_port = htons(1224);
    for (int i = 0; i < count; ++ i) {
        co_start(async_connect_to, &bind_info);
    }
}


int main()
{
    co_main(connector, 5);
}
