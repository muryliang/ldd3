#include <linux/init.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

static char buf[1];

static void * sdev_start(struct seq_file *m , loff_t *pos)
{

	return buf;
}

static void *sdev_next(struct seq_file *m, void *v, loff_t *pos)
{
	return NULL;
}

static void sdev_stop(struct seq_file *m, void *v)
{
	set_task_state(current, TASK_INTERRUPTIBLE);
	schedule_timeout(HZ);
}

static int sdev_show(struct seq_file *m, void *v)
{
	seq_printf(m, "abcdefghijklmnopqrs\n");
	return 0;
}

static struct seq_operations sdev_ops = {
	.start = sdev_start,
	.next = sdev_next,
	.stop = sdev_stop,
	.show = sdev_show,
};

static int sdev_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &sdev_ops);
}

static struct file_operations sdev_fops = {
	.owner = THIS_MODULE,
	.open = sdev_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

static int sdev_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len ;
	len = sprintf(buf, "in sdev_proc, page %lx, start %lx, off %d, count %d, eof %lx\n",
			(unsigned long)buf, (unsigned long)start, (int)off, count, (unsigned long)eof);
	*eof = 1;
	return len ;
}

static int __init start(void)
{
	unsigned long count;
	struct proc_dir_entry  *entry = create_proc_entry("sdev", 0, NULL);
	if (entry)
		entry->proc_fops = &sdev_fops;
	return 0;
}

static void __exit end(void)
{
	remove_proc_entry("fs/sdev", NULL);
	return;
}

MODULE_LICENSE("GPL");

module_init(start);
module_exit(end);

	

