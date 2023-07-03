int http_send(char *body, size_t length) {

    char *header = (char *)malloc(length + 500);

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
    free(header);

    int ri = 0, n = 0;
    char *rbuf = malloc(1024);
    memset(rbuf, 0x0, 512);
    // response_buf = read_buf;

    while ((n = read(fd, rbuf, 1024)) > 0)
    {
        printf("接收[%d]=========\n",ri);
        printf("%s\n",rbuf);
        // parser_responder(rbuf, strlen(rbuf));
        ri++;
        memset(rbuf, 0x0, 1024);
    }

    printf("释放资源\n");
    free(rbuf);
    destroy_socket(fd);

}