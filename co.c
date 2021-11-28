#include <stdio.h>
#include "coroutine.h"

#include <winsock2.h>

#define true 1
typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

void co_sleep(int s)
{
    for (int i = 0; i < 3; ++ i) {
        sleep(s);
        printf("sleep %ds\n", s);
        fflush(stdout);
    }
}

void async_handle_connection(int conn)
{
    while(true) {
        printf("%d sleep\n", conn);
        fflush(stdout);
        sleep(1);
    }
}


void async_main()
{
    WORD	wVersionRequested;
	WSADATA wsaData;
	int		err,iLen;
	wVersionRequested	=	MAKEWORD(2,2);//create 16bit data
//(1)Load WinSock
	err	=	WSAStartup(wVersionRequested,&wsaData);	//load win socket
	if(err!=0)
	{
		return;
	}
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    printf("socket %d\n", sock);
    
    sockaddr_in bind_info; 
    bind_info.sin_family = AF_INET;
    bind_info.sin_addr.s_addr = htonl(INADDR_ANY);
    bind_info.sin_port = htons(1224);

    err = bind(sock, (sockaddr*)&bind_info, sizeof(bind_info));
    if (err) {
        printf("bind failed\n");
        return;
    }
    err = listen(sock, 100);
    while(true) {
        sockaddr_in client;
        int len = sizeof(client);
        int conn = accept(sock, (sockaddr *)&client, &len);
        printf("new connection:%d\n", conn);
        int co = co_create(async_handle_connection, (void*)conn);
        co_resume(co);
    }
}

int main()
{

    co_init(10);

    int am = co_create(async_main, NULL);
    co_resume(am);

    co_event_loop();
    printf("done\n");
    co_finish();
    return 0;
}
