#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

#ifndef BUFSIZ
#define BUFSIZ	4096
#endif

static int major = 0;
static int minor = 0;
static int devs_nr = 1;

struct scull_dev {
	struct cdev cdev;
	wait_queue_head_t inq, outq;
	struct semaphore wsem, rsem;
	int rp, wp;
	char *data;
} scull_dev;
struct scull_dev scull_dev;
static inline int buff_len(struct scull_dev *dev)
{
	return (dev->wp + BUFSIZ - dev->rp) % BUFSIZ;
}

static void print_queue(struct scull_dev *dev)
{
	int i;
	for (i = dev->rp; i != dev->wp; i = (i + 1) % BUFSIZ) {
		printk(KERN_INFO "data[%d]:%c\n",
				i, scull_dev.data[i]);
	}
}

static ssize_t scull_read_block(struct file *filp, char __user *arg, size_t count, loff_t *ppos)
{
	int len = 0;
	struct scull_dev *dev = filp->private_data;
	if (down_interruptible(&dev->rsem))
		return -ERESTARTSYS;
	for ( ; dev->rp == dev->wp; ) {
#if 0
		printk(KERN_INFO "dev->rp :%d, dev->wp :%d\n",
				dev->rp, dev->wp);
#endif
		up(&dev->rsem);
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		if (wait_event_interruptible(dev->inq, (dev->rp != dev->wp)))
			return -ERESTARTSYS;
		if (down_interruptible(&dev->rsem))
			return -ERESTARTSYS;
	}
	if (dev->wp > dev->rp) {
		len = count > dev->wp - dev->rp ? dev->wp - dev->rp : count;
		if (copy_to_user(arg, dev->data + dev->rp, len))
			goto efault;
	} else if (count <= BUFSIZ - 1 - dev->rp) {
		len = count;
		if (copy_to_user(arg, dev->data + dev->rp, len))
			goto efault;
	} else {
		len = count > buff_len(dev) ? buff_len(dev) : count;
		if (copy_to_user(arg, dev->data + dev->rp, BUFSIZ - 1 - dev->rp))
			goto efault;
		if (copy_to_user(arg + BUFSIZ - 1 - dev->rp, dev->data,
				len - (BUFSIZ - 1 - dev->rp)))
			goto efault;
	}
	dev->rp = (dev->rp + len) % BUFSIZ;
	up(&dev->rsem);
	wake_up_interruptible(&dev->outq);
	return len;
efault:
	up(&dev->rsem);
	return -EFAULT;
}

static int wait_write(struct scull_dev *dev, struct file *filp)
{
	while ((dev->wp + 1) % BUFSIZ == dev->rp) {
		DEFINE_WAIT(wait);
		up(&dev->wsem);
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		prepare_to_wait(&dev->outq, &wait, TASK_INTERRUPTIBLE);
		if ((dev->wp + 1) % BUFSIZ == dev->rp)
			schedule();
		finish_wait(&dev->outq, &wait);
		if (signal_pending(current)) {
#if 1
			printk(KERN_INFO "singal operation\n");
#endif
			return -ERESTARTSYS;
		}
		if (down_interruptible(&dev->wsem))
			return -ERESTARTSYS;
	}
	return 0;
}

static ssize_t scull_write_block(struct file *filp, const char __user *arg, size_t count, loff_t *ppos)
{
	int len;
	struct scull_dev *dev = filp->private_data;
	int space;
	int ret;
	if (down_interruptible(&dev->wsem))
		return -ERESTARTSYS;
	ret = wait_write(dev, filp);
	if (ret)
		return ret;
	space = BUFSIZ - buff_len(dev) - 1;
	len = count < space ? count : space;
	if (dev->wp + len > BUFSIZ - 1) {
		if(copy_from_user(dev->data + dev->wp, arg, BUFSIZ - 1 - dev->wp))
			goto efault;
		if(copy_from_user(dev->data,
				arg + BUFSIZ - 1 - dev->wp, len - (BUFSIZ - 1 - dev->wp)))
			goto efault;
	} else {
		if(copy_from_user(dev->data + dev->wp, arg, len))
			goto efault;
	}
	dev->wp = (dev->wp + len) % BUFSIZ;
	up(&dev->wsem);
	wake_up_interruptible(&dev->inq);
	return len;
efault:
	up(&dev->wsem);
	return -EFAULT;
}

int scull_open_block(struct inode *inode, struct file *filp)
{
	struct scull_dev *dev;
#if 0
#define container_of(ptr, type, member)
#endif
	dev = container_of(inode->i_cdev, struct scull_dev, cdev);
	filp->private_data = dev;
	return 0;
}

int scull_release_block(struct inode *inode, struct file *filp)
{
	return 0;
}
struct file_operations scull_ops = {
	.open		= scull_open_block,
	.release	= scull_release_block,
	.read		= scull_read_block,
	.write		= scull_write_block,
};

static int __init scull_init(void)
{
	dev_t dev;
	int ret;
	dev = MKDEV(major, minor);
	if (major) {
		ret = register_chrdev_region(dev, devs_nr, "scullb");
	} else {
		ret = alloc_chrdev_region(&dev, minor, devs_nr, "scullb");
		major = MAJOR(dev);
	}
	if (ret)
		goto failed1;
	cdev_init(&scull_dev.cdev, &scull_ops);
	scull_dev.cdev.owner = THIS_MODULE;
	init_MUTEX(&scull_dev.wsem);
	init_MUTEX(&scull_dev.rsem);
	if (down_interruptible(&scull_dev.wsem))
		return -ERESTARTSYS;
	ret = cdev_add(&scull_dev.cdev, dev, devs_nr);
	if (ret)
		goto failed1;
#if 0
static __always_inline void *kmalloc(size_t size, gfp_t flags)
#endif
	scull_dev.data = kmalloc(sizeof(*scull_dev.data) * BUFSIZ, GFP_KERNEL);
	if (!scull_dev.data)
		goto failed2;
	scull_dev.rp = scull_dev.wp = 0;
	init_waitqueue_head(&scull_dev.inq);
	init_waitqueue_head(&scull_dev.outq);
	up(&scull_dev.wsem);
	return 0;
failed2:
	cdev_del(&scull_dev.cdev);
failed1:
	return ret;
}

static void __exit scull_exit(void)
{
	cdev_del(&scull_dev.cdev);
	return;
}

module_init(scull_init);
module_exit(scull_exit);

MODULE_LICENSE("GPL");
