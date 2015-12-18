#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <asm/errno.h>
#include <linux/string.h>

static int scull_major = SCULL_MAJOR;
static int scull_minor = 16;

static __init int dev_number_init(void)
{
	dev_t dev;
	int ret;
	if (scull_major) {
		dev = MKDEV(scull_major, scull_minor);
		ret = register_chrdev_region(dev, 2, "scull");
	} else {
		ret = alloc_chrdev_region(&dev, scull_minor, 2, "scull");
		scull_major = MAJOR(dev);

	}
	printk(KERN_INFO "major :%d, minor :%d\n",
			MAJOR(dev), MINOR(dev));
	if (0 > ret) {
		printk(KERN_INFO "register_chrdev_region error\n");
		return -ret;
	}
	return 0;
}

static __exit void dev_number_exit(void)
{
	dev_t dev;
	dev = MKDEV(scull_major, scull_minor);
	unregister_chrdev_region(dev, 2);
	return;
}

module_init(dev_number_init);
module_exit(dev_number_exit);

MODULE_LICENSE("GPL");
