#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/debugfs.h>

struct identity {
    char name[20];
    int id;
    bool busy;
};

struct my_list{
     struct list_head list;     //linux kernel list implementation
     struct identity data;
};

static struct dentry *dir = 0;
static struct kmem_cache *my_list_cache;

LIST_HEAD(Head_Node);

int identity_create(char *name, int id)
{
    struct my_list * temp_node = kmem_cache_alloc(my_list_cache, GFP_KERNEL);
    if(unlikely(temp_node == NULL))
    {
        pr_alert("kmem_cache_alloc failed\n");
        return ENOMEM;
    }

    memset(temp_node->data.name, 0, 20);
    memcpy(temp_node->data.name, name, strlen(name));
    temp_node->data.id = id;
    temp_node->data.busy = false;

    list_add(&temp_node->list, &Head_Node);
    pr_alert("Created: name: '%s'  id: %id\n", temp_node->data.name, temp_node->data.id);
    return 0;
}

struct identity *identity_find(int id)
{
    struct my_list *temp;
    list_for_each_entry(temp, &Head_Node, list) {
        if(temp->data.id == id)
        {
            pr_alert("Found:  temp->data.id: %d   temp->data.name: '%s'\n", temp->data.id, temp->data.name);
            return &temp->data;
        }
    }

    pr_alert("Didn't find id: %i d\n", id);
    return NULL;
}

void identity_destroy(int id)
{
    struct my_list *cursor, *temp;
    list_for_each_entry_safe(cursor, temp, &Head_Node, list) {
        if(cursor->data.id == id)
        {
            pr_alert("Removing:  cursor->data.id: %d   cursor->data.name: '%s'\n", cursor->data.id, cursor->data.name);
            list_del(&cursor->list);
            kmem_cache_free(my_list_cache, cursor);
        }
    }
}

static ssize_t identity_destroy_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
    int id;
    char input[10] = {0};
    int number_of_bytes_to_read = len < sizeof(input) ? len : sizeof(input);
    copy_from_user(input, buf, number_of_bytes_to_read);
    sscanf(input, "%d", &id);
    identity_destroy(id);
    return len;
}

static ssize_t identity_find_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
    int id;
    char input[10] = {0};
    int number_of_bytes_to_read = len < sizeof(input) ? len : sizeof(input);
    copy_from_user(input, buf, number_of_bytes_to_read);
    sscanf(input, "%d", &id);
    identity_find(id);
    return len;
}

static ssize_t identity_create_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
    int id;
    char name[20] = {0};
    char input[31] = {0};
    int number_of_bytes_to_read = len < sizeof(input) ? len : sizeof(input);
    copy_from_user(input, buf, number_of_bytes_to_read);
    sscanf(input, "%d %s", &id, &name);
    identity_create(name, id);
    return len;
}

static struct file_operations identity_destroy_fops = { 
        .owner = THIS_MODULE,
        .write = identity_destroy_write, 
};

static struct file_operations identity_find_fops = { 
        .owner = THIS_MODULE,
        .write = identity_find_write, 
};

static struct file_operations identity_create_fops = { 
        .owner = THIS_MODULE,
        .write = identity_create_write, 
};

static int __init LinkedListStart(void) 
{
    pr_alert("__init\n");

    my_list_cache = KMEM_CACHE(my_list, SLAB_POISON); // SLAB_POISON so it will show up in /proc/slabinfo for sure and won't be merged with another identical cache
    if(!my_list_cache)
    {
            pr_alert("KMEM_CACHE failed\n");
            return ENOMEM;
    }

    dir = debugfs_create_dir("eudyptula2", 0);
    if (!dir) {
        pr_err("failed to create /sys/kernel/debug/eudyptula\n");
        return -1;
    }

    struct dentry *return_value = debugfs_create_file(
            "identity_destroy",
            0666,
            dir,
            NULL,
            &identity_destroy_fops);
    if (!return_value) {
        pr_err("failed to create /sys/kernel/debug/eudyptula/identity_destroy\n");
        return -1;
    }

    return_value = debugfs_create_file(
            "identity_create",
            0666,
            dir,
            NULL,
            &identity_create_fops);
    if (!return_value) {
        pr_err("failed to create /sys/kernel/debug/eudyptula/identity_create_fops\n");
        return -1;
    }

    return_value = debugfs_create_file(
            "identity_find",
            0666,
            dir,
            NULL,
            &identity_find_fops);
    if (!return_value) {
        pr_err("failed to create /sys/kernel/debug/eudyptula/identity_find_fops\n");
        return -1;
    }

    return 0;

}

static void __exit LinkedListExit (void) 
{
    pr_alert("__exit\n");
    struct my_list *cursor, *temp;
    list_for_each_entry_safe(cursor, temp, &Head_Node, list) {
        pr_alert("__exit removing: cursor.data.name: '%s'  cursor.data.id: %d\n", cursor->data.name, cursor->data.id);
        list_del(&cursor->list);
        kmem_cache_free(my_list_cache, cursor);
    }
    kmem_cache_destroy(my_list_cache);
    debugfs_remove_recursive(dir);
}

module_init(LinkedListStart);
module_exit(LinkedListExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple example Linux module.");
MODULE_VERSION("0.1");
