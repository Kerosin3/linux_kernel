#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/limits.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/fs.h>


static int example_value;

static struct kobject *kobj_stack;

static ssize_t example_attr_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
	return sprintf(buf,"%d\n",example_value);
}

static ssize_t example_attr_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
	int ret = kstrtoint(buf, 10, &example_value);
    if (ret < 0) {
        return ret;
    }
	return count;
}
// functions example_ettr store & loaded
static struct kobj_attribute example_attr = __ATTR_RW(example_attr);

static ssize_t text_attr_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
	return sprintf(buf,"example!\n");
}

static struct kobj_attribute text_attr = __ATTR_RO(text_attr);

static struct attribute *attrs[] = {
    &example_attr.attr,
	&text_attr.attr,
    NULL,
};

static struct attribute_group attr_group = {
    .attrs = attrs,
};

static void __exit hello_exit(void)
{
	sysfs_remove_group(kobj_stack, &attr_group);
	kobject_put(kobj_stack);
	pr_info("%s: exit: Module unloaded!\n", KBUILD_MODNAME);
}

static int __init hello_init(void)
{
	int ret;
	//kobj_stack = kobject_create_and_add("test_stack", kernel_kobj);
	kobj_stack = kobject_create_and_add("test_stack", kernel_kobj);
	if (!kobj_stack)
		return -ENOMEM;
	ret = sysfs_create_group(kobj_stack, &attr_group);
	if (ret){
		kobject_put(kobj_stack);
		return ret;
	}
	pr_info("%s: init: Module loaded!\n", KBUILD_MODNAME);
	return 0;
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alex V");
MODULE_DESCRIPTION("Homework 2");
MODULE_VERSION("1.0");
