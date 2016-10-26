#include <linux/init.h>
#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/fs.h>


MODULE_AUTHOR("muryliang@gmail.com");
MODULE_DESCRIPTION("use blk_alloc_queue instead of blk_init_ququ");
MODULE_VERSION("1.0");
MODULE_LICENSE("GPL");

#define pdebug(fmt,...)  printk(KERN_ERR fmt,__VA_ARGS__)
#define DISK_SECTOR_SIZE 512
#define DISK_SECTOR 32*1024
#define DISK_SIZE  (DISK_SECTOR * DISK_SECTOR_SIZE)

static int major;
static char *name = "blkdev";
static struct private_disk {
	void *data;
	struct gendisk *gd;
} pdisk;

static int blk_open(struct block_device *blk, fmode_t mode)
{
	printk("in blkopen\n");
	return 0;
}

static int blk_release(struct gendisk *gd, fmode_t mode)
{
	printk("in of blkrelease\n");
	return 0;
}

static int blk_ioctl(struct block_device *blk, fmode_t mode, unsigned int cmd, unsigned long arg)
{
	return -ENOTTY;
}

static struct block_device_operations blk_fops = {
	.owner = THIS_MODULE,
	.open  = blk_open,
	.release = blk_release,
	.ioctl = blk_ioctl,
};

static int request_fn(struct request_queue *rq, struct bio *bio)
{
	unsigned long i;
	char *buf;
	char *mbuf;
	struct bio_vec *pbvec;

	i = bio->bi_sector * DISK_SECTOR_SIZE + bio->bi_size;
	if (i > DISK_SIZE)
		goto fail;

	buf = pdisk.data + bio->bi_sector * DISK_SECTOR_SIZE;
	printk("now about to bio_for_each_segment\n");
	
	bio_for_each_segment(pbvec, bio, i)
	{
		mbuf = kmap(pbvec->bv_page) + pbvec->bv_offset;
		switch(bio_data_dir(bio))
		{
			case READA:
			case READ:
				memcpy(mbuf, buf, pbvec->bv_len);
				break;
			case WRITE:
				memcpy(buf, mbuf, pbvec->bv_len);
				break;
			default:
				kunmap(pbvec->bv_page);
				printk("switch default error\n");
				goto fail;
		}
		kunmap(pbvec->bv_page);
		buf += pbvec->bv_len;
	}
	bio_endio(bio, 0);
	return 0;
fail:
	bio_io_error(bio);
	printk("io error\n");
	return 0;
}
		
static int __init start(void)
{
	if ((major = register_blkdev(0, name)) < 0)
		return -EAGAIN;
	pdebug("the major is %d\n", major);
	//alloc queue
	pdisk.data = vmalloc(DISK_SIZE);
	if (!pdisk.data)
	{
		printk("vmalloc error\n");
		unregister_blkdev(major, name);
		return -EFAULT;
	}
	printk(KERN_ERR "alloc disk size done\n");
	pdisk.gd = alloc_disk(1);
	pdisk.gd->major = major;
	pdisk.gd->first_minor = 0;
	pdisk.gd->queue = blk_alloc_queue(GFP_KERNEL);
	blk_queue_make_request(pdisk.gd->queue, request_fn);
	printk(KERN_ERR "make request set done\n");
	pdisk.gd->fops = &blk_fops;
	pdisk.gd->private_data = &pdisk;
	set_capacity(pdisk.gd, DISK_SECTOR);
	printk(KERN_ERR "set capacity done\n");
	snprintf(pdisk.gd->disk_name, 32, "bdisk%c",'a');
	add_disk(pdisk.gd);
	printk(KERN_ERR "init done ram2\n");
	return 0;
}
	
static void __exit end(void)
{
	del_gendisk(pdisk.gd);
	vfree(pdisk.data);
	blk_cleanup_queue(pdisk.gd->queue);
	unregister_blkdev(major, name);
}

module_init(start);
module_exit(end);
