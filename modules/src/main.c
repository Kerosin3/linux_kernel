#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/limits.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/kobject.h>
#include <linux/fs.h>

#include "sysfs.h"
#include "hashmap.h"

// export symbols
EXPORT_SYMBOL(g_entry_cache);
EXPORT_SYMBOL(my_hashtable);
EXPORT_SYMBOL(kobj);
EXPORT_SYMBOL(nr_buckets);

static int __init mainmod_init(void)
{
	int ret;
	ret = sysfs_init();
	if (ret)
		return ret;
	ret = hash_initialize();
	if (ret)
		return -ENOMEM;
	return ret;
}

static void __exit mainmod_exit(void)
{
	sysfs_exit();
	hash_exit();
	pr_info("%s: exit: Module unloaded!\n", KBUILD_MODNAME);
}

module_init(mainmod_init);
module_exit(mainmod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alex V");
MODULE_DESCRIPTION("Homework 6");
MODULE_VERSION("1.0");
