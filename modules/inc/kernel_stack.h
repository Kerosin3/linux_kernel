#ifndef KERNEL_STACK_H
#define KERNEL_STACK_H

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
#include "stack.h"

// push attribute
static int push_val;

ssize_t push_show(struct kobject*, struct kobj_attribute*, char*);

ssize_t push_store(struct kobject*, struct kobj_attribute*, const char*,size_t);

extern struct attribute_group attr_group;

// size attribute

static unsigned size_val;

ssize_t size_show(struct kobject*, struct kobj_attribute*, char*);

// peek attribute
static int peek_val;

ssize_t peek_show(struct kobject*, struct kobj_attribute*, char*);

#endif
