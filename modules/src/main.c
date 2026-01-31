
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

static int __init hello_init(void)
{
	/* allocate fifo with 64 int capacity */
	int ret = fifo_init(64);
	if (!ret) {
		pr_info("%s: init: Module loaded!\n", KBUILD_MODNAME);
		return 0;
	}
	return ret;
}

static void __exit hello_exit(void)
{
	fifo_cleanup();
	pr_info("%s: exit: Module unloaded!\n", KBUILD_MODNAME);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alex V");
MODULE_DESCRIPTION("Homework 4");
MODULE_VERSION("1.0");
