#include "sorter.h"

static int filename_cmp(const void *a, const void *b)
{
	const char *const *pa = a;
	const char *const *pb = b;
	return strcmp(*pa, *pb);
}

int print_sorted_filenames(char *buf, ssize_t len)
{
	struct hashmap_entry *entry;
	unsigned int bkt;
	size_t count = 0;
	size_t i;
	char **names = NULL;

	for (bkt = 0; bkt < nr_buckets; bkt++) {
		hlist_for_each_entry(entry, &my_hashtable[bkt], node) {
			count++;
		}
	}
	if (count == 0) {
		return -EPERM;
	}

	names = kvmalloc_array(count, sizeof(char *), GFP_KERNEL);
	if (!names) {
		pr_err("Failed to allocate memory for names array\n");
		return -ENOMEM;
	}

	i = 0;
	for (bkt = 0; bkt < nr_buckets; bkt++) {
		hlist_for_each_entry(entry, &my_hashtable[bkt], node) {
			names[i++] = entry->filename;
		}
	}

	sort(names, count, sizeof(char *), filename_cmp, NULL);

	for (i = 0; i < count; i++) {
		ssize_t added =
			scnprintf(buf + len, PAGE_SIZE - len, "%s\n", names[i]);
		len += added;
		if (added == 0 || len >= PAGE_SIZE) {
			break;
		}
	}
	kvfree(names);

	return len;
}
