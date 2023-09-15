/*
 * kthreads_spinlock.c: Example of kthreads using spinlock_t
 */

#include <linux/module.h>	/* module_*(), MODULE_*() */
#include <linux/init.h>		/* __init, __exit */
#include <linux/printk.h>	/* printk() and utils */
#include <linux/err.h>		/* Error-checking APIs */
#include <linux/kthread.h>	/* kthread APIs */
#include <linux/sched.h>	/* scheduler APIs */
#include <linux/jiffies.h>	/* jiffies and utils */
#include <linux/types.h>	/* data types */
#include <linux/spinlock.h>	/* spin-lock APIs */

#define KT0_NAME	"kt-example-0"
#define KT1_NAME	"kt-example-1"
#define KT2_NAME	"kt-example-2"

#define SHARED_INIT	2
#define SHARED_LIMIT	100

static struct task_struct *kt[3] = {NULL};
spinlock_t lock;			/* see DEFINE_SPINLOCK() as well */
static u64 shared = SHARED_INIT;	/* shared-access resource */

int kt0_func(void *data)
{
	unsigned long delay = msecs_to_jiffies(1000);
	pr_info("[%s] Starting up...\n", KT0_NAME);

	while (!kthread_should_stop()) {
		/* access shared resource */
		if (spin_trylock(&lock)) {
			pr_debug("[%s] Shared resource initial value: 0x%llu\n",
				KT0_NAME, shared);

			shared *= 2;
			pr_debug("[%s] Shared resource final value: 0x%llu\n",
					KT0_NAME, shared);

			spin_unlock(&lock);
		}

		schedule_timeout_interruptible(delay);
	}

	return 0;
}

int kt1_func(void *data)
{
	unsigned long delay = msecs_to_jiffies(750);
	pr_info("[%s] Starting up...\n", KT1_NAME);

	while (!kthread_should_stop()) {
		/* access shared resource */
		if (spin_trylock(&lock)) {
			pr_debug("[%s] Shared resource initial value: 0x%llu\n",
					KT1_NAME, shared);

			shared *= shared;
			pr_debug("[%s] Shared resource final value: 0x%llu\n",
					KT1_NAME, shared);

			spin_unlock(&lock);
		}
		schedule_timeout_interruptible(delay);
	}

	return 0;
}

int kt2_func(void *data)
{
	unsigned long delay = msecs_to_jiffies(250);
	pr_info("[%s] Starting up...\n", KT2_NAME);

	while (!kthread_should_stop()) {
		/* access shared resource */
		spin_lock(&lock);

		if (shared >= SHARED_LIMIT) {
			shared = SHARED_INIT;
			pr_debug("[%s] Shared resource value reset: 0x%llu\n",
				KT2_NAME, shared);
		}

		spin_unlock(&lock);
		schedule_timeout_interruptible(delay);
	}

	return 0;
}

static int __init kthreads_spinlock_init(void)
{
	int ret = 0;

	spin_lock_init(&lock);

	kt[0] = kthread_run(kt0_func, NULL, KT0_NAME);
	if (IS_ERR_OR_NULL(kt[0])) {
		ret = PTR_ERR(kt[0]);
		pr_err("kthread_run() failed for %s, %d\n", KT0_NAME, ret);
		goto err;
	}

	kt[1] = kthread_run(kt1_func, NULL, KT1_NAME);
	if (IS_ERR_OR_NULL(kt[1])) {
		ret = PTR_ERR(kt[1]);
		pr_err("kthread_run() failed for %s, %d\n", KT1_NAME, ret);
		goto err;
	}

	kt[2] = kthread_run(kt2_func, NULL, KT2_NAME);
	if (IS_ERR_OR_NULL(kt[2])) {
		ret = PTR_ERR(kt[2]);
		pr_err("kthread_run() failed for %s, %d\n", KT2_NAME, ret);
		goto err;
	}

	return 0;

err:
	if (!IS_ERR_OR_NULL(kt[0]))
		(void) kthread_stop(kt[0]);

	if (!IS_ERR_OR_NULL(kt[1]))
		(void) kthread_stop(kt[1]);

	if (!IS_ERR_OR_NULL(kt[2]))
		(void) kthread_stop(kt[2]);

	return ret;
}

static void __exit kthreads_spinlock_exit(void)
{
	int ret = 0;

	ret = kthread_stop(kt[0]);
	if (ret)
		pr_err("kthread_stop() failed for %s, %d\n", KT0_NAME, ret);

	ret = kthread_stop(kt[1]);
	if (ret)
		pr_err("kthread_stop() failed for %s, %d\n", KT1_NAME, ret);

	ret = kthread_stop(kt[2]);
	if (ret)
		pr_err("kthread_stop() failed for %s, %d\n", KT2_NAME, ret);

	return;
}

module_init(kthreads_spinlock_init);
module_exit(kthreads_spinlock_exit);

MODULE_AUTHOR("Udayan Prabir Sinha");
MODULE_LICENSE("Dual MIT/GPL");
