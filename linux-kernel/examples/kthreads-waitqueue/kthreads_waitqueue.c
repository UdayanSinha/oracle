/*
 * kthreads_waitqueue.c: Example of kthreads using wait-queues
 */

#include <linux/module.h>	/* module_*(), MODULE_*() */
#include <linux/init.h>		/* __init, __exit */
#include <linux/printk.h>	/* printk() and utils */
#include <linux/err.h>		/* Error-checking APIs */
#include <linux/errno.h>	/* Return error codes */
#include <linux/kthread.h>	/* kthread APIs */
#include <linux/sched.h>	/* scheduler APIs */
#include <linux/jiffies.h>	/* jiffies and utils */
#include <linux/types.h>	/* data types */
#include <linux/atomic.h>	/* atomic-type APIs */
#include <linux/wait.h>		/* wait-queue APIs */

#define KT0_NAME	"kt-example-0"
#define KT1_NAME	"kt-example-1"

#define ATOMIC_VAR_MAX	0x10

static struct task_struct *kt[2] = {NULL};
static atomic_t atomic_var = ATOMIC_INIT(0);	/* writer = kt0, reader = kt1 */
static wait_queue_head_t wq;	/* see DECLARE_WAIT_QUEUE_HEAD() as well */

int kt0_func(void *data)
{
	unsigned long delay = msecs_to_jiffies(100);
	pr_info("[%s] Starting up...\n", KT0_NAME);

	while (!kthread_should_stop()) {
		/* send wake event */
		if (atomic_read(&atomic_var) >= ATOMIC_VAR_MAX) {
			pr_debug("[%s] Sending wake event, atomic variable: 0x%x\n",
				KT0_NAME, atomic_read(&atomic_var));

			wake_up_interruptible(&wq);
		} else {
			atomic_inc(&atomic_var);
			pr_debug("[%s] Incrementing atomic variable: 0x%x\n",
				KT0_NAME, atomic_read(&atomic_var));
		}

		schedule_timeout_interruptible(delay);
	}

	return 0;
}

int kt1_func(void *data)
{
	int ret = 0;
	pr_info("[%s] Starting up...\n", KT1_NAME);

	while (!kthread_should_stop()) {
		/* sleep until an event occurs */
		ret = wait_event_interruptible(wq,
				(atomic_read(&atomic_var) >= ATOMIC_VAR_MAX));

		if (ret == -ERESTARTSYS) {
			pr_debug("[%s] Wait-on-event interrupted\n", KT1_NAME);
			continue;
		}

		pr_debug("[%s] Received wake event, atomic variable: 0x%x\n",
				KT1_NAME, atomic_read(&atomic_var));

		atomic_set(&atomic_var, 0);
	}

	return 0;
}

static int __init kthreads_waitqueue_init(void)
{
	int ret = 0;

	init_waitqueue_head(&wq);

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

static void __exit kthreads_waitqueue_exit(void)
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

module_init(kthreads_waitqueue_init);
module_exit(kthreads_waitqueue_exit);

MODULE_AUTHOR("Udayan Prabir Sinha");
MODULE_LICENSE("Dual MIT/GPL");
