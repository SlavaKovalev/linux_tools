#include <linux/kernel.h>
#include <linux/kern_levels.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kallsyms.h>
#include <linux/unistd.h>

/*
 * https://elixir.bootlin.com/linux/latest/source/arch/x86/entry/syscalls/syscall_64.tbl:
 *
 * 0	common	read			sys_read
 * 1	common	write			sys_write
 * ...
 */

static void **p_sys_call_table = NULL;

extern unsigned long __force_order;

typedef asmlinkage long (*sys_hook_func_t)(const struct pt_regs *regs);

sys_hook_func_t sys_read_orig;
sys_hook_func_t sys_write_orig;

static unsigned int sys_read_count = 0;
static unsigned int sys_write_count = 0;

#define HOOK_PROLOG() __module_get(THIS_MODULE)
#define HOOK_EPILOG() module_put(THIS_MODULE)

inline void mywrite_cr0(unsigned long cr0) {
  asm volatile("mov %0,%%cr0" : "+r"(cr0), "+m"(__force_order));
}

void disable_write_protection(void) {
  unsigned long cr0 = read_cr0();
  clear_bit(16, &cr0);
  mywrite_cr0(cr0);
}

void enable_write_protection(void) {
  unsigned long cr0 = read_cr0();
  set_bit(16, &cr0);
  mywrite_cr0(cr0);
}

asmlinkage long sys_read_hook(const struct pt_regs *regs) {
	++sys_read_count;
	return sys_read_orig(regs);
}

asmlinkage long sys_write_hook(const struct pt_regs *regs) {
	++sys_write_count;
	return sys_write_orig(regs);
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

void hook_sys_call(int sys_call_ind) {
	switch (sys_call_ind)
	{
	case __NR_read:
	{
		sys_read_orig = p_sys_call_table[__NR_read];
		p_sys_call_table[__NR_read] = (void*)&sys_read_hook;
		break;
	}
	case __NR_write:
	{
		sys_write_orig = p_sys_call_table[__NR_write];
		p_sys_call_table[__NR_write] = (void*)&sys_write_hook;
		break;
	}
	default:
		printk("unexpected syscall index: %d\n", sys_call_ind);
	}
}

int __init syscall_hook_mod_init(void){
	int ret = 0;
	unsigned long irq_flags = 0;
	printk("[syscall_hook] init\n");
	p_sys_call_table = (void**)find_sym("sys_call_table");
	printk("[syscall_hook] sys_call_table %u\n", p_sys_call_table);

	preempt_disable();
	local_irq_save(irq_flags);
	disable_write_protection();
	
	hook_sys_call(__NR_read);
	hook_sys_call(__NR_write);
	
	enable_write_protection();
	local_irq_restore(irq_flags);
	irq_flags = 0;
	preempt_enable();

	return ret;
}

void __exit syscall_hook_mod_exit(void){
	printk("[syscall_hook] exit\n");
	preempt_disable();
	unsigned long irq_flags = 0;
	local_irq_save(irq_flags);
	disable_write_protection();
	p_sys_call_table[__NR_read] = sys_read_orig;
	p_sys_call_table[__NR_write] = sys_write_orig;
	enable_write_protection();
	local_irq_restore(irq_flags);
	irq_flags = 0;
	preempt_enable();
	printk("sys_read_count=%u\n", sys_read_count);
	printk("sys_write_count=%u\n", sys_write_count);
}

module_init(syscall_hook_mod_init);
module_exit(syscall_hook_mod_exit);

MODULE_LICENSE("GPL");
