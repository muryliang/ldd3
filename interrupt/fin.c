#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/slab.h>

#define SSIZE 40

struct sdev {
	char *buf;
	char *wp, *rp;
	dev_t num;
	int size;
	struct cdev cdev;
	spinlock_t spin;
};

static struct sdev *sptr;

static int sdev_open(struct inode *inode, struct file *file)
{
	pr_info("in open\n");
	return 0;
}

static int sdev_release(struct inode *inode, struct file *file)
{
	pr_info("in release\n");
	return 0;
}

static ssize_t sdev_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
	pr_info("in read\n");
	return len;
}

static ssize_t sdev_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
	pr_info("in write\n");
	return len;
}

static struct file_operations sdev_fops =  {
	.open = sdev_open,
	.release = sdev_release,
	.read = sdev_read,
	.write = sdev_write,
};
	

static int __init start(void)
{
	struct sdev *ptr;
	int err;

	ptr = kzalloc(sizeof(struct sdev), GFP_KERNEL);
	if (!ptr)
		return -ENOMEM;
	sptr = ptr;	

	if (alloc_chrdev_region(&ptr->num, 0, 1, "sdev") < 0)
	{
		pr_info("error when alloc chrdev region\n");
		err = -EFAULT;
		goto err1;
	}
	pr_info("dev num is %x\n", ptr->num);

	cdev_init(&ptr->cdev, &sdev_fops);
	spin_lock_init(&ptr->spin);
	if (!(ptr->buf = kmalloc(SSIZE, GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto err2;
	}
	ptr->wp = ptr->rp = ptr->buf;
	if (cdev_add(&ptr->cdev, ptr->num, 1) < 0) {
		err = -EFAULT;
		goto err3;
	}
	pr_info("init done\n");
	return 0;

err3:
	kfree(ptr->buf);
	cdev_del(&ptr->cdev);
err2:
	unregister_chrdev_region(ptr->num, 1);
err1:
	kfree(ptr);
	return err;
}

static void __exit end(void)
{
	kfree(sptr->buf);
	cdev_del(&sptr->cdev);
	unregister_chrdev_region(sptr->num, 1);
	kfree(sptr);
	pr_info("done unregister\n");
}

MODULE_LICENSE("GPL");

module_init(start);
module_exit(end);
	
		
		
