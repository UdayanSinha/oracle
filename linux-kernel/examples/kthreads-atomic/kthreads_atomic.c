/*
 * kthreads_atomic.c: Example of kthreads using atomic_t
 */

#include <linux/module.h>	/* module_*(), MODULE_*() */
#include <linux/init.h>		/* __init, __exit */
#include <linux/printk.h>	/* printk() and utils */
#include <linux/err.h>		/* Error-checking APIs */
#include <linux/kthread.h>	/* kthread APIs */
#include <linux/sched.h>	/* scheduler APIs */
#include <linux/jiffies.h>	/* jiffies and utils */
#include <linux/types.h>	/* data types */
#include <linux/limits.h>	/* limits for data types */
#include <linux/atomic.h>	/* atomic-type APIs */

#define KT0_NAME	"kthread-example-0"
#define KT1_NAME	"kthread-example-1"

static struct task_struct *kt[2] = {NULL};
static atomic_t atomic_var = ATOMIC_INIT(0);	/* writer = kt0, reader = kt1 */

int kt0_func(void *data)
{
	int atomic_temp = 0;
	unsigned long delay = msecs_to_jiffies(1000);

	pr_info("[%s] Starting up...\n", KT0_NAME);

	while (!kthread_should_stop()) {
		/* write atomic variable */
		if (atomic_temp == S32_MAX) {
			atomic_set(&atomic_var, 0);
			atomic_temp = 0;
		} else {
			atomic_temp = atomic_inc_return(&atomic_var);
		}

		pr_debug("[%s] Atomic variable write: %d\n",
				KT0_NAME, atomic_read(&atomic_var));

		schedule_timeout_interruptible(delay);
	}

	return 0;
}

int kt1_func(void *data)
{
	unsigned long delay = msecs_to_jiffies(750);

	pr_info("[%s] Starting up...\n", KT1_NAME);

	while (!kthread_should_stop()) {
		/* read atomic variable */
		pr_debug("[%s] Atomic variable read: %d\n",
				KT1_NAME, atomic_read(&atomic_var));

		schedule_timeout_interruptible(delay);
	}

	return 0;
}

static int __init kthreads_atomic_init(void)
{
	int ret = 0;

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

	return 0;

err:
	if (!IS_ERR_OR_NULL(kt[0]))
		(void) kthread_stop(kt[0]);

	if (!IS_ERR_OR_NULL(kt[1]))
		(void) kthread_stop(kt[1]);

	return ret;
}

static void __exit kthreads_atomic_exit(void)
{
	int ret = 0;

	ret = kthread_stop(kt[0]);
	if (ret)
		pr_err("kthread_stop() failed for %s, %d\n", KT0_NAME, ret);

	ret = kthread_stop(kt[1]);
	if (ret)
		pr_err("kthread_stop() failed for %s, %d\n", KT1_NAME, ret);

	return;
}

module_init(kthreads_atomic_init);
module_exit(kthreads_atomic_exit);

MODULE_AUTHOR("Udayan Prabir Sinha");
MODULE_LICENSE("Dual MIT/GPL");
