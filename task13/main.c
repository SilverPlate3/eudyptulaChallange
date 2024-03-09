#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>

struct identity {
    char name[20];
    int id;
    bool busy;
};

struct my_list{
     struct list_head list;     //linux kernel list implementation
     struct identity data;
};

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

static int __init LinkedListStart(void) 
{
    pr_alert("__init\n");

    my_list_cache = KMEM_CACHE(my_list, SLAB_POISON); // SLAB_POISON so it will show up in /proc/slabinfo for sure and won't be merged with another identical cache
    if(!my_list_cache)
    {
            pr_alert("KMEM_CACHE failed\n");
            return ENOMEM;
    }

    struct identity *temp;
    int rv;

    rv = identity_create("Alice", 1);
    if(rv)
        return rv;

    rv = identity_create("Bob", 2);
    if(rv)
        return rv;

    rv = identity_create("Dave", 3);
    if(rv)
        return rv;

    rv = identity_create("Gena", 10);
    if(rv)
        return rv;

    rv = identity_create("JJ", 31);
    if(rv)
        return rv;

    temp = identity_find(3);
    pr_alert("id 3 = %s\n", temp->name);

    temp = identity_find(42);
    if (temp == NULL)
            pr_alert("id 42 not found\n");
    
    identity_destroy(2);
    //identity_destroy(1);
    identity_destroy(10);
    identity_destroy(42);
    //identity_destroy(3);

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
}

module_init(LinkedListStart);
module_exit(LinkedListExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ariel");
MODULE_DESCRIPTION("A simple example Linux module.");
MODULE_VERSION("0.1");
