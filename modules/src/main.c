
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/limits.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/kobject.h>
#include <linux/fs.h>

#include "kfifo.h"
#include "sysfs.h"

static struct kobject *kobj_fifo;

static int __init hello_init(void)
{
	int ret;
	kobj_fifo = kobject_create_and_add("kernel_fifo", kernel_kobj);
	if (!kobj_fifo)
		return -ENOMEM;
	ret = sysfs_create_group(kobj_fifo, &attr_group);
	if (ret) {
		kobject_put(kobj_fifo);
		return ret;
	}
	ret = fifo_init(0);
	pr_info("%s: init: Module loaded!\n", KBUILD_MODNAME);
	return 0;
}

static void __exit hello_exit(void)
{
	sysfs_remove_group(kobj_fifo, &attr_group);
	kobject_put(kobj_fifo);
	fifo_cleanup();
	pr_info("%s: exit: Module unloaded!\n", KBUILD_MODNAME);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alex V");
MODULE_DESCRIPTION("Homework 4");
MODULE_VERSION("1.0");
