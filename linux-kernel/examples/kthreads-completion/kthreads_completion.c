/*
 * kthreads_completion.c: Example of kthreads using struct completion
 */

#include <linux/module.h>	/* module_*(), MODULE_*() */
#include <linux/init.h>		/* __init, __exit */
#include <linux/printk.h>	/* printk() and utils */
#include <linux/err.h>		/* Error-checking APIs */
#include <linux/kthread.h>	/* kthread APIs */
#include <linux/sched.h>	/* scheduler APIs */
#include <linux/jiffies.h>	/* jiffies and utils */

#define KT0_NAME	"kt-example-0"
#define KT1_NAME	"kt-example-1"

unsigned int data = 0;
static struct task_struct *kt[2] = {NULL};

/*
 * This will declare and initialize a "struct completion".
 * Can also be defined as "struct completion compl" and then initialized later
 * with init_completion().
 */
DECLARE_COMPLETION(compl);

static int kt0_func(void *arg)
{
	unsigned long delay = msecs_to_jiffies(1000);

	pr_info("[%s] Starting up...\n", KT0_NAME);

	while (!kthread_should_stop()) {
		if (data > 100)
			data = 0;
		else
			data++;

		complete(&compl);	/* wake every waiter */
		schedule_timeout_interruptible(delay);
	}

	return 0;
}

static int kt1_func(void *arg)
{
	int ret = 0;
	pr_info("[%s] Starting up...\n", KT1_NAME);

	while (!kthread_should_stop()) {
		ret = wait_for_completion_interruptible(&compl);
		if (ret == -ERESTARTSYS)		/* interruption */
			continue;

		reinit_completion(&compl);
		pr_debug("Reading shared variable: %u\n", data);
	}

	return 0;
}

static int __init kthreads_completion_init(void)
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

static void __exit kthreads_completion_exit(void)
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

module_init(kthreads_completion_init);
module_exit(kthreads_completion_exit);

MODULE_AUTHOR("Udayan Prabir Sinha");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_DESCRIPTION("Completion example with kthreads");
