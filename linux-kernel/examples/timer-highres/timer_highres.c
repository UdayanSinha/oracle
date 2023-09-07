/*
 * timer_highres.c: High-Resolution Timer Example
 */

#include <linux/module.h>	/* module_*(), MODULE_*() */
#include <linux/init.h>		/* __init, __exit */
#include <linux/printk.h>	/* printk() and utils */
#include <linux/types.h>	/* data types */
#include <linux/slab.h>		/* memory allocation APIs */
#include <linux/ktime.h>	/* ktime_t and utils */
#include <linux/hrtimer.h>	/* high-res timer APIs */

struct callback_struct {
	u64 data;		/* dummy data for use by the callback */
	struct hrtimer timer;
	ktime_t delay;
};

static struct callback_struct *sample_data;

enum hrtimer_restart timer_callback(struct hrtimer *t)
{
	struct callback_struct *data_struct = container_of(t,
			struct callback_struct, timer);

	ktime_t now = t->base->get_time();	/* current absolute time */

	pr_debug("Callback executing in timer_highres, data: 0x%llu\n",
			data_struct->data);

	/* manipulate dummy data */
	if (data_struct->data == U64_MAX)
		data_struct->data = 0;
	else
		data_struct->data++;

	/* re-activate timer */
	(void)hrtimer_forward(t, now, data_struct->delay);

	/*
	 * HRTIMER_RESTART: recurring timer
	 * HRTIMER_NORESTART: one-shot timer
	 */
	return HRTIMER_RESTART;
}

static int __init timer_highres_init(void)
{
	sample_data = kmalloc(sizeof(struct callback_struct), GFP_KERNEL);
	if (!sample_data)
		return -ENOMEM;

	/* populate dummy data */
	sample_data->data = 0;

	/* setup timer struct */
	sample_data->delay = ktime_set(1, 0);	/* 1 second */
	hrtimer_init(&sample_data->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	sample_data->timer.function = timer_callback;

	/* activate timer */
	hrtimer_start(&sample_data->timer, sample_data->delay, HRTIMER_MODE_REL);
	return 0;
}

static void __exit timer_highres_exit(void)
{
	/* cancel timer */
	(void)hrtimer_cancel(&sample_data->timer);

	kfree(sample_data);
	return;
}

module_init(timer_highres_init);
module_exit(timer_highres_exit);

MODULE_AUTHOR("Udayan Prabir Sinha");
MODULE_LICENSE("Dual MIT/GPL");
