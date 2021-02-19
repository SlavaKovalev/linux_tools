#include <linux/kernel.h>
#include <linux/kern_levels.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kallsyms.h>

static void **p_sys_call_table = NULL;

extern unsigned long __force_order;

void *sys_open_orig;

typedef asmlinkage long (*sys_open_func_t)(const struct pt_regs *regs);

inline void mywrite_cr0(unsigned long cr0) {
  asm volatile("mov %0,%%cr0" : "+r"(cr0), "+m"(__force_order));
}

void enable_write_protection(void) {
  unsigned long cr0 = read_cr0();
  set_bit(16, &cr0);
  mywrite_cr0(cr0);
}

void disable_write_protection(void) {
  unsigned long cr0 = read_cr0();
  clear_bit(16, &cr0);
  mywrite_cr0(cr0);
}

asmlinkage long sys_open_hook(const struct pt_regs *regs) {
	long ret = -EINVAL;
	
	printk("di %s, si %lu, dx %lu", (char*)&regs->di, regs->si, regs->dx);
	
	//HOOK_PROLOG();
	sys_open_func_t sys_open_orig;
	sys_open_orig = sys_open_orig;
	ret = sys_open_orig(regs);

	//HOOK_EPILOG();

	return ret;
}

static unsigned long find_sym(const char *sym){ 
	static unsigned long faddr = 0; 
	int symb_fn(void *data, const char *sym, struct module *mod, unsigned long addr) { 
		if (0 == strcmp((char*)data, sym)) { 
			faddr = addr; 
			return 1; 
		} 
		else return 0; 
	}; 
 
	kallsyms_on_each_symbol(symb_fn, (void*)sym); 
	return faddr; 
}

int __init syscall_hook_mod_init(void){
	int ret = 0;
	p_sys_call_table = (void**)find_sym("sys_call_table");

	disable_write_protection();

	sys_open_orig = (void *)p_sys_call_table[2];
	p_sys_call_table[2] = sys_open_hook;

	enable_write_protection();

	return ret;
}

void __exit syscall_hook_mod_exit(void){
	disable_write_protection();
	p_sys_call_table[2] = sys_open_orig;
	enable_write_protection();
}

module_init(syscall_hook_mod_init);
module_exit(syscall_hook_mod_exit);

MODULE_LICENSE("GPL");
