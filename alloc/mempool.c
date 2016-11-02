#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/mm.h>
#include <linux/mempool.h>

static struct kmem_cache *kmem;
static mempool_t * mp;

struct sdev {
	int a;
	char b;
	short c;
};
static int sdev_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{

	mempool_t *ptr = (mempool_t *)data;
	int len;
	struct sdev *sptr = mempool_alloc(ptr, GFP_KERNEL);
	if (!sptr)
		return -ENOMEM;
	sptr->a = 2;
	sptr->b = 'a';
	sptr->c = 10;
	len = sprintf(page, "%d %c %hd\n", sptr->a, sptr->b, sptr->c);
	mempool_free(sptr, ptr);
	*eof = 1;
	return len;
}
	
static int __init start(void)
{
	kmem = kmem_cache_create("mycache", sizeof(struct sdev), 0, SLAB_HWCACHE_ALIGN, NULL);
	if (!kmem)
		return -ENOMEM;
	mp = mempool_create(20, mempool_alloc_slab, mempool_free_slab, kmem);
	if (!mp)
		return -ENOMEM;
	if (!create_proc_read_entry("sdev", 0, NULL, sdev_proc, mp))
		return -EFAULT;
	printk("start\n");
	return 0;
}

static void __exit end(void)
{
	mempool_destroy(mp);
	kmem_cache_destroy(kmem);
	remove_proc_entry("sdev",NULL);
	printk("end \n");
}

MODULE_LICENSE("GPL");

module_init(start);
module_exit(end);
	
	
