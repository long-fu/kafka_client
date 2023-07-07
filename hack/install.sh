sudo chmod +x ../build/alarm_send
# chmod +x alarm_send
# 二进制文件 /usr/local/bin/alarm_send
sudo cp ../build/alarm_send /usr/local/bin

# 守护进程文件 /usr/lib/systemd/system/alarm_send.service

sudo cp kafka_config.service /usr/lib/systemd/system

sudo cp kafka.service /usr/lib/systemd/system

sudo cp alarm_send.service /usr/lib/systemd/system

# 启动
sudo systemctl start kafka_config.service

sudo systemctl start kafka.service

sudo systemctl start alarm_send.service

# 守护进程自启动

sudo systemctl enable kafka_config.service

sudo systemctl enable kafka.service

sudo systemctl enable alarm_send.service

# 查看 日志

# journalctl -u kafka_config.service -f

# journalctl -u kafka.service -f

# journalctl -u alarm_send.service -f

# 停止

# sudo systemctl stop alarm_send.service

# sudo systemctl stop kafka_config.service

# sudo systemctl stop kafka.service