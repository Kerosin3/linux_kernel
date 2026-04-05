#ifndef _KERNEL_SYNC_DEMO_H
#define _KERNEL_SYNC_DEMO_H

#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/ktime.h>
#include <linux/atomic.h>
#include <linux/sched.h>

#define KSD_THREADS_MIN 1u
#define KSD_THREADS_MAX 64u
#define KSD_ITER_MIN 1u
#define KSD_ITER_MAX 1000000u

extern unsigned int param_lock_type;

enum ksd_lock_type {
	KSD_LOCK_SPINLOCK = 0,
	KSD_LOCK_MUTEX = 1,
	KSD_LOCK_SEMAPHORE = 2,
	KSD_LOCK_MAX = KSD_LOCK_SEMAPHORE,
};

struct sync_ctx {
	unsigned int num_threads;
	unsigned int iterations;
	unsigned int lock_type;

	// shared resource
	long long shared_counter;

	spinlock_t slock;
	struct mutex mlock;
	struct semaphore sem;

	ktime_t total_wait_time;
	atomic_t contention_count;

	struct task_struct **threads;
	struct worker_args *worker_args;
	atomic_t threads_done;
	int last_run_result;

	// runtime reconf control
	struct mutex ctrl_lock;
	atomic_t test_running;
};

extern struct sync_ctx *g_ctx;
extern const char *const ksd_lock_names[];

int ksd_lock_reinit(struct sync_ctx *ctx, unsigned int new_type);

#endif
