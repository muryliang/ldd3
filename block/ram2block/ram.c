#include <linux/init.h>
#include <linux/module.h>

#define pdebug(fmt,...)  printk(KERN_ERR fmt,__VA_ARGS__)
#define DISK_SECTOR_SIZE 512
#define DISK_SECTOR 1024
#define DISK_SIZE  (DISK_SECTOR * DISK_SECTOR_SIZE)

static int major;
static char name[] = "blkdev";
static struct private_disk {
	void *data;
	struct gendisk *gd;
} pdisk;

static struct block_device_operations blk_fops = {
	.owner = THIS_MODULE,
	.open  = blk_open,
	.release = blk_release,
	.ioctl = blk_ioctl,
};

static int __init start(void)
{
	if ((major = register_blkdev(0, name)) < 0)
		goto error1;
	pdebug("the major is %d\n", major);
	//alloc queue
	pdisk.gd = alloc_disk(1);
	pdisk.gd->major = major;
	pdisk.gd->first_minor = 0;
	pdisk.gd->queue = blk_alloc_queue(GFP_KERNEL);
	blk_queue_make_request(pdisk.gd->queue, request_fn);
	pdisk.gd->fops = &blk_fops;
	pdisk.gd->private_data = &pdisk;
	set_capacity(pdisk.gd, DISK_SIZE);
	add_disk(device.gd);
	return 0;
}
	
