#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");

#define SDEVSIZE 42
#define DEVNAME "scullb"
#define  per(fmt,...)  do{ \
printk(fmt,##__VA_ARGS__); \
return -EFAULT; \
} while(0)

DECLARE_WAIT_QUEUE_HEAD(rwait);
//DECLARE_WAIT_QUEUE_HEAD(wwait);
static int flag = 0;

struct scullb {
	void *buf;
	int read;
	int write;
	int size;
	dev_t dnum;
	struct cdev scdev;
} sdev;

static int sdev_open(struct inode *inode, struct file *file)
{
	file->private_data = &sdev;
	printk("success open\n");
	return 0;
}

static int sdev_close(struct inode *inode , struct file *file)
{
	file->private_data = NULL;
	printk("success close\n");
	return 0;
}

static ssize_t sdev_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
	int count, sum = 0;
	pr_info("process %i (%s) going to sleep\n", current->pid, current->comm);
	wait_event_interruptible(rwait, flag != 0);
	flag = 0;
	if (sdev.write > sdev.read ) // have something to read, and not round
	{
		count = sdev.write - sdev.read;
		if (count > len)
			len = count;
		if (copy_to_user((void*)buf, (void*)(sdev.buf + sdev.read), count) < 0)
			per("can not read from %d for %d bytes\n", sdev.read, count);
		sdev.read += count;
		sum = count;
		pr_info("read from %lx pos %d, count %d\n",(unsigned long)(sdev.buf), sdev.read,count);
		printk("all read %d bytes\n", sum);
		goto done;
	}
	else if (sdev.write < sdev.read)  //read around needed
	{
		printk("in another read\n");
		count = sdev.size - sdev.read;
		if (count > len)
			count = len;
		if (copy_to_user((void*)buf, (void*)(sdev.buf + sdev.read), count) < 0)
			per("can not read from %d for %d bytes\n", sdev.read, count);
		if (len < sdev.size - sdev.read) {
			sum = count;
			sdev.read += count;
			goto done;
		}

		sdev.read = 0;
		sum = count;
		count = sdev.write;
		if (count > len - sum)
			count = len - sum;
		if (count >= sdev.write)
			count = sdev.write -1;
		count = max(count,0);
		if (copy_to_user((void*)(buf + sum), (void*)sdev.buf, count) < 0)
			per("can not read round from 0 for %d\n", count);
		sum += count;
		sdev.read += count;
		printk("all read %d bytes\n", sum);
		goto done;
	}
	else 
		printk("nothing to read\n");
done:		
	pr_info("awoken %i (%s) \n", current->pid, current->comm);
	return sum;
}

static ssize_t sdev_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
	int count;
	int sum;
	pr_info("process %i(%s) going to write\n", current->pid, current->comm);
	flag = 1;
	if (access_ok(VERIFY_READ, buf, len)){
		printk("len: %d,got string: %s\n",len, buf);
	}
	if (len < sdev.size - sdev.write) {
		count = len;
//		if (sdev.write < sdev.read && sdev.write + len >= sdev.read)
//			count = sdev.read - sdev.write - 1;
		if (copy_from_user((void*)(sdev.buf + sdev.write), (void*)buf, count) < 0)
			per("can not copy from user in first branch\n");
		sdev.write += count;
		sum = count;
		pr_info("written from %lx pos %d, count %d\n",(unsigned long)(sdev.buf), sdev.write,count);
		goto done;
	}
	else 
	{
		printk("in another write\n");
		count = sdev.size - sdev.write;
		if (copy_from_user((void*)(sdev.buf + sdev.write), (void*)buf, count) < 0)
			per("can not copy from user in first branch\n");
		sdev.write = 0;
		sum = count;
		count = len - count;
		if (count > sdev.size)
			count = sdev.size;
		if (copy_from_user((void*)sdev.buf, (void*)(buf + sum), count) < 0)
			per("can not round write\n");
		sum += count;
		sdev.write = count;
		goto done;
	}
done:
	pr_info("write done ,wakup read\n");
	wake_up_interruptible(&rwait);
	return sum;
}
	
static struct file_operations sdev_fops = {
	.owner = THIS_MODULE,
	.read = sdev_read,
	.write = sdev_write,
	.open = sdev_open,
	.release = sdev_close,
};

static int __init start(void)
{
	if (alloc_chrdev_region(&sdev.dnum, 0, 1, DEVNAME) < 0)
		per("can not alloc devnum\n");
	pr_info("devnum is %x\n", sdev.dnum);
	sdev.read = 0;
	sdev.write = 0;
	sdev.size = SDEVSIZE;
	if (IS_ERR(sdev.buf = vmalloc(SDEVSIZE))) {
		unregister_chrdev_region(sdev.dnum, 1);
		per("can not alloc buf\n");
	}
	cdev_init(&sdev.scdev, &sdev_fops);
	if (cdev_add(&sdev.scdev, sdev.dnum, 1) < 0) {
		vfree(sdev.buf);
		unregister_chrdev_region(sdev.dnum, 1);
		per("can not add chrdev\n");
	}
	printk("init done\n");
	return 0;
}

static void __exit end(void)
{
	cdev_del(&sdev.scdev);
	vfree(sdev.buf);
	unregister_chrdev_region(sdev.dnum, 1);
	printk("exit done\n");
}

module_init(start);
module_exit(end);

