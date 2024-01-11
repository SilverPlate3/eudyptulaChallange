// SPDX-License-Identifier: GPL-2.0+
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/kthread.h> 
#include <linux/err.h>

#define EUDYPTULA_ID "fake_id_123"
#define EUDYPTULA_ID_LENGTH 11 // Without the NULL

DECLARE_WAIT_QUEUE_HEAD(wee_wait);
struct task_struct *eudyptula_thread;

static int eudyptula_thread_function(void *arg)
{
	pid_t thread_id = current->pid;
	pr_alert("Thread %d started\n", thread_id);
	int err = wait_event_interruptible(wee_wait, kthread_should_stop());
	pr_alert("Thread %d woke up\n", thread_id);
	return err;
}

// This function will be called when we write the Misc Device file
static ssize_t misc_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
	if (len != EUDYPTULA_ID_LENGTH) {
		pr_alert("message length isn't good. %d != %d\n", len, EUDYPTULA_ID_LENGTH);
		return -EINVAL;
	}

	char message[EUDYPTULA_ID_LENGTH] = {0};

	if (copy_from_user(message, buf, EUDYPTULA_ID_LENGTH)) {
		pr_alert("copy_from_user failed\n");
		return -EFAULT;
	}

	if (strncmp(message, EUDYPTULA_ID, EUDYPTULA_ID_LENGTH)) {
		pr_alert("'%s' != '%s'\n", message, EUDYPTULA_ID);
		return -EINVAL;
	}

	pr_alert("'%s' == '%s'\n", message, EUDYPTULA_ID);
	return len;
}

// File operation structure
static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = misc_write,
};
// Misc device structure
struct miscdevice misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "eudyptula",
    .fops = &fops,
	.mode = S_IWUGO,
};

static int __init misc_init(void)
{
    int error;

    error = misc_register(&misc_device);
    if (error) {
	pr_err("misc_register failed!!!\n");
	return error;
    }

	eudyptula_thread = kthread_create(eudyptula_thread_function, NULL, "eudyptula thread"); 
    if (IS_ERR(eudyptula_thread)) {
		goto ERROR_THREAD;
	}  
	wake_up_process(eudyptula_thread); 

    pr_info("misc_register init done!!!\n");
    return 0;

ERROR_THREAD:
	return -1;

}

static void __exit misc_exit(void)
{
	pr_info("__exit start\n");
	kthread_stop(eudyptula_thread);
    misc_deregister(&misc_device);
    pr_info("__exit done\n");
}

module_init(misc_init)
module_exit(misc_exit)

MODULE_LICENSE("GPL");
