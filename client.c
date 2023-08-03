#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>


#include <librdkafka/rdkafka.h>

int http_send(const char *body, size_t length);
int consumer(const char *brokers, const char *groupid, char **topics, int topic_cnt);

static volatile sig_atomic_t run = 1;

/**
 * @brief Signal termination of program
 */
static void stop(int sig)
{
    run = 0;
}

int consumer(const char *brokers, const char *groupid, char **topics, int topic_cnt)
{
    rd_kafka_t *rk;                                /* Consumer instance handle */
    rd_kafka_conf_t *conf;                         /* Temporary configuration object */
    rd_kafka_resp_err_t err;                       /* librdkafka API error code */
    char errstr[512];                              /* librdkafka API error reporting buffer */
    rd_kafka_topic_partition_list_t *subscription; /* Subscribed topics */
    int i;
    /*
     * Create Kafka client configuration place-holder
     */
    conf = rd_kafka_conf_new();

    /* Set bootstrap broker(s) as a comma-separated list of
     * host or host:port (default port 9092).
     * librdkafka will use the bootstrap brokers to acquire the full
     * set of brokers from the cluster. */
    if (rd_kafka_conf_set(conf, "bootstrap.servers", brokers, errstr,
                          sizeof(errstr)) != RD_KAFKA_CONF_OK)
    {
        fprintf(stderr, "%s\n", errstr);
        rd_kafka_conf_destroy(conf);
        return 1;
    }

    /* Set the consumer group id.
     * All consumers sharing the same group id will join the same
     * group, and the subscribed topic' partitions will be assigned
     * according to the partition.assignment.strategy
     * (consumer config property) to the consumers in the group. */
    if (rd_kafka_conf_set(conf, "group.id", groupid, errstr,
                          sizeof(errstr)) != RD_KAFKA_CONF_OK)
    {
        fprintf(stderr, "%s\n", errstr);
        rd_kafka_conf_destroy(conf);
        return 1;
    }

    /* If there is no previously committed offset for a partition
     * the auto.offset.reset strategy will be used to decide where
     * in the partition to start fetching messages.
     * By setting this to earliest the consumer will read all messages
     * in the partition if there was no previously committed offset. */
    if (rd_kafka_conf_set(conf, "auto.offset.reset", "earliest", errstr,
                          sizeof(errstr)) != RD_KAFKA_CONF_OK)
    {
        fprintf(stderr, "%s\n", errstr);
        rd_kafka_conf_destroy(conf);
        return 1;
    }

    /*
     * Create consumer instance.
     *
     * NOTE: rd_kafka_new() takes ownership of the conf object
     *       and the application must not reference it again after
     *       this call.
     */
    rk = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr));
    if (!rk)
    {
        fprintf(stderr, "%% Failed to create new consumer: %s\n",
                errstr);
        return 1;
    }

    conf = NULL; /* Configuration object is now owned, and freed,
                  * by the rd_kafka_t instance. */

    /* Redirect all messages from per-partition queues to
     * the main queue so that messages can be consumed with one
     * call from all assigned partitions.
     *
     * The alternative is to poll the main queue (for events)
     * and each partition queue separately, which requires setting
     * up a rebalance callback and keeping track of the assignment:
     * but that is more complex and typically not recommended. */
    rd_kafka_poll_set_consumer(rk);

    /* Convert the list of topics to a format suitable for librdkafka */
    subscription = rd_kafka_topic_partition_list_new(topic_cnt);
    for (i = 0; i < topic_cnt; i++)
    {
        rd_kafka_topic_partition_list_add(subscription, topics[i],
                                          /* the partition is ignored
                                           * by subscribe() */
                                          RD_KAFKA_PARTITION_UA);
    }

    /* Subscribe to the list of topics */
    err = rd_kafka_subscribe(rk, subscription);
    if (err)
    {
        fprintf(stderr, "%% Failed to subscribe to %d topics: %s\n",
                subscription->cnt, rd_kafka_err2str(err));
        rd_kafka_topic_partition_list_destroy(subscription);
        rd_kafka_destroy(rk);
        return 1;
    }

    fprintf(stderr,
            "%% Subscribed to %d topic(s), "
            "waiting for rebalance and messages...\n",
            subscription->cnt);

    rd_kafka_topic_partition_list_destroy(subscription);

    /* Signal handler for clean shutdown */
    signal(SIGINT, stop);

    /* Subscribing to topics will trigger a group rebalance
     * which may take some time to finish, but there is no need
     * for the application to handle this idle period in a special way
     * since a rebalance may happen at any time.
     * Start polling for messages. */

    while (run)
    {
        rd_kafka_message_t *rkm;

        rkm = rd_kafka_consumer_poll(rk, 100);
        if (!rkm)
            continue; /* Timeout: no message within 100ms,
                       *  try again. This short timeout allows
                       *  checking for `run` at frequent intervals.
                       */

        /* consumer_poll() will return either a proper message
         * or a consumer error (rkm->err is set). */
        if (rkm->err)
        {
            /* Consumer errors are generally to be considered
             * informational as the consumer will automatically
             * try to recover from all types of errors. */
            fprintf(stderr, "%% Consumer error: %s\n",
                    rd_kafka_message_errstr(rkm));
            rd_kafka_message_destroy(rkm);
            continue;
        }
        int leader_epoch = rd_kafka_message_leader_epoch(rkm);
        /* Proper message. */
        printf("Message on %s [%" PRId32 "] at offset %" PRId64
               " (leader epoch %" PRId32 "):\n",
               rd_kafka_topic_name(rkm->rkt), rkm->partition,
               rkm->offset, leader_epoch);

        /* Print the message key. */
        if (rkm->key)
        {
            // 接收的数据
            printf(" Key: (%d bytes): %s\n", (int)rkm->key_len, (const char*)rkm->key);
        }

        /* Print the message value/payload. */
        if (rkm->payload)
        {
            // 接收的数据
            fprintf(stderr," Value: (%d bytes)\n", (int)rkm->len);
            // 调用http消息发送
            // int ret = http_send((const char*)rkm->payload,(size_t)rkm->len);
            // if (ret < 0) {
            //     continue;
            // }
        }

        rd_kafka_message_destroy(rkm);
    }

    /* Close the consumer: commit final offsets and leave the group. */
    fprintf(stderr, "%% Closing consumer\n");
    rd_kafka_consumer_close(rk);

    /* Destroy the consumer */
    rd_kafka_destroy(rk);

    return 0;
}
