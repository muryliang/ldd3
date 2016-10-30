#include <linux/init.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/sched.h>

static int __init start(void)
{
	unsigned long count1, count2;
	int i;
	count1 = jiffies;
	set_task_state(current, TASK_INTERRUPTIBLE);
	schedule_timeout(1);	
	count2 = jiffies;

	if (time_after(count2, count1) > 0 )
		printk("%lu after %lu\n", count2, count1);
	else if(time_before(count2, count1) > 0)
		printk("impossible\n");
	else
		printk("maybe\n");

	printk("count1 %lu, count2 %lu\n", count1, count2);
	return 0;
}

static void __exit end(void)
{
	return;
}

MODULE_LICENSE("GPL");

module_init(start);
module_exit(end);

	

