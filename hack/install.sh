#!/bin/bash
sudo apt install openjdk-8-jdk-headless

java -version
javac -version

echo $JAVA_HOME

root_path=$(pwd)
echo $root_path

kafka_broker=$root_path'/kafka_2.13-3.5.0'
echo $kafka_broker

# 替换字符串

kafka_path=$root_path'/kafka_2.13-3.5.0'

kafka_start=$root_path'/kafka_2.13-3.5.0/start.sh'

kafka_stop=$root_path'/kafka_2.13-3.5.0/stop.sh'

kafka_service=$root_path'/kafka_2.13-3.5.0/kafka.service'

rm $kafka_start

rm $kafka_stop

cp $root_path'/kafka_2.13-3.5.0/start_kafka.sh' $kafka_start

cp $root_path'/kafka_2.13-3.5.0/stop_kafka.sh' $kafka_stop

old_path="root_path"

sed -i "s#$old_path#$kafka_path#g" $kafka_start

sed -i "s#$old_path#$kafka_path#g" $kafka_stop

sudo chmod +x $kafka_start

sudo chmod +x $kafka_stop

old_path="JDK_HOME"

cp $kafka_service $kafka_service'.bak'

sed -i "s#$old_path#$JAVA_HOME#g" $kafka_service'.bak'

old_path="start.sh"

sed -i "s#$old_path#$kafka_start#g" $kafka_service'.bak'

old_path="stop.sh"

sed -i "s#$old_path#$kafka_stop#g" $kafka_service'.bak'

sudo mv $kafka_service'.bak' /usr/lib/systemd/system/kafka.service

kafka_client_lib=$root_path'/librdkafka'
echo $kafka_client_lib

cd $kafka_client_lib
sudo make clean
sudo ./configure
sudo make
sudo make install

sudo sh -c "echo "/usr/local/lib" >> /etc/ld.so.conf"
sudo ldconfig

kafka_client=$root_path'/kafka_client'
echo $kafka_client

cd $kafka_client
rm -rf build
mkdir build
cd build
cmake ..
cmake --build .

alarm_send=$root_path'/kafka_client/build/alarm_send'

sudo chmod +x $alarm_send

alarm_service=$root_path'/kafka_client/alarm_send.service'

cp $alarm_service $alarm_service'.bak'

old_path='alarm_send'
# 
sed -i s#$old_path#$alarm_send#g $alarm_service'.bak'

sudo mv $alarm_service'.bak' /usr/lib/systemd/system/alarm_send.service

sudo systemctl daemon-reload

sudo systemctl start kafka.service

cd $kafka_broker

sleep 1s

bin/kafka-topics.sh --create --topic alarm-events --bootstrap-server localhost:9092

bin/kafka-topics.sh --list --bootstrap-server localhost:9092

sudo systemctl start alarm_send.service

bin/kafka-consumer-groups.sh --bootstrap-server localhost:9092 --list


sudo systemctl enable kafka.service

sudo systemctl enable alarm_send.service

# sudo systemctl restart alarm_send.service

# journalctl -u kafka.service -f

# journalctl -u alarm_send.service -f

