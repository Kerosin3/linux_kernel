#ifndef HASHMAP_S
#define HASHMAP_S

// struct for hashtable entry
struct hashmap_entry {
	u32 key;
	char *filename;
	struct hlist_node node;
};

// cache for entries
extern struct kmem_cache *g_entry_cache;

// hastable
extern struct hlist_head *my_hashtable;

// number of buckets
extern unsigned long nr_buckets;

#endif
