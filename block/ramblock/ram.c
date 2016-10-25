#include <linux/init.h>
#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/fs.h>

MODULE_AUTHOR("muryliang@gmail.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("a ram block device");

static int major = 0;
static char *blk_name = "myblk";
static struct gendisk *gd;
static spinlock_t slock;
static char flash[32*512];

static void blk_transfer(unsigned long sector, unsigned long nsectors, char *buffer, int write)
{
	int read = !write;
	printk("in blk_transfer: sector: %d, nsectors: %d, write: %d\n", sector, nsectors, write);
	if (read)
		memcpy(buffer, flash + sector * 512, nsectors * 512);
	else
		memcpy(flash + sector * 512, buffer, nsectors * 512);
}

void request_fn(struct request_queue *rq)
{
	struct request *rst;
	printk("in request_fn, about to begin while\n");
	while ((rst = blk_fetch_request(rq)))
	{
		printk("in request_fn, in while\n");
		blk_transfer(blk_rq_pos(rst), blk_rq_cur_sectors(rst), bio_data(rst->bio), rq_data_dir(rst));
		blk_end_request_all(rst, 0);
	}
}

static int blk_ioctl(struct block_device *bdev, fmode_t mode, unsigned cmd, unsigned long arg)
{
	return -ENOTTY;
}

static int blk_open(struct block_device *bdev, fmode_t mode)
{
	printk("opened block device %s\n", bdev->bd_disk->disk_name);
	return 0;
}

static void blk_release(struct gendisk *gd, fmode_t mode)
{
	printk("close block device %s\n", gd->disk_name);
	return;
}

static struct block_device_operations blk_fops = {
	.owner = THIS_MODULE,
	.ioctl = blk_ioctl,
	.open  = blk_open,
	.release = blk_release,
};


static int __init start(void)
{
	if ((major = register_blkdev(0, blk_name)) < 0)
		return -EINVAL;
	printk("major of device is %d\n", major);

	gd = alloc_disk(1); //only one minor disk
	if (!gd)
		goto error1;
	spin_lock_init(&slock);
	gd->queue = blk_init_queue(request_fn, &slock);
	if (!gd->queue)
		goto error2;
	gd->fops = &blk_fops;
	gd->major = major;
	gd->first_minor = 0;
	snprintf(gd->disk_name, 32, "blk%c",'a');
	blk_queue_logical_block_size(gd->queue, 512);
	set_capacity(gd, 32);
	add_disk(gd);
	printk("blk dev init done\n");
	return 0;


error2:
	del_gendisk(gd);
error1:
	unregister_blkdev(major, blk_name);
	return -EFAULT;
}

static void __exit end(void)
{
	blk_cleanup_queue(gd->queue);
	del_gendisk(gd);
	unregister_blkdev(major, blk_name);
}

module_init(start);
module_exit(end);
