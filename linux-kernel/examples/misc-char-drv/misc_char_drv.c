/*
 * misc_char_drv.c: Misc. Character Device Driver Example
 */

#include <linux/module.h>	/* module_*(), MODULE_*() */
#include <linux/init.h>		/* __init, __exit */
#include <linux/printk.h>	/* printk() and utils */
#include <linux/errno.h>	/* error codes */
#include <linux/fs.h>		/* struct file, file_operations, inode */
#include <linux/slab.h>		/* memory allocation APIs */
#include <linux/ioctl.h>	/* ioctl() and utils */
#include <linux/miscdevice.h>	/* misc char-drv APIs */
#include <linux/random.h>	/* random-number APIs */
#include <linux/panic.h>	/* panic() */

#define DEV_NAME "misc_char_drv"		/* name under /dev */

/* ioctl() commands */
#define MISC_CHAR_DRV_BASE	'X'
#define MISC_CHAR_DRV_PANIC	_IO(MISC_CHAR_DRV_BASE, 0)

static void *rand_num_buf = NULL;

static int misc_chardrv_open(struct inode *inode, struct file *filep)
{
	pr_debug("open() called. Major: %u, Minor: %u\n",
		imajor(inode), iminor(inode));

	return 0;
}

static int misc_chardrv_release(struct inode *inode, struct file *filep)
{
	pr_debug("release() called. Major: %u, Minor: %u\n",
		imajor(inode), iminor(inode));

	return 0;
}

static ssize_t misc_chardrv_read(struct file *filep, char __user *buf,
		size_t lbuf, loff_t *ppos)
{
	unsigned long num_bytes_read = 0;

	pr_debug("read() called. Major: %u, Minor: %u\n",
		imajor(filep->f_inode), iminor(filep->f_inode));

	rand_num_buf = kzalloc(lbuf, GFP_KERNEL);
	if (!rand_num_buf)
		return -ENOMEM;

	get_random_bytes(rand_num_buf, (int)lbuf);
	num_bytes_read = lbuf - copy_to_user(buf, rand_num_buf, lbuf);
	*ppos += num_bytes_read;

	kfree(rand_num_buf);
	return num_bytes_read;
}

static void misc_chardrv_unlocked_ioctl_usage(unsigned int cmd)
{
	pr_err("Unknown command received: 0x%x\n", cmd);
	pr_debug("Supported ioctl() Commands:\n\t"
			"MISC_CHAR_DRV_PANIC[0x%x]\n\t", MISC_CHAR_DRV_PANIC);
}

static long misc_chardrv_unlocked_ioctl(struct file *filep,
		unsigned int cmd, unsigned long arg)
{
	pr_debug("unlocked_ioctl() called. Major: %u, Minor: %u\n",
		imajor(filep->f_inode), iminor(filep->f_inode));

	if (_IOC_TYPE(cmd) != MISC_CHAR_DRV_BASE) {
		misc_chardrv_unlocked_ioctl_usage(cmd);
		return -EINVAL;
	}

	switch (cmd)
	{
	case MISC_CHAR_DRV_PANIC:
		panic("MISC_CHAR_DRV_PANIC issued! Major: %u, Minor: %u\n",
			imajor(filep->f_inode), iminor(filep->f_inode));
		break;

	default:
		misc_chardrv_unlocked_ioctl_usage(cmd);
		return -EINVAL;
	}

	return 0;
}

static const struct file_operations misc_chardrv_ops = {
	.owner = THIS_MODULE,
	.open = misc_chardrv_open,
	.release = misc_chardrv_release,
	.read = misc_chardrv_read,
	.unlocked_ioctl = misc_chardrv_unlocked_ioctl,
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
