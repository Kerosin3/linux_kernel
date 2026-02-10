#include "io.h"

struct dev_ctx dev_ctx_data[MAX_DEV];

int dev_major = 0;

struct class *xchardev_class = NULL;

int xchardev_open(struct inode *inode, struct file *file){
	return 0;
}

int xchardev_release(struct inode *inode, struct file *file){
	return 0;
}

long xchardev_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	return 0;
}

ssize_t xchardev_read(struct file *file, char __user *buf, size_t count, loff_t *offset){
	return 0;
}

ssize_t xchardev_write(struct file *file, const char __user *buf, size_t count, loff_t *offset){
	return 0;
}


