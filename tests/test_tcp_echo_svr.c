#include "coroutine.h"


void serve(int conn)
{
    char buff[1024] = {0};
    while(true) {
        int rc = arecv(conn, buff, 1024, 0);
        if (rc == 0) {
            printf("connection[%d] closed\n", conn);
            break;
        }
        asend(conn, buff, rc, 0);
    }
    aclose(conn);
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
        int len = sizeof(client);
        int conn = aaccept(sock, (struct sockaddr *)&client, &len);
        unsigned char * ip = (char*)&client.sin_addr.s_addr;
        printf("[%d][%d.%d.%d.%d:%d]\n", conn, ip[0], ip[1], ip[2], ip[3], htons(client.sin_port));
        co_start(serve, conn);
    }
}

int main()
{
    co_main(accpetor, 1224);
}
