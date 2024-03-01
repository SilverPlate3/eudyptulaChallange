
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>

#include "list_related.h"

struct kmem_cache *my_list_cache;

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

    list_add_tail(&temp_node->list, &Head_Node);
    pr_alert("Created: name: '%s'  id: %i\n", temp_node->data.name, temp_node->data.id);
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

struct identity *identity_get(void)
{
    if(list_empty(&Head_Node))
    {
        pr_info("identity_get: list is empty\n");
        return NULL;
    }
    struct my_list * previous_head = list_first_entry(&Head_Node, struct my_list, list);
    list_del(&previous_head->list);

    // Task wants the caller to release the memory of identity*
    // But in order to not have a memory leak we must free the memory of previous_head as well
    // But beacuse previous_head contains identity we must create a copy of identity and return the copy after freeing previous_head.
    struct identity * rv = kmalloc(sizeof(struct identity), GFP_KERNEL);
    if(unlikely(rv == NULL))
    {
        pr_alert("kmalloc failed\n");
        kmem_cache_free(my_list_cache, previous_head);
        return NULL;
    }

    memcpy(rv, &previous_head->data, sizeof(struct identity));
    kmem_cache_free(my_list_cache, previous_head);
    pr_info("identity_get: name: '%s'  id: %id\n", rv->name, rv->id);
    return rv;
}

void delete_list(void)
{
    struct my_list *cursor, *temp;
    list_for_each_entry_safe(cursor, temp, &Head_Node, list) {
        pr_alert("delete_list removing: cursor.data.name: '%s'  cursor.data.id: %d\n", cursor->data.name, cursor->data.id);
        list_del(&cursor->list);
        kmem_cache_free(my_list_cache, cursor);
    }
}

MODULE_LICENSE("Dual BSD/GPL");