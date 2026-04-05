#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/ktime.h>
#include <linux/atomic.h>
#include <linux/string.h>
#include <linux/completion.h>

#include "kernel_sync_demo.h"
#include "work.h"

// global context struct
struct sync_ctx *g_ctx;

const char *const ksd_lock_names[] = {
	[KSD_LOCK_SPINLOCK] = "spinlock",
	[KSD_LOCK_MUTEX] = "mutex",
	[KSD_LOCK_SEMAPHORE] = "semaphore",
};

static void ksd_lock_destroy(struct sync_ctx *ctx)
{
	switch (ctx->lock_type) {
	case KSD_LOCK_SPINLOCK:
		memset(&ctx->slock, 0, sizeof(ctx->slock));
		break;
	case KSD_LOCK_MUTEX:
		mutex_destroy(&ctx->mlock);
		memset(&ctx->mlock, 0, sizeof(ctx->mlock));
		break;
	case KSD_LOCK_SEMAPHORE:
		memset(&ctx->sem, 0, sizeof(ctx->sem));
		break;
	}
}

// init lock primitives
static int ksd_lock_init(struct sync_ctx *ctx)
{
	switch (ctx->lock_type) {
	case KSD_LOCK_SPINLOCK:
		spin_lock_init(&ctx->slock);
		return 0;
	case KSD_LOCK_MUTEX:
		mutex_init(&ctx->mlock);
		return 0;
	case KSD_LOCK_SEMAPHORE:
		sema_init(&ctx->sem, 1);
		return 0;
	default:
		pr_err("unknown lock_type %u\n", ctx->lock_type);
		return -EINVAL;
	}
}

// reinit locktype in runtime (check ctrl logic)
int ksd_lock_reinit(struct sync_ctx *ctx, unsigned int new_type)
{
	if (new_type > KSD_LOCK_MAX)
		return -EINVAL;
	if (new_type == ctx->lock_type)
		return 0;

	ksd_lock_destroy(ctx);
	ctx->lock_type = new_type;
	return ksd_lock_init(ctx);
}

// default param values
static unsigned int param_num_threads = 4;
static unsigned int param_iterations = 1000;
unsigned int param_lock_type = KSD_LOCK_SPINLOCK;

static struct sync_ctx *__init ksd_ctx_alloc(void)
{
	struct sync_ctx *ctx;
	int ret;

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return ERR_PTR(-ENOMEM);

	// set params to ctx
	ctx->num_threads = param_num_threads;
	ctx->iterations = param_iterations;
	ctx->lock_type = param_lock_type;

	// alloc handlers
	ctx->threads =
		kcalloc(ctx->num_threads, sizeof(*ctx->threads), GFP_KERNEL);
	if (!ctx->threads) {
		kfree(ctx);
		return ERR_PTR(-ENOMEM);
	}

	// alloc threads
	ctx->worker_args = kcalloc(ctx->num_threads, sizeof(*ctx->worker_args),
				   GFP_KERNEL);
	if (!ctx->worker_args) {
		kfree(ctx->threads);
		kfree(ctx);
		return ERR_PTR(-ENOMEM);
	}

	ctx->total_wait_time = ktime_set(0, 0);
	atomic_set(&ctx->threads_done, 0);
	atomic_set(&ctx->test_running, 0);
	init_completion(&ctx->threads_start);

	// init ctrl mutex
	mutex_init(&ctx->ctrl_lock);

	// init first lock primitive
	ret = ksd_lock_init(ctx);
	if (ret) {
		mutex_destroy(&ctx->ctrl_lock);
		kfree(ctx->worker_args);
		kfree(ctx->threads);
		kfree(ctx);
		return ERR_PTR(ret);
	}

	return ctx;
}

static void ksd_ctx_free(struct sync_ctx *ctx)
{
	if (!ctx || IS_ERR(ctx))
		return;

	kfree(ctx->worker_args);
	ctx->worker_args = NULL;

	kfree(ctx->threads);
	ctx->threads = NULL;

	ksd_lock_destroy(ctx);

	mutex_destroy(&ctx->ctrl_lock);

	kfree(ctx);
}

module_param_named(num_threads, param_num_threads, uint, 0444);
MODULE_PARM_DESC(num_threads, "Worker kthreads [1..64] (default: 4)");

module_param_named(iterations, param_iterations, uint, 0444);
MODULE_PARM_DESC(iterations,
		 "Iterations per thread [1..1000000] (default: 1000)");

static int __init ksd_params_validate(void)
{
	int ret = 0;

	if (param_num_threads < KSD_THREADS_MIN ||
	    param_num_threads > KSD_THREADS_MAX) {
		pr_err("kernel_sync_demo: num_threads=%u out of range [%u..%u]\n",
		       param_num_threads, KSD_THREADS_MIN, KSD_THREADS_MAX);
		ret = -EINVAL;
	}
	if (param_iterations < KSD_ITER_MIN ||
	    param_iterations > KSD_ITER_MAX) {
		pr_err("kernel_sync_demo: iterations=%u out of range [%u..%u]\n",
		       param_iterations, KSD_ITER_MIN, KSD_ITER_MAX);
		ret = -EINVAL;
	}
	// 0,1,2
	if (param_lock_type > KSD_LOCK_MAX) {
		pr_err("kernel_sync_demo: lock_type=%u unknown (max %u)\n",
		       param_lock_type, KSD_LOCK_MAX);
		ret = -EINVAL;
	}

	return ret;
}

static int __init ksd_init(void)
{
	int ret;

	pr_info("kernel_sync_demo: loading with parameters: threads=%u  iter=%u  lock=%s\n",
		param_num_threads, param_iterations,
		ksd_lock_names[param_lock_type]);

	ret = ksd_params_validate();
	if (ret)
		return ret;

	g_ctx = ksd_ctx_alloc();
	if (IS_ERR(g_ctx)) {
		ret = PTR_ERR(g_ctx);
		g_ctx = NULL;
		return ret;
	}

	pr_info("kernel_sync_demo: loaded\n");
	return 0;
}

static void __exit ksd_exit(void)
{
	if (g_ctx) {
		ksd_threads_stop(g_ctx);
		ksd_ctx_free(g_ctx);
		g_ctx = NULL;
	}
	pr_info("kernel_sync_demo: unloaded\n");
}

module_init(ksd_init);
module_exit(ksd_exit);

MODULE_AUTHOR("AlexVd");
MODULE_DESCRIPTION("Kernel synch primitives demo");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("0.1.0");


