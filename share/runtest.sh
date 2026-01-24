#!/bin/sh

insmod /mnt/hw2.ko || { echo "Error loading module"; exit 1; }

str="Hello world!"
len=${#str}
index=5

for i in $(seq 0 $(($len - 1))); do
  char=${str:$i:1}
  ascii=$(printf "%d" "'$char")
  echo $index > /sys/modules/hw2/parameters/idx
  echo $ascii > /sys/modules/hw2/parameters/ch_val
  index=$(($index + 1))
done

read_back=$(cat /sys/modules/hw2/parameters/my_str)
if [ "$read_back" = "Hello world!" ]; then
  echo "test passed!"
fi
