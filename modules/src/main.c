#include "kernel_stack.h"

static struct kobject *kobj_stack;

static int __init hello_init(void)
{
	int ret;
	kobj_stack = kobject_create_and_add("kernel_stack", kernel_kobj);
	if (!kobj_stack)
		return -ENOMEM;
	ret = sysfs_create_group(kobj_stack, &attr_group);
	if (ret){
		kobject_put(kobj_stack);
		return ret;
	}
	stack_init();
	pr_info("%s: init: Module loaded!\n", KBUILD_MODNAME);
	return 0;
}

static void __exit hello_exit(void)
{
	struct stack_entry *node;
    struct list_head *pos, *tmp;

	sysfs_remove_group(kobj_stack, &attr_group);
	kobject_put(kobj_stack);

	// cleanup
    list_for_each_safe(pos, tmp, &stack_head) {
        node = list_entry(pos, struct stack_entry, list);
        list_del(pos);
        kfree(node);
    }
	pr_info("%s: exit: Module unloaded!\n", KBUILD_MODNAME);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alex V");
MODULE_DESCRIPTION("Homework 2");
MODULE_VERSION("1.0");

