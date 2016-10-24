#include <linux/init.h>
#include <linux/module.h>

MODULE_AUTHOR("muryliang@gmail.com");
MODULE_DESCRIPTION("this is a test module");
MODULE_LICENSE("GPL");

static int __init start(void)
{
	printk("hello\n");
	return 0;
}

static void __exit end(void)
{
	printk("end\n");
	return ;
}

module_init(start);
module_exit(end);
