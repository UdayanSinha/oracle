/*
 * misc-char-drv.c: Misc. Character Device Driver
 *		    This driver provides a random number value on reads.
 * Author: Udayan Prabir Sinha
 * Date: 2023-08-21
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/random.h>

#define DEV_NAME "misc-char-drv"		/* name under /dev */

static u64 rand_num_buf = ~0;
static u32 rand_num_size = 4;	/* number of bytes */

static int misc_chardrv_open(struct inode *inode, struct file *filep)
{
	pr_devel("open() called. Major: %u, Minor: %u\n",
		imajor(inode), iminor(inode));

	return 0;
}

static int misc_chardrv_release(struct inode *inode, struct file *filep)
{
	pr_devel("close() called. Major: %u, Minor: %u\n",
		imajor(inode), iminor(inode));

	return 0;
}

static ssize_t misc_chardrv_read(struct file *filep, char __user *buf,
		size_t lbuf, loff_t *ppos)
{
	unsigned long num_bytes_read = 0;

	if (lbuf < rand_num_size)
		return -EFAULT;

	get_random_bytes(&rand_num_buf, rand_num_size);
	num_bytes_read = lbuf - copy_to_user(buf, &rand_num_buf, rand_num_size);
	*ppos += num_bytes_read;

	return num_bytes_read;
}

static const struct file_operations misc_chardrv_ops = {
	.owner = THIS_MODULE,
	.open = misc_chardrv_open,
	.release = misc_chardrv_release,
	.read = misc_chardrv_read,
};

static struct miscdevice misc_chardrv = {
	.name = DEV_NAME,
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &misc_chardrv_ops,
};

static int __init misc_chardrv_init(void)
{
	int ret = 0;

	ret = misc_register(&misc_chardrv);
	if (ret)
		return ret;

	return 0;
}

static void __exit misc_chardrv_exit(void)
{
	misc_deregister(&misc_chardrv);
	return;
}

module_init(misc_chardrv_init);
module_exit(misc_chardrv_exit);

MODULE_AUTHOR("Udayan Prabir Sinha");
MODULE_LICENSE("Dual MIT/GPL");
