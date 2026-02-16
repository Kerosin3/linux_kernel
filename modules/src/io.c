#include "io.h"

struct xchardev_data *xchardev;

struct class *xchardev_class = NULL;

int xchardev_open(struct inode *inode, struct file *file)
{
	struct xchardev_data *data;
	data = container_of(inode->i_cdev, struct xchardev_data, cdev);
	file->private_data = data;
	pr_info("%s: Device opened!", KBUILD_MODNAME);
	return 0;
}

int xchardev_release(struct inode *inode, struct file *file)
{
	pr_info("%s: Device %s closed\n", KBUILD_MODNAME, DEVICE_NAME);
	return 0;
}

long xchardev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct xchardev_data *data = file->private_data;
	int ret, value;

	if (_IOC_TYPE(cmd) != XCDEV_IOC_MAGIC) {
		pr_warn("%s: wrong ioctl magic\n", DEVICE_NAME);
		return -ENOTTY;
	}
	// max number
	if (_IOC_NR(cmd) > 3) {
		pr_warn("%s: Invalid ioctl command number\n", DEVICE_NAME);
		return -ENOTTY;
	}
	// lock mutex
	if (mutex_lock_interruptible(&data->lock))
		return -ERESTARTSYS;
	switch (cmd) {
	case XCDEV_IOC_GETFREEBLOCKS:
		pr_info("%s: GETTING FREE BLOCKS\n", DEVICE_NAME);
		pr_info("%s: -->[%u]<-->\n", DEVICE_NAME,
			alloc_ctx->allocated_blocks);
		put_user(alloc_ctx->initial_block_limit -
				 get_number_of_allocated(),
			 (unsigned __user *)arg);
		break;
	case XCDEV_IOC_ALLOCBLOCK:
		pr_info("%s ALLOCATE A BLOCK\n", DEVICE_NAME);
		ret = alloc_block();
		if (ret < 0) {
			pr_err("%s: No free blocks available\n", DEVICE_NAME);
			ret = -ENOSPC;
			break;
		}
		break;
	case XCDEV_IOC_FREEBLOCK:
		pr_info("%sFREE A BLOCK\n", DEVICE_NAME);
		ret = free_some_block();
		if (ret < 0) {
			pr_err("%s: all blocks are free\n", DEVICE_NAME);
			ret = -EAGAIN;
			break;
		}
		break;
	default:
		pr_warn("%s: Unknown ioctl command: 0x%x\n", DEVICE_NAME, cmd);
		ret = -ENOTTY;
		break;
	}

	mutex_unlock(&data->lock);
	return ret;
}

ssize_t xchardev_read(struct file *file, char __user *buf, size_t count,
		      loff_t *offset)
{
	return 0;
}

ssize_t xchardev_write(struct file *file, const char __user *buf, size_t count,
		       loff_t *offset)
{
	return 0;
}

int xdev_init(void)
{
	int ret;

	// init chardev
	xchardev = kvzalloc(sizeof(struct xchardev_data), GFP_KERNEL);
	if (!xchardev) {
		pr_err("%s: cannot allocate momory for chardev!\n",
		       KBUILD_MODNAME);
		return -ENOMEM;
	}
	// init buffer memory
	xchardev->buffer = kvzalloc(BUF_SIZE, GFP_KERNEL);
	if (!xchardev->buffer) {
		pr_err("%s: cannot allocate momory for buffer!\n",
		       KBUILD_MODNAME);
		ret = -ENOMEM;
		goto err_free_chardev;
	}

	// init mutex
	mutex_init(&xchardev->lock);

	// init dev numbers
	ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if (ret < 0) {
		pr_err("%s: cannot allocate chardev region\n", KBUILD_MODNAME);
		goto err_buf_mem_alloc;
	}
	pr_info("%s: allocated device: MAJ:%d MIN:%d\n", DEVICE_NAME,
		MAJOR(dev_num), MINOR(dev_num));

	cdev_init(&xchardev->cdev, &xchardev_fops);
	xchardev->cdev.owner = THIS_MODULE;

	// add cdev
	ret = cdev_add(&xchardev->cdev, dev_num, 1);
	if (ret < 0) {
		pr_err("%s: failed to add chardev %s\n", KBUILD_MODNAME,
		       DEVICE_NAME);
		goto err_dev_numbers;
	}

	// create device class
	xchardev_class = class_create(CLASSNAME);
	if (IS_ERR(xchardev_class)) {
		pr_err("%s: failed create devclass %s\n", KBUILD_MODNAME,
		       DEVICE_NAME);
		ret = PTR_ERR(xchardev_class);
		goto err_cdev_add;
	}

	// create device
	xchardev->device =
		device_create(xchardev_class, NULL, dev_num, NULL, DEVICE_NAME);
	if (IS_ERR(xchardev->device)) {
		pr_err("%s: failed to create device %s\n", KBUILD_MODNAME,
		       DEVICE_NAME);
		ret = PTR_ERR(xchardev->device);
		goto err_class_reg;
	}

	pr_info("%s: Device created successfully at /dev/%s\n", DEVICE_NAME,
		DEVICE_NAME);

	return 0;
err_class_reg:
	class_destroy(xchardev_class);

err_cdev_add:
	cdev_del(&xchardev->cdev);

err_dev_numbers:
	unregister_chrdev_region(dev_num, 1);

err_buf_mem_alloc:
	kvfree(xchardev->buffer);

err_free_chardev:
	kvfree(xchardev);
	return ret;
}

void xdev_exit(void)
{
	pr_info("%s: Cleaning up character device\n", DEVICE_NAME);

	device_destroy(xchardev_class, dev_num);

	class_destroy(xchardev_class);

	cdev_del(&xchardev->cdev);

	unregister_chrdev_region(dev_num, 1);

	kfree(xchardev->buffer);
	kfree(xchardev);

	pr_info("%s: Module unloaded\n", DEVICE_NAME);
}
