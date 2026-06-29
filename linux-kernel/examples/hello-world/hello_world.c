/*
 * hello_world.c: Hello-World Kernel Module
 */

#include <linux/module.h>	/* module_*(), MODULE_*() */
#include <linux/init.h>		/* __init, __exit */
#include <linux/printk.h>	/* printk() and utils */
#include <linux/sched.h>	/* scheduler APIs */
#include <linux/slab.h>		/* memory allocation APIs */
#include <linux/list.h>		/* linked-list utils */
#include <linux/percpu.h>	/* percpu utils */
#include <linux/err.h>		/* error handling utils */
#include <linux/preempt.h>	/* kernel preemption utils */
#include <linux/utsname.h>	/* sys info utils */
#include <uapi/linux/utsname.h>	/* sys info utils */

struct test_struct {
	struct list_head link;
	char *str;
};

static LIST_HEAD(lhead);

void log_sysinfo(void);

/* exported symbols */
EXPORT_SYMBOL(log_sysinfo);

/*
 * Can also be an array of per-CPU variables.
 * E.g. DEFINE_PERCPU(int[3], var_name);
 */
static DEFINE_PER_CPU(int, percpu_var) = 0;

void log_sysinfo(void)
{
	struct new_utsname *uts = NULL;

	uts = init_utsname();
	pr_info("sysname: %s\n", uts->sysname);
	pr_info("nodename: %s\n", uts->nodename);
	pr_info("release: %s\n", uts->release);
	pr_info("version: %s\n", uts->version);
	pr_info("machine: %s\n", uts->machine);
	pr_info("domainname: %s\n", uts->domainname);
}

static void log_current_task(void)
{
	pr_info("Current task:\n");
	pr_info("comm: %s\n", current->comm);
	pr_info("tgid: %d\n", current->tgid);
	pr_info("pid: %d\n", current->pid);
	pr_info("state: %u\n", get_current_state());
}

static void log_task_list(void)
{
	struct task_struct *p = NULL;

	pr_info("Task List:");
	for_each_process(p) {
		pr_info("comm: %s\n", p->comm);
		pr_info("tgid: %d\n", p->tgid);
		pr_info("pid: %d\n", p->pid);
		pr_info("state: %d\n", task_state_to_char(p));
	}
}

static void list_usage(void)
{
	struct test_struct *it = NULL;
	struct test_struct *tmp = NULL;

	struct test_struct *p1 = kzalloc(sizeof(*p1), GFP_KERNEL);
	if (IS_ERR_OR_NULL(p1)) {
		pr_err("Failed to allocate p1\n");
		return;
	}

	struct test_struct *p2 = kzalloc(sizeof(*p2), GFP_ATOMIC);
	if (IS_ERR_OR_NULL(p2)) {
		pr_err("Failed to allocate p2\n");
		kfree(p2);
		return;
	}

	struct test_struct *p3 = kzalloc(sizeof(*p3), GFP_KERNEL);
	if (IS_ERR_OR_NULL(p3)) {
		pr_err("Failed to allocate p3\n");
		kfree(p1);
		kfree(p2);
		return;
	}

	struct test_struct *p4 = kzalloc(sizeof(*p4), GFP_ATOMIC);
	if (IS_ERR_OR_NULL(p4)) {
		pr_err("Failed to allocate p4\n");
		kfree(p1);
		kfree(p2);
		kfree(p3);
		return;
	}

	struct test_struct *p5 = kzalloc(sizeof(*p5), GFP_KERNEL);
	if (IS_ERR_OR_NULL(p5)) {
		pr_err("Failed to allocate p5\n");
		kfree(p1);
		kfree(p2);
		kfree(p3);
		kfree(p4);
		return;
	}

	p1->str = "p1";
	INIT_LIST_HEAD(&p1->link);
	list_add_tail(&p1->link, &lhead);

	p2->str = "p2";
	INIT_LIST_HEAD(&p2->link);
	list_add_tail(&p2->link, &lhead);

	p3->str = "p3";
	INIT_LIST_HEAD(&p3->link);
	list_add_tail(&p3->link, &lhead);

	p4->str = "p4";
	INIT_LIST_HEAD(&p4->link);
	list_add_tail(&p4->link, &lhead);

	p5->str = "p5";
	INIT_LIST_HEAD(&p5->link);
	list_add_tail(&p5->link, &lhead);

	list_for_each_entry(it, &lhead, link)
		pr_info("Element String: %s\n", it->str);

	list_for_each_entry_safe(it, tmp, &lhead, link) {
		list_del(it->link);
		kfree(it);
	}
}

static void percpu_var_usage(void)
{
	unsigned long irq_flags = 0;
	unsigned int current_cpu = 0;
	int *percpu_alloc_var = NULL;
	int *percpu_it = NULL;
	unsigned int i = 0;

	/* may sleep, do not use in atomic context */
	percpu_alloc_var = alloc_percpu(int);
	if (IS_ERR_OR_NULL(percpu_alloc_var)) {
		pr_err("Failed to allocate per-cpu variable\n");
		return;
	}

	/* get current cpu info */
	preempt_disable();
	local_irq_save(irq_flags);
	current_cpu = smp_processor_id();
	pr_info("Current CPU: %u\n", current_cpu);
	local_irq_restore(irq_flags);
	preempt_enable();

	/* access variables for currently executing CPU */
	get_cpu_var(percpu_var)++;
	put_cpu_var(percpu_var);

	pr_info("Per-CPU var for current CPU: %d\n", get_cpu_var(percpu_var));
	put_cpu_var(percpu_var);

	/* write to the per-CPU variables */
	for_each_possible_cpu(i) {
		percpu_it = per_cpu_ptr(percpu_alloc_var, i);
		*percpu_it = i;

		percpu_it = &per_cpu(percpu_var, i);
		*percpu_it = i * 2;
	}

	/* read from the per-CPU variables */
	for_each_possible_cpu(i) {
		percpu_it = per_cpu_ptr(percpu_alloc_var, i);
		pr_info("Per-CPU pre-defined var for CPU[%u]: %d\n",
				i, *percpu_it);

		percpu_it = &per_cpu(percpu_var, i);
		pr_info("Per-CPU allocated var for CPU[%u]: %d\n",
				i, *percpu_it);
	}

	free_percpu(percpu_alloc_var);
}

static int __init hello_world_init(void)
{
	pr_info("Hello from a humble Linux Kernel Module!\n");

	log_sysinfo();
	log_current_task();
	log_task_list();
	list_usage();
	percpu_var_usage();
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
MODULE_DESCRIPTION("Hello World Kernel Module");
