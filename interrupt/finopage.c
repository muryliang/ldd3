#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/uaccess.h>

#define SSIZE 40

struct sdev {
	char *buf;
	char *wp, *rp, *end;
	dev_t num;
	int size;
	struct cdev cdev;
	struct semaphore sem;
};

DECLARE_WAIT_QUEUE_HEAD(waitq);
EXPORT_SYMBOL_GPL(waitq);

static struct sdev *sptr;

static int sdev_open(struct inode *inode, struct file *file)
{
	pr_info("in open\n");
	file->private_data = (struct sdev *)container_of(inode->i_cdev, struct sdev, cdev);
	return 0;
}

static int sdev_release(struct inode *inode, struct file *file)
{
	pr_info("in release\n");
	return 0;
}

static size_t have_read(struct sdev *ptr)
{
	if (ptr->rp < ptr->wp)
		return ptr->wp - ptr->rp;  // write ahead read
	else if (ptr->rp  == ptr->wp)
		return 0;        //nothing to read
	else
		return ptr->end - ptr->rp; //write round
}

static ssize_t sdev_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
	struct sdev *ptr = (struct sdev *)file->private_data;
	size_t count = 0;

	pr_info("in read\n");
	if (down_interruptible(&ptr->sem) < 0)
		return -ERESTARTSYS;
	while (!(count = have_read(ptr))) {
		pr_info("read begin to wait\n");
		up(&ptr->sem);
		wait_event(waitq, have_read(ptr) != 0);
		if (down_interruptible(&ptr->sem) < 0)
			return -ERESTARTSYS;
		pr_info("wait done\n");
	}

	count = min(count, len);
	count = min(count, (size_t)(ptr->end - ptr->rp));
	if (copy_to_user(buf, ptr->rp, count) < 0)
		return -EAGAIN;
	ptr->rp += count;
	if (ptr->rp == ptr->end)
		ptr->rp = ptr->buf;

	up(&ptr->sem);
	return count;
}

static ssize_t sdev_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
	size_t count;
	int err;
	struct sdev *ptr = (struct sdev *)file->private_data;
	pr_info("in write\n");
	if (down_interruptible(&ptr->sem) < 0)
		return -ERESTARTSYS;
	count = min(len, (size_t)(ptr->end - ptr->wp));
	if ((err = copy_from_user(ptr->wp, buf, count)) < 0)
		return -EAGAIN;
	ptr->wp += count;
	if (ptr->wp == ptr->end)
		ptr->wp = ptr->buf;
	up(&ptr->sem);
	wake_up(&waitq);
	pr_info("wakeup the reader\n");
	return count;
}

static void sdev_vm_open(struct vm_area_struct *vma)
{
	pr_info("in vm open\n");
}

static void sdev_vm_close(struct vm_area_struct *vma)
{
	pr_info("in vm close\n");
}

static int sdev_vm_nopage(struct vm_area_struct *vma,
				struct vm_fault *vmf)
{
	struct page *pageptr;
	void *ptr = (void*)((unsigned long)sptr->buf & PAGE_MASK);
//	unsigned long pageframe = vma->vm_pgoff + vmf->pgoff;

//	if (!pfn_valid(pageframe))
//		return -EFAULT;
//	pageptr = pfn_to_page(pageframe);
	pageptr = virt_to_page(ptr);
	get_page(pageptr);
	/*
	if (type)
		*type = VM_FAULT_MINOR;
		*/
	vmf->page = pageptr;
	return 0;
}

static struct vm_operations_struct  sdev_vm_ops = {
	.open = sdev_vm_open,
	.close = sdev_vm_close,
	.fault = sdev_vm_nopage,
};

static int sdev_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;


	if (offset >= __pa(high_memory) || (file->f_flags & O_SYNC))
		vma->vm_flags |= VM_IO;
	vma->vm_flags |= VM_RESERVED;

	vma->vm_ops = &sdev_vm_ops;
	sdev_vm_open(vma);
	return 0;
}

				

static struct file_operations sdev_fops =  {
	.open = sdev_open,
	.release = sdev_release,
	.read = sdev_read,
	.write = sdev_write,
	.mmap = sdev_mmap,
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
	sema_init(&ptr->sem, 1);
	if (!(ptr->buf = (char*)__get_free_page(GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto err2;
	}
	pr_info("buf's vritual address is %lx\n", (unsigned long)ptr->buf);
	ptr->wp = ptr->rp = ptr->buf;
	ptr->end = ptr->buf + SSIZE;
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
	free_page(sptr->buf);
	cdev_del(&sptr->cdev);
	unregister_chrdev_region(sptr->num, 1);
	kfree(sptr);
	pr_info("done unregister\n");
}

MODULE_LICENSE("GPL");

module_init(start);
module_exit(end);
	
		
		
