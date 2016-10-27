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

/*these all can be made as params*/
static int major = 0;
static char *devname = "mycdev";
static u64 scull_qset = 1000;
static u64 scull_quantum = 4000;

struct scull_dev {
	dev_t dnum;
	u64 size;
	void *data;
	struct semaphore sema;
	struct cdev scdev;
} scull;

struct qset {
	struct qset *next;
	void **data;
};

struct qset * scull_follow(struct scull_dev *dev, int idx)
{
	int i;
	struct qset **qtr = (struct qset**)&dev->data;
	struct qset *cur = NULL;
	for (i = 0; i < idx; i++) {
		if (*qtr == NULL)  {
			*qtr = vmalloc(sizeof(struct qset));
			if (*qtr == NULL) 
			 	return ERR_PTR(-EFAULT);
			memset(*qtr, 0, sizeof(struct qset));
		}
		cur = *qtr;
		qtr = &cur->next;
	}
	return cur;
}
	
	
static void scull_trim(struct scull_dev *dev)
{
	struct qset *qtr, *next;
	int i;

	for(qtr = (struct qset*)dev->data; qtr; qtr = next) {
		if (qtr->data) {
			for(i = 0; i < scull_qset; i++) {
				if (qtr->data[i])
					vfree(qtr->data[i]);
			}
			vfree(qtr->data);
		}
		next = qtr->next;
		vfree(qtr);
	}
	dev->data = NULL;
}
			

static int scull_open(struct inode *inode, struct file *file)
{
	file->private_data = &scull;
	if ((file->f_flags & O_ACCMODE) == O_WRONLY) {
		scull_trim(&scull);
		printk("success trimed\n");
	}
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
	down(&scull.sema);
	file->f_pos = newpos;
	up(&scull.sema);
	return newpos;
}
/*
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
*/

static ssize_t scull_read(struct file *file, char __user *buffer, size_t len, loff_t *off)
{
	struct scull_dev *dev = file->private_data;
	struct qset *curset;
	int index, res, pos, offset, count = 0;

	down(&scull.sema);
	printk("in read\n");
	if (*off + len > scull.size) {
		printk("error over end read\n");
		return -EINVAL;
	}

	index = *off / scull.size;
	res = *off % scull.size;
	pos = res / scull_quantum;
	offset = res % scull_quantum;
	if (IS_ERR(curset = scull_follow(dev, index)))
		return -EFAULT;
	if (!curset->data || pos >= scull_qset || !curset->data[pos]) {
		printk("at index: %d, pos %d, offset %d, nothing to read\n", index, pos, offset);
		return -EFAULT;
	}
	count = min((u64)len, (u64)scull_quantum - (u64)offset);
	printk("about to read %d bytes\n", count);
	if (copy_to_user(buffer, curset->data[pos], count))
		return -EAGAIN;
	*off += count;
	up(&scull.sema);
	return count;
}

static ssize_t scull_write(struct file *file,const char __user *buffer, size_t len, loff_t *off)
{
	int index, offset, pos, res, count;
	struct qset *qtr;
	struct scull_dev *dev = file->private_data;
	
	down(&scull.sema);
	index = *off / scull.size;
	res = *off % scull.size;
	pos = res / scull_quantum;
	offset = res % scull_quantum;
	printk("about to write len %llu, offset %llu,"
		"index %d res %d pos %d offset %d\n", (u64)len, (u64)*off,
		index, res, pos, offset);
	if (IS_ERR((qtr = scull_follow(dev, index))))	
	{
		printk("error when follow in write\n");
		return -EFAULT;
	}
	if (pos >= scull_qset) {
		printk("error over qset bondanry\n");
		return -EINVAL;
	}

	if (!qtr->data) {
		qtr->data = vmalloc(scull_qset * sizeof(void*));
		if (!qtr->data) {
			printk("error when alloc for qset->data\n");
			return -EFAULT;
		}
		memset(qtr->data, 0, sizeof(scull_qset * sizeof(void*)));
		printk("success alloc qsets \n");
	}

	if (!qtr->data[pos]) {
		qtr->data[pos] = vmalloc(scull_quantum);
		if (!qtr->data[pos]) {
			printk("error when alloc for quantum\n");
			return -EFAULT;
		}
		memset(qtr->data[pos], 0, sizeof(scull_quantum));
		printk("success alloc quantum for pos %d\n", pos);
	}
	count = min((u64)len, (u64)scull_quantum - (u64)offset);
	printk("about to write %d bytes\n", count);
	if (copy_from_user(qtr->data[pos] + offset, buffer, count)){
		printk("copy from user error\n");
		return -EAGAIN;
	}
	*off += count;
	up(&scull.sema);
	return count;
}
/*
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
*/

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
	memset(&scull, 0, sizeof(scull));
//	scull.data = vmalloc(scull_qset * scull_quantum);
/*	if (scull.data == NULL ) {
		printk("alloc mem for data error\n");
		return -EFAULT;
	}
*/
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
	scull.size = scull_quantum * scull_qset;
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
	scull_trim(&scull);
//	vfree(scull.data);
	return;
}

module_init(start);
module_exit(end);
