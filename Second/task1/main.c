#include <linux/module.h> 
#include <linux/printk.h> 
 
int init_module(void) 
{ 
    pr_debug("Hello world 3.\n"); 
    return 0; 
} 
 
void cleanup_module(void) 
{ 
    pr_debug("Goodbye world 3.\n"); 
} 
 
MODULE_LICENSE("GPL");