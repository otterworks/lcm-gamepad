#!/usr/bin/env bash

for e in /sys/class/input/event*; do 
  echo ${e};
  cat ${e}/device/name;
done;

tail -n13 /proc/bus/input/devices
