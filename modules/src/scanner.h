#ifndef SCANNER_H
#define SCANNER_H

#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/jhash.h>

#include "hashmap.h"

int scan_directory(const char *path);

struct dir_ctx {
	struct dir_context ctx;
	int error;
};

bool dir_analysis_cb(struct dir_context *ctx, const char *name, int namelen,
		     loff_t offset, u64 ino, unsigned int d_type);

#endif
