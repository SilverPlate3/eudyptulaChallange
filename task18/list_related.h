#include <linux/kernel.h>

struct identity {
    char name[20];
    int id;
    bool busy;
};

struct my_list{
     struct list_head list;     //linux kernel list implementation
     struct identity data;
};

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
