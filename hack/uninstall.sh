#停止服务
sudo systemctl stop alarm_send.service

sudo systemctl stop kafak_config.service

sudo systemctl stop kafka.service

# 移除二进制文件
sudo rm /usr/local/bin/alarm_send

# 移除守护进程文件
sudo rm /usr/lib/systemd/system/alarm_send.service

sudo rm /usr/lib/systemd/system/kafak_config.service

sudo rm /usr/lib/systemd/system/kafka.service