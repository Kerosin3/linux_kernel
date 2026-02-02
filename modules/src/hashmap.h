#ifndef HASHMAP_H
#define HASHMAP_H

#include <linux/hashtable.h>
#include <linux/slab.h>
#include <linux/hash.h>

#include "hashmap_s.h"

int hash_initialize(void);

void hash_exit(void);

void add_to_hashmap(struct hashmap_entry *);

void clean(void);

#endif
