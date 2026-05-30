#!/bin/sh
cd /mnt/
insmod rust_module.ko
./test_mmap
rmmod rust_module

