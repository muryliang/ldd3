#include <linux/init.h>
#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/fs.h>

#define MMAJOR 234
#define KERNEL_SECTOR_SIZE 512
#define MMINORS 16
#define INVALIDATE_DEALY 10

#define DEBUGG
#ifdef DEBUGG
#define PR(fmt,...)  pr_info(fmt,__VA_ARGS_)
#else
#define PR(fmt,...) 
#endif
static int major;
static char name[] = "mblk";
static int hardsect_size = 512;
static int nsectors = 1024;  // so 512k
static int which = 0;  // specify sequence of this device

static struct sbull {
	int size;
	u8 *data;
	short users;
	short media_change;
	spin_lock_t lock;
	struct request_queue *queue;
	struct gendisk *gd;
	struct timer_list timer;
} msbull;

static int sbull_media_changed(struct gendisk *gd)
{
	struct sbull *dev = gd->private_data;
	return dev->media_change;
}

static int sbull_revalidate(struct gendisk *gd)
{
	struct sbull *dev = gd->private_data;
	if (dev->media_change)
		dev->media_change = 0;
		memset(dev->data, 0, dev->size);
	}
	return 0;
}

static int sbull_ioctl(struct block_device *bdisk, fmode_t mode, unsigned int cmd, unsigned long arg)
{
	struct sbull *dev = bdisk->bd_disk->private_data;
	struct hd_geometry geo;
	long size;

	switch(cmd) {
	case HDIO_GETGEO:
		size = dev->size;
		geo.cylinders = (size & ~0x3f) >> 6;
		geo.heads = 4;
		geo.sectors = 16;
		geo.start = 4;
		if (copy_to_user((void __user *)arg, &geo, sizeof(geo)))
			return -EFAULT;
		return 0;
	}
	return -ENOTTY;
}

static int sbull_open(struct block_device *bdev, fmode_t mode)
{
	struct sbull *dev = bdev->bd_disk->private_data;

	del_timer_sync(&dev->timer);
	spin_lock(&dev->lock);
	if (! dev->users)
		check_disk_change(bdev);
	dev->users++;
	spin_unlock(&dev->lock);
	return 0;
}

static int sbull_release(struct gendisk *gd, fmode_t mode)
{
	struct sbull *dev = bdev->bd_disk->private_data;
	spin_lock(&dev->lock);
	dev->users--;
	if (!dev->users)
		dev->timer.jiffies = jiffies + INVALIDATE_DELAY;
		add_timer(&dev->timer);
	}
	spin_unlock(&dev->lock);
	return 0;
}

static struct block_device_operations blk_fops = {
	.owner = THIS_MODULE,
	.open = sbull_open,
	.release = sbull_release,
};

static void sbull_transfer(struct sbull *dev, unsigned long sector, unsigned long nsect, char *buffer, int write)
{
	unsigned long offset = sector * KERNEL_SECTOR_SIZE;
	unsigned long nbytes = nsect * KERNEL_SECTOR_SIZE;

	if ((offset + nbytes) > dev->size) {
		printk("beyond end write offset: %x, bytes: %s\n", offset, nbytes);
		return ;
	}

	if (write)
		memcpy (dev->data + offset, buffer, nbytes);
	else
		memcpy (buffer, dev->data + offset, nbytes);
}

static void sbull_request(struct request_queue *rq)
{
	struct request *req;
	int ret = 0;

	req = blk_fetch_request(rq);
	while (req != NULL )
	{
		struct sbull *dev = req->rq_disk->private_data;
		if (!blk_fs_request(req)) {
			printk("skip non fs request\n");
			ret = -EIO;
			goto done;
		}
		sbull_transfer(dev, blk_rq_pos(req), blk_rq_cur_sectors(req), 
				req->buffer, req_data_dir(req));
		ret = 0;
done:
		if (!__blk_end_request_cur(req,ret)
			req = blk_fetch_request(rq);
	}
}

static int __init start(void)
{
	
	memset(&msbull, 0, sizeof(struct sbull));
	major = register_blkdev(major, name);
	if (major <=0 ) {
		printk("unable to get blkdev number\n");
		return -EBUSY;
	}
	PR("get major %d\n", major);

	msbull.size = nsectors * hardsect_size;

	spin_lock_init(&msbull.lock);
	msbull.queue = blk_init_queue(sbull_request, &msbull.lock);	
	if (msbull.queue == NULL ) {
		printk("alloc queue failed\n");
		unregister_blkdev(major, name);
		return -EFAULT;
	}
	PR("queue init done\n");
	msbull.data = vmalloc(nsectors * hardsect_size);
	if (!msbull.data) {
		printk("alloc disk size failed\n");
		blk_cleanup_queue(msbull.queue);
		unregister_blkdev(major,name);
		return -EFAULT;
	}
	PR("memory disk size allocated %ld\n", nsectors * hardsect_size);
	msbull.gd = alloc_disk(MMINORS);
	if (!msbull.gd) {
		printk("failed to alloc disk\n");
		vfree(msbull.data);
		blk_cleanup_queue(msbull.queue);
		unregister_blkdev(major,name);
		return -EFAULT;
	}
	msbull.gd->major = major;
	msbull.gd->fops = &blk_fops;
	msbull.gd->first_minor = which * MMINORS;
	msbull.gd->private_data = &msbull;
	msbull.gd->queue = msbull.queue;
	snprintf(msbull.gd->disk_name, 32, "blkdev%c", which + 'a');
	block_queue_hardsect_size(msbull.queue, hardsect_size);   // set hardsect size
	set_capacity(msbull.gd, nsectors * (hardsect_size / KERNEL_SECTOR_SIZE)); //set kernel recognized size
	add_disk(msbull.gd); //activate gendisk
	PR("sbull init done\n");
	return 0;
}

static void __exit end(void)
{
	del_gendisk(msbull.gd);
	vfree(msbull.data);
	blk_cleanup_queue(msbull.queue);
	unregister_blkdev(major, name);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("muryliang@gmail.com");
MODULE_DESCRIPTION("a test of ldd3's sbull driver");
MODULE_VERSION("v1.0");

module_init(start);
module_exit(end);
