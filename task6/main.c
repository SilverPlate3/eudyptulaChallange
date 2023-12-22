// SPDX-License-Identifier: GPL-2.0+
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#define EUDYPTULA_ID "fake_id_123"
#define EUDYPTULA_ID_LENGTH 11 // Without the NULL

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

// This function will be called when we read the Misc Device file
static ssize_t misc_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
        if (*f_pos >= EUDYPTULA_ID_LENGTH || copy_to_user(buf, EUDYPTULA_ID, EUDYPTULA_ID_LENGTH)) {
                pr_alert("copy_to_user failed\n");
                return 0;
        }
        pr_alert("copy_to_user success\n");
        *f_pos += EUDYPTULA_ID_LENGTH;
        return EUDYPTULA_ID_LENGTH;
}
// File operation structure 
static const struct file_operations fops = {
    .owner          = THIS_MODULE,
    .write          = misc_write,
    .read           = misc_read,
};
// Misc device structure
struct miscdevice misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "eudyptula",
    .fops = &fops,
};

static int __init misc_init(void)
{
    int error;

    error = misc_register(&misc_device);
    if (error) {
        pr_err("misc_register failed!!!\n");
        return error;
    }

    pr_info("misc_register init done!!!\n");
    return 0;
}

static void __exit misc_exit(void)
{
    misc_deregister(&misc_device);
    pr_info("misc_register exit done!!!\n");
}

module_init(misc_init)
module_exit(misc_exit)

MODULE_LICENSE("GPL");
