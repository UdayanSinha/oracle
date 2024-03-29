/*
 * hello_world.c: Hello-World Kernel Module
 */

#include <linux/module.h>	/* module_*(), MODULE_*() */
#include <linux/init.h>		/* __init, __exit */
#include <linux/printk.h>	/* printk() and utils */

static int __init hello_world_init(void)
{
	pr_info("Hello from a humble Linux Kernel Module!\n");
	return 0;
}

static void __exit hello_world_exit(void)
{
	pr_info("Goodbye from a humble Linux Kernel Module!\n");
	return;
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_AUTHOR("Udayan Prabir Sinha");
MODULE_LICENSE("Dual MIT/GPL");
