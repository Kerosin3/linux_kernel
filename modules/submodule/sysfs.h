#ifndef SYSFS_H
#define SYSFS_H

#include <linux/module.h>
#include <linux/string.h>
#include <linux/kobject.h>

#include "hashmap_s.h"
#include "sorter.h"

extern struct kobject *kobj;

int sysfs_init(void);

void sysfs_exit(void);

#endif
