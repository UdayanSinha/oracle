/*
 * irq_handler.c: IRQ Handler Example
 */

#include <linux/module.h>	/* module_*(), MODULE_*() */
#include <linux/init.h>		/* __init, __exit */
#include <linux/printk.h>	/* printk() and utils */
#include <linux/interrupt.h>	/* IRQ APIs */
#include <linux/irqreturn.h>	/* irqreturn_t */
#include <linux/moduleparam.h>	/* module parameter utils */

#define IRQ_NAME "test-irq-handler"	/* will show up under /proc/interrupts */

static unsigned int irq_num = 24;	/* IRQ line, see /proc/interrupts */
static unsigned int irqf_flags = IRQF_SHARED;
static unsigned int irq_data = 0;	/* dummy data to be used by handler */

/* set IRQ to service during module loading, will use default (24) otherwise */
module_param(irq_num, uint, 0644);
MODULE_PARM_DESC(irq_num, "IRQ number to install handler for");

static irqreturn_t irq_handler(int irq, void *ptr)
{
	unsigned int *data = (unsigned int*) ptr;
	pr_debug("Running %s[%d], *data: 0x%x\n", IRQ_NAME, irq, *data);
	*data = (*data == 0xFFFFFFFF) ? 0 : (*data + 1);

	/*
	 * If no IRQ handler for an IRQ returns IRQ_HANDLED/IRQ_WAKE_THREAD,
	 * it will result in warnings from the kernel.
	 */
	return IRQ_HANDLED;
}

static int __init irq_handler_init(void)
{
	int ret = 0;

	/*
	 * irq_data must be passed (and be unique) for shared IRQs, as that is
	 * used by kernel as the basis to know which handler to de-register when
	 * calling free_irq() (which passes the same irq_data). It is also used
	 * as private data by the IRQ handler.
	 *
	 * For shared IRQs, all handlers must use IRQF_SHARED or there may be
	 * warnings/errors from the kernel. In such a case IRQF_PROBE_SHARED may
	 * be needed.
	 */
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
MODULE_DESCRIPTION("Non-Threaded IRQ Handling Example");
