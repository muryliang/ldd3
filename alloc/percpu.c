#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/mm.h>
#include <linux/mempool.h>
#include <linux/percpu.h>

static struct kmem_cache *kmem;
static mempool_t * mp;
DEFINE_PER_CPU(int, sdevcpu);

struct sdev {
	int a;
	char b;
	short c;
};
static int sdev_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{

	int len;
	int *ptr = alloc_percpu(int);
	int cpu = get_cpu();
	int *ptr2 = per_cpu_ptr(ptr, cpu);
	*ptr2 = 10;
	put_cpu();


	get_cpu_var(sdevcpu)++;
	put_cpu_var(sdevcpu);
	len = sprintf(page, "%d %d\n", *(int*)per_cpu_ptr(ptr,cpu), (int)per_cpu(sdevcpu, cpu));
	*eof = 1;
	return len;
}
	
static int __init start(void)
{
	if (!create_proc_read_entry("sdev", 0, NULL, sdev_proc, mp))
		return -EFAULT;
	printk("start\n");
	return 0;
}

static void __exit end(void)
{
	remove_proc_entry("sdev",NULL);
	printk("end \n");
}

MODULE_LICENSE("GPL");

module_init(start);
module_exit(end);
	
	
