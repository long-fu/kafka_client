#include <stdio.h>
#include <librdkafka/rdkafka.h>
#include <string.h>


#define ALARM_ENDPOINT "http://22.11.123.78:80/api/smartbox/AlarmPost"

int main(int argc, char const *argv[])
{

        return 0;
        // port = (u.field_set & (1 << UF_PORT)) ?
        //       u.port : 0;
        //     // 服务地址
        //     const char *brokers = "localhost:9092";

        //     // 生产者发送 topic
        //     const char *topic = "haoshuai-events";

        //     // 消费者订阅的topic列表
        //     char *topics[] = {"haoshuai-events"};
        //     // 消费者组ID 消费组ID bin/kafka-consumer-groups.sh --bootstrap-server localhost:9092 --list 查看
        //     char *groupid = "console-consumer-9205";
        //     // 消费者订阅几个topic
        //     int topic_cnt = 1;
        //         // 消费者
        //     return consumer(brokers, groupid, topics, 1);

        // char *data = "hello my world";
        // 生产者 发送消息
        // return producer(brokers, topic, data, strlen(data));

        // char rbuf[2048] = { 0 };
        // int fd = create_socket("wlp0s20f3","192.168.2.4",8989,"",9090);

        // int result = write(fd, http_heads, strlen(http_heads));

        // if (result != strlen(http_heads)) {
        //         perror("数据写入失败");
        //         exit(-1);
        // }

        // int ri ,n;
        // while ((n = read(fd, rbuf, 2048))>0) {
        //         rbuf[n] = 0;
        //         printf("%d 接受的数据 %s \r\n",ri ,rbuf);
        //         parser_responder(rbuf,strlen(rbuf));
        //         ri ++;
        // }
        // // close(sock);
        // printf("数据接收完成 %d", n);
}
