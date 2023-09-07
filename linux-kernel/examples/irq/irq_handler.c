/*
 * irq_handler.c: IRQ Handler Example
 */

#include <linux/module.h>	/* module_*(), MODULE_*() */
#include <linux/init.h>		/* __init, __exit */
#include <linux/printk.h>	/* printk() and utils */
#include <linux/interrupt.h>	/* IRQ APIs */
#include <linux/irqreturn.h>	/* irqreturn_t */

#define IRQ_NAME "test-irq-handler"	/* IRQ source, will show up under /proc/interrupts */

static unsigned int irq_num = 24;	/* IRQ line, see /proc/interrupts */
static unsigned int irqf_flags = IRQF_SHARED;
static unsigned int irq_data = 0;	/* dummy data to be used by handler */

static irqreturn_t irq_handler(int irq, void *dev)
{
	unsigned int *data_ptr = (unsigned int*) dev;
	pr_debug("Running %s, *data_ptr: 0x%x\n", IRQ_NAME, *data_ptr);
	*data_ptr = (*data_ptr == 0xFFFFFFFF) ? 0 : (*data_ptr + 1);
	return IRQ_RETVAL(1);
}

static int __init irq_handler_init(void)
{
	int ret = 0;

	ret = request_irq(irq_num, irq_handler, irqf_flags, IRQ_NAME, &irq_data);
	if (ret < 0) {
		pr_err("Unable to request IRQ %u, %d", irq_num, ret);
		return ret;
	}

	return 0;
}

static void __exit irq_handler_exit(void)
{
	free_irq(irq_num, &irq_data);
}

module_init(irq_handler_init);
module_exit(irq_handler_exit);

MODULE_AUTHOR("Udayan Prabir Sinha");
MODULE_LICENSE("Dual MIT/GPL");
