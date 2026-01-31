#ifndef SYSFS_H
#define SYSFS_H

#include <linux/sysfs.h>
#include <linux/kobject.h>
#include "kfifo.h"

extern struct attribute_group attr_group;

// enqueue attribute
static int enqueue_val;

ssize_t enqueue_store(struct kobject *, struct kobj_attribute *, const char *,
		    size_t);

#endif
