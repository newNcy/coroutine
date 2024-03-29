
#include "coroutine.h"
#include <errno.h>
#ifdef WIN32
typedef unsigned socklen_t;
#else
#include <netinet/in.h>
#endif

typedef struct connection_t
{
    int fd;
    int error;
    int id;
}connection_t;


int connection_count = 0;

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
    connection_count ++;
    while (true) {
        char buff[] = "hello";
        int sc = co_send(conn->fd, buff, sizeof(buff), 0); 
        printf("[%d] send %d bytes\n", conn->id, sc);
        co_sleep_ms(conn->id + 10000);
    }
}

void async_connect_to(int id)
{

    struct sockaddr_in peer; 
    peer.sin_family = AF_INET;
    peer.sin_addr.s_addr = inet_addr("127.0.0.1");
    peer.sin_port = htons(12240);

    int sock = co_socket(AF_INET, SOCK_STREAM, 0);    
    socklen_t len = sizeof(struct sockaddr_in);
    int err = co_connect(sock, (struct sockaddr*)&peer, len);
    if (err != 0) {
#ifdef WIN32
        int e = WSAGetLastError();
        printf("wsa error %d\n", e);
#endif
    } else {
        printf("connected %d\n", sock);
        connection_t * conn = make_connection(sock);
        conn->id = id;
        co_start(connection_task, (void*)conn);
    }
}


void show_connection_count()
{
    for (;;) {
        printf("connection_count:%d\n", connection_count);
        co_sleep_ms(1000);
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
    printf("%d to connect\n", count);
    co_start(show_connection_count, NULL);

    for (int i = 0; i < count; ++ i) {
        co_start(async_connect_to, (void*)i);
    }
}


int main()
{
    co_main(connector, (void*)500);
}
