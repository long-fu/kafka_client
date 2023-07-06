sudo chmod +x ../build/alarm_send
# chmod +x alarm_send
# 二进制文件 /usr/local/bin/alarm_send
sudo cp ../build/alarm_send /usr/local/bin

# 守护进程文件 /usr/lib/systemd/system/alarm_send.service
sudo cp alarm_send.service /usr/lib/systemd/system

# 启动
sudo systemctl start alarm_send.service

# 守护进程自启动
sudo systemctl enable alarm_send.service

# 查看 alarm_send 日志
# journalctl -u alarm_send.service -f

# sudo systemctl stop alarm_send.service