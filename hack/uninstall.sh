#停止服务
sudo systemctl stop alarm_send.service

# 移除二进制文件
sudo rm /usr/local/bin/alarm_send

# 移除守护进程文件
sudo rm /usr/lib/systemd/system/alarm_send.service