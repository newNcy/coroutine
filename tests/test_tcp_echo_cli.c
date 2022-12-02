
#include "coroutine.h"
#include <arpa/inet.h>


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
        int sock = asocket(AF_INET, SOCK_STREAM, 0);
        socklen_t len = sizeof(bind_info);
        int err = aconnect(sock, (struct socketaddr*)&bind_info, len);
        if (err > 0) {
            asend(sock, "hello", 6, 0);
            char buff[100] = {0};
            arecv(sock, buff, 100, 0);
            printf("%s\n", buff);
        }
    }
}

int main()
{
    co_main(connector, 1224);
}
