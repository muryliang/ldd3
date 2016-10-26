#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define QSET 1000
#define QUANTUM 4000
#define SCULLNAME "scull0"

static int major = 0;
static int minor = 0;
static	dev_t dnum;

struct scull_device {
	int qset;     //every qlist has qset number of pointers
	int quantum;  //every pointer of qset pointer to a range of memory of quantum size(byte)
	struct cdev scullcdev;  // scull's struct cdev
	void *qsptr;  //pointer to struct qlist
};

static struct scull_device sculldev = {
	.qset = QSET,
	.quantum = QUANTUM,
};

static int scull_open(struct inode * inode, struct file *file)
{
	return 0;
}

static int scull_release(struct inode * inode, struct file *file)
{
	return 0;
}

static loff_t scull_llseek(struct file * file, loff_t off, int pos)
{
	return 0;
}

static ssize_t scull_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
	return 0;
}

static ssize_t scull_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
	return 0;
}

static struct file_operations scull_fops = {
	.llseek = scull_llseek,
	.open	= scull_open,
	.release = scull_release,
	.read	= scull_read,
	.write 	= scull_write,
};

static int __init scull_start(void)
{

	/*register devnumber using user specified major minor or dynamically if failed*/
	if (major) {
		if (register_chrdev_region(MKDEV(major,minor), 1, SCULLNAME)) {
			if (alloc_chrdev_region(&dnum, minor, 1, SCULLNAME))
				return -EFAULT;
		} else
			dnum = MKDEV(major, minor);
	}else {
		if (alloc_chrdev_region(&dnum, minor, 1, SCULLNAME))
			return -EFAULT;
	}
	printk("using devnumber %x\n", dnum);
		
	cdev_init(&sculldev.scullcdev, &scull_fops);
	if (cdev_add(&sculldev.scullcdev, dnum, 1)) {
		printk ("register cdev_add failed\n");
		return -ERESTARTSYS;
	}
	return 0;
}

static void __exit scull_end(void)
{
	cdev_del(&sculldev.scullcdev);
	unregister_chrdev_region(dnum, 1);
}

module_init(scull_start);
module_exit(scull_end);
