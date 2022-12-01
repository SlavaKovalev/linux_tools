#include <linux/kernel.h>
#include <linux/kern_levels.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kallsyms.h>
#include <asm/unistd.h>

/*
 * https://elixir.bootlin.com/linux/latest/source/arch/x86/entry/syscalls/syscall_64.tbl:
 *
 * 0	common	read			sys_read
 * 1	common	write			sys_write
 * 2	common	open			sys_open
 * 3	common	close			sys_close
 * 4	common	stat			sys_newstat
 * 5	common	fstat			sys_newfstat
 * 6	common	lstat			sys_newlstat
 * 7	common	poll			sys_poll
 * 8	common	lseek			sys_lseek
 * 9	common	mmap			sys_mmap
 * 10	common	mprotect		sys_mprotect
 * 11	common	munmap			sys_munmap
 * 12	common	brk			sys_brk
 * 13	64	rt_sigaction		sys_rt_sigaction
 * 14	common	rt_sigprocmask		sys_rt_sigprocmask
 * 15	64	rt_sigreturn		sys_rt_sigreturn
 * 16	64	ioctl			sys_ioctl
 * 17	common	pread64			sys_pread64
 * 18	common	pwrite64		sys_pwrite64
 * 19	64	readv			sys_readv
 * 20	64	writev			sys_writev
 * 21	common	access			sys_access
 * 22	common	pipe			sys_pipe
 * 23	common	select			sys_select
 * 24	common	sched_yield		sys_sched_yield
 * 25	common	mremap			sys_mremap
 * 26	common	msync			sys_msync
 * 27	common	mincore			sys_mincore
 * 28	common	madvise			sys_madvise
 * 29	common	shmget			sys_shmget
 * 30	common	shmat			sys_shmat
 * 31	common	shmctl			sys_shmctl
 * 32	common	dup			sys_dup
 * 33	common	dup2			sys_dup2
 * 34	common	pause			sys_pause
 * 35	common	nanosleep		sys_nanosleep
 * 36	common	getitimer		sys_getiti
 * ...
 */

static void **p_sys_call_table = NULL;

struct syscall_entry{
	int pos;
	char* name;
};
struct syscall_entry syscalls_map[] = 
	{{__NR_read, "read"},
	{__NR_creat, "creat"},
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
	printk("sys_call_tabe=%u\n", p_sys_call_table);
	for (i = 0; i < sizeof(syscalls_map)/sizeof(struct syscall_entry); ++i){
		printk("%s[%u]=%u\n", syscalls_map[i].name, syscalls_map[i].pos, (unsigned int)p_sys_call_table[syscalls_map[i].pos]);
	}
	return 0;
}

void __exit syscall_table_dump_exit(void){
}

module_init(syscall_table_dump_init);
module_exit(syscall_table_dump_exit);

MODULE_LICENSE("GPL");
