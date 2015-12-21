#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <asm/errno.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>

#define DEVS_NR	2
#ifndef BUFSIZ
#define BUFSIZ	4096
#endif
static int scull_major = 0;
static int scull_minor = 0;
struct mem_devp {
	char *data;
	int size;
} *mem_devp;
static int writebuff = 1;
#if 0
struct cdev {
	struct kobject kobj;
	struct module *owner;
	struct file_operations *ops;
	struct list_head list;
	dev_t dev;
	unsigned int count;
};
struct inode {
	...
	struct cdev		*i_cdev;
	...
};
#endif
struct cdev cdev;

int scull_open(struct inode *inode, struct file *filp)
{
	int ret;
#if 0
static inline unsigned imajor(struct inode *inode)
#endif
	if (scull_major != imajor(inode)
		|| scull_minor > iminor(inode)
		|| scull_minor + DEVS_NR - 1 < iminor(inode)) {
		ret = -EINVAL;
		goto flaid1;
	}
#if 0
struct file {
	...
	void			*private_data;
	...
};
#endif
	filp->private_data = &mem_devp[iminor(inode) - scull_minor];
#define DMESG_DEBUG 0
#if DMESG_DEBUG
	printk(KERN_INFO "open :%p\n",
			filp->private_data);
#endif

	return 0;
flaid1:
	return ret;
}

int scull_release(struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t scull_read(struct file * filp, char __user *buffer, size_t size, loff_t *ppos)
{
	int ret;
#if 1
	int p = *ppos;
	int count = size;
	struct mem_devp *mem_devp;
#if DMESG_DEBUG
	printk(KERN_INFO "p %d\n", p);
#endif
	if (p > BUFSIZ - 1) {
		return 0;
	}
	if (count + p > BUFSIZ - 1) {
		count = BUFSIZ - 1 - p;
	}
	mem_devp = filp->private_data;
#if DMESG_DEBUG
	printk(KERN_INFO "mem_devp :%p\n", mem_devp);
#endif
#if 0
unsigned long
copy_to_user(void __user *to, const void *from, unsigned long n)
#endif
	if (copy_to_user(buffer, mem_devp->data + p, count)) {
		return -EFAULT;
	} else {
		*ppos += count;
		ret = count;
	}
#endif

	return ret;
}

ssize_t scull_write(struct file *filp, const char __user *buffer, size_t size, loff_t *ppos)
{
#if 0
unsigned long
copy_from_user(void *to, const void __user *from, unsigned long n)
#endif
	int count = size;
	int p = *ppos;
	int ret;
	struct mem_devp *mem_devp;
	if (p > BUFSIZ - 1)
		return 0;
	if (count > BUFSIZ - 1 - p) {
		count = BUFSIZ - 1 - p;
	}
	mem_devp = filp->private_data;
	if (copy_from_user(&mem_devp->data[p], buffer, count)) {
		ret = -EFAULT;
		goto flaid1;
	}
	ret = count;
	*ppos += count;
flaid1:
	return ret;
}

loff_t scull_llseek(struct file *filp, loff_t loff, int whene)
{
	loff_t new_pos;
	switch (whene) {
		case 0:
			new_pos = loff;
			break;
		case 1:
			new_pos = filp->f_pos + loff;
			break;
		case 2:
			new_pos = BUFSIZ - 1 + loff;
			break;
		default:
			return -EINVAL;
	}
	if (new_pos < 0 || new_pos > BUFSIZ - 1)
		return -EINVAL;
	filp->f_pos = new_pos;
	return new_pos;
}

struct file_operations scull_ops = {
	.owner		= THIS_MODULE,
	.open		= scull_open,
	.release	= scull_release,
	.read		= scull_read,
	.write		= scull_write,
	.llseek		= scull_llseek,
};
#if 0
typedef	int (read_proc_t)(char *page, char **start, off_t off,
			  int count, int *eof, void *data);
#endif
static int scull_read_proc(char *page, char **start, off_t off,
			  int count, int *eof, void *data)
{
	int len = 0;
	len += sprintf(page + len, "scull_major :%d, scull_minor :%d\n",
			scull_major, scull_minor);
	len += sprintf(page + len, "writebuff :%d\n", writebuff);
	*eof = 1;
	return len;
}
#if 0
typedef	int (write_proc_t)(struct file *file, const char __user *buffer,
			   unsigned long count, void *data);
#endif
int scull_write_proc(struct file *file, const char __user *buffer,
			   unsigned long count, void *data)
{
	char tmpbuff[10];
	char *endp;
	int len = count > sizeof(tmpbuff) / sizeof(*tmpbuff) - 1
		? sizeof(tmpbuff) / sizeof(*tmpbuff) - 1 : count;
#if 0
unsigned long
copy_from_user(void *to, const void __user *from, unsigned long n)
#endif
	memset(tmpbuff, 0x00, sizeof(tmpbuff) / sizeof(*tmpbuff));
	if (copy_from_user(tmpbuff, buffer, len)) {
		printk(KERN_INFO "copy_from_user error\n");
		return -EFAULT;
	}
#if 0
long simple_strtol(const char *cp,char **endp,unsigned int base)
#endif
#if DMESG_DEBUG
	printk(KERN_INFO "tmpbuff :%s\n", tmpbuff);
#endif
	if (tmpbuff[0] == '\n')
		return 1;
	if (tmpbuff[0] < '0' || tmpbuff[0] > '9')
		return -EINVAL;
	writebuff = simple_strtol(tmpbuff, &endp, 10);
	memset(tmpbuff, 0x00, sizeof(tmpbuff) / sizeof(*tmpbuff));
	len = endp - tmpbuff;
	return len;
}
static __init int dev_number_init(void)
{
	dev_t dev;
	int ret;
	int i, tmp;
	struct proc_dir_entry *proc_scull_entry;
	if (scull_major) {
		dev = MKDEV(scull_major, scull_minor);
		ret = register_chrdev_region(dev, DEVS_NR, "scull");
	} else {
		ret = alloc_chrdev_region(&dev, scull_minor, DEVS_NR, "scull");
		scull_major = MAJOR(dev);

	}
#if DMESG_DEBUG
	printk(KERN_INFO "major :%d, minor :%d\n",
			MAJOR(dev), MINOR(dev));
#endif
	if (0 > ret) {
		printk(KERN_INFO "register_chrdev_region error\n");
		goto flaid1;
	}
#if 0
void cdev_init(struct cdev *cdev, struct file_operations *fops)
#endif
	cdev_init(&cdev, &scull_ops);
	cdev.owner 	= THIS_MODULE;
	cdev.ops	= &scull_ops;
#if 0
int cdev_add(struct cdev *p, dev_t dev, unsigned count)
#endif
	ret = cdev_add(&cdev, dev,  MKDEV(scull_major, 0));
	if (0 > ret) {
		goto flaid2;
	}
	if (NULL == (mem_devp = kmalloc(sizeof(*mem_devp) * DEVS_NR, GFP_KERNEL))) {
		ret = -ENOMEM;
		goto flaid2;
	}
	memset(mem_devp, 0x00, sizeof(*mem_devp) * DEVS_NR);
	for (i = 0; i < DEVS_NR; i++) {
		mem_devp[i].data = kmalloc(sizeof(*mem_devp[i].data) * BUFSIZ, GFP_KERNEL);
		if (NULL == mem_devp[i].data) {
			tmp = i;
			ret = -ENOMEM;
			goto flaid3;
		}
		memset(mem_devp[i].data, 0x00, sizeof(*mem_devp[i].data) * BUFSIZ);
		mem_devp[i].size = BUFSIZ;
	}
#if DMESG_DEBUG
	printk(KERN_INFO "&mem_devp[0]:%p\n", &mem_devp[0]);
#endif
#undef DMESG_DEBUG
#if 0	/* read test */
	mem_devp[0].data[0] = '0';
	mem_devp[0].data[1] = '1';
#endif
#if 0
struct proc_dir_entry *create_proc_entry(const char *name, mode_t mode,
					 struct proc_dir_entry *parent)
struct proc_dir_entry {
	read_proc_t *read_proc;
	write_proc_t *write_proc;
};
#endif
	proc_scull_entry = create_proc_entry("scull_mem", 0, NULL);
	if (NULL == proc_scull_entry) {
		printk(KERN_INFO "create_proc_entry flaid\n");
	} else {
		proc_scull_entry->read_proc	= scull_read_proc;
		proc_scull_entry->write_proc	= scull_write_proc;
	}

	return 0;
flaid3:
	for (i = tmp; i >= 0; i--) {
		kfree(mem_devp[i].data);
	}
	kfree(mem_devp);
flaid2:
#if 0
void cdev_del(struct cdev *p)
#endif
	cdev_del(&cdev);
	unregister_chrdev_region(dev, DEVS_NR);
flaid1:
	return ret;
}

static __exit void dev_number_exit(void)
{
	dev_t dev;
	int i;
#if 0
void remove_proc_entry(const char *name, struct proc_dir_entry *parent)
#endif
	remove_proc_entry("scull_mem", NULL);
	for (i = 0; i < DEVS_NR; i++) {
		if (mem_devp[i].data) {
			kfree(mem_devp[i].data);
			mem_devp[i].data = NULL;
		}
	}
	if (mem_devp) {
		kfree(mem_devp);
		mem_devp = NULL;
	}
	cdev_del(&cdev);
	dev = MKDEV(scull_major, scull_minor);
	unregister_chrdev_region(dev, DEVS_NR);
	return;
}

module_init(dev_number_init);
module_exit(dev_number_exit);

MODULE_LICENSE("GPL");
