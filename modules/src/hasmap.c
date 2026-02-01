#include "hashmap.h"

int hash_initialize(void)
{
	unsigned long nr_buckets = 1UL << hash_bits;
	hash_size = (1UL << hash_bits) * sizeof(struct hlist_head);
	my_hashtable = kvmalloc(hash_size, GFP_KERNEL);
	if (!my_hashtable) {
		return -ENOMEM;
	}
	for (unsigned long i = 0; i < nr_buckets; i++) {
		INIT_HLIST_HEAD(&my_hashtable[i]);
	}
	pr_info("initialized hasmap with %lu entries\n", nr_buckets);
	return 0;
}

int hash_exit(void)
{
	kvfree(my_hashtable);
	return 0;
}
