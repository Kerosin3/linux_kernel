#ifndef SYSFS_H
#define SYSFS_H

#include <linux/module.h>
#include <linux/string.h>
#include <linux/kobject.h>

#include "scanner.h"

#define PATH_MAX_LENGTH 256

static struct kobject *kobj;

int sysfs_init(void);

void sysfs_exit(void);

extern int scan_directory(const char *path);

#endif
