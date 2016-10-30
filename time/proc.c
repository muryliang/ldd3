#include <linux/init.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/wait.h>


static int sdev_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len;
	unsigned long  jif;
	unsigned long  jif64;
	struct timeval val;
	struct timespec spec;
//	unsigned long  j1 = jiffies + HZ;

/* // using wait interruptible
	DECLARE_WAIT_QUEUE_HEAD(wait);
	if (wait_event_interruptible_timeout(wait, 0, HZ) < 0)
		return -ERESTARTSYS;
*/

/*	//using schedule_timeout
	set_task_state(current, TASK_INTERRUPTIBLE);
	schedule_timeout(HZ);
*/
/*  //using just schedule and time_berfore
	while (time_before(jiffies, j1))
		schedule();
*/
/*
	while(time_before(jiffies, j1))
		cpu_relax();
*/
//	mdelay(500);
	jif = jiffies;
	jif64 = get_jiffies_64();
	do_gettimeofday(&val);
	spec = current_kernel_time();
	len = sprintf(buf,"%#8lx %#16lx %ld.%ld\n%ld.%ld\n", 
			jif, jif64, val.tv_sec, val.tv_usec, spec.tv_sec, spec.tv_nsec);
	*start = buf; //this is important to let proc output contingously
	*eof = 1;
	return len;
}

static int __init start(void)
{
	unsigned long count;
	count = jiffies;
	if (create_proc_read_entry("sdev", 0, NULL, sdev_proc, NULL) == NULL) {
		printk("error when create proc entry\n");
		return -EFAULT;
	}


	return 0;
}

static void __exit end(void)
{
	remove_proc_entry("sdev", NULL);
	return;
}

MODULE_LICENSE("GPL");

module_init(start);
module_exit(end);

	

