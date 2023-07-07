#!/bin/bash

KAFKA_CLUSTER_ID="$(root_path/bin/kafka-storage.sh random-uuid)"

root_path/bin/kafka-storage.sh format -t $KAFKA_CLUSTER_ID -c root_path/config/kraft/server.properties

root_path/bin/kafka-server-start.sh root_path/config/kraft/server.properties