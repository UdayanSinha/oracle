/*
 * addr_space.c: Process Address Space Exploration Example
 */

#include <linux/module.h>	/* module_*(), MODULE_*() */
#include <linux/init.h>		/* __init, __exit */
#include <linux/printk.h>	/* printk() and utils */
#include <linux/atomic.h>	/* atomic-type APIs */
#include <linux/container_of.h>	/* container_of() utils */
#include <linux/sched.h>	/* scheduler APIs */
#include <linux/mm.h>		/* address space utils */

static void log_current_task_addr_space(void)
{
	struct task_struct *last_task = NULL;
	struct mm_struct *mm_ref = NULL;
	struct vm_area_struct *vma = NULL;

	if (current->mm) {
		pr_info("Current task is a user-space task: %s\n", current->comm);
		mm_ref = current->mm;
	} else {
		last_task = container_of(&current->active_mm, struct task_struct, mm);
		pr_info("Current task is a kthread, using last user-space task as reference: %s\n",
				last_task->comm);
		mm_ref = current->active_mm;
	}

	if(!mm_ref) {
		pr_info("Could not retrieve an address space to inspect\n");
		return;
	}

	VMA_ITERATOR(vmi, mm_ref, 0);

	pr_info("Size of virtual address space: %lu\n", mm_ref->task_size);
	pr_info("PGD: %p\n", mm_ref->pgd);
	pr_info("Threads: %u\n", atomic_read(&mm_ref->mm_users));
	pr_info("Number of memory regions: %d\n", mm_ref->map_count);
	pr_info("Largest RSS usage value: %lu\n", mm_ref->hiwater_rss);
	pr_info("Largest VM usage value: %lu\n", mm_ref->hiwater_vm);
	pr_info("Total pages mapped: %lu\n", mm_ref->total_vm);
	pr_info("Page table size: %lu\n", atomic_long_read(&mm_ref->pgtables_bytes));

	pr_info("Memory Regions: \n");
	for_each_vma(vmi, vma) {
		pr_info("Start Addr: %lu, End Addr: %lu\n",
			vma->vm_start, vma->vm_end);
	}
}

static int __init addr_space_init(void)
{
	log_current_task_addr_space();
	return 0;
}

static void __exit addr_space_exit(void)
{
	return;
}

module_init(addr_space_init);
module_exit(addr_space_exit);

MODULE_AUTHOR("Udayan Prabir Sinha");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_DESCRIPTION("Process Address Space Exploration Example");
