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
#include <linux/jiffies.h>

#include "list_related.h"

#define DEVICE_NAME "eudyptula"

static struct task_struct *tsk;
static DECLARE_WAIT_QUEUE_HEAD(wq);
static int id_counter = 0;
static int condition = 0;

static int Foo(void * arg)
{
    pr_info("Foo started\n");
    while(!kthread_should_stop())
    {
        int wait_rv = wait_event_interruptible_timeout(wq, condition != 0 || kthread_should_stop(), msecs_to_jiffies(5000));
        if(kthread_should_stop())
        {
            pr_info("Foo: kthread_should_stop() - returning\n");
            return 0;
        }
        if(wait_rv == 0 || wait_rv == 1)
        {
            pr_info("Foo: Wait event timeout. condition true: %d\n", wait_rv);
        }
        else if(wait_rv == -ERESTARTSYS)
        {
            pr_info("Foo: Wait event interrupted\n");
        }
        else 
        {
            pr_info("Foo: Wait event condition true\n");
            struct identity * temp = identity_get();
            if(temp)
            {
                pr_info("Foo: name: '%s' id: '%d'\n", temp->name);
                kfree(temp);
                temp = NULL;
            }
        }

        condition = 0;
    }

    return 0;
}

static ssize_t misc_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    int len = count < NAME_MAX_LENGTH ? count : NAME_MAX_LENGTH;
    char name[NAME_MAX_LENGTH] = {0};
    if(copy_from_user(name, buf, NAME_MAX_LENGTH))
    {
        pr_alert("misc_write: copy_from_user failed\n");
        return -EFAULT;
    }

    if(identity_create(name, id_counter))
    {
        pr_alert("misc_write: identity_create failed\n");
        return -ENOMEM;
    }
    ++id_counter;

    condition = 1;
    wake_up_interruptible(&wq);
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
        my_list_cache = KMEM_CACHE(my_list, SLAB_POISON);
         if(!my_list_cache)
        {
            pr_alert("KMEM_CACHE failed\n");
            return ENOMEM;
        }

        identity_create("Alice", 1);
        ++id_counter;
        identity_create("Bob", 2);
        ++id_counter;

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
        delete_list();
        kmem_cache_destroy(my_list_cache);
        pr_info("kthread_stop returned %d\n", kthread_stop(tsk));
        misc_deregister(&misc_example);
}

module_init(misc_init);
module_exit(misc_exit);

MODULE_AUTHOR("Ariel");
MODULE_DESCRIPTION("Example misc driver.");
MODULE_LICENSE("Dual BSD/GPL");