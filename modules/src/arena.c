#include "arena.h"

struct allocator_data *alloc_ctx = NULL;

int init_arena(unsigned blocksize, unsigned n_blocks)
{
	// allocate structure
	alloc_ctx = kzalloc(sizeof(struct allocator_data), GFP_KERNEL);
	if (!alloc_ctx) {
		pr_err("cannot alocate structure!\n");
		return -ENOMEM;
	}
	alloc_ctx->blocksize = blocksize;
	alloc_ctx->allocated_blocks = 0;
	// use x2 block by default
	alloc_ctx->max_blocks = n_blocks * 2;
	alloc_ctx->initial_block_limit = n_blocks;

	// allocate array of pointers
	alloc_ctx->base_ptr = kmalloc_array(
		alloc_ctx->max_blocks, sizeof(void *), GFP_KERNEL | __GFP_ZERO);
	if (!alloc_ctx->base_ptr) {
		kfree(alloc_ctx);
		pr_err("cannot allocate arrays!\n");
		return -ENOMEM;
	}

	// allocate bitmap
	alloc_ctx->bitmap = kzalloc(BITS_TO_LONGS(alloc_ctx->max_blocks) *
					    sizeof(unsigned long),
				    GFP_KERNEL);
	if (!alloc_ctx->bitmap) {
		kfree(alloc_ctx->base_ptr);
		kfree(alloc_ctx);
		pr_err("cannot allocate bitmap!\n");
		return -ENOMEM;
	}
	spin_lock_init(&alloc_ctx->lock);
	return 0;
}

int alloc_a_block(unsigned blocknumber)
{
	void *mem = NULL;
	int bitisset = 0;

	if (blocknumber > alloc_ctx->max_blocks)
		return -EINVAL;

	spin_lock(&alloc_ctx->lock);

	bitisset = test_bit(blocknumber, alloc_ctx->bitmap);

	if (blocknumber >= alloc_ctx->initial_block_limit)
		alloc_ctx->initial_block_limit = alloc_ctx->max_blocks;

	// block is occupied
	if (bitisset) {
		spin_unlock(&alloc_ctx->lock);
		return -EADDRINUSE;
	}
	// block is not occupied
	// allocate memory
	mem = vzalloc(alloc_ctx->blocksize);
	if (!mem) {
		spin_unlock(&alloc_ctx->lock);
		return -ENOMEM;
	}
	// test pointer just in case
	if (alloc_ctx->base_ptr[blocknumber]) {
		spin_unlock(&alloc_ctx->lock);
		return -EADDRINUSE;
	}
	set_bit(blocknumber, alloc_ctx->bitmap);
	alloc_ctx->base_ptr[blocknumber] = mem;
	alloc_ctx->allocated_blocks++;
	spin_unlock(&alloc_ctx->lock);
	pr_info("allocated block[%u] at [0x%px]", blocknumber, mem);
	// return allocated block number
	return blocknumber;
}

int alloc_block(void)
{
	unsigned long freebit;
	void *mem = NULL;

	// find
	spin_lock(&alloc_ctx->lock);

	// search to initial limit
	freebit = find_first_zero_bit(alloc_ctx->bitmap,
				      alloc_ctx->initial_block_limit);

	// not possible to find in initial limit
	if (freebit >= alloc_ctx->initial_block_limit) {
		if (alloc_ctx->initial_block_limit == alloc_ctx->max_blocks)
			return -ENOMEM;
		// extend limit
		alloc_ctx->initial_block_limit = alloc_ctx->max_blocks;
	}
	// allocate in limit
	if (freebit < alloc_ctx->initial_block_limit) {
		// allocate memory
		mem = vzalloc(alloc_ctx->blocksize);
	}
	if (!mem) {
		spin_unlock(&alloc_ctx->lock);
		return -ENOMEM;
	}
	set_bit(freebit, alloc_ctx->bitmap);
	alloc_ctx->allocated_blocks++;
	alloc_ctx->base_ptr[freebit] = mem;
	//set this bit in bitmap
	spin_unlock(&alloc_ctx->lock);
	pr_info("allocated block[%lu] at [0x%px]", freebit, mem);
	// return allocated block number
	return freebit;
}

int free_a_block(unsigned block_idx)
{
	void *ptr = NULL;

	spin_lock(&alloc_ctx->lock);

	if (test_bit(block_idx, alloc_ctx->bitmap)) {
		ptr = alloc_ctx->base_ptr[block_idx];
		alloc_ctx->base_ptr[block_idx] = NULL;
		clear_bit(block_idx, alloc_ctx->bitmap);
	}
	spin_unlock(&alloc_ctx->lock);
	if (ptr) {
		vfree(ptr);
		ptr = NULL;
		return 0;
	}
	// no such block to free
	return -1;
}

int free_some_block(void)
{
	unsigned a_bit;
	void *ptr = NULL;

	spin_lock(&alloc_ctx->lock);

	a_bit = find_first_bit(alloc_ctx->bitmap,
			       alloc_ctx->initial_block_limit);
	if (a_bit >= alloc_ctx->initial_block_limit) {
		// all blocks are free
		spin_unlock(&alloc_ctx->lock);
		return -1;
	}
	ptr = alloc_ctx->base_ptr[a_bit];
	alloc_ctx->base_ptr[a_bit] = NULL;
	clear_bit(a_bit, alloc_ctx->bitmap);
	alloc_ctx->allocated_blocks--;

	spin_unlock(&alloc_ctx->lock);

	if (ptr) {
		vfree(ptr);
		return a_bit;
	}

	return -1;
}

//bitmap_weight

void destroy_arena(void)
{
	unsigned i = 0;
	void **base_ptr_copy;
	unsigned long *bitmap_copy;
	unsigned max_blocks;

	if (!alloc_ctx)
		return;

	spin_lock(&alloc_ctx->lock);

	base_ptr_copy = alloc_ctx->base_ptr;
	bitmap_copy = alloc_ctx->bitmap;
	max_blocks = alloc_ctx->max_blocks;

	alloc_ctx->base_ptr = NULL;
	alloc_ctx->bitmap = NULL;

	spin_unlock(&alloc_ctx->lock);

	if (base_ptr_copy && bitmap_copy) {
		for (i = 0; i < max_blocks; i++) {
			if (test_bit(i, bitmap_copy) &&
			    base_ptr_copy[i] != NULL) {
				vfree(base_ptr_copy[i]);
			}
		}
	}
	kfree(base_ptr_copy);
	kfree(bitmap_copy);
	kfree(alloc_ctx);
	alloc_ctx = NULL;
}

unsigned get_number_of_allocated(void)
{
	return bitmap_weight(alloc_ctx->bitmap,
			     BITS_TO_LONGS(alloc_ctx->max_blocks));
}

void printstat(void)
{
	unsigned max_run = 0;
	unsigned current_run = 0;
	unsigned bit = 0;
	unsigned nbits = alloc_ctx->initial_block_limit;

	spin_lock(&alloc_ctx->lock);
	while (bit < nbits) {
		bit = find_next_bit(alloc_ctx->bitmap, nbits, bit);

		if (bit >= nbits)
			break;

		current_run = 0;
		while (bit < nbits && test_bit(bit, alloc_ctx->bitmap)) {
			current_run++;
			bit++;
		}

		// choose max continuous bits
		if (current_run > max_run) {
			max_run = current_run;
		}
	}
	spin_unlock(&alloc_ctx->lock);
	pr_info("max number of adjacent blocks is %u, total amount of allocated blocks %u, allocator capacity [%u] blocks\n",
		max_run, get_number_of_allocated(),
		alloc_ctx->initial_block_limit);
}
