
#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/mutex.h>

#include "io.h"
#include "arena.h"

#define M_BLOCK_SIZE 128
#define M_BLOCK_NUMBER 1024 // 2048 reserved

static int __init mainmod_init(void)
{
	int ret;
	ret = xdev_init();
	if (ret)
		return ret;
	ret = init_arena(M_BLOCK_SIZE, M_BLOCK_NUMBER);
	if (ret) {
		pr_err("%s: init: cannot alocate arena!\n", KBUILD_MODNAME);
		return ret;
	}
	pr_info("%s: init: Module loaded!\n", KBUILD_MODNAME);
	return ret;
}

static void __exit mainmod_exit(void)
{
	xdev_exit();
	destroy_arena();
	pr_info("%s: exit: Module unloaded!\n", KBUILD_MODNAME);
}

module_init(mainmod_init);
module_exit(mainmod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alex V");
MODULE_DESCRIPTION("Homework 5");
MODULE_VERSION("1.0");
