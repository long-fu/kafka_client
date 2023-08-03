#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>

int consumer(const char *brokers, const char *groupid, char **topics, int topic_cnt);
int http_send(const char *body, size_t length);
int producer(const char *brokers, const char *topic, char *buf, size_t buf_size);
// cameraName, alarmTime, algCode, deviceId,alarmExtension, alarmBase
char msg[] = "cameraName\r\nalarmTime\r\nalgCode\r\ndeviceId\r\nalarmExtension\r\nalarmBase\r\n192.168.2.2\r\n8080";

// char g_body_buf[1024*2048*3] = {0};
int main(int argc, char const *argv[])
{
        // const char *broker;
        // const char *topic; /* Argument: topic to produce to */
        // const char *groupid;
        // broker = argv[1];
        // topic = argv[2];
        // groupid = argv[3];
        // printf("[kafak config] %s %s %s \n", broker, topic, groupid);
        // char *topics[1] = {topic};
        // return consumer(broker, groupid, topics, 1);
        FILE *fp;
        fp = fopen("/home/haoshuai/code/kafka_client/opencv-3.4.16.zip", "r");
        if (fp == NULL)
        {

                printf("文件读取失败");
                return 0;
        }
        fseek(fp, 0, SEEK_END);
        int file_size = 0;
        file_size = ftell(fp);
        void *data = malloc(file_size);
        fseek(fp, 0, SEEK_SET);
        fread(data, 1, file_size, fp);
        printf("发送消息大小 %d\n", file_size);
        producer("localhost:9092", "alarm-events1", data, file_size);
        free(data);
        fclose(fp);
        return 0;

        // char *topics[1] = {"alarm-events"};
        // return consumer("localhost:9092", "console-consumer-62377", topics, 1);
}
