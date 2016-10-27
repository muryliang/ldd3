#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

MODULE_AUTHOR("muryliang@gmail.com");
MODULE_DESCRIPTION("a test cdev");
MODULE_VERSION("v1.0");
MODULE_LICENSE("GPL");

static int major = 0;
static char *devname = "mycdev";
static u64 scull_qset = 1000;
static u64 scull_quantum = 4000;

struct scull_dev {
	dev_t dnum;
	void *data;
	struct semaphore sema;
	struct cdev scdev;
} scull;

static int scull_open(struct inode *inode, struct file *file)
{
	file->private_data = &scull;
	printk("success open\n");
	return 0;
}

static int scull_release(struct inode *inode, struct file *file)
{
	printk("success closed\n");
	return 0;
}

static loff_t scull_llseek(struct file *file, loff_t off, int whence)
{
	loff_t newpos;
	switch(whence) {
	case 0: /*SEEK_SET*/
		newpos = off;
		break;
	case 1: /*SEEK_CUR*/
		newpos = file->f_pos + off;
		break;
	case 2: /*SEEK_END*/
		newpos = scull_quantum * scull_qset + off;
		break;
	default:
		return -EINVAL;
	}
	if (newpos < 0 || newpos > scull_quantum * scull_qset) {
		printk("error seek pos %lu\n", (unsigned long)newpos);
		return -EINVAL;
	}
	file->f_pos = newpos;
	return newpos;
}

static ssize_t scull_read(struct file *file, char __user *buffer, size_t len, loff_t *off)
{
	down(&scull.sema);
	printk("in read ,len: %d, offset: %d\n", (int)len, (int)*off);
	if (*off + len > scull_qset * scull_quantum) {
		printk("error over end read\n");
		return -EINVAL;
	}
	if (copy_to_user(buffer, (char*)scull.data + *off, len))
		return -EINVAL;
	*off += len;
	up(&scull.sema);
	return len;
}

static ssize_t scull_write(struct file *file, const char __user *buffer, size_t len, loff_t *off)
{
	down(&scull.sema);
	printk("in write ,len: %d, offset: %d\n",(int) len,(int) *off);
	if (*off + len > scull_qset * scull_quantum) {
		printk("error over end write\n");
		return -EINVAL;
	}
	if (copy_from_user(scull.data + *off,  buffer, len))
		return -EINVAL;
	*off += len;
	up(&scull.sema);
	return len;
}

static struct file_operations scull_fops = {
	.owner = THIS_MODULE,
	.open = scull_open,
	.release = scull_release,
	.read = scull_read,
	.write = scull_write,
	.llseek = scull_llseek,
};

static int __init start(void)
{
	scull.data = vmalloc(scull_qset * scull_quantum);
	if (scull.data == NULL ) {
		printk("alloc mem for data error");
		return -EFAULT;
	}
	if (major == 0) {
		if (alloc_chrdev_region(&scull.dnum, 0, 1, devname)) {
			printk("error when allocate dev number\n");
			return -EAGAIN;
		}
	}
	else {
		if (register_chrdev_region(MKDEV(major, 0), 1, devname)) {
			printk("error when register major %d\n", major);
			return -EAGAIN;
		}
		scull.dnum = MKDEV(major, 0);
	}
	printk("registered dev num %x\n", scull.dnum);
	sema_init(&scull.sema, 1);
	cdev_init(&scull.scdev, &scull_fops);

	if (cdev_add(&scull.scdev, scull.dnum, 1) < 0)
		unregister_chrdev_region(scull.dnum, 1);
	printk("init done\n");
	return 0;
}

static void __exit end(void)
{
	cdev_del(&scull.scdev);
	unregister_chrdev_region(scull.dnum, 1);
	vfree(scull.data);
	return;
}

module_init(start);
module_exit(end);
