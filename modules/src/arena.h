#ifndef ARENA_H
#define ARENA_H

#include <linux/slab.h>
#include <linux/vmalloc.h>

struct allocator_data {
	void **base_ptr;
	unsigned blocksize;
	unsigned allocated_blocks;
	unsigned initial_block_limit;
	unsigned max_blocks;
	unsigned long *bitmap;
	spinlock_t lock;
};

extern struct allocator_data *alloc_ctx;

int init_arena(unsigned blocksize, unsigned n_blocks);

int alloc_block(void);

int alloc_a_block(unsigned);

int free_a_block(unsigned n_block);

int free_some_block(void);

void destroy_arena(void);

unsigned get_number_of_allocated(void);

void printstat(void);

#endif
