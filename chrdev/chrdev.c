#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
//#include <asm-generic/sections.h>
//#include <linux/spinlock.h>

static int major = 0;
module_param(major, int, 0600);
MODULE_PARM_DESC(major, "the major of  device");


DEFINE_PER_CPU(int, mpcpu) = 2;
static char *name = "mdev";

struct mycdev {
	struct cdev cdev;
	dev_t  num;
};

struct mycdev *mdev;

static int mdev_open(struct inode *inode, struct file *file)
{
	file->private_data = mdev;
	return 0;
}

static int mdev_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	pr_info("in release\n");
	return 0;
}

static ssize_t mdev_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
	pr_info("in read\n");
	return len;
}

static ssize_t mdev_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
	pr_info("in write\n");
	return len;
}

static struct file_operations mdev_fops =  {
	.open = mdev_open,
	.release = mdev_release,
	.read = mdev_read,
	.write = mdev_write,
};

static struct mycdev * do_chrdev_init(void)
{
	mdev = kmalloc(sizeof(struct mycdev), GFP_KERNEL);
	if (!mdev)
		return ERR_PTR(-ENOMEM);
	if (major) {
		if (register_chrdev_region(MKDEV(major,0),1, name))
			goto err1;
		mdev->num = MKDEV(major, 0);
	}
	else
		if (alloc_chrdev_region(&mdev->num, 0, 1, name))
			goto err1;
	cdev_init(&mdev->cdev, &mdev_fops);
	if (cdev_add(&mdev->cdev, mdev->num, 1) < 0 )
		goto err2;
	return mdev;

err2:
	unregister_chrdev_region(mdev->num, 1);
err1:
	kfree(mdev);
	return ERR_PTR(-EFAULT);
}
		
static void do_chrdev_exit(struct mycdev *mdev)
{
	cdev_del(&mdev->cdev);
	unregister_chrdev_region(mdev->num, 1);
	kfree(mdev);
}


static int __init start(void)
{

	mdev = do_chrdev_init();
	if (IS_ERR(mdev))
		return -EFAULT;
	pr_info("success alloc chrdev with devnum %x\n", mdev->num);
	return 0;
}

static void __exit end(void)
{
	do_chrdev_exit(mdev);
	pr_info("success exit chrdev\n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("a test chrdev driver\n");
MODULE_AUTHOR("muryliang@gmail.com");

module_init(start);
module_exit(end);

