#include <linux/kernel.h>
#include <linux/kern_levels.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kallsyms.h>
#include <linux/unistd.h>
#include "syscall_hook.h"

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

static sys_hook_func_t sys_read_orig;
static sys_hook_func_t sys_write_orig;

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
	return sys_read_orig(regs);
}

asmlinkage long sys_write_hook(const struct pt_regs *regs) {
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
		printk(KERN_ERR "unexpected syscall index: %d\n", sys_call_ind);
	}
}

int syscall_hook_activate(void){
	int ret = 0;
	unsigned long irq_flags = 0;
	p_sys_call_table = (void**)find_sym("sys_call_table");
	if (p_sys_call_table == NULL)
		return ENOTSUPP;

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

void  syscall_hook_deactivate(void){
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
}

