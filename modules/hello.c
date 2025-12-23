#include <linux/module.h>    // Required for all kernel modules
#include <linux/init.h>      // For module_init and module_exit macros
#include <linux/kernel.h>    // For printk and log levels
#include <linux/delay.h>

static int x;

static void testfunc(void){
	//msleep(1000);
	x = x+1;
    printk(KERN_INFO "x is %d\n",x);
}

// Function called when the module is loaded
static int __init hello_init(void) {
    printk(KERN_INFO "Hello, World! Kernel module loaded.\n");
	for (int j=0; j < 10; j++) {
		testfunc();
	}
    return 0;  // Success
}

// Function called when the module is unloaded
static void __exit hello_exit(void) {
    printk(KERN_INFO "Goodbye, World! Kernel module unloaded.\n");
}

// Register the initialization and exit functions
module_init(hello_init);
module_exit(hello_exit);

// Module metadata
MODULE_LICENSE("GPL");                // License type (GPL is common for open-source)
MODULE_AUTHOR("Your Name");           // Author information
MODULE_DESCRIPTION("Simple Hello World Kernel Module");  // Brief description
MODULE_VERSION("1.0");                // Version
