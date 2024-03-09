/* 
 * hello-sysfs.c sysfs example 
 */ 
#include <linux/fs.h> 
#include <linux/init.h> 
#include <linux/kobject.h> 
#include <linux/module.h> 
#include <linux/string.h> 
#include <linux/sysfs.h> 

#define EUDYPTULA_ID "fake_id_123"
#define EUDYPTULA_ID_LENGTH 11 // Without the NULL

static char foo_data[PAGE_SIZE] = {0};
static DEFINE_MUTEX(mymutex);

static struct kobject *mymodule; 
 
static ssize_t id_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) 
{ 
    return sprintf(buf, "%s\n", EUDYPTULA_ID);
} 

static ssize_t id_store(struct kobject *kobj, struct kobj_attribute *attr, char *buf, size_t len) 
{ 
        if (len != EUDYPTULA_ID_LENGTH) {
		pr_alert("message length isn't good. %lu != %d\n", len, EUDYPTULA_ID_LENGTH);
		return -EINVAL;
	}

	char message[EUDYPTULA_ID_LENGTH] = {0};
        sscanf(buf, "%s", &message);

	if (strncmp(message, EUDYPTULA_ID, EUDYPTULA_ID_LENGTH)) {
		pr_alert("'%s' != '%s'\n", message, EUDYPTULA_ID);
		return -EINVAL;
	}

	pr_alert("'%s' == '%s'\n", message, EUDYPTULA_ID);
	return len;
} 

static ssize_t jiffies_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) 
{ 
    return sprintf(buf, "%llu\n", get_jiffies_64()); 
} 

static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) 
{ 
        mutex_lock(&mymutex);
        int rv = sprintf(buf, "%s\n", foo_data); 
        mutex_unlock(&mymutex);
        return rv;
} 

static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr, char *buf, size_t len) 
{ 
        if(len >= PAGE_SIZE)
        {
            pr_alert("foo input size: %d > PAGE_SIZE.  Truncating\n", len);
        }
        int number_of_bytes_to_read = len < PAGE_SIZE - 1 ? len : PAGE_SIZE - 1;
        
        mutex_lock(&mymutex);
        memset(foo_data, 0, sizeof(foo_data));
        strncpy(foo_data, buf, number_of_bytes_to_read);
        pr_alert("foo recived: '%s'\n", foo_data);
        mutex_unlock(&mymutex);

	    return len;
} 

static struct kobj_attribute id_attribute = 
    __ATTR(id, 0664, id_show, (void *)id_store); 

static struct kobj_attribute jiffies_attribute = 
    __ATTR(jiffies, 0444, jiffies_show, NULL); 

static struct kobj_attribute foo_attribute = 
    __ATTR(foo, 0664, foo_show, (void *)foo_store); 
 
static int __init mymodule_init(void) 
{ 
    int error = 0; 
 
    pr_info("mymodule: initialised\n"); 
 
    mymodule = kobject_create_and_add("eudyptula2", kernel_kobj); 
    if (!mymodule) 
        return -ENOMEM; 
 
    error = sysfs_create_file(mymodule, &id_attribute.attr); 
    if (error) { 
        pr_alert("failed to create /sys/kernel/eudyptula/id\n"); 
    } 

    error = sysfs_create_file(mymodule, &jiffies_attribute.attr); 
    if (error) { 
        pr_alert("failed to create /sys/kernel/eudyptula/jiffies\n"); 
    } 

    error = sysfs_create_file(mymodule, &foo_attribute.attr); 
    if (error) { 
        pr_alert("failed to create /sys/kernel/eudyptula/foo\n"); 
    } 
 
    return error; 
} 
 
static void __exit mymodule_exit(void) 
{ 
    pr_info("mymodule: Exit success\n"); 
    kobject_put(mymodule); 
} 
 
module_init(mymodule_init); 
module_exit(mymodule_exit); 
 
MODULE_LICENSE("GPL");