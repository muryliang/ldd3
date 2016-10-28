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
static u64 scull_qset = 10;
static u64 scull_quantum = 10;

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
/*used in read and write, so have got lock before call this*/
struct qset * scull_follow(struct scull_dev *dev, int idx)
{
	int i;
	struct qset **qtr = (struct qset**)&dev->data;
	struct qset *cur = NULL;
	for (i = 0; i <= idx; i++) {
		pr_info("traverse index %d\n", i);
		if (*qtr == NULL)  {
			*qtr = vmalloc(sizeof(struct qset));
			if (IS_ERR(*qtr)) {
				printk("error when vmalloc for idx %d,in follow\n", idx);
			 	return ERR_PTR(-EFAULT);
			}
			memset(*qtr, 0, sizeof(struct qset));
//			(*qtr)->next = NULL;
//			(*qtr)->data = NULL;
			pr_info("idex %d alloc success\n", i);
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

	down(&scull.sema);
	pr_info("in trim\n");
	for(qtr = (struct qset*)dev->data; qtr; qtr = next) {
		pr_info("for qtr %lx\n", (unsigned long)qtr);
		if (qtr->data) {
			for(i = 0; i < scull_qset; i++) {
				if (qtr->data[i]) {
					pr_info("pos %d has addr %lx,free it\n", i, (unsigned long)qtr->data[i]);
					vfree(qtr->data[i]);
				}
				qtr->data[i] = NULL;
			}
			pr_info("free %lx\n", (unsigned long)qtr);
			vfree(qtr->data);
			qtr->data = NULL; //this is important, otherwise will get invalid old address!!
		}
		next = qtr->next;
		pr_info("get next %lx, about to free qtr\n", (unsigned long)next);
		vfree(qtr);
	}
	dev->data = NULL;
	up(&scull.sema);
	printk("trim success\n");
}
			

static int scull_open(struct inode *inode, struct file *file)
{
	file->private_data = &scull;
	if ((file->f_flags & O_ACCMODE) == O_WRONLY) {
		scull_trim(&scull);
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
	pr_info("in seek:fpos %d, off %d, whence %d\n", (int)file->f_pos, (int)off, (int)whence);
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
	if (newpos < 0) {
		printk("error seek pos %lu\n", (unsigned long)newpos);
		return -EINVAL;
	}
	pr_info("new pos %d\n", (int)newpos);
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
	/*
	if (*off + len > scull.size) {
		printk("error over end read\n");
		return -EINVAL;
	}
	*/

	index = *off / scull.size;
	res = *off % scull.size;
	pos = res / scull_quantum;
	offset = res % scull_quantum;
	printk("about to read len %llu, offset %llu,"
		"index %d res %d pos %d offset %d\n", (u64)len, (u64)*off,
		index, res, pos, offset);
	if (IS_ERR(curset = scull_follow(dev, index))) {
		up(&scull.sema);
		return -EFAULT;
	}
	pr_info("in read, get qtr %lx\n", (unsigned long)curset);
	if (!curset->data || pos >= scull_qset || !curset->data[pos]) {
		printk("nothing to read:at index: %d, pos %d, offset %d\n", index, pos, offset);
		up(&scull.sema);
		return -EFAULT;
	}
	pr_info("in read, get qtr->data[pos], pos:%d, ptr->data[pos]: %lx\n", pos, (unsigned long)curset->data[pos]);
	count = min((u64)len, (u64)scull_quantum - (u64)offset);
	printk("about to read %d bytes,done with pos %d\n", count, pos);
	if (copy_to_user(buffer, curset->data[pos] + offset, count)) {
		up(&scull.sema);
		return -EAGAIN;
	}
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
		up(&scull.sema);
		return -EFAULT;
	}
	pr_info("in write, get qtr %lx\n", (unsigned long)qtr);
	if (pos >= scull_qset) {
		printk("error over qset bondanry\n");
		up(&scull.sema);
		return -EINVAL;
	}

	if (!qtr->data) {
		qtr->data = vmalloc(scull_qset * sizeof(void*));
		if (IS_ERR(qtr->data)) {
			printk("error when alloc for qset->data\n");
			up(&scull.sema);
			return -EFAULT;
		}
		memset(qtr->data, 0, scull_qset * sizeof(void*));
		printk("success alloc qsets \n");
	}
	pr_info("in write, get qtr->data %lx\n", (unsigned long)qtr->data);

	if (!qtr->data[pos]) {
		qtr->data[pos] = vmalloc(scull_quantum);
		if (IS_ERR(qtr->data[pos])) {
			printk("error when alloc for quantum\n");
			up(&scull.sema);
			return -EFAULT;
		}
		memset(qtr->data[pos], 0, scull_quantum);
		printk("success alloc quantum for pos %d\n", pos);
	}
	pr_info("in write, get qtr->data[pos], pos:%d, ptr->data[pos]: %lx\n", pos, (unsigned long)qtr->data[pos]);
	pr_info("in write, and  qtr->data[pos+1], pos:%d, ptr->data[pos+1]: %lx\n", pos+1, (unsigned long)qtr->data[pos+1]);
	count = min((u64)len, (u64)scull_quantum - (u64)offset);
	printk("about to write %d bytes,done with pos %d\n", count, pos);
	if (copy_from_user(qtr->data[pos] + offset, buffer, count)){
		printk("copy from user error\n");
		up(&scull.sema);
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
	memset(&scull, 0, sizeof(struct scull_dev));
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
	return;
}

module_init(start);
module_exit(end);
