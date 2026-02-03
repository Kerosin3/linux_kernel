#include "scanner.h"

bool dir_analysis_cb(struct dir_context *ctx, const char *name, int namelen,
		     loff_t offset, u64 ino, unsigned int d_type)
{
	struct dir_ctx *m_ctx = container_of(ctx, struct dir_ctx, ctx);
	struct hashmap_entry *entry;
	u32 hash_val;
	unsigned bkt;

	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
		return 1;
	}
	// only reg files
	if (d_type != DT_REG) {
		return 1;
	}
	// check if entry exist
	hash_val = jhash(name, namelen, 0);
	bkt = hash_32(hash_val, hash_bits);

	struct hashmap_entry *existing;
	hlist_for_each_entry(existing, &my_hashtable[bkt], node) {
		// not add existing files
		if (strcmp(existing->filename, name) == 0) {
			return 1;
		}
	}

	// alloca cache for entry
	entry = kmem_cache_alloc(g_entry_cache, GFP_KERNEL);
	if (!entry) {
		m_ctx->error = -ENOMEM;
		return -ENOMEM;
	}

	// copy filename
	entry->filename = kstrndup(name, namelen, GFP_KERNEL);
	if (!entry->filename) {
		kmem_cache_free(g_entry_cache, entry);
		m_ctx->error = -ENOMEM;
		return -ENOMEM;
	}

	add_to_hashmap(entry);

	return 1;
}

int scan_directory(const char *path)
{
	struct file *filp;
	struct dir_ctx m_ctx = { .ctx.actor = dir_analysis_cb, .error = 0 };
	int ret;
	filp = filp_open(path, O_RDONLY | O_DIRECTORY, 0);
	if (IS_ERR(filp)) {
		return PTR_ERR(filp);
	}

	ret = iterate_dir(filp, &m_ctx.ctx);
	if (ret < 0) {
		goto out;
	}

	if (m_ctx.error < 0) {
		ret = m_ctx.error;
	}

out:
	filp_close(filp, NULL);
	return ret;
	return 0;
}
