#include <linux/init.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/wait.h>
#include <linux/timer.h>
#include <linux/hardirq.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

#define LOOPS 10
struct sdevdata {
	unsigned long prejif;
	char *buf;
	struct timer_list timer;
	struct tasklet_struct t;
	int loops;
	wait_queue_head_t wait;
	struct workqueue_struct *wq;
	struct work_struct wk;
	struct delayed_work dwk;
};

//static struct sdevdata data;

static void sdev_timer(struct work_struct  *work)
{
	struct sdevdata *ptr = container_of(work, struct sdevdata, dwk.work);
	unsigned long j = jiffies;
	ptr->buf += sprintf(ptr->buf, "%9li  %3li  %i  %6i %i %s\n",
			j, j - ptr->prejif, in_interrupt() ? 1 : 0,
			current->pid, smp_processor_id(), current->comm);
	if (--ptr->loops) {
		ptr->prejif = j;
//		tasklet_hi_schedule(&ptr->t);
	//	queue_delayed_work(ptr->wq, &ptr->dwk,10);
		queue_delayed_work(ptr->wq, &ptr->dwk,10);
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
	init_waitqueue_head(&data->wait);
	data->prejif = j;
	data->buf = buf2;
	data->loops = LOOPS;
	data->wq = create_workqueue("swork");
	INIT_DELAYED_WORK(&data->dwk, sdev_timer);
	queue_delayed_work(data->wq, &data->dwk, 10);
	wait_event_interruptible(data->wait, !data->loops);
	if (signal_pending(current))
		return -ERESTARTSYS;

//	*start = buf; //this is important to let proc output contingously
	buf2 = data->buf;
//	kfree(data);
	destroy_workqueue(data->wq);
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

	

