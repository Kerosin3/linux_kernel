#include "kfifo.h"

int fifo_init(int size)
{
	int ret;
	kbuffer = (void*)__get_free_pages(GFP_KERNEL, BUFFER_ORDER);
	if (!kbuffer) {
		pr_err("%s: error getting memory\n", KBUILD_MODNAME);
        return -ENOMEM;
    }
	/* set element size equals one int */
	ret = __kfifo_init(&my_fifo.kfifo, kbuffer, BUFFER_SIZE, sizeof(int));
    if (ret) {
		pr_err("%s: failed to initialize kfifo\n", KBUILD_MODNAME);
        __free_pages(virt_to_page(kbuffer), BUFFER_ORDER);
        return ret;
    }
	unsigned int fifo_capacity_elements = BUFFER_SIZE / sizeof(int);
	pr_debug("%s: memory for kfifo was allocated with capacity %u\n", KBUILD_MODNAME, fifo_capacity_elements);
	return 0;
}


void fifo_cleanup(void)
{
	if (kbuffer) {
        __free_pages(virt_to_page(kbuffer), BUFFER_ORDER);
		pr_debug("%s: memory for kfifo was feed up!\n", KBUILD_MODNAME);
    }
}


int fifo_enqueue(int val){
	if (kfifo_put(&my_fifo, val)) {
        printk(KERN_INFO "Added value %d to KFIFO\n", val);
    } else {
        printk(KERN_INFO "KFIFO is full, cannot add\n");
		return -FIFO_INVALID;
    }
	return FIFO_OK;
}
