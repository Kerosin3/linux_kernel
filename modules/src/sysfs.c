#include "sysfs.h"


ssize_t enqueue_store(struct kobject *kobj, struct kobj_attribute *attr,
		    const char *buf, size_t count)
{
	int ret = kstrtoint(buf, 10, &enqueue_val);
	if (ret < 0) {
		return ret;
	}
	ret = fifo_enqueue(enqueue_val);
	if (!ret)
		pr_err("%s: failed to add an element to kfifo\n", KBUILD_MODNAME);
	return count;
}

static struct kobj_attribute enqueue = __ATTR_WO(enqueue);

/* sysfs attributes */

static struct attribute *attrs[] = {
	&enqueue.attr,
	NULL,
};

struct attribute_group attr_group = {
	.attrs = attrs,
};
