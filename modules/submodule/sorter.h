#ifndef SORTER_H
#define SORTER_H

#include <linux/string.h>
#include <linux/hashtable.h>
#include <linux/slab.h>
#include <linux/sort.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include "hashmap_s.h"

int print_sorted_filenames(char *, ssize_t);

#endif
