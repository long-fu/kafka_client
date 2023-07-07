#!/bin/bash
sudo systemctl stop alarm_send.service

sudo systemctl stop kafka.service

sudo rm /usr/lib/systemd/system/kafka.service

sudo rm /usr/lib/systemd/system/alarm_send.service

sudo systemctl daemon-reload