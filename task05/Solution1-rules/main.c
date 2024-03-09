#include <linux/module.h> 
#include <linux/printk.h> 

static int __init InitFunction(void) 
{
    pr_alert("Hotplug kmod loaded\n");
    return 0;
}

static void __exit RemoveFunction(void) 
{
    pr_alert("Hotplug kmod unloaded\n");
}


module_init(InitFunction); 
module_exit(RemoveFunction); 
 
MODULE_LICENSE("GPL");