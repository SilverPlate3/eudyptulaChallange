#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/delay.h>

#define EUDYPTULA_ID "fake_id_123"
#define EUDYPTULA_ID_LENGTH 11 // Without the NULL


static struct dentry *dir = 0;
static char foo_data[PAGE_SIZE] = {0};
static DEFINE_MUTEX(mymutex);

static ssize_t id_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
	if (len != EUDYPTULA_ID_LENGTH) {
		pr_alert("message length isn't good. %lu != %d\n", len, EUDYPTULA_ID_LENGTH);
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

static ssize_t id_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	if (*f_pos >= EUDYPTULA_ID_LENGTH || copy_to_user(buf, EUDYPTULA_ID, EUDYPTULA_ID_LENGTH)) {
		pr_alert("copy_to_user failed\n");
		return 0;
	}
	pr_alert("copy_to_user success\n");
	*f_pos += EUDYPTULA_ID_LENGTH;
	return EUDYPTULA_ID_LENGTH;
}

static ssize_t jiffies_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    if(*f_pos > 0)
    {
        pr_alert("Finished jiffies_read\n");
        return 0;
    }
    
    int bytes_read = 0;
    u64 current_jiffies = get_jiffies_64();
    char jiffies_string[21] = {0};
    bytes_read = snprintf(jiffies_string, 21, "%llu", current_jiffies);
    if(bytes_read <= 0)
    {
        pr_alert("snprintf failed\n");
        return 0;
    }
    jiffies_string[bytes_read - 1] = '\0';

    bytes_read = simple_read_from_buffer(buf, 21, f_pos, jiffies_string, 21);
    pr_alert("Bytes read: %d   pos: %d   jiffies_string: '%s'\n", bytes_read, *f_pos, jiffies_string);
    if(bytes_read < 0)
    {
        pr_alert("simple_read_from_buffer failed\n");
        return 0;
    }

    return bytes_read;
}

static ssize_t foo_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
    if(len > PAGE_SIZE)
    {
        pr_alert("foo input size: %d > PAGE_SIZE.  Truncating\n", len);
    }

    int number_of_bytes_to_read = len < PAGE_SIZE ? len : PAGE_SIZE;

    mutex_lock(&mymutex);
    memset(foo_data, 0, sizeof(foo_data));
	if (copy_from_user(foo_data, buf, number_of_bytes_to_read)) {
        mutex_unlock(&mymutex);
		pr_alert("copy_from_user failed\n");
		return -EFAULT;
	}
    pr_alert("foo recived: '%s'\n", foo_data);
    mutex_unlock(&mymutex);

	return number_of_bytes_to_read;
}

static ssize_t foo_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    const int number_of_bytes_to_write = strlen(foo_data);
    mutex_lock(&mymutex);
	if (*f_pos >= number_of_bytes_to_write || copy_to_user(buf, foo_data, number_of_bytes_to_write)) {
        mutex_unlock(&mymutex);
		pr_alert("copy_to_user failed\n");
		return 0;
	}
    *f_pos += number_of_bytes_to_write;
    mutex_unlock(&mymutex);

	pr_alert("copy_to_user success\n");
	return number_of_bytes_to_write;
}

static struct file_operations id_fops = { 
        .owner = THIS_MODULE,
        .read = id_read, 
        .write = id_write, 
};

static struct file_operations jiffies_fops = { 
        .owner = THIS_MODULE,
        .read = jiffies_read, 
};

static struct file_operations foo_fops = { 
        .owner = THIS_MODULE,
        .read = foo_read, 
        .write = foo_write, 
};

int init_module(void)
{
    dir = debugfs_create_dir("eudyptula3", 0);
    if (!dir) {
        pr_err("failed to create /sys/kernel/debug/eudyptula3\n");
        return -1;
    }

    struct dentry *return_value = debugfs_create_file(
            "id",
            0666,
            dir,
            NULL,
            &id_fops);
    if (!return_value) {
        pr_err("failed to create /sys/kernel/debug/eudyptula3/id\n");
        return -1;
    }

    return_value = debugfs_create_file(
            "jiffies",
            0444,
            dir,
            NULL,
            &jiffies_fops);
    if (!return_value) {
        pr_err("failed to create /sys/kernel/debug/eudyptula3/jiffies\n");
        return -1;
    }

    return_value = debugfs_create_file(
            "foo",
            0644,
            dir,
            NULL,
            &foo_fops);
    if (!return_value) {
        pr_err("failed to create /sys/kernel/debug/eudyptula3/foo\n");
        return -1;
    }

    pr_alert("__init done!\n");
    return 0;
}

void cleanup_module(void)
{
    debugfs_remove_recursive(dir);
    pr_alert("__exit done!\n");
}

MODULE_LICENSE("GPL");