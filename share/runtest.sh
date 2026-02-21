#!/bin/sh
insmod /mnt/bitmap.ko
./mnt/a.out 0
rmmod bitmap
exit
