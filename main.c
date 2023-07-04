#include <stdio.h>
#include <string.h>


int consumer(const char *brokers, const char *groupid, char **topics, int topic_cnt);

int main(int argc, char const *argv[])
{
        char *topics[1] = {"alarm-events"};
        return consumer("localhost:9092","console-consumer-62319",topics,1);
        return 0;
}
