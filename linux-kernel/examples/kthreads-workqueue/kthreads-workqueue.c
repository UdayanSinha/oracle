/*
 * workqueue.c: Workqueue Example
 */

#include <linux/module.h>	/* module_*(), MODULE_*() */
#include <linux/init.h>		/* __init, __exit */
#include <linux/printk.h>	/* printk() and utils */
#include <linux/kthread.h>	/* kthread APIs */
#include <linux/workqueue.h>	/* workqueue utils */
#include <linux/moduleparam.h>	/* module parameter utils */

#define WQ_NAME		"wq-example"
#define KT_NAME		"kt-example"

static struct task_struct *kt = NULL;
static struct workqueue_struct *wq = NULL;
static struct work_struct work;

static void work_fn(struct work_struct *work_item)
{
	(void) work_item;

	pr_debug("Handling work that was queued on %s\n", WQ_NAME);
}

static int kt_func(void *data)
{
	unsigned long delay = msecs_to_jiffies(750);
	pr_info("[%s] Starting up...\n", KT_NAME);

	while (!kthread_should_stop()) {
		INIT_WORK(&work, work_fn);
		queue_work(wq, &work);

		schedule_timeout_interruptible(delay);
	}

	return 0;
}

static int __init kthreads_wq_init(void)
{
	int ret = 0;

	wq = alloc_workqueue(WQ_NAME, WQ_UNBOUND, 0);
	if (IS_ERR_OR_NULL(wq)) {
		ret = PTR_ERR(wq);
		pr_err("alloc_workqueue() failed for %s, %d\n", WQ_NAME, ret);
		goto err;
	}

	kt = kthread_run(kt_func, NULL, KT_NAME);
	if (IS_ERR_OR_NULL(kt)) {
		ret = PTR_ERR(kt);
		pr_err("kthread_run() failed for %s, %d\n", KT_NAME, ret);
		goto err;
	}

	return 0;

err:
	if (!IS_ERR_OR_NULL(kt))
		(void) kthread_stop(kt);
	if (wq)
		destroy_workqueue(wq);

	return ret;
}

static void __exit kthreads_wq_exit(void)
{
	int ret = 0;

	ret = kthread_stop(kt);
	if (ret)
		pr_err("kthread_stop() failed for %s, %d\n", KT_NAME, ret);

	destroy_workqueue(wq);

	return;
}

module_init(kthreads_wq_init);
module_exit(kthreads_wq_exit);

MODULE_AUTHOR("Udayan Prabir Sinha");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_DESCRIPTION("Workqueue example with kthreads");
