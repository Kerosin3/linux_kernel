# Homework 8

ls /lib/modules/$(uname -r)/kernel/drivers/misc/softpwm.ko

# load
sudo modprobe softpwm gpio_chip_label=gpio1 gpio_pin=7
