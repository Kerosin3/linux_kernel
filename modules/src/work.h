/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef WORK_H
#define WORK_H

#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/ktime.h>
#include <linux/atomic.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/atomic.h>
#include <linux/err.h>

#include "kernel_sync_demo.h"

#define THRESHHOLD_NS 100UL

struct worker_args {
	struct sync_ctx *ctx;
	unsigned int thread_id;
	ktime_t wait_time; // thread time wait total
};

int ksd_threads_start(struct sync_ctx *ctx);
void ksd_threads_stop(struct sync_ctx *ctx);

int worker_fn_semaphore(void *data);
int worker_fn_spinlock(void *data);
int worker_fn_mutex(void *data);

// worker functions

typedef int (*worker_fn_t)(void *);

static const worker_fn_t worker_fn_table[] = {
	[KSD_LOCK_SPINLOCK] = worker_fn_spinlock,
	[KSD_LOCK_MUTEX] = worker_fn_mutex,
	[KSD_LOCK_SEMAPHORE] = worker_fn_semaphore,
};

#endif /* WORK_H */
