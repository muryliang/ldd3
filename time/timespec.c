#include <linux/init.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <asm/msr.h>

static int __init start(void)
{
	unsigned long count1, count2,count3;
	struct timespec spec;
	struct timeval val;
	int i;
	count1 = jiffies;
	rdtscll(count2);
	rdtscll(count3);
	printk("count is %lu\n", count1);
	jiffies_to_timespec(count1, &spec);
	jiffies_to_timeval(count1, &val);
	printk("got timespec %lu %lu\n", spec.tv_sec, spec.tv_nsec);
	printk("got timeval %lu %lu\n", val.tv_sec, val.tv_usec);
	printk("got  origin from timespec %lu\n", timespec_to_jiffies(&spec));
	printk("got  origin from timeval %lu\n", timeval_to_jiffies(&val));
	printk("got from jiffies64 %llu\n", get_jiffies_64());
	printk("got tsc %lu %lu\n", count2, count3);
	printk("got cyles %lu %lu\n", get_cycles(), get_cycles());

	return 0;
}

static void __exit end(void)
{
	return;
}

MODULE_LICENSE("GPL");

module_init(start);
module_exit(end);

	

