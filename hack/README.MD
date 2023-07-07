# 首次运行配置

```sh
#1. 安装jdk
sudo apt install openjdk-8-jdk-headless

#2. 配置并启动kafak(单击版本)
cd ~

wget https://archive.apache.org/dist/kafka/3.4.0/kafka_2.13-3.4.0.tgz

tar -xzf kafka_2.13-3.4.0.tgz

# cd kafka_2.13-3.4.0

# KAFKA_CLUSTER_ID="$(bin/kafka-storage.sh random-uuid)"

# # 2.1格式化日志
# bin/kafka-storage.sh format -t $KAFKA_CLUSTER_ID -c config/kraft/server.properties

# # 2.1启动kafka
# bin/kafka-server-start.sh config/kraft/server.properties

# # 2.1  查看topic 
# bin/kafka-topics.sh --list --bootstrap-server localhost:9092

#3.编译kafka c/c++客户端 librdkafka

git clone https://github.com/confluentinc/librdkafka.git
cd librdkafka
./configure
make
sudo make install

# 后续可能会出现此错误 Error while loading shared libraries: librdkafka.so.1: cannot open shared object file: No such file or directory

# Either that or add /usr/local/lib to /etc/ld.so.conf (and run ldconfig afterwards to update)
# 编辑 /etc/ld.so.conf 增加 /usr/local/lib 配置后，执行 sudo ldconfig 命令

#4 编译kafka客户端 kafka_client

cd kafka_client
mkdir build
cd build
cmake ..
cmake --build .
cd ../hack
sudo bash install.sh

```