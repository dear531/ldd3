#include <linux/module.h>
#include <linux/init.h>
#include <asm/current.h>
#include <linux/sched.h>

char *a[2] = {NULL, NULL};
int b[2] = {0, 0};
int anum;
int bnum;
module_param_array(a, charp, &anum, S_IWUSR | S_IRUGO);
module_param_array(b, int, &bnum, S_IWUSR | S_IRUGO);

static __init int hello_init(void)
{
	struct task_struct *task;
#if defined(CONFIG_SCHEDSTATS) || defined(CONFIG_TASK_DELAY_ACCT)
	struct sched_info sched_info;
#endif
	printk(KERN_INFO "init hello world module\n");
	printk(KERN_INFO "anum :%d, a[0]:%s, a[1]:%s\n",
			anum, a[0], a[1]);
	task = current;
	printk(KERN_INFO "pid :%d, %s\n", task->pid, task->comm);
	return 0;
}

static __exit void hello_exit(void)
{
	printk(KERN_INFO "exit hello world module\n");
	return;
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
