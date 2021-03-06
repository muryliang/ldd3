#include <linux/init.h>
#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/fs.h>
#include <linux/hdreg.h>
#include <linux/mm.h>
#include <linux/slab.h>

#define DEVNAME "bdev"
#define NSECTORS 50000
#define HARDSECT 512
//#define SSIZE 16*1024*1024 

struct bdev {
	int major;
	int size;
	char *data;
	int users;
	spinlock_t lock;
	struct request_queue *rq;
	struct timer_list  timer;
	struct gendisk *gd;
};

struct bdev  *sptr;

static int bdev_open(struct block_device *bd, fmode_t mode)
{
	struct bdev *ptr = (struct bdev*)bd->bd_disk->private_data;
	spin_lock(&ptr->lock);
	ptr->users++;
	spin_unlock(&ptr->lock);
//	pr_info("in bdev_open\n");

	return 0;
}

static int  bdev_release(struct gendisk *gd, fmode_t mode)
{
	struct bdev *ptr = (struct bdev*)gd->private_data;
	spin_lock(&ptr->lock);
	ptr->users--;
	spin_unlock(&ptr->lock);
//	pr_info("in bdev_release\n");
	return 0;
}

static int bdev_ioctl(struct block_device *bd, fmode_t mode, unsigned cmd, unsigned long arg)
{
	long size;
	struct hd_geometry geo;
//	struct bdev *ptr = (struct bdev*)bd->bd_disk->private_data;

	pr_info("in bdev  ioctl\n");
	switch(cmd) {
		case HDIO_GETGEO :
		size = NSECTORS * HARDSECT;
		geo.cylinders = (size & 0x3f) >> 6;
		geo.heads = 4;
		geo.sectors = 16;
		geo.start = 4;
		if (copy_to_user((void __user*) arg, &geo, sizeof(geo)))
			return -EFAULT;
		return 0;
	}
	return -ENOTTY;
}

static struct block_device_operations bdev_ops = {
	.owner = THIS_MODULE,
	.open = bdev_open,
	.release = bdev_release,
	.ioctl = bdev_ioctl,
};

static void blk_transfer(unsigned long sector, unsigned long sectors ,char* buffer, int write)
{
//	printk(KERN_ERR "in blk transfer , sector %ld, sectors %d ,buffer %lx, write %d\n",
//		sector, sectors, (unsigned long)buffer, write);
	if (write)
		memcpy(sptr->data + sector * 512, buffer, sectors * 512);
	else
		memcpy(buffer, sptr->data+ sector * 512 , sectors * 512);
}

static int bdev_request(struct request_queue *rq, struct bio *bio)
{
	int i;
	struct bio_vec *vec;
	char * buf, *mbuf;

	if (bio->bi_sector * HARDSECT + bio->bi_size > sptr->size)
		goto fail;
	buf = sptr->data + bio->bi_sector * HARDSECT;

	bio_for_each_segment(vec, bio, i) {
		mbuf = kmap(vec->bv_page) + vec->bv_offset;
		switch(bio_data_dir(bio)) {
		case READA:
		case READ:
			memcpy(mbuf, buf, vec->bv_len);
			break;
		case WRITE:
			memcpy(buf, mbuf, vec->bv_len);
			break;
		default:
			kunmap(vec->bv_page);
			pr_info("default error\n");
			goto fail;
		}
		kunmap(vec->bv_page);
		buf += vec->bv_len;
	}
	bio_endio(bio, 0);
	return 0;
fail:
	bio_io_error(bio);
	pr_info("error in request\n");
	return 0;
}

static int __init start(void)
{
	struct bdev *ptr = (struct bdev*)kmalloc(sizeof(struct bdev), GFP_KERNEL);
	int err;
	if (!ptr)
		return -ENOMEM;
	memset(ptr, 0, sizeof(struct bdev));
	sptr = ptr;
	ptr->size = NSECTORS * HARDSECT;
	ptr->data = vmalloc(ptr->size);
	if (ptr->data == NULL) {
		err = -ENOMEM;
		goto err2;
	}
	spin_lock_init(&ptr->lock);
//	ptr->rq = blk_init_queue(bdev_request, &ptr->lock);
	ptr->rq = blk_alloc_queue(GFP_KERNEL);
	blk_queue_make_request(ptr->rq, bdev_request);
	if ((ptr->major = register_blkdev(0, DEVNAME)) < 0)
		return -EFAULT;
	pr_info("dev major is %d\n", ptr->major);

	ptr->gd = alloc_disk(1);
	if (!ptr->gd) {
		err = -ENOMEM;
		goto err1;
	}
	ptr->gd->major = ptr->major;
	ptr->gd->first_minor = 0;
	ptr->gd->fops = &bdev_ops;
	ptr->gd->queue = ptr->rq;
	ptr->gd->private_data = ptr;
	snprintf(ptr->gd->disk_name, 32, "bdev%c", 'a');
	blk_queue_logical_block_size(ptr->gd->queue, HARDSECT);
	set_capacity(ptr->gd, NSECTORS*(HARDSECT/512));
//	set_capacity(ptr->gd, 32768);
	add_disk(ptr->gd);

	pr_info("init done\n");
	return 0;
err1:
	unregister_blkdev(ptr->major, DEVNAME);
err2:
	vfree(ptr->data);
	kfree(ptr);
	return err;
}

static void __exit end(void)
{
	blk_cleanup_queue(sptr->rq);
	del_gendisk(sptr->gd);
	unregister_blkdev(sptr->major, DEVNAME);
	vfree(sptr->data);
	kfree(sptr);
	pr_info("exit done\n");
}

MODULE_LICENSE("GPL");
module_init(start);
module_exit(end);
