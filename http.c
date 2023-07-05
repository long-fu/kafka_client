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

//1,048,576 284,555
#define BODY_BUF_SIZE 1048576
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

  if (dlen < len) {
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

  if (len > 0) {
    ncpy = slen < len ? slen : (len - 1);
    memcpy(dst, src, ncpy);
    dst[ncpy] = '\0';
  }

//   assert(len > slen);
  return slen;
}

int http_send(const char *body, size_t length) {

    if (body == NULL || length == 0) {
        return 0;
    }

    printf("内存分配0\n");
    if(g_header_buf == NULL) {
        g_header_buf = (char *)malloc(HEADER_BUF_SIZE);
    }
    printf("内存分配1\n");
    if(g_body_buf == NULL) {
        g_body_buf = (char *)malloc(BODY_BUF_SIZE);
    }
    printf("内存分配2\n");
    memset(g_body_buf, 0x0, BODY_BUF_SIZE);

    char des_ip[32] = {0};
    int des_port = 0;
    printf("内存分配21\n");
    sscanf(body,"%s\r\n%s:%d",g_body_buf, des_ip, &des_port);
    
    printf("内存分配3\n");
    memset(g_header_buf, 0x0, HEADER_BUF_SIZE);
    printf("内存分配4\n");
    printf("msg body %s\n", g_body_buf);
    printf("host %s:%d \n", des_ip,des_port);
    printf("内存分配5\n");
    char* header = g_header_buf;

    char cl[32] = {0};
    sprintf(cl,"Content-Length: %ld", length);

    strcat(header, "POST /api/smartbox/AlarmPost HTTP/1.1");
    strcat(header, "\r\n");

    strcat(header, "Cache-Control: no-cache");
    strcat(header, "\r\n");

    strcat(header, "Connection: close");
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

    strcat(header, g_body_buf);
    strcat(header, "\r\n");
    strcat(header, "\r\n");

    int fd = create_socket("enp0s31f6", "22.10.133.111", 9291, des_ip, des_port);
    if (fd < 0) {
        printf("soc 创建失败\n");
        return -1;
    }
    // 创建连接
    int result = write(fd, header, strlen(header));
    if (result != strlen(header))
    {
        printf("数据发送失败\n");
        return -1;
    } else {
        
        printf("数据发送成功\n");

    }

    int ri = 0, n = 0;
    if(g_read_buf == NULL) {
        g_read_buf = (char*)malloc(READ_BUF_SIZE);
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