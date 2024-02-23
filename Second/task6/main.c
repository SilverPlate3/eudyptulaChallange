#include <linux/module.h> 
#include <linux/printk.h> 
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/module.h>

#define DEVICE_NAME "eudyptula"
#define EUDYPTULA_ID "f3e7d7d3e3e7"
#define EUDYPTULA_ID_LEN 13
#define EUDYPTULA_ID_LEN_EXCLUDING_NULL EUDYPTULA_ID_LEN - 1

static ssize_t misc_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    if (*f_pos != 0)
        return 0;

    if (copy_to_user(buf, EUDYPTULA_ID, EUDYPTULA_ID_LEN))
        return -EFAULT;

    *f_pos += EUDYPTULA_ID_LEN;
    return EUDYPTULA_ID_LEN;
}

static ssize_t misc_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    if(count != EUDYPTULA_ID_LEN_EXCLUDING_NULL)
        return -EINVAL;

    char input[EUDYPTULA_ID_LEN] = {0};
    if (copy_from_user(input, buf, EUDYPTULA_ID_LEN_EXCLUDING_NULL)) 
    {
        return -EFAULT;
    }
    input[EUDYPTULA_ID_LEN_EXCLUDING_NULL] = '\0';

    if (strcmp(input, "f3e7d7d3e3e7") != 0) 
    {
        return -EINVAL;
    }

    return count;
}

const struct file_operations misc_fops = {
        .owner = THIS_MODULE,
        .read = misc_read,
        .write = misc_write,
};

static struct miscdevice misc_example = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = DEVICE_NAME,
        .fops = &misc_fops,
};
 
static int misc_init(void)
{
        pr_debug("Hello there\nMy minor number is: %i\n", misc_example.minor);
        return misc_register(&misc_example);
}

static void misc_exit(void)
{
        pr_debug("Goodbye there\n");
        misc_deregister(&misc_example);
}

module_init(misc_init);
module_exit(misc_exit);

MODULE_AUTHOR("Chuckleberryfinn");
MODULE_DESCRIPTION("Example misc driver.");
MODULE_LICENSE("Dual BSD/GPL");