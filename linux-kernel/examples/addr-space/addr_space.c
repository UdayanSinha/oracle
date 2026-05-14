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
#include <linux/types.h>	/* type utils */
#include <linux/moduleparam.h>	/* module parameter APIs */

static pid_t taskpid = 0;
module_param(taskpid, int, 0);
MODULE_PARM_DESC(taskpid, "User-space task PID (TGID) whose address space info is to be inspected");

static void do_log_task_addr_space(struct mm_struct *mm)
{
	struct vm_area_struct *vma = NULL;
	VMA_ITERATOR(vmi, mm, 0);

	pr_info("Size of virtual address space: %lu\n", mm->task_size);
	pr_info("PGD: %p\n", mm->pgd);
	pr_info("Threads: %u\n", atomic_read(&mm->mm_users));
	pr_info("Number of memory regions: %d\n", mm->map_count);
	pr_info("Largest RSS usage value: %lu\n", mm->hiwater_rss);
	pr_info("Largest VM usage value: %lu\n", mm->hiwater_vm);
	pr_info("Total pages mapped: %lu\n", mm->total_vm);
	pr_info("Page table size: %lu\n", atomic_long_read(&mm->pgtables_bytes));

	pr_info("Memory Regions: \n");
	for_each_vma(vmi, vma) {
		pr_info("Start Addr: 0x%lx, End Addr: 0x%lx, Flags: 0x%lx\n",
			vma->vm_start, vma->vm_end, vma->vm_flags);
	}
}

static void log_task_addr_space(void)
{
	struct task_struct *p = NULL;
	struct mm_struct *mm = NULL;

	if (!taskpid) {
		pr_info("Cannot use invalid PID %d for address space inspection\n",
				taskpid);
		return;
	}

	for_each_process(p) {
		if (p->tgid == taskpid) {
			pr_info("Attempting address space inspection for %s[%d]\n",
					p->comm, p->tgid);
			mm = p->mm;
			break;
		}
	}

	if(!mm) {
		pr_info("PID does not exist or belongs to kthread\n");
		return;
	}

	do_log_task_addr_space(mm);
}

static int __init addr_space_init(void)
{
	log_task_addr_space();
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
