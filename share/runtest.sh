#!/bin/sh
REQUIRED_CPUS=4
ACTUAL_CPUS=$(grep -c "^processor" /proc/cpuinfo)
if [ "$ACTUAL_CPUS" -lt "$REQUIRED_CPUS" ]; then
    echo "error: required $REQUIRED_CPUS CPUs, got $ACTUAL_CPUS"
    exit 1
fi
echo "CPUs: $ACTUAL_CPUS — OK"
cd /mnt/
insmod kernel_sync_demo.ko  lock_type=0 num_threads=60 iterations=100000
cd /sys/module/kernel_sync_demo//parameters/
sleep 0.1
echo "----------------------------------------------------------"
echo "setting SPINLOCK locktype"
echo "----------------------------------------------------------"
echo 1 > run
cat stats
cat result
echo 1 > reset
echo "----------------------------------------------------------"
echo "setting MUTEX locktype"
echo "----------------------------------------------------------"
sleep 0.1
echo 1 > lock_type
echo 1 > run
cat stats
cat result
echo 1 > reset
echo "----------------------------------------------------------"
echo "setting SEMAPHORE locktype (please wait a bit longer)"
echo "----------------------------------------------------------"
sleep 0.1
echo 2 > lock_type
echo 1 > run
cat stats
cat result
echo "----------------------------------------------------------"
rmmod kernel_sync_demo
if [ $? -eq 0 ]; then
    echo "module unloaded successfully"
	echo "All tests were complited succesfully"
else
    echo "module unload failed: $?"
fi

