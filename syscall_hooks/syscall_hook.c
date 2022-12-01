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

extern unsigned long __force_order;

typedef asmlinkage long (*sys_open_func_t)(const char __user *filename, int flags, umode_t mode);
typedef asmlinkage long (*sys_creat_func_t)(const char __user *pathname, umode_t mode);
typedef asmlinkage long (*sys_read_func_t)(int fd, char __user *buf, size_t count);
typedef asmlinkage long (*sys_write_func_t)(int fd, char __user *buf, size_t count);
typedef asmlinkage long (*sys_unlink_func_t)(char __user *buf);

sys_open_func_t sys_open_orig;
sys_creat_func_t sys_creat_orig;
sys_read_func_t sys_read_orig;
sys_write_func_t sys_write_orig;
sys_unlink_func_t sys_unlink_orig;

static unsigned int sys_creat_count = 0;
static unsigned int sys_read_count = 0;
static unsigned int sys_open_count = 0;
static unsigned int sys_write_count = 0;
static unsigned int sys_unlink_count = 0;

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

asmlinkage long sys_open_hook(const char __user *filename, int flags, umode_t mode) {
	++sys_open_count;
	return sys_open_orig(filename, flags, mode);
}

asmlinkage long sys_unlink_hook(char __user *filename) {
	++sys_unlink_count;
	return sys_unlink_orig(filename);
}

asmlinkage long sys_creat_hook(const char __user *pathname, umode_t mode) {
	++sys_creat_count;
	return sys_creat_orig(pathname, mode);
}

asmlinkage long sys_read_hook(int fd, char __user *buf, size_t count) {
	++sys_read_count;
	if (sys_read_count == 999) {
		printk("fd %d, buf %s, count %u\n", fd, buf, count);
	}
	return sys_read_orig(fd, buf, count);
}

asmlinkage long sys_write_hook(int fd, char __user *buf, size_t count) {
	++sys_write_count;
	return sys_write_orig(fd, buf, count);
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
	case __NR_open:
	{
		sys_open_orig = p_sys_call_table[__NR_open];
		p_sys_call_table[__NR_open] = (void*)&sys_open_hook;
		break;
	}
	case __NR_creat:
	{
		sys_creat_orig = p_sys_call_table[__NR_creat];
		p_sys_call_table[__NR_creat] = (void*)&sys_creat_hook;
		break;
	}
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
	case __NR_unlink:
	{
		sys_unlink_orig = p_sys_call_table[__NR_unlink];
		p_sys_call_table[__NR_unlink] = (void*)&sys_unlink_hook;
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
	
	hook_sys_call(__NR_creat);
	hook_sys_call(__NR_read);
	hook_sys_call(__NR_open);
	hook_sys_call(__NR_write);
	hook_sys_call(__NR_unlink);
	
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
	p_sys_call_table[__NR_open] = sys_open_orig;
	p_sys_call_table[__NR_creat] = sys_creat_orig;
	p_sys_call_table[__NR_read] = sys_read_orig;
	p_sys_call_table[__NR_write] = sys_write_orig;
	p_sys_call_table[__NR_unlink] = sys_unlink_orig;
	enable_write_protection();
	local_irq_restore(irq_flags);
	irq_flags = 0;
	preempt_enable();
	printk("sys_read_count=%u\n", sys_read_count);
	printk("sys_creat_count=%u\n", sys_creat_count);
	printk("sys_open_count=%u\n", sys_open_count);
	printk("sys_write_count=%u\n", sys_write_count);
	printk("sys_unlink_count=%u\n", sys_unlink_count);
}

module_init(syscall_hook_mod_init);
module_exit(syscall_hook_mod_exit);

MODULE_LICENSE("GPL");
