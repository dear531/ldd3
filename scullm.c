#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>

#ifndef MAJOR_SCULL
#define MAJOR_SCULL	0
#endif
#ifndef DEVS_NR
#define DEVS_NR		2
#endif

#ifndef BUFSIZ
#define BUFSIZ		4096
#endif
static int major	= MAJOR_SCULL;
static int minor	= 0;
static int scull_ioctl_num = 0;
struct dev_chr {
	struct cdev cdev;
	char *data;
	int size;
	struct semaphore sem;
} devs[DEVS_NR];
static int scull_open(struct inode *inode, struct file *filp)
{
	struct dev_chr *dev;
	dev = container_of(inode->i_cdev, struct dev_chr, cdev);
	filp->private_data = dev;
	return 0;
}

static ssize_t scull_read(struct file *filp, char __user *buffer, size_t count, loff_t *ppos)
{
	struct dev_chr *dev;
	int len, p = *ppos;
	dev = filp->private_data;
	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	if (0 >= dev->size) {
		len = 0;
		goto finash1;
	}
	if (p + count > dev->size) {
		len = dev->size - p;
	} else {
		len = count;
	}
	if (copy_to_user(buffer, dev->data + p, len))
		return -EFAULT;
	*ppos += len;
	up(&dev->sem);
finash1:
	return len;
}

ssize_t scull_write(struct file *filp, const char __user *buffer, size_t count, loff_t *ppos)
{
	int len, p = *ppos;
	struct dev_chr *dev = filp->private_data;
	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	if (p + count > BUFSIZ)
		len = BUFSIZ - p;
	else
		len = count;
	if (copy_from_user(dev->data + p, buffer, len))
		return -ERESTARTSYS;
	if (p + len < dev->size) {
		memset(dev->data + p + len, 0x00, dev->size - (p + len));
	}
	*ppos += len;
	dev->size = p + len;
	while (dev->size >= 2 && dev->data[dev->size - 1] == '\n' && dev->data[dev->size - 2] == '\n') {
		dev->data[dev->size - 1] = 0;
		dev->size--;
		(*ppos)--;
	}
	up(&dev->sem);
	return len;
}

static int scull_release(struct inode *inode, struct file *filp)
{
	return 0;
}
#if 0
off_t lseek(int fd, off_t offset, int whence);
#endif
loff_t scull_llseek(struct file *filp, loff_t offset, int whence)
{
	loff_t pos;
	struct dev_chr *dev = filp->private_data;
	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	switch (whence) {
		case 0:
			pos = 0 + offset;
			break;
		case 1:
			pos = filp->f_pos + offset;
			break;
		case 2:
			pos = dev->size + offset;
			break;
		default:
			return -EINVAL;
	}
	filp->f_pos = pos;
	up(&dev->sem);
	return pos;
}

#if 0
#define _IO(type,nr)		_IOC(_IOC_NONE,(type),(nr),0)
#define _IOR(type,nr,size)	_IOC(_IOC_READ,(type),(nr),sizeof(size))
#define _IOW(type,nr,size)	_IOC(_IOC_WRITE,(type),(nr),sizeof(size))
#define _IOWR(type,nr,size)	_IOC(_IOC_READ|_IOC_WRITE,(type),(nr),sizeof(size))
#endif

#define SCULL_IOC_MAGIC		'k'
#define SCULL_IOC_MAXNR		6
#define SCULL_ICO_RESET		_IO(SCULL_IOC_MAGIC, 0)
#define SCULL_IOC_RNUMP		_IOR(SCULL_IOC_MAGIC, 1, int)
#define SCULL_IOC_WNUMP		_IOW(SCULL_IOC_MAGIC, 2, int)
#define SCULL_IOC_WRNUMP	_IOWR(SCULL_IOC_MAGIC, 3, int)
#define SCULL_IOC_RNUMV		_IO(SCULL_IOC_MAGIC, 4)
#define SCULL_IOC_WNUMV		_IO(SCULL_IOC_MAGIC, 5)
#define SCULL_IOC_WRNUMV	_IO(SCULL_IOC_MAGIC, 6)

static int scull_ioctl(struct inode *inode, struct file *flip, unsigned int cmd, unsigned long arg)
{
	int retval = 0, err = 0;
	int tmp;
#define _IOC_TYPE(nr)		(((nr) >> _IOC_TYPESHIFT) & _IOC_TYPEMASK)
	if (SCULL_IOC_MAGIC != _IOC_TYPE(cmd))
		return -ENOTTY;
	if (SCULL_IOC_MAXNR < _IOC_NR(cmd))
		return -ENOTTY;
	if (_IOC_WRITE == _IOC_DIR(cmd))
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_READ == _IOC_DIR(cmd))
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err)
		return -EFAULT;
	switch(cmd) {
		case SCULL_ICO_RESET:
			scull_ioctl_num = 0;
			break;
		case SCULL_IOC_RNUMP:
			retval = __put_user(scull_ioctl_num, (int __user*)arg);
			break;
		case SCULL_IOC_WNUMP:
			if (!capable(CAP_SYS_ADMIN))
				return -EPERM;
			retval = __get_user(scull_ioctl_num, (int __user*)arg);
			break;
		case SCULL_IOC_WRNUMP:
			if (!capable(CAP_SYS_ADMIN))
				return -EPERM;
			tmp = scull_ioctl_num;
			retval = __get_user(scull_ioctl_num, (int __user*)arg);
			if (retval == 0)
				__put_user(tmp, (int __user *)arg);
			break;
		case SCULL_IOC_RNUMV:
			retval = scull_ioctl_num;
			break;
		case SCULL_IOC_WNUMV:
			if (!capable(CAP_SYS_ADMIN))
				return -EPERM;
			scull_ioctl_num = arg;
			break;
		case SCULL_IOC_WRNUMV:
			if (!capable(CAP_SYS_ADMIN))
				return -EPERM;
			tmp = scull_ioctl_num;
			scull_ioctl_num = arg;
			retval = tmp;
		default:
			return -EINVAL;
	}
	return retval;
}

struct file_operations scull_ops = {
	.owner	= THIS_MODULE,
	.open	= scull_open,
	.release= scull_release,
	.read	= scull_read,
	.write	= scull_write,
	.llseek	= scull_llseek,
	.ioctl		= scull_ioctl,
};
static int __init scull_init(void)
{
	int ret, i, tmp;
	dev_t dev = MKDEV(major, minor);
	if (major) {
		ret = register_chrdev_region(dev, DEVS_NR, "scullm");
	} else {
		ret = alloc_chrdev_region(&dev, minor, DEVS_NR, "scullm");
		major = MAJOR(dev);
	}
	if (ret < 0) {
		goto flaid1;
	}
	for (i = 0; i < DEVS_NR; i++) {
		cdev_init(&devs[i].cdev, &scull_ops);
		devs[i].cdev.owner	= THIS_MODULE;
		sema_init(&devs[i].sem, 0);
		ret = cdev_add(&devs[i].cdev, MKDEV(major,minor + i), 1);
		if (0 > ret) {
			tmp = i;
			goto flaid2;
		}
		devs[i].data = kmalloc(sizeof(*devs[i].data) * BUFSIZ, GFP_KERNEL);
		if (NULL == devs[i].data) {
			tmp = i;
			goto flaid2;
		}
		devs[i].size = 0;
		up(&devs[i].sem);
	}
	return 0;
flaid2:
	for (i = tmp; i >= 0; i--) {
		if (devs[i].data)
			kfree(devs[i].data);
		cdev_del(&devs[i].cdev);
	}
flaid1:
	return ret;
}

static void __exit scull_exit(void)
{
	int i;
	for (i = 0; i < DEVS_NR; i++) {
		kfree(devs[i].data);
		cdev_del(&devs[i].cdev);
	}
	unregister_chrdev_region(MKDEV(major, minor), DEVS_NR);
}

module_init(scull_init);
module_exit(scull_exit);

MODULE_LICENSE("GPL");
