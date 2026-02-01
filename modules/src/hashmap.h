#ifndef HASHMAP_H
#define HASHMAP_H

#include <linux/hashtable.h>
#include <linux/slab.h>
#include <linux/hash.h>

// cache pointer
extern struct kmem_cache *g_entry_cache;

extern struct hlist_head *my_hashtable;

// struct for thje hashtable entry
struct hashmap_entry {
	u32 key;
	char *filename;
	struct hlist_node node;
};

int hash_initialize(void);

void hash_exit(void);

#endif
