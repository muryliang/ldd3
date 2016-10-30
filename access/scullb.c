#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/poll.h>

MODULE_LICENSE("GPL");

#define DEVSIZE 42
#define DEVNAME "scullb"
#define  per(fmt,...)  do{ \
printk(fmt,##__VA_ARGS__); \
return -EFAULT; \
} while(0)

//static int flag = 0;
static int user_count = 0;
static int user_owner = -1;
static DECLARE_WAIT_QUEUE_HEAD(sdev_wait);

struct scullb {
	wait_queue_head_t inq, outq;
	char *buf, *end;
	char *rp, *wp;
	int nreaders, nwriters;
	spinlock_t spin;
	struct semaphore sem;
	dev_t dnum;
	int bufsize;
	struct cdev cdev;
	struct fasync_struct *async_queue;
} sdev;

static int sdev_available(void) 
{
	return !(user_count && ! capable(CAP_DAC_OVERRIDE) && user_owner != current_uid() &&
		user_owner != current_euid());
}

static int sdev_open(struct inode *inode, struct file *file)
{
	spin_lock(&sdev.spin);
	while (!sdev_available()) {
		spin_unlock(&sdev.spin);
		if (file->f_flags & O_NONBLOCK) return -EAGAIN;
		if (wait_event_interruptible(sdev_wait, sdev_available()))
			return -ERESTARTSYS;
		spin_lock(&sdev.spin);
	}
	if (user_count == 0)
		user_owner = current_uid();
	user_count++;
	spin_unlock(&sdev.spin);
	file->private_data = &sdev;
	printk("success open\n");
	return 0;
}

static int sdev_fasync(int fd, struct file *file, int mode);

static int sdev_close(struct inode *inode , struct file *file)
{
	int tmp;
	file->private_data = NULL;
	sdev_fasync(-1, file, 0);
	spin_lock(&sdev.spin);
	user_count--;
	tmp = user_count;
	spin_unlock(&sdev.spin);
	if (tmp == 0)
		wake_up_interruptible(&sdev_wait);
	printk("success close\n");
	return 0;
}

static ssize_t sdev_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
	size_t count = len;
	pr_info("%i (%s)read wakedup\n", current->pid, current->comm);
	if(down_interruptible(&sdev.sem)) {
//	if(down_trylock(&sdev.sem)) {
		pr_info("some thing wrong in write\n");
		return -ERESTARTSYS;
	}
	while(sdev.rp == sdev.wp) { // nothing to read , so wait
		up(&sdev.sem);
		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;
		pr_info("%s going to sleep\n", current->comm);
		if (wait_event_interruptible(sdev.inq, sdev.rp != sdev.wp))
			return -ERESTARTSYS;  // signal happened
		pr_info("read wake again from sleep\n");
		if (down_interruptible(&sdev.sem))
			return -ERESTARTSYS;
	}
	pr_info("got things to read\n");
	if (sdev.rp < sdev.wp)   // at most read to end
		count = min(count, (size_t)(sdev.wp - sdev.rp));
	else
		count = min(count, (size_t)(sdev.end - sdev.rp));
	if (copy_to_user(buf, sdev.rp, count) < 0) {
		up(&sdev.sem);
		return -EFAULT;
	}
	sdev.rp += count;
	if (sdev.rp == sdev.end)
		sdev.rp = sdev.buf;
	up(&sdev.sem);
	pr_info("going to wakeup write\n");
	pr_info("%s did read %li bytes\n", current->comm, count);
	wake_up_interruptible(&sdev.outq);
	return count;
}

static ssize_t have_space(struct scullb *dev)
{
	if (dev->wp == dev->rp)
		return dev->bufsize - 1;
	return (dev->wp - dev->rp + dev->bufsize) % dev->bufsize;
}

static ssize_t sdev_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
	size_t count = len;
	int ret;
	pr_info("%i(%s) write waked up\n", current->pid, current->comm);
	if (down_interruptible(&sdev.sem)) {
//	if (down_trylock(&sdev.sem)) {
		pr_info("some thing wrong in write\n");
		return  -ERESTARTSYS;
	}
	while(!have_space(&sdev)) { // if buf is full
		DEFINE_WAIT(wait);
		up(&sdev.sem);
		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;
		prepare_to_wait(&sdev.outq, &wait, TASK_INTERRUPTIBLE);
		if (!have_space(&sdev))
			schedule();
		finish_wait(&sdev.outq, &wait);
		if (signal_pending(current))
			return -ERESTARTSYS;
		if (down_interruptible(&sdev.sem))
			return -ERESTARTSYS;
	}
	pr_info("now have space to write\n");
	count = min(count, (size_t)have_space(&sdev));
	if (sdev.wp >= sdev.rp) 
		count = min(count, (size_t)(sdev.end - sdev.wp));
	else
		count = min(count, (size_t)(sdev.rp - sdev.wp -1));
	if ((ret = copy_from_user(sdev.wp, buf, count) )< 0) {
		up(&sdev.sem);
		return -EFAULT;
	}
	sdev.wp += count;
	if (sdev.wp == sdev.end)
		sdev.wp = sdev.buf;
	pr_info("%s finish write %li bytes, about to wakeup read\n", current->comm, count);
	up(&sdev.sem);
	wake_up_interruptible(&sdev.inq);
	if (sdev.async_queue)
		kill_fasync(&sdev.async_queue, SIGIO, POLL_IN);
	return count;
}

static unsigned int sdev_poll(struct file *file, struct poll_table_struct * table) 
{
	int mask = 0;
	if (down_interruptible(&sdev.sem) )
		return -ERESTARTSYS;
	poll_wait(file, &sdev.inq, table);
	poll_wait(file, &sdev.outq, table);
	if (sdev.rp != sdev.wp) {
		mask |= POLLIN | POLLRDNORM;
		pr_info("poll happened read\n");
	}
	if (have_space(&sdev)) {
		mask |= POLLOUT | POLLWRNORM;
		pr_info("poll happened write\n");
	}
	up(&sdev.sem);
	return mask;
}

static int sdev_fasync(int fd, struct file *file, int mode)
{
	return fasync_helper(fd, file, mode, &sdev.async_queue);
}

static struct file_operations sdev_fops = {
	.owner = THIS_MODULE,
	.read = sdev_read,
	.write = sdev_write,
	.open = sdev_open,
	.release = sdev_close,
	.poll = sdev_poll,
	.fasync = sdev_fasync,
};

static int __init start(void)
{
	memset(&sdev, 0, sizeof(struct scullb));
	init_waitqueue_head(&sdev.inq);
	init_waitqueue_head(&sdev.outq);
	if (alloc_chrdev_region(&sdev.dnum, 0, 1, DEVNAME) < 0)
		per("can not alloc devnum\n");
	pr_info("devnum is %x\n", sdev.dnum);
	sdev.bufsize = DEVSIZE;
	if (IS_ERR(sdev.buf = vmalloc(DEVSIZE))) {
		unregister_chrdev_region(sdev.dnum, 1);
		per("can not alloc buf\n");
	}
	sdev.end = sdev.buf + DEVSIZE;
	sdev.rp = sdev.wp = sdev.buf;
	sdev.nreaders = sdev.nwriters = 0;
	init_MUTEX(&sdev.sem);
	spin_lock_init(&sdev.spin);
	cdev_init(&sdev.cdev, &sdev_fops);
	if (cdev_add(&sdev.cdev, sdev.dnum, 1) < 0) {
		vfree(sdev.buf);
		unregister_chrdev_region(sdev.dnum, 1);
		per("can not add chrdev\n");
	}
	printk("init done\n");
	return 0;
}

static void __exit end(void)
{
	cdev_del(&sdev.cdev);
	vfree(sdev.buf);
	unregister_chrdev_region(sdev.dnum, 1);
	printk("exit done\n");
}

module_init(start);
module_exit(end);

