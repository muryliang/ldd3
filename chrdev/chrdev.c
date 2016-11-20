#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
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
	char *km; /*kmalloc*/
	char *vm; /*vmalloc*/
	char *pvm; /*__get_free_pages*/
	struct page *mp; /*alloc_pages*/
	struct kmem_cache *kmc;
	struct mycdev *tcdev;
	unsigned long pfn;
	char *cpcpu = alloc_percpu(char);
	char *tmp;
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

	pr_info("pring percpu pos\n");
//	pr_info("load: %p, start %p, end %p\n", __per_cpu_load, __per_cpu_start, __per_cpu_end);
	pr_info("pcpu base addr : %p\n", pcpu_base_addr);
	pr_info("NR_CPUS %d\n", NR_CPUS);
	pr_info("offset  %lx %lx %lx %lx\n", __per_cpu_offset[0], __per_cpu_offset[1], __per_cpu_offset[2], __per_cpu_offset[3]);
	pr_info("offset  %lx %lx %lx %lx\n", per_cpu(this_cpu_off, 0), per_cpu(this_cpu_off, 1), per_cpu(this_cpu_off, 2),  per_cpu(this_cpu_off,3));
	pr_info("percpu var is %d\n", get_cpu_var(mpcpu));
	put_cpu_var(mpcpu);
	get_cpu_var(mpcpu) += 2;
	put_cpu_var(mpcpu);
	pr_info("percpu var is %d\n", get_cpu_var(mpcpu));
	pr_info("the cpu now is %d\n", smp_processor_id());
	put_cpu_var(mpcpu);

	get_cpu();
	tmp = per_cpu_ptr(cpcpu, 0);
	*tmp = 'a';
	tmp = per_cpu_ptr(cpcpu, 2);
	*tmp = 'b';
	put_cpu();
	pr_info("per cpu ptr for 0: %c, 2: %c, 1: %x\n", *per_cpu_ptr(cpcpu, 0), *per_cpu_ptr(cpcpu, 2), *per_cpu_ptr(cpcpu, 1));
	pr_info("begin disabled irq for cpu %d\n", smp_processor_id());

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

	spinlock_t a;
	rwlock_t b;
	struct semaphore sem;
	seqlock_t seq;
	unsigned  start;
	atomic_t aa = ATOMIC_INIT(2);

	seqlock_init(&seq);
	spin_lock_init(&a);
	rwlock_init(&b);

	mdev = do_chrdev_init();
	if (IS_ERR(mdev))
		return -EFAULT;
	pr_info("success alloc chrdev with devnum %x\n", mdev->num);
	pr_info("start disable local irq\n");
	local_irq_disable();
	pr_info("now disabled on cpu %d\n", smp_processor_id());
	local_irq_enable();
	spin_lock(&a);
	pr_info("in spinlock\n");
	if (spin_trylock(&a) == 0)
		pr_info("in spinlock try lock failed\n");
	spin_unlock(&a);

	pr_info("lock write rwlock\n");
	write_lock(&b);
	write_unlock(&b);
	read_lock(&b);
	pr_info("read lock success\n");
	read_unlock(&b);
	pr_info("again begin write lock\n");
	write_lock(&b);
	write_unlock(&b);

	sema_init(&sem, 1);
	if (down_interruptible(&sem) ==  0)
		pr_info("down success\n");
	up(&sem);
	pr_info("up success\n");
	pr_info("begin seq read\n");
	do {
		start = read_seqbegin(&seq);
		pr_info("now begin write seq lock\n");
//		write_seqlock(&seq);
//		pr_info("now in seq write\n");
//		write_sequnlock(&seq);
	} while(read_seqretry(&seq, start));

	pr_info("atomic read return %d\n", (int)atomic_read(&aa));
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

