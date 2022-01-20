#!/usr/bin/bash

sudo hcitool lescan > list_devices.txt &
PID=$!
sleep 5
sudo kill -INT $PID
