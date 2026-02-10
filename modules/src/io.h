#ifndef IO_H
#define IO_H

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#define MAX_DEV 1

int xchardev_open(struct inode *inode, struct file *file);
int xchardev_release(struct inode *inode, struct file *file);
long xchardev_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
ssize_t xchardev_read(struct file *file, char __user *buf, size_t count, loff_t *offset);
ssize_t xchardev_write(struct file *file, const char __user *buf, size_t count, loff_t *offset);

typedef struct dev_ctx{
    struct cdev cdev;
} dev_ctx;

extern int dev_major;
extern struct class *xchardev_class;
extern struct dev_ctx dev_ctx_data[MAX_DEV];

#endif
