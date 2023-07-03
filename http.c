#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "soc.h"
#include "http_parser.h"

#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>


static int on_info(http_parser *p)
{
    // printf("on_info %d\r\n", p->flags);
    return 0;
}

static int on_headers_complete(http_parser *p)
{
    // http_parser_pause(p, 1);
    printf("on_headers_complete %d\r\n", p->method);
    return 0;
}

static int on_message_complete(http_parser *p)
{
    printf("on_message_complete %s\r\n", (char *)p->data);
    // http_parser_pause(p, 1);
    return 0;
}

static int on_data(http_parser *p, const char *at, size_t length)
{
    // http_parser_pause(p, 1);
    // char buffer[1024] = {0};
    // strncpy(buffer, at, length);
    // printf("on_data type: %d %s==%zu\r\n", p->type, buffer, length);
    return 0;
}

static int on_header_field(http_parser *p, const char *at, size_t length)
{
    // http_parser_pause(p, 1);
    // char buffer[1024] = {0};
    // strncpy(buffer, at, length);
    // printf("%s: ", buffer);
    return 0;
}

static int on_header_value(http_parser *p, const char *at, size_t length)
{
    // http_parser_pause(p, 1);
    // char buffer[1024] = {0};
    // strncpy(buffer, at, length);
    // printf("%s\r\n", buffer);
    return 0;
}

static int on_url(http_parser *p, const char *at, size_t length)
{
    // http_parser_pause(p, 1);
    // char buffer[1024] = {0};
    // strncpy(buffer, at, length);
    // printf("on_url %s\r\n", buffer);
    return 0;
}

static int on_status(http_parser *p, const char *at, size_t length)
{
    // http_parser_pause(p, 1);
    // char buffer[1024] = {0};
    // strncpy(buffer, at, length);
    // printf("on_status %s\r\n", buffer);
    return 0;
}

//   strlncat(messages[num_messages].body,
//            sizeof(messages[num_messages].body),
//            buf,
//            len);

static char* response_buf;

static int on_body(http_parser *p, const char *buf, size_t len)
{
    strlncat(response_buf, sizeof(response_buf), buf, len);
    return 0;
}

struct http_parser parser_response;

static http_parser_settings settings = {
    .on_message_begin = on_info,
    .on_headers_complete = on_headers_complete,
    .on_message_complete = on_message_complete,
    .on_header_field = on_header_field,
    .on_header_value = on_header_value,
    .on_url = on_url,
    .on_status = on_status,
    .on_body = on_body};

int parser_responder(char *data, size_t data_len)
{

    size_t parsed;
    static int is_init = 0;
    if (is_init == 0)
    {
        http_parser_init(&parser_response, HTTP_RESPONSE);
        is_init = 1;
    }
    parsed = http_parser_execute(&parser_response, &settings, data, data_len);
    
    if (parsed == data_len)
    {
        printf("解析成功 %zu\n", parsed);
    }
    else
    {
        printf("解析失败  %zu\n", parsed);
    }
    printf("解析一次最后返回\n");

    return 0;
}

static char *substring(const char *src, int off, int len)
{
    int i;
    const char *pch = src;

    char *dest = (char *)calloc(sizeof(char), len + 1);

    pch = pch + off;

    for (i = 0; i < len; i++)
    {
        dest[i] = *(pch++);
    }
    dest[len] = '\0'; // 加上字符串结束符。
    return dest;
}

int http(const char *endpoint, enum http_method method, const char *body, char *read_buf)
{
    int rv;
    struct http_parser_url u;
    char *schema = NULL;
    char *host = NULL;
    char *port = NULL;
    char *path = NULL;
    char *query = NULL;
    char *fragment = NULL;
    char *userinfo = NULL;

    http_parser_url_init(&u);
    rv = http_parser_parse_url(endpoint, strlen(endpoint), 0, &u);

    schema = substring(endpoint, u.field_data[UF_SCHEMA].off, u.field_data[UF_SCHEMA].len);
    host = substring(endpoint, u.field_data[UF_HOST].off, u.field_data[UF_HOST].len);
    port = substring(endpoint, u.field_data[UF_PORT].off, u.field_data[UF_PORT].len);
    int port_int = (u.field_set & (1 << UF_PORT)) ? u.port : 0;
    path = substring(endpoint, u.field_data[UF_PATH].off, u.field_data[UF_PATH].len);
    query = substring(endpoint, u.field_data[UF_QUERY].off, u.field_data[UF_QUERY].len);
    fragment = substring(endpoint, u.field_data[UF_FRAGMENT].off, u.field_data[UF_FRAGMENT].len);
    userinfo = substring(endpoint, u.field_data[UF_USERINFO].off, u.field_data[UF_USERINFO].len);
    const char *method_str = http_method_str(HTTP_GET);

    printf("method %s \n", method_str);
    printf("schema %s \n", schema);
    printf("host %s \n", host);
    printf("port %s \n", port);
    printf("path %s \n", path);
    printf("query %s \n", query);
    printf("fragment %s \n", fragment);
    printf("userinfo %s \n", userinfo);

    char *header = malloc(1024);
    memset(header, 0x0, 1024);
    strcat(header, method_str);
    strcat(header, " ");
    strcat(header, path);
    strcat(header, "?");
    strcat(header, query);
    strcat(header, " ");
    strcat(header, "HTTP/1.1");
    strcat(header, "\r\n");

    char host_ip[32] = {0};
    sprintf(host_ip, "Host: %s:%s", host, port);
    strcat(header, host_ip);
    strcat(header, "\r\n");

    strcat(header, "Accept: text/html");
    strcat(header, "\r\n");

    strcat(header, "Cache-Control:no-cache");
    strcat(header, "\r\n");

    strcat(header, "User-Agent: client");
    strcat(header, "\r\n");

    strcat(header, "Connection: close");
    strcat(header, "\r\n");

    // strcat(header, "Connection: keep-alive");
    // strcat(header, "\r\n");

    strcat(header, "Content-Type: application/json; text/html; charset=UTF-8");
    strcat(header, "\r\n");

    if (body != NULL)
    {
        char content_len[64] = {0};
        sprintf(content_len, "Content-Length: %ld", strlen(body));
        strcat(header, content_len);
        strcat(header, "\r\n");
    }

    strcat(header, "\r\n");

    if (body != NULL)
    {
        strcat(header, body);
        strcat(header, "\r\n");

        strcat(header, "\r\n");
    }

    if (schema != NULL)
    {
        free(schema);
    }
    if (host != NULL)
    {
        free(host);
    }
    if (port != NULL)
    {
        free(port);
    }
    if (path != NULL)
    {
        free(path);
    }
    if (query != NULL)
    {
        free(query);
    }
    if (fragment != NULL)
    {
        free(fragment);
    }
    if (userinfo != NULL)
    {
        free(userinfo);
    }

    printf("========header===========\n");
    printf("%s\n", header);

    int fd = create_socket("wlp0s20f3", "192.168.2.4", 8989, "127.0.0.1", port_int);
    
    // 创建连接
    
    int result = write(fd, header, strlen(header));
    if (result != strlen(header))
    {
        perror("数据写入失败");
        exit(-1);
    }
    free(header);

    int ri = 0, n = 0;
    // char rbuf[1024] = { 0 };
    char *rbuf = malloc(1024);
    memset(rbuf, 0x0, 512);
    response_buf = read_buf;

    while ((n = read(fd, rbuf, 1024)) > 0)
    {
        printf("接收[%d]=========\n",ri);
        printf("%s\n",rbuf);
        
        parser_responder(rbuf, strlen(rbuf));
        ri++;
        memset(rbuf, 0x0, 1024);
    }
    printf("释放资源\n");
    free(rbuf);
    destroy_socket(fd);
}