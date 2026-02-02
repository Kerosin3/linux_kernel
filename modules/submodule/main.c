#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/limits.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/kobject.h>

#include "sysfs.h"
#include "hashmap_s.h"

static int __init submod_init(void)
{
	int ret;
	ret = sysfs_init();
	return ret;
}

static void __exit submod_exit(void)
{
	sysfs_exit();
	pr_info("%s: exit: Module unloaded!\n", KBUILD_MODNAME);
}

module_init(submod_init);
module_exit(submod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alex V");
MODULE_DESCRIPTION("Homework 6 submodule");
MODULE_VERSION("1.0");
MODULE_SOFTDEP("pre: ex_bin_tree");
