#include "coroutine.h"
#include <errno.h>
#ifdef WIN32
typedef unsigned socklen_t;
#else
#include <netinet/in.h>
#endif


int connection_count = 0;

void serve(int conn)
{
    connection_count ++;
    while(true) {
        char buff[1024] = {0};
        int rc = arecv(conn, buff, 1024, 0);
        printf("%d->%d:%s\n", conn, rc, buff);
        if (rc <= 0) {
            printf("connection[%d] closed\n", conn);
#ifdef WIN32
            int e = WSAGetLastError();
            printf("wsa error %d, errno:%d\n", e, errno);
#endif
            break;
        }
        int sc = asend(conn, buff, rc, 0);
        printf("send %d bytes\n", sc);
    }
    connection_count --;
    aclose(conn);
}

void show_connection_count()
{
    for (;;) {
        printf("connection_count:%d\n", connection_count);
        fflush(stdout);
        sleep(1000);
    }
}

void accpetor(int port)
{
    #ifdef WIN32
    WORD word = MAKEWORD(2, 2);
    WSADATA wdata;
    int serr = WSAStartup(word, &wdata);
    if (serr != 0) {
        return -1;
    }
#endif

    co_start(show_connection_count,0);
    int sock = asocket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in bind_info; 
    bind_info.sin_family = AF_INET;
    bind_info.sin_addr.s_addr = htonl(INADDR_ANY);
    bind_info.sin_port = htons(port);

    int err = bind(sock, (struct sockaddr*)&bind_info, sizeof(bind_info));

    int dummy;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &dummy, sizeof(dummy));
    if (err) {
        printf("bind failed\n");
        aclose(sock);
        return;
    }

    err = listen(sock, 1024);
    
    printf("%d listen on %d\n", sock, 80);
    while(true) {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int conn = aaccept(sock, (struct sockaddr *)&client, &len);
        if (conn< 0) {
            perror("accept");
            continue;
        }
        unsigned char * ip = (char*)&client.sin_addr.s_addr;
        printf("[%d][%d.%d.%d.%d:%d]\n", conn, ip[0], ip[1], ip[2], ip[3], htons(client.sin_port));
        co_start(serve, conn);
    }
    aclose(sock);
}

int main()
{
    co_main(accpetor, 1224);
}
