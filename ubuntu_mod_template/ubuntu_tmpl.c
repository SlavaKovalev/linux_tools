#include <linux/kernel.h>
#include <linux/kern_levels.h>
#include <linux/module.h>
#include <linux/printk.h>

int __init ubuntu_tmpl_init(void){
	printk("ubuntu_tmpl_init\n");
	return 0;
}

void __exit ubuntu_tmpl_exit(void){
	printk("ubuntu_tmpl_exit\n");
}

module_init(ubuntu_tmpl_init);
module_exit(ubuntu_tmpl_exit);

MODULE_LICENSE("GPL");
