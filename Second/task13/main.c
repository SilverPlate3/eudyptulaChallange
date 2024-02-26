#include <linux/module.h> 
#include <linux/printk.h> 
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/list.h>

struct identity
{
    char  name[20];
    int   id;
    bool  busy;
};

struct my_list{
    struct list_head list;     //linux kernel list implementation
    struct identity data;
};

LIST_HEAD(Head_Node);

static struct kmem_cache *node_cache;

int identity_create(char *name, int id)
{
    struct my_list * node = kmem_cache_alloc(node_cache, GFP_KERNEL);
    if(unlikely(!node))
    {
        pr_info("Failed to allocate memory\n");
        return -ENOMEM;
    }
    INIT_LIST_HEAD(&node->list);
    list_add_tail(&node->list, &Head_Node);
    node->data.id = id;
    strcpy(node->data.name, name);
    pr_info("Created name: '%s'  id: '%d'\n", name, id);

    return 0;
}

struct identity *identity_find(int id)
{
    struct my_list * cursor, * temp;
    list_for_each_entry_safe(cursor, temp, &Head_Node, list) 
    {
        if(cursor->data.id == id)
        {
            pr_info("Found name: '%s'  id: '%d'\n", cursor->data.name, cursor->data.id);
            return &cursor->data;
        }
    }

    pr_info("Not found node: '%d'\n", id);
    return NULL;
}

void identity_destroy(int id)
{
    struct identity * person = identity_find(id);
    if(!person)
    {
        return;
    }

    struct my_list * node = container_of(person, struct my_list, data);
    if(unlikely(!node))
    {
        pr_info("container_of failed\n");
        return;
    }

    pr_info("Removing name: '%s'  id: '%d'\n", node->data.name, node->data.id);
    list_del(&node->list);
    kmem_cache_free(node_cache, node);
}

static int Init(void)
{
    pr_info("Hello task 13\n");
    node_cache = KMEM_CACHE(my_list, SLAB_POISON);
    struct identity *temp;

    identity_create("Alice", 1);
    identity_create("Bob", 2);
    identity_create("Dave", 3);
    identity_create("Gena", 10);

    temp = identity_find(3);
    pr_debug("id 3 = %s\n", temp->name);
    temp = identity_find(42);

    if (temp == NULL)
            pr_debug("id 42 not found\n");
    
    identity_destroy(2);
    identity_destroy(1);
    //identity_destroy(10);
    identity_destroy(42);
    //identity_destroy(3);
    
    return 0;
}

static void Exit(void)
{
    struct my_list *cursor, *temp;
    list_for_each_entry_safe(cursor, temp, &Head_Node, list) 
    {
        pr_info("__exit Removing name: '%s'  id: '%d'\n", cursor->data.name, cursor->data.id);
        list_del(&cursor->list);
        kmem_cache_free(node_cache, cursor);
    }
    kmem_cache_destroy(node_cache);
    pr_info("Bye\n");
}

module_init(Init);
module_exit(Exit);

MODULE_AUTHOR("Chuckleberryfinn");
MODULE_DESCRIPTION("Example misc driver.");
MODULE_LICENSE("Dual BSD/GPL");