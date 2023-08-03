#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include <librdkafka/rdkafka.h>

static void
dr_msg_cb(rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque)
{
    if (rkmessage->err)
        fprintf(stderr, "%% Message delivery failed: %s\n",
                rd_kafka_err2str(rkmessage->err));
    else
        fprintf(stderr,
                "%% Message delivered (%zd bytes, "
                "partition %" PRId32 ")\n",
                rkmessage->len, rkmessage->partition);

    /* The rkmessage is destroyed automatically by librdkafka */
}

int producer(const char *brokers, const char *topic, char *buf, size_t buf_size)
{
    rd_kafka_t *rk;        /* Producer instance handle */
    rd_kafka_conf_t *conf; /* Temporary configuration object */
    rd_kafka_resp_err_t err;
    char errstr[512];

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

    if(rd_kafka_conf_set(conf, "message.max.bytes", "33554432", errstr,sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        fprintf(stderr, "%s\n", errstr);
        rd_kafka_conf_destroy(conf);
        return 1;
    }

    /* Enable the idempotent producer */
    if (rd_kafka_conf_set(conf, "enable.idempotence", "true", errstr,
                          sizeof(errstr)) != RD_KAFKA_CONF_OK)
    {
        fprintf(stderr, "%s\n", errstr);
        rd_kafka_conf_destroy(conf);
        return 1;
    }

    /* Set the delivery report callback.
     * This callback will be called once per message to inform
     * the application if delivery succeeded or failed.
     * See dr_msg_cb() above.
     * The callback is only triggered from rd_kafka_poll() and
     * rd_kafka_flush(). */
    rd_kafka_conf_set_dr_msg_cb(conf, dr_msg_cb);

    /*
     * Create producer instance.
     *
     * NOTE: rd_kafka_new() takes ownership of the conf object
     *       and the application must not reference it again after
     *       this call.
     */
    rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
    if (!rk)
    {
        fprintf(stderr, "%% Failed to create new producer: %s\n",
                errstr);
        return 1;
    }

    if (buf_size == 0)
    {
        /* Empty line: only serve delivery reports */
        rd_kafka_poll(rk, 0 /*non-blocking */);
        return 1;
    }
retry:
    err = rd_kafka_producev(
        /* Producer handle */
        rk,
        /* Topic name */
        RD_KAFKA_V_TOPIC(topic),
        /* Make a copy of the payload. */
        RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY),
        /* Message value and length */
        RD_KAFKA_V_VALUE(buf, buf_size),
        /* Per-Message opaque, provided in
         * delivery report callback as
         * msg_opaque. */
        RD_KAFKA_V_OPAQUE(NULL),
        /* End sentinel */
        RD_KAFKA_V_END);

    if (err)
    {
        /*
         * Failed to *enqueue* message for producing.
         */
        fprintf(stderr,
                "%% Failed to produce to topic %s: %s\n", topic,
                rd_kafka_err2str(err));

        if (err == RD_KAFKA_RESP_ERR__QUEUE_FULL)
        {
            /* If the internal queue is full, wait for
             * messages to be delivered and then retry.
             * The internal queue represents both
             * messages to be sent and messages that have
             * been sent or failed, awaiting their
             * delivery report callback to be called.
             *
             * The internal queue is limited by the
             * configuration property
             * queue.buffering.max.messages and
             * queue.buffering.max.kbytes */
            rd_kafka_poll(rk,
                          1000 /*block for max 1000ms*/);
            goto retry;
        }
    }
    else
    {
        fprintf(stderr,
                "%% Enqueued message (%zd bytes) "
                "for topic %s\n",
                buf_size, topic);
    }

    /* A producer application should continually serve
     * the delivery report queue by calling rd_kafka_poll()
     * at frequent intervals.
     * Either put the poll call in your main loop, or in a
     * dedicated thread, or call it after every
     * rd_kafka_produce() call.
     * Just make sure that rd_kafka_poll() is still called
     * during periods where you are not producing any messages
     * to make sure previously produced messages have their
     * delivery report callback served (and any other callbacks
     * you register). */
    rd_kafka_poll(rk, 0 /*non-blocking*/);

    /* Wait for final messages to be delivered or fail.
     * rd_kafka_flush() is an abstraction over rd_kafka_poll() which
     * waits for all messages to be delivered. */
    fprintf(stderr, "%% Flushing final messages..\n");
    rd_kafka_flush(rk, 10 * 1000 /* wait for max 10 seconds */);

    /* If the output queue is still not empty there is an issue
     * with producing messages to the clusters. */
    if (rd_kafka_outq_len(rk) > 0)
        fprintf(stderr, "%% %d message(s) were not delivered\n",
                rd_kafka_outq_len(rk));

    /* Destroy the producer instance */
    rd_kafka_destroy(rk);

    return 0;
}
