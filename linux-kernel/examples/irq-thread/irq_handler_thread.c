/*
 * irq_handler_thread.c: IRQ Threaded Handler Example
 */

#include <linux/module.h>	/* module_*(), MODULE_*() */
#include <linux/init.h>		/* __init, __exit */
#include <linux/printk.h>	/* printk() and utils */
#include <linux/interrupt.h>	/* IRQ APIs */
#include <linux/irqreturn.h>	/* irqreturn_t */

#define IRQ_NAME "test-irq-threaded-handler"	/* will show up under /proc/interrupts */


static unsigned int irq_num = 24;	/* IRQ line, see /proc/interrupts */
static unsigned int irqf_flags = IRQF_SHARED;
static unsigned int irq_data = 0;	/* dummy data to be used by handler */

static irqreturn_t irq_handler(int irq, void *ptr)
{
	unsigned int *data = (unsigned int*) ptr;
	pr_debug("Running %s top-half\n", IRQ_NAME);
	*data = (*data == 0xFFFFFFFF) ? 0 : (*data + 1);

	if ((*data) % 2)
		return IRQ_HANDLED;

	return IRQ_WAKE_THREAD;
}

static irqreturn_t irq_thread(int irq, void *ptr)
{
	unsigned int *data = (unsigned int*) ptr;

	pr_debug("Running %s bottom-half, *data=0x%x\n", IRQ_NAME, *data);
	return IRQ_HANDLED;
}

static int __init irq_handler_thread_init(void)
{
	int ret = 0;

	ret = request_threaded_irq(irq_num, irq_handler, irq_thread,
			irqf_flags, IRQ_NAME, &irq_data);
	if (ret < 0) {
		pr_err("Unable to request IRQ %u, %d", irq_num, ret);
		return ret;
	}

	return 0;
}

static void __exit irq_handler_thread_exit(void)
{
	free_irq(irq_num, &irq_data);
}

module_init(irq_handler_thread_init);
module_exit(irq_handler_thread_exit);

MODULE_AUTHOR("Udayan Prabir Sinha");
MODULE_LICENSE("Dual MIT/GPL");
