#include "sysfs.h"

/* enqueue */
int enqueue_val = 0;

static int enqueue(const char *val, const struct kernel_param *kp)
{
	int ret = param_set_int(val, kp);
	if (!ret) {
		ret = fifo_enqueue(enqueue_val);
		if (ret == 0) {
			return FIFO_OK;
		}
	}
	return ret;
}

const struct kernel_param_ops enqueue_ops = {
	.set = enqueue,
	.get = NULL,
};

module_param_cb(enqueue, &enqueue_ops, &enqueue_val, 0644);
MODULE_PARM_DESC(enqueue_val, "Value to enqueue into KFIFO");

/* dequeue */
int dequeue = 0;

static int dequeuef(char *buffer, const struct kernel_param *kp)
{
	int ret;
	ret = fifo_dequeue(&dequeue);
	if (!ret)
		return param_get_int(buffer, kp);
	return ret;
}

const struct kernel_param_ops dequeue_ops = {
	.set = NULL,
	.get = dequeuef,
};

module_param_cb(dequeue, &dequeue_ops, &dequeue, 0444);
MODULE_PARM_DESC(dequeue, "Value denqueue from KFIFO");

/* is_empty */
int is_empty_val = 0;

static int is_empty(char *buffer, const struct kernel_param *kp)
{
	is_empty_val = fifo_is_empty() ? 1 : 0;
	return param_get_int(buffer, kp);
}

const struct kernel_param_ops is_empty_ops = {
	.set = NULL,
	.get = is_empty,
};

module_param_cb(is_empty, &is_empty_ops, &is_empty_val, 0444);
MODULE_PARM_DESC(is_empty_val, "if KFIFO is empty");

/* is_full */
int is_full_val = 0;

static int is_full(char *buffer, const struct kernel_param *kp)
{
	is_full_val = fifo_is_full() ? 1 : 0;
	return param_get_int(buffer, kp);
}

const struct kernel_param_ops is_full_ops = {
	.set = NULL,
	.get = is_full,
};

module_param_cb(is_full, &is_full_ops, &is_full_val, 0444);
MODULE_PARM_DESC(is_full_val, "if KFIFO if full");

/* curren length */
unsigned size_val = 0;

static int clen(char *buffer, const struct kernel_param *kp)
{
	size_val = fifo_clen();
	return param_get_uint(buffer, kp);
}

const struct kernel_param_ops clen_ops = {
	.set = NULL,
	.get = clen,
};

module_param_cb(size, &clen_ops, &size_val, 0444);
MODULE_PARM_DESC(size_val, "current size of KFIFO (elements)");

/* available */
unsigned available_val = 0;

static int available(char *buffer, const struct kernel_param *kp)
{
	available_val = fifo_available();
	return param_get_uint(buffer, kp);
}

const struct kernel_param_ops available_ops = {
	.set = NULL,
	.get = available,
};

module_param_cb(available, &available_ops, &available_val, 0444);
MODULE_PARM_DESC(available_val, "available elements in KFIFO");

/* peek */
int peek_val = 0;

static int peek(char *buffer, const struct kernel_param *kp)
{
	int ret;
	ret = fifo_peek(&peek_val);
	if (!ret) {
		return param_get_int(buffer, kp);
	} else {
		return ret;
	}
}

const struct kernel_param_ops peek_ops = {
	.set = NULL,
	.get = peek,
};

module_param_cb(peek, &peek_ops, &peek_val, 0444);
MODULE_PARM_DESC(peek_val, "peek a value from KFIFO");

/* clear */
int clear_val = 0;

static int clear(const char *val, const struct kernel_param *kp)
{
	int ret = param_set_int(val, kp);
	if (!ret) {
		fifo_clear();
		return 0;
	}
	return ret;
}

const struct kernel_param_ops clear_ops = {
	.set = clear,
	.get = NULL,
};

module_param_cb(clear, &clear_ops, &clear_val, 0644);
MODULE_PARM_DESC(clear_val, "clear KFIFO content");
