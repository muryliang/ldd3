#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/mm.h>

static char *buf;
static int sdev_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{

	char *p = (char*)data;
	int len;
	p[0] = 'h';
	p[1] = 'a';
	p[2] = 'b';
	p[3] = '\0';
	len = sprintf(page, "%s\n", p);
	*eof = 1;
	return len;
}
	
static int __init start(void)
{
	char *buf = kmalloc(20, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;
	if (!create_proc_read_entry("sdev", 0, NULL, sdev_proc, buf))
		return -EFAULT;
	printk("start\n");
	return 0;
}

static void __exit end(void)
{
	kfree(buf);
	remove_proc_entry("sdev",NULL);
	printk("end \n");
}

MODULE_LICENSE("GPL");

module_init(start);
module_exit(end);
	
	
