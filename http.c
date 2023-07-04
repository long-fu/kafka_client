#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "soc.h"

// 512
#define READ_BUF_SIZE 1024

// 1024 * 1024 + 512
#define HEADER_BUF_SIZE 1049088 
static char *g_read_buf = NULL;
static char *g_header_buf = NULL;

int http_send(const char *body, size_t length) {

    if (body == NULL || length == 0) {
        return 0;
    }

    if(g_header_buf == NULL) {
        g_header_buf = (char *)malloc(HEADER_BUF_SIZE);
    }

    memset(g_header_buf, 0x0, HEADER_BUF_SIZE);
    char* header = g_header_buf;

    char cl[32] = {0};
    sprintf(cl,"Content-Length: %ld", length);

    strcat(header, "POST /api/smartbox/AlarmPost HTTP/1.1");
    strcat(header, "\r\n");

    strcat(header, "Cache-Control: no-cache");
    strcat(header, "\r\n");

    strcat(header, "Connection: Keep-Alive");
    strcat(header, "\r\n");

    strcat(header, "Accept-Encoding: gzip,deflate,br");
    strcat(header, "\r\n");

    strcat(header, "Accept: */*");
    strcat(header, "\r\n");

    strcat(header, "Content-Type: application/json");
    strcat(header, "\r\n");

    strcat(header, "User-Agent: Mozilla/5.0");
    strcat(header, "\r\n");

    // strcat(header, "host:22.11.123.78:80\r\n");
    // strcat(header, "\r\n");

    strcat(header, cl);
    strcat(header, "\r\n");
    strcat(header, "\r\n");

    strcat(header, body);
    strcat(header, "\r\n");
    strcat(header, "\r\n");


    int fd = create_socket("wlp0s20f3", "192.168.2.4", 8989, "127.0.0.1", 80);
    
    // 创建连接
    int result = write(fd, header, strlen(header));
    if (result != strlen(header))
    {
        printf("数据发送失败");
        return -1;
    }
    
    int ri = 0, n = 0;
    if(g_read_buf == NULL) {
        g_read_buf = malloc(READ_BUF_SIZE);
    }
    memset(g_read_buf, 0x0, READ_BUF_SIZE);

    char temp[512] = {0};
    while ((n = read(fd, temp, 512)) > 0)
    {
        // 接收的数据很小
        // printf("接收[%d]=========\n",ri);
        // printf("%s\n",temp);
        strlncat(g_read_buf, strlen(g_read_buf), temp, n);
        memset(temp,0x0, 512);
    }
    printf("接收的消息 %s \n", g_read_buf);
    // TODO: 解析接收的消息
    printf("释放资源\n");
    destroy_socket(fd);
    return 0;
}