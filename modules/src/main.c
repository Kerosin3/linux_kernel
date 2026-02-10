#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

#include "io.h"

static const struct file_operations mychardev_fops = {
    .owner      = THIS_MODULE,
    .open       = xchardev_open,
    .release    = xchardev_release,
    .unlocked_ioctl = xchardev_ioctl,
    .read = xchardev_read,
    .write       = xchardev_write
};

static int __init mainmod_init(void)
{
	int err;
	dev_t dev;
	err = alloc_chrdev_region(&dev, 0, MAX_DEV, "xchardev");
	dev_major = MAJOR(dev);
	xchardev_class = class_create("xchardev");
	cdev_init(&dev_ctx_data->cdev, &mychardev_fops);
	dev_ctx_data->cdev.owner = THIS_MODULE;
	cdev_add(&dev_ctx_data->cdev, MKDEV(dev_major, 0), 1);
	device_create(xchardev_class, NULL, MKDEV(dev_major, 0), NULL, "xchardev-%d", 0);
	return err;
}

static void __exit mainmod_exit(void)
{
	device_destroy(xchardev_class, MKDEV(dev_major, 0));

    class_unregister(xchardev_class);
    class_destroy(xchardev_class);

    unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);

	pr_info("%s: exit: Module unloaded!\n", KBUILD_MODNAME);
}

module_init(mainmod_init);
module_exit(mainmod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alex V");
MODULE_DESCRIPTION("Homework 6");
MODULE_VERSION("1.0");
