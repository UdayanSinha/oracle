/*
 * kthreads_seqlock.c: Example of kthreads using seqlock_t
 */

#include <linux/module.h>	/* module_*(), MODULE_*() */
#include <linux/init.h>		/* __init, __exit */
#include <linux/printk.h>	/* printk() and utils */
#include <linux/err.h>		/* Error-checking APIs */
#include <linux/kthread.h>	/* kthread APIs */
#include <linux/sched.h>	/* scheduler APIs */
#include <linux/jiffies.h>	/* jiffies and utils */
#include <linux/types.h>	/* data types */
#include <linux/seqlock.h>	/* seqlock APIs */

#define KT0_NAME	"kt-example-0"
#define KT1_NAME	"kt-example-1"
#define KT2_NAME	"kt-example-2"

#define SHARED_INIT	2
#define SHARED_LIMIT	100

static struct task_struct *kt[3] = {NULL};
static seqlock_t lock;
static u64 shared = SHARED_INIT;	/* shared-access resource */

static int kt0_func(void *data)
{
	unsigned long delay = msecs_to_jiffies(1000);
	unsigned long irq_flags = 0;

	pr_info("[%s] Starting up...\n", KT0_NAME);

	while (!kthread_should_stop()) {
		/* access shared resource with write lock + IRQ disable */
		write_seqlock_irqsave(&lock, irq_flags);

		if (shared <= SHARED_LIMIT)
			shared *= 2;
		else
			shared = SHARED_INIT;

		pr_debug("[%s] Shared resource write value: 0x%llu\n",
				KT0_NAME, shared);

		write_sequnlock_irqrestore(&lock, irq_flags);
		schedule_timeout_interruptible(delay);
	}

	return 0;
}

static int kt1_func(void *data)
{
	unsigned long delay = msecs_to_jiffies(1000);

	pr_info("[%s] Starting up...\n", KT1_NAME);

	while (!kthread_should_stop()) {
		/* access shared resource with write lock */
		write_seqlock(&lock);

		if (shared <= SHARED_LIMIT)
			shared *= 2;
		else
			shared = SHARED_INIT;

		pr_debug("[%s] Shared resource write value: 0x%llu\n",
				KT1_NAME, shared);

		write_sequnlock(&lock);
		schedule_timeout_interruptible(delay);
	}

	return 0;
}

static int kt2_func(void *data)
{
	unsigned long delay = msecs_to_jiffies(1000);
	unsigned int seq = 0;

	pr_info("[%s] Starting up...\n", KT2_NAME);

	while (!kthread_should_stop()) {
		/* access shared resource from reader */
		do {
			/*
			 * Reader does not take an actual lock. If the loop
			 * repeats, it means that a writer was holding a lock
			 * while the reader attempted to access shared resource.
			 */
			seq = read_seqbegin(&lock);
			pr_debug("[%s] Shared resource read value: 0x%llu\n",
				KT2_NAME, shared);
		} while (read_seqretry(&lock, seq));


		schedule_timeout_interruptible(delay);
	}

	return 0;
}

static int __init kthreads_seqlock_init(void)
{
	int ret = 0;

	seqlock_init(&lock);

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

static void __exit kthreads_seqlock_exit(void)
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

module_init(kthreads_seqlock_init);
module_exit(kthreads_seqlock_exit);

MODULE_AUTHOR("Udayan Prabir Sinha");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_DESCRIPTION("Seqlock example with kthreads");
