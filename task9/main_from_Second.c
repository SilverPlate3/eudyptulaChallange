#include <linux/module.h> 
#include <linux/printk.h> 
#include <linux/fs.h>
#include <linux/kobject.h>
#include <linux/init.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/minmax.h>
#include <linux/mutex.h>

#define DIR_NAME "eudyptula"
#define EUDYPTULA_ID "f3e7d7d3e3e7"
#define EUDYPTULA_ID_LEN 13
#define EUDYPTULA_ID_LEN_EXCLUDING_NULL EUDYPTULA_ID_LEN - 1

char id[EUDYPTULA_ID_LEN] = EUDYPTULA_ID;
char foo[PAGE_SIZE] = {0};

static DEFINE_MUTEX(mymutex);

static ssize_t id_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "%s\n", id);
}

static ssize_t id_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    pr_info("id_store - 0 %d\n", count);
    if(count != EUDYPTULA_ID_LEN_EXCLUDING_NULL)
        return -EINVAL;

    pr_info("id_store - 1\n");
    char input[EUDYPTULA_ID_LEN_EXCLUDING_NULL] = {0};
    sscanf(buf, "%s", input); 

    pr_info("id_store - 2\ninput: '%s'\nbuf: '%s'\n", input, buf);
    if (strcmp(input, EUDYPTULA_ID) != 0) 
    {
        return -EINVAL;
    }
    pr_info("id_store - 3\n");
    return count;
}

static struct kobj_attribute id_attribute = __ATTR(id, 0644, id_show, id_store);

static ssize_t jiffies_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "%lu\n", jiffies);
}

static struct kobj_attribute jiffies_attribute = __ATTR(jiffies, 0644, jiffies_show, NULL);

static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    mutex_lock(&mymutex);
    int rv = sysfs_emit(buf, "%s\n", foo);
    mutex_unlock(&mymutex);
    return rv;
}

static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    pr_info("foo_write - 0 size: %d\n", count);
    mutex_lock(&mymutex);
    int size = min(count, (size_t)PAGE_SIZE - 1);
    pr_info("foo_write - 1\n");
    memset(foo, 0, PAGE_SIZE);
    sscanf(buf, "%s", foo); 

    pr_info("foo_write - 2. foo: '%s'\n", foo);
    foo[size] = '\0';
    pr_info("foo_write - 3. foo: '%s'\n", foo);
    mutex_unlock(&mymutex);
    return size;
}

static struct kobj_attribute foo_attribute = __ATTR(foo, 0644, foo_show, foo_store);

static struct attribute *attrs[] = {
	&id_attribute.attr,
	&jiffies_attribute.attr,
	&foo_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

static struct kobject *example_kobj;

static int Init(void)
{
    pr_info("111111\n");

    example_kobj = kobject_create_and_add(DIR_NAME, kernel_kobj);
    pr_info("22222222\n");
	if (!example_kobj)
		return -ENOMEM;

    pr_info("3333333\n");
    if(sysfs_create_group(example_kobj, &attr_group))
    {
        pr_info("4444444444\n");
        kobject_put(example_kobj);
        return -1;
    }

    return 0;
}

static void Exit(void)
{
    pr_info("Bye\n");
    kobject_put(example_kobj);
}

module_init(Init);
module_exit(Exit);

MODULE_AUTHOR("Chuckleberryfinn");
MODULE_DESCRIPTION("Example misc driver.");
MODULE_LICENSE("Dual BSD/GPL");