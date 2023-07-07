# 唯一ID
KAFKA_CLUSTER_ID="$(bin/kafka-storage.sh random-uuid)"
# echo $KAFKA_CLUSTER_ID
# 格式化日志
bin/kafka-storage.sh format -t $KAFKA_CLUSTER_ID -c config/kraft/server.properties

# 启动kafka
bin/kafka-server-start.sh config/kraft/server.properties