#include <stdio.h>
#include <string.h>
#include <netinet/in.h>

int consumer(const char *brokers, const char *groupid, char **topics, int topic_cnt);
int http_send(const char *body, size_t length);

// char g_body_buf[1024*2048*3] = {0}; 
int main(int argc, char const *argv[])
{       
        // char helo[] = "1.2.3.4";
        // char *ret = strstr(helo, ".");
        // char msg[] = "{\"hello\":\"nihao\"}\r\n127.0.0.1\r\n8080";
        // return http_send(msg,strlen(msg));
        // struct sockaddr_in addre;
        // printf("%d\n",sizeof(addre));
        // return 0;
        // char body[] = "你好\r\n192.168.2.1\r\n8080";
        // char *body,*ip,*port;
        // body = strtok((char*)g_body, "\r\n");
        // printf("body[%d]: -\n",strlen(body));
        // ip = strtok(NULL, "\r\n");
        // printf("ip %s\n", ip);
        // port = strtok(NULL, "\r\n");
        // printf("port %s\n", port);
        // while (p){
        //         printf("%s\n", p);
        //         p = strtok(NULL, "\r\n");
	// }        
        // char des_ip[16] = {0};
        // char des_port[8] = {8};
        // sscanf(g_body,"%s\r\n%s\r\n%s",g_body_buf, des_ip, des_port);
        // printf("port %s \n", des_port);
        // int port = atoi(des_port);
        // printf("%s \n%s\n%d\n",g_body_buf,des_ip,port);
        char *topics[1] = {"alarm-events"};
        return consumer("localhost:9092","console-consumer-62319",topics,1);
}
