#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/atomic.h>
#include <linux/ktime.h>
#include <linux/string.h>

#include "kernel_sync_demo.h"
#include "work.h"

static int ksd_lock_type_set(const char *val, const struct kernel_param *kp)
{
	unsigned int new_type;
	int ret;

	ret = kstrtouint(val, 0, &new_type);
	if (ret)
		return ret;

	if (new_type > KSD_LOCK_MAX)
		return -EINVAL;

	// first time module load (no global context)
	if (!g_ctx) {
		*(unsigned int *)kp->arg = new_type;
		return 0;
	}

	// check test running
	mutex_lock(&g_ctx->ctrl_lock);

	if (atomic_read(&g_ctx->test_running)) {
		mutex_unlock(&g_ctx->ctrl_lock);
		return -EBUSY;
	}

	// reinit locktype
	ret = ksd_lock_reinit(g_ctx, new_type);
	if (!ret)
		pr_info("kernel_sync_demo: lock_type → %s\n",
			ksd_lock_names[new_type]);

	mutex_unlock(&g_ctx->ctrl_lock);
	return ret;
}

static int ksd_lock_type_get(char *buf, const struct kernel_param *kp)
{
	unsigned int type = g_ctx ? g_ctx->lock_type : *(unsigned int *)kp->arg;
	return scnprintf(buf, PAGE_SIZE, "%u\n", type);
}

static const struct kernel_param_ops ksd_ops_lock_type = {
	.set = ksd_lock_type_set,
	.get = ksd_lock_type_get,
};

module_param_cb(lock_type, &ksd_ops_lock_type, &param_lock_type, 0644);
MODULE_PARM_DESC(lock_type, "0=spinlock 1=mutex 2=semaphore (default: 0)");

static int ksd_run_set(const char *val, const struct kernel_param *kp)
{
	unsigned int trigger;
	int ret;

	ret = kstrtouint(val, 0, &trigger);
	if (ret || trigger != 1)
		return ret ? ret : -EINVAL;

	// exclusive test lock
	mutex_lock(&g_ctx->ctrl_lock);

	if (atomic_read(&g_ctx->test_running)) {
		mutex_unlock(&g_ctx->ctrl_lock);
		// test is already running
		return -EBUSY;
	}

	mutex_unlock(&g_ctx->ctrl_lock);

	pr_info("kernel_sync_demo: run triggered (threads=%u iter=%u lock=%s)\n",
		g_ctx->num_threads, g_ctx->iterations,
		ksd_lock_names[g_ctx->lock_type]);
	// run test
	ksd_threads_start(g_ctx);

	return 0;
}

static const struct kernel_param_ops ksd_ops_run = {
	.set = ksd_run_set,
	.get = NULL,
};
module_param_cb(run, &ksd_ops_run, NULL, 0200);
MODULE_PARM_DESC(run, "echo 1 to runtest");

static int ksd_result_get(char *buf, const struct kernel_param *kp)
{
	if (atomic_read(&g_ctx->test_running))
		return scnprintf(buf, PAGE_SIZE, "test is running\n");

	if (g_ctx->last_run_result == 0)
		return scnprintf(buf, PAGE_SIZE, "ok  shared_counter=%lld\n",
				 g_ctx->shared_counter);

	return scnprintf(buf, PAGE_SIZE, "error=%d\n", g_ctx->last_run_result);
}

static const struct kernel_param_ops ksd_ops_result = {
	.set = NULL,
	.get = ksd_result_get,
};
module_param_cb(result, &ksd_ops_result, NULL, 0444);
MODULE_PARM_DESC(result, "result of last test");

static int ksd_stats_get(char *buf, const struct kernel_param *kp)
{
	unsigned int cnt;
	s64 wait_ns;

	if (atomic_read(&g_ctx->test_running))
		return scnprintf(buf, PAGE_SIZE, "test is running\n");

	cnt = atomic_read(&g_ctx->contention_count);
	wait_ns = ktime_to_ns(g_ctx->total_wait_time);

	return scnprintf(buf, PAGE_SIZE,
			 "contention_count=%u\n"
			 "total_wait_us=%lld\n"
			 "avg_wait_ns=%lld\n", // NS
			 cnt,
			 wait_ns / 1000, // US
			 cnt ? wait_ns / cnt : 0LL); // contentions may be 0...?
}

static const struct kernel_param_ops ksd_ops_stats = {
	.set = NULL,
	.get = ksd_stats_get,
};
module_param_cb(stats, &ksd_ops_stats, NULL, 0444);
MODULE_PARM_DESC(stats, "test statistics");

static int ksd_reset_set(const char *val, const struct kernel_param *kp)
{
	unsigned int trigger;
	int ret;

	ret = kstrtouint(val, 0, &trigger);
	if (ret || trigger != 1)
		return ret ? ret : -EINVAL;

	mutex_lock(&g_ctx->ctrl_lock);

	// check if test is running
	if (atomic_read(&g_ctx->test_running)) {
		mutex_unlock(&g_ctx->ctrl_lock);
		return -EBUSY;
	}

	// reset all statistics
	g_ctx->shared_counter = 0LL;
	g_ctx->total_wait_time = ktime_set(0, 0);
	atomic_set(&g_ctx->contention_count, 0);
	g_ctx->last_run_result = 0;
	atomic_set(&g_ctx->threads_done, 0);

	pr_info("kernel_sync_demo: reset done\n");

	mutex_unlock(&g_ctx->ctrl_lock);
	return 0;
}

static const struct kernel_param_ops ksd_ops_reset = {
	.set = ksd_reset_set,
	.get = NULL,
};
module_param_cb(reset, &ksd_ops_reset, NULL, 0200);
MODULE_PARM_DESC(reset, "resets all stats and counters");
