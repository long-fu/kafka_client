#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "soc.h"

// 1024
#define READ_BUF_SIZE 1024
static char *g_read_buf = NULL;

// 1024 * 1024 + 512 284,555 1,049,088
#define HEADER_BUF_SIZE 1049088
static char *g_header_buf = NULL;

#define BODY_BUF_SIZE 1024 * 1024
static char *g_body_buf = NULL;

size_t
strnlen(const char *s, size_t maxlen)
{
    const char *p;

    p = memchr(s, '\0', maxlen);
    if (p == NULL)
        return maxlen;

    return p - s;
}

size_t
strlncat(char *dst, size_t len, const char *src, size_t n)
{
    size_t slen;
    size_t dlen;
    size_t rlen;
    size_t ncpy;

    slen = strnlen(src, n);
    dlen = strnlen(dst, len);

    if (dlen < len)
    {
        rlen = len - dlen;
        ncpy = slen < rlen ? slen : (rlen - 1);
        memcpy(dst + dlen, src, ncpy);
        dst[dlen + ncpy] = '\0';
    }

    //   assert(len > slen + dlen);
    return slen + dlen;
}

size_t
strlncpy(char *dst, size_t len, const char *src, size_t n)
{
    size_t slen;
    size_t ncpy;

    slen = strnlen(src, n);

    if (len > 0)
    {
        ncpy = slen < len ? slen : (len - 1);
        memcpy(dst, src, ncpy);
        dst[ncpy] = '\0';
    }

    //   assert(len > slen);
    return slen;
}

int http_send(const char *body, size_t length)
{

    if (body == NULL || length == 0)
    {
        return 0;
    }

    if (g_header_buf == NULL)
    {
        g_header_buf = (char *)malloc(HEADER_BUF_SIZE);
    }

    if (g_body_buf == NULL)
    {
        g_body_buf = (char *)malloc(BODY_BUF_SIZE);
    }

    char *cameraName = NULL;
    char *alarmTime = NULL;
    char *algCode = NULL;
    char *deviceId = NULL;
    char *alarmExtension = NULL;
    char *alarmBase = NULL;

    char *des_ip = NULL;
    int des_port = 0;
    char *temp_port = NULL;

    // char *body_buf = NULL;

    cameraName = strtok((char *)body, "\r\n");
    if (cameraName == NULL)
    {
        printf("消息解析错误 cameraName\n");
        return -1;
    }

    alarmTime = strtok(NULL, "\r\n");
    if (alarmTime == NULL)
    {
        printf("消息解析错误 alarmTime\n");
        return -1;
    }

    algCode = strtok(NULL, "\r\n");
    if (algCode == NULL)
    {
        printf("消息解析错误 algCode\n");
        return -1;
    }

    deviceId = strtok(NULL, "\r\n");
    if (deviceId == NULL)
    {
        printf("消息解析错误 deviceId\n");
        return -1;
    }

    alarmExtension = strtok(NULL, "\r\n");
    if (alarmExtension == NULL)
    {
        printf("消息解析错误 alarmExtension\n");
        return -1;
    }

    alarmBase = strtok(NULL, "\r\n");
    if (alarmBase == NULL)
    {
        printf("消息解析错误 alarmBase\n");
        return -1;
    }

    des_ip = strtok(NULL, "\r\n");
    if (des_ip == NULL)
    {
        printf("消息解析错误 des_ip\n");
        return -1;
    }

    temp_port = strtok(NULL, "\r\n");
    if (temp_port == NULL)
    {
        printf("消息解析错误 temp_port\n");
        return -1;
    }

    des_port = atoi(temp_port);
    if (des_port == 0)
    {
        printf("des_port == 0\n");
        return -1;
    }
    
    memset(g_header_buf, 0x0, HEADER_BUF_SIZE);
    
    memset(g_body_buf, 0x0, BODY_BUF_SIZE);

    sprintf(g_body_buf, "{\"CameraName\":\"%s\",\"SiteData\":{\"Latitude\":\"16.24463,44.179439\",\"Longitude\":\"001\",\"Name\":\"001\"},\"ChannelName\":\"\",\"AlarmTime\":\"%s\",\"AlgCode\":\"%s\",\"DeviceId\":\"%s\",\"AlarmBoxs\":[{\"X\":1236,\"Y\":545,\"Height\":529,\"Width\":234},{\"X\":1419,\"Y\":509,\"Height\":337,\"Width\":126},{\"X\":1203,\"Y\":545,\"Height\":388,\"Width\":123}],\"AlarmExtension\":\"%s\",\"ChannelId\":\"eb5d32\",\"AlarmBase\":\"%s\"}",
            cameraName, alarmTime, algCode, deviceId,alarmExtension, alarmBase);

    // printf("msg body [%ld]:%s\n", strlen(body_buf), body_buf);
    printf("host %s:%d \n", des_ip, des_port);

    char *header = g_header_buf;

    char cl[32] = {0};
    sprintf(cl, "Content-Length: %ld", length);

    strcat(header, "POST /api/smartbox/AlarmPost HTTP/1.1");
    strcat(header, "\r\n");

    strcat(header, "Cache-Control: no-cache");
    strcat(header, "\r\n");

    strcat(header, "Connection: keep-alive");
    strcat(header, "\r\n");

    strcat(header, "Accept-Encoding: gzip,deflate,br");
    strcat(header, "\r\n");

    strcat(header, "Accept: */*");
    strcat(header, "\r\n");

    strcat(header, "Content-Type: application/json");
    strcat(header, "\r\n");

    strcat(header, "User-Agent: Mozilla/5.0");
    strcat(header, "\r\n");

    // strcat(header, "Host: 128.0.0.1\r\n");

    strcat(header, cl);
    strcat(header, "\r\n");
    strcat(header, "\r\n");

    strcat(header, g_body_buf);
    strcat(header, "\r\n");
    strcat(header, "\r\n");

    struct sockaddr_in *dest_addr;

    // printf("mesgbodu\n%s\n", header);

    int fd = socket_create("wlp0s20f3", "192.168.2.4", 7890, des_ip, des_port, dest_addr);

    if (fd < 0)
    {
        printf("soc 创建失败\n");
        return -1;
    }

    printf("http buf[%ld]\n", strlen(header));

    int result = write(fd, header, strlen(header));
    if (result != strlen(header))
    {
        printf("数据发送失败\n");
        return -1;
    }
    else
    {
        printf("数据发送成功\n");
    }

    int ri = 0, n = 0;

    // if (g_read_buf == NULL)
    // {
    //     g_read_buf = (char *)malloc(READ_BUF_SIZE);
    // }

    // memset(g_read_buf, 0x0, READ_BUF_SIZE);

    char temp[512] = {0};

    while ((n = read(fd, temp, 512)) > 0)
    {
        // 接收的数据很小
        printf("接收[%d]=========\n", ri);
        printf("%s\n", temp);
        // strlncat(g_read_buf, strlen(g_read_buf), temp, n);
        // memset(temp, 0x0, 512);
    }
    printf("接收的消息 %s \n", temp);
    // TODO: 解析接收的消息
    // printf("释放资源\n");
    socket_destroy(fd, dest_addr);
    return 0;
}