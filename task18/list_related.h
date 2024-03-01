#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>

#define NAME_MAX_LENGTH 19
#define NAME_MAX_LENGTH_WITH_NULL NAME_MAX_LENGTH + 1

struct identity {
    char name[NAME_MAX_LENGTH  + 1];
    int id;
    bool busy;
};

struct my_list{
     struct list_head list;     //linux kernel list implementation
     struct identity data;
};

extern struct kmem_cache *my_list_cache;

int identity_create(char *name, int id);

struct identity *identity_find(int id);

void identity_destroy(int id);

void delete_list(void);

struct identity *identity_get(void);