#include <linux/module.h> 
#include <linux/printk.h> 

static int __init InitFunction(void) 
{
    pr_debug("Hello World!\n");
    return 0;
}

static void __exit RemoveFunction(void) 
{
    pr_debug("Bye world!\n");
}


module_init(InitFunction); 
module_exit(RemoveFunction); 
 
MODULE_LICENSE("GPL");