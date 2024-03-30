#include <complex.h>
#include <math.h>
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
#include "map.h"



//typedef struct sockaddr_in sockaddr_in;
//typedef struct sockaddr sockaddr;


typedef struct string_t
{
    char * c_str;
    size_t length;
    size_t cap;
}string_t;

typedef struct 
{
    int conn;
    string_t * method;
    string_t * uri;
    string_t * version;
    map_t * headers;
}http_request_t;

typedef struct 
{
    int status;
    string_t * status_msg;
    map_t * headers;
    string_t * text;
}http_response_t;



string_t * work_dir = NULL;

string_t * string_create()
{
    string_t * str = (string_t*)malloc(sizeof(string_t));
    str->length = 0;
    str->cap = 10;
    str->c_str = (char*)malloc(str->cap);
    return str;
}

string_t * string_create_with_buff(char * buff, int length)
{
    string_t * str = (string_t*)malloc(sizeof(string_t));
    str->length = length;
    str->cap = length;
    str->c_str = (char*)malloc(str->cap);
    memcpy(str->c_str, buff, length);
    str->c_str[length] = 0;
    return str;
}

static inline string_t * string_create_with_c_str(char * c_str)
{
    return string_create_with_buff(c_str, strlen(c_str));
}


void string_destroy(string_t * str) 
{
    free(str->c_str);
    free(str);
}



void string_preserve(string_t * str, size_t new_cap)
{
    if (new_cap >= str->cap) {
        str->cap = new_cap + 1;
        str->c_str = realloc(str->c_str, str->cap);
    }
}

void string_resize(string_t * str, size_t new_size)
{
    string_preserve(str, new_size);
    str->length = new_size;
}

void string_assign_buff(string_t * str, char * buff, size_t length)
{
    string_resize(str, length);
    memcpy(str->c_str, buff, length);
    str->c_str[length] = 0;
}

static inline void string_assign(string_t * str, string_t * t)
{
    string_assign_buff(str, t->c_str, t->length);
}

static inline void string_assign_c_str(string_t * str, const char * c_str)
{
    string_assign_buff(str, c_str, strlen(c_str));
}

static inline int string_equals_c_str(string_t * str, const char * c_str)
{
    return strcmp(str->c_str, c_str) == 0;
}

int string_equals(string_t * str, string_t * t)
{
    return strcmp(str->c_str, t->c_str) == 0;
}

int string_less(string_t * str, string_t * t) 
{
    return strcmp(str->c_str, t->c_str) < 0;
}

void string_concat_buff(string_t * str, char * buff, int length)
{
    string_preserve(str, str->length + length);
    memcpy(str->c_str + str->length, buff, length + 1); 
    str->length += length;
}

static inline void string_concat(string_t * str, string_t * t)
{
    string_concat_buff(str, t->c_str, t->length);
}

static inline void string_concat_c_str(string_t * str, char * c_str)
{
    string_concat_buff(str, c_str, strlen(c_str));
}

string_t * string_combine(string_t * a, string_t * b)
{
    string_t * str = string_create_with_buff(a->c_str, a->length);
    string_concat(str, b);

    return str;
}

http_request_t * http_request_create()
{
    http_request_t * http_req = (http_request_t*)malloc(sizeof(http_request_t));
    http_req->method = string_create();
    http_req->uri = string_create();
    http_req->version = string_create();
    http_req->headers = map_create(string_less, string_equals);
    return http_req;
}

void http_request_destroy(http_request_t * http_req)
{
    map_destroy(http_req->headers);
    string_destroy(http_req->method);
    string_destroy(http_req->uri);
    string_destroy(http_req->version);
    free(http_req);
}

http_response_t * http_response_create()
{
    http_response_t * http_res = (http_response_t*)malloc(sizeof(http_response_t));
    http_res->status = 404;
    http_res->status_msg = string_create();
    http_res->headers = map_create(string_less, string_equals);
    http_res->text = string_create();
    return http_res;
}

void http_response_destroy(http_response_t * http_res)
{
    map_destroy(http_res->headers);
    string_destroy(http_res->text);
    free(http_res);
}

int take_util(char * stream, int stream_len, int * pos, string_t * str, char stop)
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
    string_assign_buff(str, stream + old_pos, count);
    return 1;
}

int take_header(char * buff, int len, int * pos, string_t * header)
{
    if (take_util(buff, len, pos, header, '\r')) {
        (*pos) ++;
        if (buff[*pos] != '\n') {
            return 0;
        }
        (*pos) ++;
        return 1;
    }
    return 0;
}

void parse_http_header(string_t * header, map_t * headers)
{
    int field_length = 0;
    for (int  i = 0 ; i < header->length; ++ i) {
    }
}

int parse_http_request(char * buff, int len, http_request_t * request)
{
    int pos = 0;
    take_util(buff, len, &pos, request->method, ' ');
    pos ++;
    take_util(buff, len, &pos, request->uri, ' ');
    pos ++;
    take_util(buff, len, &pos, request->version, '\r');
    pos ++;

    if (buff[pos] != '\n') {
        return 0;
    }
    pos ++;


    //printf("|%s|%s|%s|\n", request->method, request->uri, request->version);

    while (pos < len && buff[pos] != '\r') {
        string_t * key = string_create();
        take_util(buff, len, &pos, key, ':');
        while ( buff[pos] == ' ' || buff[pos] == ':') pos ++;
        string_t * value = string_create();
        take_util(buff, len, &pos, value, '\r');
        pos += 2;
        map_set( request->headers, key, value);
    }
    return 1;
}


void handle_http_request(http_request_t * http_req, http_response_t * http_res)
{
    if (string_equals_c_str(http_req->uri, "/")) {
        string_concat_c_str(http_req->uri, "index.html");
    }

    printf("request %s\n", http_req->uri->c_str);
    char res_path[2048] = {0};
    sprintf(res_path, "%s%s", work_dir->c_str, http_req->uri->c_str);
    FILE * res_fp = fopen(res_path, "rb");
    if (res_fp) {
        fseek(res_fp, 0, SEEK_END);
        int size = ftell(res_fp);
        fseek(res_fp, 0, SEEK_SET);

        string_resize(http_res->text, size);
        int len = fread(http_res->text->c_str, 1, size,  res_fp);

        string_t * content_length = string_create();
        sprintf(content_length->c_str, "%d", size);

        map_set(http_res->headers, string_create_with_c_str("content-type"), string_create_with_c_str("text/html"));
        map_set(http_res->headers, string_create_with_c_str("content-length"), content_length);
        http_res->status = 200;
        string_assign_c_str(http_res->status_msg, "OK");
        fclose(res_fp);
    }
}

void on_http_request(http_request_t * req)
{
    http_response_t * http_res = http_response_create();
    handle_http_request(req, http_res);

    string_t * buff = string_create();
    char line[1024] = {0};
    sprintf(line, "%s %d %s\r\n", req->version->c_str, http_res->status, http_res->status_msg->c_str);
    string_concat_c_str(buff, line);

    for (map_iterator_t it = map_begin(http_res->headers); it != map_end(http_res->headers); it = map_next(it)) {
        memset(line, 0, 1024);
        string_t * key = (string_t*)it->key;
        string_t * value = (string_t*)it->value;
        sprintf(line, "%s: %s\r\n", key->c_str, value->c_str);
        string_concat_c_str(buff, line);
    }
    string_concat_c_str(buff, "\r\n");
    string_concat(buff, http_res->text);

    co_send(req->conn, buff->c_str, buff->length, 0);
    http_response_destroy(http_res);
}

void async_handle_connection(int conn)
{
    printf("new connection[%d]\n", conn);
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
        used += rc;
        if (strstr(buff + used - rc, "\r\n\r\n")) {
            http_request_t * request = http_request_create();
            if (parse_http_request(buff, used, request)) {
                request->conn = conn;
                co_start(on_http_request, request); 
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


int main(int argc, char * argv[])
{
    if (argc < 2) {
        work_dir = string_create_with_c_str(".");
    } else {
        work_dir = string_create_with_c_str(argv[1]);
    }
    co_main(async_main, 0);
    return 0;
}
