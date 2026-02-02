#include "hashmap.h"

struct kmem_cache *g_entry_cache;

struct hlist_head *my_hashtable;

static unsigned hash_bits = 10;

static size_t g_hash_size;

unsigned long nr_buckets;

int hash_initialize(void)
{
	nr_buckets = 1UL << hash_bits;
	g_hash_size = nr_buckets * sizeof(struct hlist_head);

	// allocate memory for hasthable
	my_hashtable = kvmalloc(g_hash_size, GFP_KERNEL);
	if (!my_hashtable) {
		pr_err("Failed to allocate hash table\n");
		return -ENOMEM;
	}
	// allocate memory for hashtable entry cache
	g_entry_cache = kmem_cache_create("hashmap_cache",
					  sizeof(struct hashmap_entry), 0,
					  SLAB_HWCACHE_ALIGN, NULL);
	if (!g_entry_cache) {
		pr_err("Failed to create slab cache\n");
		return -ENOMEM;
	}
	// init hashtable
	for (unsigned long i = 0; i < nr_buckets; i++) {
		INIT_HLIST_HEAD(&my_hashtable[i]);
	}
	pr_info("initialized hasmap with %lu entries\n", nr_buckets);
	return 0;
}

void hash_exit(void)
{
	struct hashmap_entry *entry;
	struct hlist_node *tmp;
	unsigned int bkt;

	// free caches
	for (bkt = 0; bkt < nr_buckets; bkt++) {
		hlist_for_each_entry_safe(entry, tmp, &my_hashtable[bkt],
					  node) {
			hash_del(&entry->node);
			kfree(entry->filename); // Free copied string
			kmem_cache_free(g_entry_cache, entry);
		}
	}

	// free cache structure
	kmem_cache_destroy(g_entry_cache);

	// free hashtable
	kfree(my_hashtable);

	pr_info("Hashmap cleaned up\n");
}

void add_to_hashmap(struct hashmap_entry *entry)
{
	hlist_add_head_rcu(&entry->node, my_hashtable);
}

void clean()
{
	struct hashmap_entry *entry;
	struct hlist_node *tmp;
	unsigned int bkt;

	for (bkt = 0; bkt < nr_buckets; bkt++) {
		hlist_for_each_entry_safe(entry, tmp, &my_hashtable[bkt],
					  node) {
			hash_del(&entry->node);
			kfree(entry->filename);
			kmem_cache_free(g_entry_cache, entry);
		}
	}
}
