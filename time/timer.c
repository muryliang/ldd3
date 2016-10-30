#include <linux/init.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/wait.h>
#include <linux/timer.h>
#include <linux/hardirq.h>

#define LOOPS 10
struct sdevdata {
	unsigned long prejif;
	char *buf;
	struct timer_list timer;
	int loops;
};

static struct sdevdata data;
static DECLARE_WAIT_QUEUE_HEAD(sdev_wait);

void sdev_timer(unsigned long arg)
{
	struct sdevdata *ptr = (struct sdevdata*)arg;
	unsigned long j = jiffies;
	ptr->buf += sprintf(ptr->buf, "%9li  %3li  %i  %6i %i %s\n",
			j, j - ptr->prejif, in_interrupt() ? 1 : 0,
			current->pid, smp_processor_id(), current->comm);
	if (--ptr->loops) {
		ptr->timer.expires += HZ;
		ptr->prejif = j;
		add_timer(&ptr->timer);
	} else
		wake_up_interruptible(&sdev_wait);
}

static int sdev_proc(char *buf, char **start, off_t off, int count, int *eof, void *dat)
{
	int len;
	unsigned long  jif;
	unsigned long  jif64;
	struct timeval val;
	struct timespec spec;

	init_timer(&data.timer);
	data.prejif = jiffies;
	data.loops = LOOPS;
	data.timer.data = (unsigned long)&data;
	data.timer.function = sdev_timer;
	data.timer.expires =  data.prejif + HZ;
	add_timer(&data.timer);
	if (wait_event_interruptible(sdev_wait, data.loops == 0) < 0)
		return -ERESTARTSYS;
	*start = buf; //this is important to let proc output contingously
	len = data.buf - buf;
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

	

