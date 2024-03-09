#include <linux/module.h> 
#include <linux/printk.h> 
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/kthread.h>

#define DEVICE_NAME "eudyptula"

static struct task_struct *tsk;
static DECLARE_WAIT_QUEUE_HEAD(wq);
static int condition = 0;

static int Foo(void * arg)
{
    pr_info("Foo started\n");
    if(wait_event_interruptible(wq, condition != 0 || kthread_should_stop()))
    {
        pr_info("Wait event interrupted\n");
    }
    else
    {
        pr_info("Wait event finished\n");
    }

    return 0;
}

static ssize_t misc_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    condition = 1;
    wake_up_interruptible(&wq);
    pr_info("write callback finished\n");
    return count;
}

const struct file_operations misc_fops = {
        .owner = THIS_MODULE,
        .read = NULL,
        .write = misc_write,
};

static struct miscdevice misc_example = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = DEVICE_NAME,
        .fops = &misc_fops,
        .mode = 0222,
};
 
static int misc_init(void)
{
        pr_debug("Hello there\nMy minor number is: %i\n", misc_example.minor);
        tsk = kthread_create(Foo, NULL, "eudyptula");
        if (IS_ERR(tsk)) 
        {
            pr_err("Failed to create kernel thread\n");
            return PTR_ERR(tsk);
        }
        wake_up_process(tsk); 

        return misc_register(&misc_example);
}

static void misc_exit(void)
{
        pr_debug("Goodbye there\n");
        if(!condition)
        {
            pr_info("kthread_stop returned %d\n", kthread_stop(tsk));
        }
        misc_deregister(&misc_example);
}

module_init(misc_init);
module_exit(misc_exit);

MODULE_AUTHOR("Ariel");
MODULE_DESCRIPTION("Example misc driver.");
MODULE_LICENSE("Dual BSD/GPL");