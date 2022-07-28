#!/bin/bash


[ "$UID" -eq 0 ] || exec sudo bash "$0" "$@"

ip link set can0 type can bitrate 1000000 sample-point 0.6 restart-ms 1
sudo ip link set can1 type can bitrate 1000000 sample-point 0.6 restart-ms 1

ip link set can0 up type can
ip link set can1 up type can
