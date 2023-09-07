/*
 * timer_lowres.c: Low-Resolution Timer Example
 */

#include <linux/module.h>	/* module_*(), MODULE_*() */
#include <linux/init.h>		/* __init, __exit */
#include <linux/printk.h>	/* printk() and utils */
#include <linux/types.h>	/* data types and utils */
#include <linux/jiffies.h>	/* jiffies and utils */
#include <linux/slab.h>		/* memory allocation APIs */
#include <linux/timer.h>	/* low-res timer APIs */

struct callback_struct {
	u64 data;		/* dummy data for use by the callback */
	unsigned long delay;	/* timer period in jiffies, measured as absolute time */
	struct timer_list timer;
};

static struct callback_struct *sample_data;

static void timer_callback(struct timer_list *t)
{
	struct callback_struct *data_struct = from_timer(data_struct, t, timer);

	pr_debug("Callback executing in timer_lowres, data: 0x%llu\n",
			data_struct->data);

	/* manipulate dummy data */
	if (data_struct->data == U64_MAX)
		data_struct->data = 0;
	else
		data_struct->data++;

	/* re-activate timer */
	data_struct->delay = get_jiffies_64() + (1 * HZ);	/* 1 second */
	(void)mod_timer(&data_struct->timer, data_struct->delay);
	return;
}

static int __init timer_lowres_init(void)
{
	sample_data = kmalloc(sizeof(struct callback_struct), GFP_KERNEL);
	if (!sample_data)
		return -ENOMEM;

	/* populate dummy data */
	sample_data->data = 0;

	/* setup timer struct */
	timer_setup(&sample_data->timer, timer_callback, TIMER_DEFERRABLE);

	/* activate timer */
	sample_data->delay = get_jiffies_64() + (1 * HZ);	/* 1 second */
	(void)mod_timer(&sample_data->timer, sample_data->delay);
	return 0;
}

static void __exit timer_lowres_exit(void)
{
	/* delete timer */
	del_timer_sync(&sample_data->timer);

	kfree(sample_data);
	return;
}

module_init(timer_lowres_init);
module_exit(timer_lowres_exit);

MODULE_AUTHOR("Udayan Prabir Sinha");
MODULE_LICENSE("Dual MIT/GPL");
