#include <stdio.h>
#include "coroutine.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#include "heap.h"
#include "map.h"

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr sockaddr;

typedef struct 
{
    char method[10];
    char uri[256];
    char version[10];
}http_request_t;

int take_util(char * stream, int stream_len, int * pos, char * to, int to_len, char stop)
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
        return 0;
    }

    int count = *pos - old_pos;
    if (to && count > 0) {
        int to_copy = count > to_len - 1 ? to_len-1:count;
        memcpy(to, stream + old_pos, to_copy);
        to[to_copy] = 0;
        return 1;
    }
    return 0;
}

int take_header(char * buff, int len, int * pos, char * to, int to_len)
{
    if (take_util(buff, len, pos, to, to_len, '\r')) {
        (*pos) ++;
        if (buff[*pos] != '\n') {
            return 0;
        }
        (*pos) ++;
        return 1;
    }
    return 0;
}

int parse_http_request(char * buff, int len, http_request_t * request)
{
    int byte_took = 0;
    take_util(buff, len, &byte_took, request->method, 10, ' ');
    byte_took ++;
    take_util(buff, len, &byte_took, request->uri, 512, ' ');
    byte_took ++;
    take_util(buff, len, &byte_took, request->version, 10, '\r');
    byte_took ++;

    if (buff[byte_took] != '\n') {
        return 0;
    }
    byte_took ++;


    co_info("|%s|%s|%s|", request->method, request->uri, request->version);

    char header[256];
    while (take_header(buff, len, &byte_took, header, 255)) {
        co_info("%s", header);
    }
    return 1;
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
            http_request_t request;
            if (parse_http_request(buff, used, &request)) {
                char *content = 
                    "<html>"
                    "<head><title>hello coroutine</title></head>"
                    "<body><h1>hello coroutine</h1></body>"
                    "</html>";
                char * json = "{\"a\" : 1}";
                char * template = 
                    "HTTP/1.1 200 OK\r\n"
                    "content-type: application/json; charset=utf-8\r\n"
                    "content-length: %d\r\n"
                    "\r\n"
                    "%s";

                char response_buff[1024] = {0};
                sprintf(response_buff, template, strlen(json), json);
                send(conn, response_buff, strlen(response_buff), 0);
                used = 0;
            }
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

    sockaddr_in bind_info; 
    bind_info.sin_family = AF_INET;
    bind_info.sin_addr.s_addr = htonl(INADDR_ANY);
    bind_info.sin_port = htons(1224);

    int err = bind(sock, (sockaddr*)&bind_info, sizeof(bind_info));

    int dummy;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &dummy, sizeof(dummy));
    if (err) {
        printf("bind failed\n");
        close(sock);
        return;
    }

    err = listen(sock, 100);
    
    co_info("listen on %d", 1224);
    while(true) {
        sockaddr_in client;
        int len = sizeof(client);
        int conn = accept(sock, (sockaddr *)&client, &len);
        unsigned char * ip = (char*)&client.sin_addr.s_addr;
        printf("[%d.%d.%d.%d:%d]\n", ip[0], ip[1], ip[2], ip[3], htons(client.sin_port));
        co_start(async_handle_connection, conn);
    }
}

void co_sleep(int us)
{
    while (1) {
        co_info("usleep %d", us);
        usleep(us);
    }
}

int compare(any_t a, any_t b)
{
    return a < b;
}

int compare_e(any_t a, any_t b)
{
    return a == b;
}

int main()
{

    co_init();
    co_event_init();

    co_start(co_sleep, 1000000);
    co_start(async_main, 0);
    co_event_loop();
    printf("done\n");
    co_finish();
    return 0;
}
