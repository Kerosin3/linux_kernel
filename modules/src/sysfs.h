#ifndef SYSFS_H
#define SYSFS_H

#include <linux/module.h>
#include <linux/moduleparam.h>
#include "kfifo.h"

/* enqueue */
extern int enqueue_val;

extern const struct kernel_param_ops enqueue_ops;

/* denqueue */
extern int dequeue_val;

extern const struct kernel_param_ops dequeue_ops;

/* is_full */
extern int is_full_val;

extern const struct kernel_param_ops if_full_ops;

/* is_empty */
extern int if_empty_val;

extern const struct kernel_param_ops if_empty_ops;

/* current size */
extern unsigned size_val;

extern const struct kernel_param_ops clen_ops;

/* available */
extern unsigned available_val;

extern const struct kernel_param_ops available_ops;

/* peek */
extern int peek_val;

extern const struct kernel_param_ops peek_ops;

#endif
