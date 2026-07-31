/*
 * kthreads_rcu.c: Example of kthreads using RCU
 */

#include <linux/module.h>	/* module_*(), MODULE_*() */
#include <linux/init.h>		/* __init, __exit */
#include <linux/printk.h>	/* printk() and utils */
#include <linux/err.h>		/* Error-checking APIs */
#include <linux/kthread.h>	/* kthread APIs */
#include <linux/sched.h>	/* scheduler APIs */
#include <linux/jiffies.h>	/* jiffies and utils */
#include <linux/rcupdate.h>	/* RCU utils */

#define WRITER_NAME	"kt-rcu-writer"
#define READER0_NAME	"kt-rcu-reader-0"
#define READER1_NAME	"kt-rcu-reader-1"

static struct task_struct *kt[3] = {NULL};
static unsigned int __rcu *data = NULL;

static int rcu_writer_func(void *arg)
{
	unsigned long delay = msecs_to_jiffies(2000);
	unsigned int *old = NULL;
	unsigned int *new = NULL;
	unsigned int i = 0;

	pr_info("Starting RCU writer...\n");

	while (!kthread_should_stop()) {
		/* write new data */
		new = kzalloc(sizeof(*new), GFP_KERNEL);
		if (IS_ERR_OR_NULL(new)) {
			pr_err("Data buffer allocation failed for RCU writer\n");
			continue;
		}

		*new = i;

		old = rcu_dereference_protected(data, 1);
		rcu_assign_pointer(data, new);
		pr_debug("New data published: %u\n", *new);

		if (i > 100)
			i = 0;
		else
			i++;

		/* synchronize RCU */
		if (old) {
			synchronize_rcu();
			kfree(old);
		}

		schedule_timeout_interruptible(delay);
	}

	return 0;
}

static int rcu_reader_func(void *arg)
{
	unsigned long delay = msecs_to_jiffies(1000);
	unsigned int *read_data = NULL;

	pr_info("Starting RCU reader...\n");

	while (!kthread_should_stop()) {
		/* RCU read-side critical section */
		rcu_read_lock();
		read_data = rcu_dereference(data);
		if (read_data)
			pr_debug("Read data: %u\n", *read_data);

		rcu_read_unlock();

		schedule_timeout_interruptible(delay);
	}

	return 0;
}

static int __init kthreads_rcu_init(void)
{
	int ret = 0;
	unsigned int *p = NULL;
	unsigned int *init = NULL;

	init = kzalloc(sizeof(*init), GFP_KERNEL);
	if (IS_ERR_OR_NULL(init)) {
		pr_err("Failed to allocate data buffer for initialization\n");
		return -ENOMEM;
	}

	*init = 0x5A5A5A5A;
	RCU_INIT_POINTER(data, init);

	kt[0] = kthread_run(rcu_writer_func, NULL, WRITER_NAME);
	if (IS_ERR_OR_NULL(kt[0])) {
		ret = PTR_ERR(kt[0]);
		pr_err("kthread_run() failed for %s, %d\n", WRITER_NAME, ret);
		goto err;
	}

	kt[1] = kthread_run(rcu_reader_func, NULL, READER1_NAME);
	if (IS_ERR_OR_NULL(kt[1])) {
		ret = PTR_ERR(kt[1]);
		pr_err("kthread_run() failed for %s, %d\n", READER1_NAME, ret);
		goto err;
	}

	kt[2] = kthread_run(rcu_reader_func, NULL, READER1_NAME);
	if (IS_ERR_OR_NULL(kt[2])) {
		ret = PTR_ERR(kt[2]);
		pr_err("kthread_run() failed for %s, %d\n", READER1_NAME, ret);
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

	p = rcu_dereference_protected(data, 1);
	rcu_assign_pointer(data, NULL);
	synchronize_rcu();
	kfree(p);

	return ret;
}

static void __exit kthreads_rcu_exit(void)
{
	int ret = 0;
	unsigned int *p = NULL;

	ret = kthread_stop(kt[0]);
	if (ret)
		pr_err("kthread_stop() failed for %s, %d\n", WRITER_NAME, ret);

	ret = kthread_stop(kt[1]);
	if (ret)
		pr_err("kthread_stop() failed for %s, %d\n", READER0_NAME, ret);

	ret = kthread_stop(kt[2]);
	if (ret)
		pr_err("kthread_stop() failed for %s, %d\n", READER1_NAME, ret);

	p = rcu_dereference_protected(data, 1);
	rcu_assign_pointer(data, NULL);
	synchronize_rcu();
	kfree(p);

	return;
}

module_init(kthreads_rcu_init);
module_exit(kthreads_rcu_exit);

MODULE_AUTHOR("Udayan Prabir Sinha");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_DESCRIPTION("RCU example with kthreads");
