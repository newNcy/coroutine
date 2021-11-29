#include <stdio.h>
#include "coroutine.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

void take_util(char * stream, int stream_len, int * pos, char * to, int to_len, char stop)
{
	int old_pos = *pos;
	if (!stream) {
		return 0;
	}
	while(* pos < stream_len && stream[*pos] != stop) {	
		(*pos) ++;	
	}
	if (stream[*pos] != stop) {
		*pos = old_pos;
		return;
	}

	int count = *pos - old_pos;
	if (to && count > 0) {
		memcpy(to, stream + old_pos, count > to_len ? to_len:count);
	}
}

int handle_http_request(char * buff, int len)
{
	int close = 1;
	char method[10] = {0}, uri[512] = {0}, version[20] = {0};

	int byte_took = 0;
	take_util(buff, len, &byte_took, method, 9, ' ');
	byte_took ++;
	take_util(buff, len, &byte_took, uri, 511, ' ');
	byte_took ++;
	take_util(buff, len, &byte_took, version, 19, '\r');

	printf("|%s|%s|%s|\n", method, uri, version);
	fflush(stdout);
	return close;
}

void async_handle_connection(int conn)
{
	char buff[1024] = {0};
	int used = 0;
    while(true) {
		int rc = recv(conn, buff + used, 1024 - used, 0);
		if (rc == 0) {
			break;
		}

		used += rc;
		if (strstr(buff + used - rc, "\r\n\r\n")) {
			handle_http_request(buff, used);
			
			char *content = 
				"<html>"
				"<head><title>hello coroutine</title></head>"
				"<body><h1>hello coroutine</h1></body>"
				"</html>";
			char * template = 
				"HTTP/1.1 200 OK\r\n"
				"content-type: text/html\r\n"
				"content-length: %d\r\n"
				"\r\n\r\n"
				"%s";

			char response_buff[1024] = {0};
			sprintf(response_buff, template, strlen(content), content);
			send(conn, response_buff, strlen(response_buff), 0);
			used = 0;
		}
		if (used == 1024) {
			break;
		}
    }
	close(conn);
}


void async_main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    printf("socket %d\n", sock);
    
    sockaddr_in bind_info; 
    bind_info.sin_family = AF_INET;
    bind_info.sin_addr.s_addr = htonl(INADDR_ANY);
    bind_info.sin_port = htons(1224);

    int err = bind(sock, (sockaddr*)&bind_info, sizeof(bind_info));

	int dummy;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &dummy, sizeof(dummy));
    if (err) {
        printf("bind failed\n");
        return;
    }

    err = listen(sock, 100);
    while(true) {
        sockaddr_in client;
        int len = sizeof(client);
        int conn = accept(sock, (sockaddr *)&client, &len);
        int co = co_create(async_handle_connection, (void*)conn);
        co_resume(co);
    }
}

void test(int input)
{
	printf("input %d\n", input);
	co_yield(input);
	printf("input %d\n", input);
}

int main()
{

    co_init(10);
    co_event_init();

    int amain = co_create(async_main, NULL);
    co_resume(amain);

    co_event_loop();
    printf("done\n");
    co_finish();
    return 0;
}
