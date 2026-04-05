#include "work.h"

int worker_fn_mutex(void *data)
{
	struct worker_args *args = data;
	struct sync_ctx *ctx = args->ctx;
	unsigned int i;
	ktime_t t_before, t_after, delta;

	// iterate over
	for (i = 0; i < ctx->iterations; i++) {
		if (kthread_should_stop())
			break;
		t_before = ktime_get();
		// lock mutex
		mutex_lock(&ctx->mlock);
		t_after = ktime_get();
		delta = ktime_sub(t_after, t_before);
		// count contention if happened
		if (ktime_to_ns(delta) > THRESHHOLD_NS) {
			// add delta to waittime
			args->wait_time = ktime_add(args->wait_time, delta);
			atomic_inc(&ctx->contention_count);
		}
		// increment shared counter
		ctx->shared_counter++;
		// unlock
		mutex_unlock(&ctx->mlock);
		// decrement side
		t_before = ktime_get();
		mutex_lock(&ctx->mlock);
		t_after = ktime_get();
		delta = ktime_sub(t_after, t_before);
		if (ktime_to_ns(delta) > THRESHHOLD_NS) {
			args->wait_time = ktime_add(args->wait_time, delta);
			atomic_inc(&ctx->contention_count);
		}
		ctx->shared_counter--;
		mutex_unlock(&ctx->mlock);
	}

	// lock before time addition
	mutex_lock(&ctx->mlock);
	ctx->total_wait_time = ktime_add(ctx->total_wait_time, args->wait_time);
	mutex_unlock(&ctx->mlock);

	// thread done
	atomic_inc(&ctx->threads_done);
	// wait threads_start stops all threads
	while (!kthread_should_stop())
		msleep(50);

	return 0;
}

int worker_fn_spinlock(void *data)
{
	struct worker_args *args = data;
	struct sync_ctx *ctx = args->ctx;
	unsigned int i;
	bool contended;
	ktime_t t_before, t_after, delta;

	for (i = 0; i < ctx->iterations; i++) {
		if (kthread_should_stop())
			break;
		// increment
		contended = false;
		// get time before spin
		t_before = ktime_get();
		// 0 means is locked by another
		while (!spin_trylock(&ctx->slock)) {
			contended = true;
		}
		// get time after getting spin
		t_after = ktime_get();
		ctx->shared_counter++;
		spin_unlock(&ctx->slock);

		// add spin waiting time
		if (contended) {
			delta = ktime_sub(t_after, t_before);
			args->wait_time = ktime_add(args->wait_time, delta);
			atomic_inc(&ctx->contention_count);
		}

		// decrement
		contended = false;
		t_before = ktime_get();
		while (!spin_trylock(&ctx->slock)) {
			contended = true;
		}
		t_after = ktime_get();
		ctx->shared_counter--;
		spin_unlock(&ctx->slock);

		if (contended) {
			delta = ktime_sub(t_after, t_before);
			args->wait_time = ktime_add(args->wait_time, delta);
			atomic_inc(&ctx->contention_count);
		}
	}

	// add total wait time
	spin_lock(&ctx->slock);
	ctx->total_wait_time = ktime_add(ctx->total_wait_time, args->wait_time);
	spin_unlock(&ctx->slock);

	atomic_inc(&ctx->threads_done);
	while (!kthread_should_stop())
		msleep(50);

	return 0;
}

int worker_fn_semaphore(void *data)
{
	struct worker_args *args = data;
	struct sync_ctx *ctx = args->ctx;
	unsigned int i;
	ktime_t t_before, t_after, delta;

	for (i = 0; i < ctx->iterations; i++) {
		if (kthread_should_stop())
			break;
		// increment
		// get time before aquiring sema
		t_before = ktime_get();
		// sheduler put to sleep if aquired
		down(&ctx->sem);
		// get time we aquire
		t_after = ktime_get();
		ctx->shared_counter++;
		// release sema
		up(&ctx->sem);

		delta = ktime_sub(t_after, t_before);
		if (ktime_to_ns(delta) > THRESHHOLD_NS) {
			args->wait_time = ktime_add(args->wait_time, delta);
			atomic_inc(&ctx->contention_count);
		}

		// dec
		t_before = ktime_get();
		down(&ctx->sem);
		t_after = ktime_get();
		ctx->shared_counter--;
		up(&ctx->sem);

		delta = ktime_sub(t_after, t_before);
		if (ktime_to_ns(delta) > THRESHHOLD_NS) {
			args->wait_time = ktime_add(args->wait_time, delta);
			atomic_inc(&ctx->contention_count);
		}
	}

	down(&ctx->sem);
	ctx->total_wait_time = ktime_add(ctx->total_wait_time, args->wait_time);
	up(&ctx->sem);

	atomic_inc(&ctx->threads_done);
	while (!kthread_should_stop())
		msleep(50);
	return 0;
}

int ksd_threads_start(struct sync_ctx *ctx)
{
	unsigned int i;
	int ret = 0;

	atomic_set(&ctx->threads_done, 0);
	atomic_set(&ctx->test_running, 1);

	for (i = 0; i < ctx->num_threads; i++) {
		ctx->worker_args[i].ctx = ctx;
		ctx->worker_args[i].thread_id = i;
		// get current time
		ctx->worker_args[i].wait_time = ktime_set(0, 0);

		// setup workers
		ctx->threads[i] = kthread_run(worker_fn_table[ctx->lock_type],
					      &ctx->worker_args[i],
					      "ksd_worker/%u", i);
		if (IS_ERR(ctx->threads[i])) {
			ret = PTR_ERR(ctx->threads[i]);
			pr_err("kernel_sync_demo: kthread_run failed at %u (%d)\n",
			       i, ret);
			ctx->threads[i] = NULL;
			ctx->last_run_result = ret;
			goto err_stop;
		}
	}

	// block untill all threads done
	while (atomic_read(&ctx->threads_done) < ctx->num_threads) {
		msleep(100);
	}
	atomic_set(&ctx->test_running, 0);

	// release threads !!
	ksd_threads_stop(ctx);
	return 0;

err_stop:
	ksd_threads_stop(ctx);
	atomic_set(&ctx->test_running, 0);
	return ret;
}

void ksd_threads_stop(struct sync_ctx *ctx)
{
	unsigned int i;

	if (!ctx || !ctx->threads)
		return;

	for (i = 0; i < ctx->num_threads; i++) {
		// ok
		if (ctx->threads[i]) {
			kthread_stop(ctx->threads[i]);
			ctx->threads[i] = NULL;
		}
	}
}
