#ifndef HASHMAP_H
#define HASHMAP_H

#include <linux/hashtable.h>
#include <linux/slab.h>

static struct hlist_head *my_hashtable;

static unsigned hash_bits = 10;

static size_t hash_size;

int hash_initialize(void);

int hash_exit(void);

#endif
