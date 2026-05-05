/*
 * mem_alloc.c: Example of memory allocations
 */

#include <linux/module.h>	/* module_*(), MODULE_*() */
#include <linux/init.h>		/* __init, __exit */
#include <linux/printk.h>	/* printk() and utils */
#include <linux/mm.h>		/* virtual memory management APIs */
#include <linux/slab.h>		/* memory allocation APIs */
#include <linux/types.h>	/* data types */

#define PAGE_ALLOC_ORDER	1	/* order of 2 => 2 pages */

struct test_struct {
	u32 num;
	char *str;
};

struct test_struct *p1 = NULL;
struct test_struct *p2 = NULL;
struct test_struct *p3 = NULL;
struct test_struct *p4 = NULL;

struct kmem_cache *cache = NULL;

unsigned long single_page = 0;
unsigned long multi_pages = 0;

static void kmalloc_init(void)
{
	/* allocation can sleep */
	p1 = kzalloc(sizeof(*p1), GFP_KERNEL);
	if (IS_ERR_OR_NULL(p1)) {
		pr_err("Failed to allocate p1\n");
		return;
	}

	/* allocation cannot sleep */
	p2 = kzalloc(sizeof(*p2), GFP_ATOMIC);
	if (IS_ERR_OR_NULL(p2)) {
		pr_err("Failed to allocate p2\n");
		return;
	}

	p1->str = "p1";
	p1->num = 0x55;

	p2->str = "p2";
	p2->num = 0xAA;

	pr_info("p1.str: %s, p1.num = 0x%x\n", p1->str, p1->num);
	pr_info("p2.str: %s, p2.num = 0x%x\n", p2->str, p2->num);
}

static void kmalloc_exit(void)
{
	kfree(p1);
	kfree(p2);
}

static void log_page_statistics(void)
{
	pr_info("Number of page frames: %lu\n", get_num_physpages());
}

static void kmem_cache_init(void)
{
	/*
	 * SLAB_POISON: Fills allocated memory with known value to catch use of
	 *              uninitialized memory.
	 * SLAB_RED_ZONE: Surround allocated memory with "red zones" to detect
	 *                out-of-bounds access.
	 */
	cache = kmem_cache_create("test-cache", 256, 0,
			SLAB_POISON | SLAB_RED_ZONE, NULL);

	/* object allocation can sleep */
	p3 = kmem_cache_zalloc(cache, GFP_KERNEL);
	if (IS_ERR_OR_NULL(p3)) {
		pr_err("Failed to allocate p3\n");
		return;
	}

	p3->str = "p3";
	p3->num = 0x5A;

	/* object allocation cannot sleep */
	p4 = kmem_cache_zalloc(cache, GFP_ATOMIC);
	if (IS_ERR_OR_NULL(p4)) {
		pr_err("Failed to allocate p4\n");
		return;
	}

	p4->str = "p4";
	p4->num = 0xA5;

	pr_info("p3.str: %s, p3.num = 0x%x\n", p3->str, p3->num);
	pr_info("p4.str: %s, p4.num = 0x%x\n", p4->str, p4->num);
}

static void kmem_cache_exit(void)
{
	kmem_cache_free(cache, p3);
	kmem_cache_free(cache, p4);

	/* destroy all cache-allocated objects first */
	kmem_cache_destroy(cache);
}

static void page_alloc_init(void)
{
	/* page allocation cannot sleep */
	single_page = get_zeroed_page(GFP_ATOMIC);
	if (!single_page) {
		pr_err("Failed to allocate single page\n");
		return;
	}

	/* page allocation can sleep */
	multi_pages = __get_free_pages(GFP_KERNEL, PAGE_ALLOC_ORDER);
	if (!multi_pages) {
		pr_err("Failed to allocate multiple pages\n");
		return;
	}

	pr_info("Single page start address: 0x%lx\n", single_page);
	pr_info("Multiple page start address: 0x%lx\n", multi_pages);
}

static void page_alloc_exit(void)
{
	free_page(single_page);
	free_pages(multi_pages, PAGE_ALLOC_ORDER);
}

static int __init hello_world_init(void)
{
	kmalloc_init();
	kmem_cache_init();
	log_page_statistics();
	page_alloc_init();
	return 0;
}

static void __exit hello_world_exit(void)
{
	kmalloc_exit();
	kmem_cache_exit();
	page_alloc_exit();
	return;
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_AUTHOR("Udayan Prabir Sinha");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_DESCRIPTION("Memory allocation example");
