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
	wait_queue_head_t wait;
};

//static struct sdevdata data;

static void sdev_timer(unsigned long arg)
{
	struct sdevdata *ptr = (struct sdevdata*)arg;
	unsigned long j = jiffies;
	ptr->buf += sprintf(ptr->buf, "%9li  %3li  %i  %6i %i %s\n",
			j, j - ptr->prejif, in_interrupt() ? 1 : 0,
			current->pid, smp_processor_id(), current->comm);
	if (--ptr->loops) {
		ptr->timer.expires += 10;
		ptr->prejif = j;
		add_timer(&ptr->timer);
	} else
		wake_up_interruptible(&ptr->wait);
}

static int sdev_proc(char *buf, char **start, off_t off, int count, int *eof, void *unused_dat)
{
	struct sdevdata *data;
	unsigned long j = jiffies;
	char *buf2 = buf;

	data = kmalloc(sizeof(*data), GFP_KERNEL);	
	if (!data)
		return -ENOMEM;
	memset(data, 0, sizeof(struct sdevdata));
	init_timer(&data->timer);
	init_waitqueue_head(&data->wait);

	data->prejif = j;
	data->buf = buf2;
	data->loops = LOOPS;
	data->timer.data = (unsigned long)data;
	data->timer.function = sdev_timer;
	data->timer.expires = j + 10;
	add_timer(&data->timer);
	wait_event_interruptible(data->wait, !data->loops);
	if (signal_pending(current))
		return -ERESTARTSYS;
//	*start = buf; //this is important to let proc output contingously
	buf2 = data->buf;
	kfree(data);
	*eof = 1;
	return buf2 - buf;
}

static int __init start(void)
{
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

	

