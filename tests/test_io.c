#include <stdio.h>
#include "coroutine.h"
#ifdef WIN32
typedef unsigned socklen_t;
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <string.h>
#include <stdlib.h>


//typedef struct sockaddr_in sockaddr_in;
//typedef struct sockaddr sockaddr;

typedef struct 
{
    char method[10];
    char uri[256];
    char version[10];
    int conn;
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


    //printf("|%s|%s|%s|\n", request->method, request->uri, request->version);

    char header[256];
    while (take_header(buff, len, &byte_took, header, 255)) {
        printf("%s\n", header);
    }
    return 1;
}

void on_http_request(http_request_t * req)
{
    char *content = 
        "<html>"
        "<head><title>hello coroutine</title></head>"
        "<body><h1>hello coroutine</h1></body>"
        "</html>";
    char * json = "{\"a\" : 1}";
    char * template = 
        "HTTP/1.1 200 OK\r\n"
        "content-type: text/html\r\n"
        "content-length: %d\r\n"
        "\r\n"
        "%s";

    char response_buff[1024] = {0};
    sprintf(response_buff, template, strlen(content), content);
    co_send(req->conn, response_buff, strlen(response_buff), 0);
}

void async_handle_connection(int conn)
{
    co_info("handle %d", conn);
    char buff[1024] = {0};
    int used = 0;
    while(true) {
        int rc = co_recv(conn, buff + used, 1024 - used, 0);
        if (rc == 0) {
            printf("connection[%d] closed\n", conn);
            break;
        } else if (rc < 0) {
            perror("connection error");
            break;
        }
        printf("%d:read %d bytes\n", conn, rc);
        used += rc;
        if (strstr(buff + used - rc, "\r\n\r\n")) {
            http_request_t request;
            if (parse_http_request(buff, used, &request)) {
                request.conn = conn;
                co_start(on_http_request, &request); 
                used = 0;
            }
        }
        if (used == 1024) {
            break;
        }
    }
    co_close(conn);
}

void heartbeat(int sec)
{
    int idx = 0;
    while (true) {
        printf("heartbeat %d\n", idx ++);
        co_sleep_ms(sec * 1000);
        fflush(stdout);
    }
}

void async_main()
{
    co_start(heartbeat, (void*)3);
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
    bind_info.sin_port = htons(80);

    int err = bind(sock, (struct sockaddr*)&bind_info, sizeof(bind_info));

    int dummy;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &dummy, sizeof(dummy));
    if (err) {
        printf("bind failed\n");
        co_close(sock);
        return;
    }

    err = listen(sock, 1024);
    
    printf("%d listen on %d\n", sock, 80);
    fflush(stdout);
    while(true) {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int conn = co_accept(sock, (struct sockaddr *)&client, &len);
        if (conn < 0) {
            perror("accept");
            break;
        }
        unsigned char * ip = (char*)&client.sin_addr.s_addr;
        printf("[%d.%d.%d.%d:%d]\n", ip[0], ip[1], ip[2], ip[3], htons(client.sin_port));
        co_start(async_handle_connection, (void*)conn);
    }
    co_close(sock);
}


int main()
{
    co_main(async_main, 0);
    return 0;
}
