#include <linux/kernel.h>
#include <linux/kern_levels.h>
#include <linux/module.h>
#include <linux/printk.h>

int __init centos_tmpl_init(void){
	printk("centos_mod_tmpl init\n");
	return 0;
}

void __exit centos_tmpl_exit(void){
	printk("centos_mod_tmpl exit\n");
}

module_init(centos_tmpl_init);
module_exit(centos_tmpl_exit);

MODULE_LICENSE("GPL");
