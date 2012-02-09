#include <linux/module.h>
#include <linux/version.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/hdreg.h>	/* HDIO_GETGEO */
#include <linux/kdev_t.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>	/* invalidate_bdev */
#include <linux/bio.h>
#include <linux/file.h>

MODULE_LICENSE("Dual BSD/GPL");

static int sbull_major = 0;
static int hardsect_size = 512;
static int nsectors = 1000000;	/* How big the drive is */
MODULE_PARM_DESC(nsectors, 
			"Size of the block device.\n");
module_param(nsectors, int, 0);
static int ndevices = 1;
static char *file_name = "./disk.bin";
module_param(file_name, charp, 0400);
MODULE_PARM_DESC(file_name,
         "File to be used as backend for this block device");


/*
 * Minor number and partition management.
 */
#define SBULL_MINORS	1
#define MINOR_SHIFT	4
#define DEVNUM(kdevnum)	(MINOR(kdev_t_to_nr(kdevnum)) >> MINOR_SHIFT

/*
 * We can tweak our hardware sector size, but the kernel talks to us
 * in terms of small sectors, always.
 */
#define KERNEL_SECTOR_SIZE	512

/*
 * After this much idle time, the driver will simulate a media change.
 */
#define INVALIDATE_DELAY	30*HZ

/*
 * The internal representation of our device.
 */
struct sbull_dev {
        int size;                       /* Device size in sectors */
        u8 *data;                       /* The data array */
        short users;                    /* How many users */
        short media_change;             /* Flag a media change? */
        spinlock_t lock;                /* For mutual exclusion */
        struct request_queue *queue;    /* The device request queue */
        struct gendisk *gd;             /* The gendisk structure */
		struct file *filp;
};

static struct sbull_dev *Devices = NULL;

/*
 *	Function to open the file
 */
struct file *open_file(char *fileName, int flags)
{
	struct file *filp = filp_open(fileName, flags, 0);
	if(!filp)
	{
		printk("Unable to open the file.\n");
		return NULL;
	}
	printk("Open returned filp: %p.\n", filp);
	return filp;
}

void close_file(struct file *filp)
{
	fput(filp);
}

/*
 * Handle an I/O request.
 */
static void sbull_transfer(struct sbull_dev *dev, unsigned long sector,
		unsigned long nsect, char *buffer, int write)
{
	loff_t offset = sector*KERNEL_SECTOR_SIZE;
	unsigned long nbytes = nsect*KERNEL_SECTOR_SIZE;
	mm_segment_t old_fs;

	if ((offset + nbytes) > dev->size) {
		printk (KERN_NOTICE "Beyond-end write (%lld %ld)\n", offset, nbytes);
		return;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	printk("filp: %p, buffer: %p, bytes: %lu, offset: %lld.\n",
		dev->filp, buffer, nbytes, offset);
	if (write)
	{
		if(vfs_write(dev->filp, buffer, nbytes, &offset) != nbytes)
		{
			printk("write failed.\n");
		}
	}
	else
	{
		if(vfs_read(dev->filp, buffer, nbytes, &offset) != nbytes)
		{
			printk("read failed.\n");
		}
	}
}

/*
 * Transfer a single BIO.
 */
static int sbull_xfer_bio(struct sbull_dev *dev, struct bio *bio)
{
	int i;
	struct bio_vec *bvec;
	sector_t sector = bio->bi_sector;

	/* Do each segment independently. */
	bio_for_each_segment(bvec, bio, i) {
		char *buffer = kmap(bvec->bv_page) + bvec->bv_offset;
		sbull_transfer(dev, sector, (bvec->bv_len >> KERNEL_SECTOR_SIZE),//bio_cur_sectors(bio),
				buffer, bio_data_dir(bio) == WRITE);
		sector += (bvec->bv_len >> KERNEL_SECTOR_SIZE); //bio_cur_sectors(bio);
		kunmap(bvec->bv_page);
	}
	return 0; /* Always "succeed" */
}

/*
 * The direct make request version.
 */
static int sbull_make_request(struct request_queue *q, struct bio *bio)
{
	struct sbull_dev *dev = q->queuedata;
	int status;

	status = sbull_xfer_bio(dev, bio);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27) 
	bio_endio(bio, bio->bi_size, status);
#else
	bio_endio(bio, status);
#endif
	return 0;
}


/*
 * Open and close.
 */

static int sbull_open(struct inode *inode, struct file *filp)
{
	struct sbull_dev *dev = inode->i_bdev->bd_disk->private_data;

	filp->private_data = dev;
	spin_lock(&dev->lock);
	if (! dev->users) 
		check_disk_change(inode->i_bdev);
	dev->users++;
	spin_unlock(&dev->lock);
	return 0;
}

static int sbull_release(struct inode *inode, struct file *filp)
{
	struct sbull_dev *dev = inode->i_bdev->bd_disk->private_data;

	spin_lock(&dev->lock);
	dev->users--;
	spin_unlock(&dev->lock);

	return 0;
}


/*
 * The ioctl() implementation
 */

int sbull_ioctl (struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
	long size;
	struct hd_geometry geo;
	struct sbull_dev *dev = filp->private_data;

	switch(cmd) {
	    case HDIO_GETGEO:
        	/*
		 * Get geometry: since we are a virtual device, we have to make
		 * up something plausible.  So we claim 16 sectors, four heads,
		 * and calculate the corresponding number of cylinders.  We set the
		 * start of data at sector four.
		 */
		size = dev->size*(hardsect_size/KERNEL_SECTOR_SIZE);
		geo.cylinders = (size & ~0x3f) >> 6;
		geo.heads = 4;
		geo.sectors = 16;
		geo.start = 4;
		if (copy_to_user((void __user *) arg, &geo, sizeof(geo)))
			return -EFAULT;
		return 0;
	}

	return -ENOTTY; /* unknown command */
}


/*
 * The device operations structure.
 */
static struct block_device_operations sbull_ops = {
	.owner           = THIS_MODULE,
	.open 	         = sbull_open,
	.release 	 = sbull_release,
	.ioctl	         = sbull_ioctl
};


/*
 * Set up our internal device.
 */
static int setup_device(struct sbull_dev *dev, int which)
{
	memset (dev, 0, sizeof (struct sbull_dev));
	dev->size = nsectors*hardsect_size;
	spin_lock_init(&dev->lock);
	
	dev->queue = blk_alloc_queue(GFP_KERNEL);
	if (dev->queue == NULL)
		goto out_vfree;
	blk_queue_make_request(dev->queue, sbull_make_request);
	blk_queue_logical_block_size (dev->queue, hardsect_size); //blk_queue_hardsect_size(dev->queue, hardsect_size);
	dev->queue->queuedata = dev;

	/* 
 	 *	We will use file as the backend for the dummy device 
	 *  instead of memory.
	 */
	dev->filp = open_file(file_name, O_RDWR);
	if(dev->filp == NULL)
	{
		printk("Unable to open the file: %s.\n", file_name);
		goto out_vfree;
	}
	printk("Successfully opened the file.\n");
	/*
	 * And the gendisk structure.
	 */
	dev->gd = alloc_disk(SBULL_MINORS);
	if (! dev->gd) {
		printk (KERN_NOTICE "alloc_disk failure\n");
		goto out_vfree;
	}
	dev->gd->major = sbull_major;
	dev->gd->first_minor = which*SBULL_MINORS;
	dev->gd->fops = &sbull_ops;
	dev->gd->queue = dev->queue;
	dev->gd->private_data = dev;
	snprintf (dev->gd->disk_name, 32, "sbull%c", which + 'a');
	set_capacity(dev->gd, nsectors*(hardsect_size/KERNEL_SECTOR_SIZE));
	add_disk(dev->gd);
	return 0;

  out_vfree:
	if(dev->filp)
		close_file(dev->filp);
	return -1;
}



static int __init sbull_init(void)
{
	int i;
	/*
	 * Get registered.
	 */
	sbull_major = register_blkdev(sbull_major, "sbull");
	if (sbull_major <= 0) {
		printk(KERN_WARNING "sbull: unable to get major number\n");
		return -EBUSY;
	}
	/*
	 * Allocate the device array, and initialize each one.
	 */
	Devices = kmalloc(ndevices*sizeof (struct sbull_dev), GFP_KERNEL);
	if (Devices == NULL)
	{
		printk("Unable to allocate memory.\n");
		goto out_unregister;
	}
	for (i = 0; i < ndevices; i++) 
	{
		if(setup_device(Devices + i, i) != 0)
		{
			printk("setup_device failed.\n");
			goto out_unregister;
		}
	}
    
	return 0;

  out_unregister:
	unregister_blkdev(sbull_major, "sbull");
	return -ENOMEM;
}

static void sbull_exit(void)
{
	int i;

	for (i = 0; i < ndevices; i++) {
		struct sbull_dev *dev = Devices + i;

		if (dev->gd) {
			del_gendisk(dev->gd);
			put_disk(dev->gd);
		}
		if (dev->queue) {
			blk_cleanup_queue(dev->queue);
		}
	}
	unregister_blkdev(sbull_major, "sbull");
	kfree(Devices);
}
	
module_init(sbull_init);
module_exit(sbull_exit);
