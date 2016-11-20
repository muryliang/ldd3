#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>

static int major = 0;
module_param(major, int, 0600);
MODULE_PARM_DESC(major, "the major of  device");


static char *name = "mdev";

struct mycdev {
	struct cdev cdev;
	dev_t  num;
};

struct mycdev *mdev;

static int mdev_open(struct inode *inode, struct file *file)
{
	char *km; /*kmalloc*/
	char *vm; /*vmalloc*/
	char *pvm; /*__get_free_pages*/
	struct page *mp; /*alloc_pages*/
	struct kmem_cache *kmc;
	struct mycdev *tcdev;
	unsigned long pfn;
	file->private_data = mdev;
	pr_info("in open\n");
	pr_info("test kmem cache");
	kmc = kmem_cache_create("mycache", sizeof(struct mycdev), 0, SLAB_HWCACHE_ALIGN, NULL);
	if (IS_ERR(km))
		return -ENOMEM;
	tcdev = kmem_cache_alloc(kmc, GFP_KERNEL);
	if (IS_ERR(tcdev))
		return -ENOMEM;
	tcdev->num = 20;
	pr_info("success alloc slab\n");
	kmem_cache_free(kmc, tcdev);
	kmem_cache_destroy(kmc);

	pr_info("begin vmalloc\n");
	vm = vmalloc(100);
	if (IS_ERR(vm))
		return -ENOMEM;
	vfree(vm);
	
	pr_info("begin kmalloc\n");
	km = kmalloc(200, GFP_ATOMIC);
	if (IS_ERR(km))
		return -ENOMEM;
	pr_info("got kmalloc addr %p\n", km);
	kfree(km);

	pr_info("begin getfreepage\n");
	pvm = (char*)__get_free_pages(GFP_NOIO, 2);
	if (!pvm)
		return -ENOMEM;
	pr_info("got get free pages addr %lx and phy addr %lx\n", (unsigned long)pvm, __pa((unsigned long)pvm));
	
	free_pages((unsigned long)pvm, 2);

	pr_info("begin alloc pages\n");
	mp = alloc_pages(GFP_DMA, 3);
	if (!mp)
		return -ENOMEM;
	pfn = page_to_pfn(mp);
	pr_info("get struct page from highmem pfn is %ld\n", pfn);
	__free_pages(mp, 3);

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

