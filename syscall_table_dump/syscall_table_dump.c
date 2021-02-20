#include <linux/kernel.h>
#include <linux/kern_levels.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kallsyms.h>
#include <asm/unistd.h>

static void **p_sys_call_table = NULL;

struct syscall_entry{
	int pos;
	char* name;
};

struct syscall_entry syscalls_map[] = 
	{{__NR_creat, "creat"},
	{__NR_open, "open"},
	{__NR_openat, "openat"},
	{__NR_write, "write"},
	{__NR_pwrite64, "pwrite64"},
	{__NR_writev, "writev"},
	{__NR_pwritev, "pwritev"},
	{__NR_pwritev2, "pwritev2"},
	{__NR_rename, "rename"},
	{__NR_renameat, "renameat"},
	{__NR_renameat2, "renameat2"},
	{__NR_unlink, "unlink"},
	{__NR_unlinkat, "unlinkat"}};

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

int __init syscall_table_dump_init(void){
	int i = 0;
	p_sys_call_table = (void**)find_sym("sys_call_table");
	for (i = 0; i < sizeof(syscalls_map)/sizeof(struct syscall_entry); ++i){
		printk("%s=%p\n", syscalls_map[i].name, p_sys_call_table[syscalls_map[i].pos]);
	}
	return 0;
}

void __exit syscall_table_dump_exit(void){
}

module_init(syscall_table_dump_init);
module_exit(syscall_table_dump_exit);

MODULE_LICENSE("GPL");
