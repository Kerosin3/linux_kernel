#include "kfifo.h"

my_fifo_t my_fifo;

int fifo_init(size_t kfifosize)
{
	int ret;
	unsigned long buffer_size_bytes;
	unsigned int fifo_capacity_elements;

	if (kfifosize == 0) {
		return -EINVAL;
	}

	buffer_size_bytes = kfifosize * sizeof(int);
	fifo_capacity_elements = buffer_size_bytes / sizeof(int);

	kbuffer = kzalloc(buffer_size_bytes, GFP_KERNEL);
	if (!kbuffer) {
		pr_err("%s %s: error allocating buffer of memory\n",
		       KBUILD_MODNAME, __func__);
		return -ENOMEM;
	}

	ret = kfifo_init(&my_fifo, kbuffer, buffer_size_bytes);
	if (ret) {
		pr_err("%s %s: failed to initialize kfifo struct\n",
		       KBUILD_MODNAME, __func__);
		kfree(kbuffer);
		return ret;
	}

	pr_info("%s: memory for kfifo was allocated (slab-allocator) with capacity of %u elements (%lu bytes)\n",
		KBUILD_MODNAME, fifo_capacity_elements, buffer_size_bytes);
	return 0;
}

void fifo_cleanup(void)
{
	if (kbuffer) {
		kfree(kbuffer);
		kbuffer = NULL;
		pr_info("%s: memory for the module was freed up!\n",
			KBUILD_MODNAME);
	}
}

int fifo_enqueue(int val)
{
	unsigned ret;
	mutex_lock(&fifo_mutex);
	ret = kfifo_in(&my_fifo, &val, 1);
	mutex_unlock(&fifo_mutex);
	if (ret == 1) {
		return FIFO_OK;
	} else if (ret == 0) {
		return -ENOSPC;
	} else {
		pr_err("%s %s: error while enqueue %d\n", KBUILD_MODNAME,
		       __func__, ret);
		return ret;
	}
}

int fifo_dequeue(int *rval)
{
	int ret;
	mutex_lock(&fifo_mutex);
	ret = kfifo_out(&my_fifo, rval, 1);
	mutex_unlock(&fifo_mutex);
	if (ret == 1) {
		return FIFO_OK;
	} else if (ret == 0) {
		return FIFO_EMPTY;
	} else {
		pr_err("%s %s: error while dequeue %d\n", KBUILD_MODNAME,
		       __func__, ret);
		return ret;
	}
}

int fifo_is_empty(void)
{
	return kfifo_is_empty(&my_fifo);
}

int fifo_is_full(void)
{
	return kfifo_is_full(&my_fifo);
}

unsigned fifo_clen(void)
{
	return kfifo_len(&my_fifo);
}

unsigned fifo_available(void)
{
	return kfifo_avail(&my_fifo);
}

int fifo_peek(int *rval)
{
	int ret;
	ret = kfifo_out_peek(&my_fifo, rval, 1);
	if (ret == 1) {
		return FIFO_OK;
	} else if (ret == 0) {
		return FIFO_EMPTY;
	} else {
		return ret;
	}
}

void fifo_clear(void)
{
	kfifo_reset(&my_fifo);
}
