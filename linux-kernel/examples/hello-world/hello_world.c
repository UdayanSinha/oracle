/*
 * hello_world.c: Hello-World Kernel Module
 * Author: Udayan Prabir Sinha
 * Date: 2023-08-12
 */

# include <linux/module.h>
# include <linux/init.h>

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
