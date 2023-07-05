#include <stdio.h>
#include <string.h>


int consumer(const char *brokers, const char *groupid, char **topics, int topic_cnt);

int main(int argc, char const *argv[])
{
        // char body[] = "你好\r\n192.168.2.1\r\n8080";
        // char g_body_buf[32] = {0}; 
        // char des_ip[16] = {0};
        // char des_port[8] = {8};
        // sscanf(body,"%s\r\n%s\r\n%s",g_body_buf, des_ip, des_port);
        // printf("port %s \n", des_port);
        // int port = atoi(des_port);
        // printf("%s \n%s\n%d\n",g_body_buf,des_ip,port);
        char *topics[1] = {"alarm-events"};
        return consumer("localhost:9092","console-consumer-62319",topics,1);
}
