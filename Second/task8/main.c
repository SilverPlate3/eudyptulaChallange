#include <linux/module.h> 
#include <linux/printk.h> 
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/minmax.h>
#include <linux/mutex.h>

#define DEVICE_NAME "eudyptula"
#define EUDYPTULA_ID "f3e7d7d3e3e7"
#define EUDYPTULA_ID_LEN 13
#define EUDYPTULA_ID_LEN_EXCLUDING_NULL EUDYPTULA_ID_LEN - 1

char id[EUDYPTULA_ID_LEN] = "eudyptula";
char foo[PAGE_SIZE] = {0};

static DEFINE_MUTEX(mymutex);

static struct dentry *dir = NULL;

static ssize_t id_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    if (*f_pos != 0)
        return 0;

    if (copy_to_user(buf, EUDYPTULA_ID, EUDYPTULA_ID_LEN))
        return -EFAULT;

    *f_pos += EUDYPTULA_ID_LEN;
    return EUDYPTULA_ID_LEN;
}

static ssize_t id_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
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

const struct file_operations id_fops = {
        .owner = THIS_MODULE,
        .read = id_read,
        .write = id_write,
};

static ssize_t jiffies_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    if(*f_pos != 0)
        return 0;
    pr_info("jiffies_read - 0");

    char jiffies_str[21] = {0};
    int rv = snprintf(jiffies_str, sizeof(jiffies_str), "%lu", jiffies);
    pr_info("jiffies_read - 1");
    if (copy_to_user(buf, jiffies_str, strlen(jiffies_str)))
        return -EFAULT;

    pr_info("jiffies_read - 2");
    *f_pos += rv;
    return rv;
}

const struct file_operations jiffies_fops = {
        .owner = THIS_MODULE,
        .read = jiffies_read,
};

static ssize_t foo_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    pr_info("foo_read - 0");
    mutex_lock(&mymutex);
    if (*f_pos != 0)
    {
        mutex_unlock(&mymutex);
        return 0;
    }

    pr_info("foo_read - 1");
    if (copy_to_user(buf, foo, strlen(foo)))
    {
        mutex_unlock(&mymutex);
        return -EFAULT;
    }

    *f_pos += strlen(foo);
    pr_info("foo_read - 2");
    mutex_unlock(&mymutex);
    return *f_pos;
}

static ssize_t foo_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    pr_info("foo_write - 0");
    mutex_lock(&mymutex);
    int size = min(count, (size_t)PAGE_SIZE - 1);
    pr_info("foo_write - 1");
    memset(foo, 0, PAGE_SIZE);
    if (copy_from_user(foo, buf, size))
    {
        mutex_unlock(&mymutex);
        return -EFAULT;
    }

    pr_info("foo_write - 2");
    foo[size] = '\0';
    pr_info("foo_write - 3");
    mutex_unlock(&mymutex);
    return size;
}

const struct file_operations foo_fops = {
        .owner = THIS_MODULE,
        .write = foo_write,
        .read = foo_read,
};

static int Init(void)
{
    pr_info("Hello\n");

    dir = debugfs_create_dir(id, 0);
    if (!dir) {
        pr_err("Failed to create debugfs directory\n");
        return -1;
    }

    if(debugfs_create_file("id", 0666, dir, NULL, &id_fops) == NULL) {
        pr_err("Failed to create debugfs file id\n");
        goto error;
    }

    if(debugfs_create_file("jiffies", 0444, dir, NULL, &jiffies_fops) == NULL) {
        pr_err("Failed to create debugfs file jiffies\n");
        goto error;
    }

    if(debugfs_create_file("foo", 0666, dir, NULL, &foo_fops) == NULL) {
        pr_err("Failed to create debugfs file foo\n");
        goto error;
    }
    
    return 0;

error:
    debugfs_remove_recursive(dir);
    return -1;
}

static void Exit(void)
{
    pr_info("Bye\n");
    debugfs_remove_recursive(dir);
}

module_init(Init);
module_exit(Exit);

MODULE_AUTHOR("Chuckleberryfinn");
MODULE_DESCRIPTION("Example misc driver.");
MODULE_LICENSE("Dual BSD/GPL");