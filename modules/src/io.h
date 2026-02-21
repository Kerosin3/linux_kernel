#ifndef IO_H
#define IO_H

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>

#include "arena.h"

#define DEVICE_NAME "xchardev"
#define CLASSNAME "xcharclass"
#define BUF_SIZE 1024

#define XCDEV_IOC_MAGIC 'x'
// get free blocks
#define XCDEV_IOC_GETFREEBLOCKS _IOR(XCDEV_IOC_MAGIC, 1, unsigned)
// allocate next block
#define XCDEV_IOC_ALLOCBLOCK _IO(XCDEV_IOC_MAGIC, 2)
// find and free some block
#define XCDEV_IOC_FREEBLOCK _IO(XCDEV_IOC_MAGIC, 3)
// get allocated blocks number
#define XCDEV_IOC_GETALLOCATEDBLOCKS _IOR(XCDEV_IOC_MAGIC, 4, unsigned)
// free specific block
#define XCDEV_IOC_FREE_A_BLOCK _IOW(XCDEV_IOC_MAGIC, 5, unsigned)
// alloc specific block
#define XCDEV_IOC_ALLOC_A_BLOCK _IOW(XCDEV_IOC_MAGIC, 6, unsigned)
// show statistics
#define XCDEV_IOC_STAT _IO(XCDEV_IOC_MAGIC, 7)

int xdev_init(void);
void xdev_exit(void);

typedef struct xchardev_data {
	struct cdev cdev;
	struct device *device;
	char *buffer;
	size_t buf_size;
	struct mutex lock;
} xchardev_data;

// allocator
typedef struct xdev_xdata {
	unsigned free_blocks_count;
} xdev_xdata;

int xchardev_open(struct inode *inode, struct file *file);
int xchardev_release(struct inode *inode, struct file *file);
long xchardev_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
ssize_t xchardev_read(struct file *file, char __user *buf, size_t count,
		      loff_t *offset);
ssize_t xchardev_write(struct file *file, const char __user *buf, size_t count,
		       loff_t *offset);

static dev_t dev_num;
extern struct class *xchardev_class;
extern struct xchardev_data *xchardev;

static const struct file_operations xchardev_fops = {
	.owner = THIS_MODULE,
	.open = xchardev_open,
	.release = xchardev_release,
	.read = xchardev_read,
	.write = xchardev_write,
	.unlocked_ioctl = xchardev_ioctl,
};

#endif
