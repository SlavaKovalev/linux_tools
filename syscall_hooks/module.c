#include <linux/kernel.h>
#include <linux/kern_levels.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kallsyms.h>
#include <linux/unistd.h>
#include "syscall_hook.h"
#include "transport.h"

static int __init mdl_init(void) {
	int ret = transport_mod_init();
	if (ret) {
		printk(KERN_ERR "Can't create transport");
		return ret;
	}

	return ret;
}

static void __exit mdl_exit(void){
	transport_mod_down();
}

module_init(mdl_init);
module_exit(mdl_exit);

MODULE_LICENSE("GPL");
