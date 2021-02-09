#include <linux/kernel.h>
#include <linux/kern_levels.h>
//#include <linux/kprobes.h>
#include <linux/ftrace.h>
#include <linux/module.h>
#include <linux/printk.h>
//#include <linux/ptrace.h>

#define MAX_SYMBOL_LEN	64
static unsigned char symbol[MAX_SYMBOL_LEN] = "SyS_open";

void my_callback_func(unsigned long ip, unsigned long parent_ip, struct ftrace_ops *op, struct pt_regs *regs);

static struct ftrace_ops ops = {
	.func = my_callback_func,
//	.next = 0,
	.flags = FTRACE_OPS_FL_SAVE_REGS,
};

void my_callback_func(unsigned long ip, unsigned long parent_ip, struct ftrace_ops *op, struct pt_regs *regs){
	static int once_was_here = 0;
	if (!rcu_is_watching())
        	goto exit;

	if (once_was_here)
		goto exit;

	once_was_here = 1;
	if (regs){
		printk("[FTRC] regs here\n");
	}
	printk("[FTRC] ip=%lu, parent_ip=%lu, name=%s\n", ip, parent_ip, symbol);
exit:
	return;
}

int __init ftrace_mod_init(void){
	int ret = 0;
	ret = ftrace_set_filter(&ops, symbol, strlen(symbol), 2);
	if (ret)
	{
		printk("[FTRC] set filter is failed: %d\n", ret);
		goto exit;
	}
	ret = register_ftrace_function(&ops);
	if (ret)
	{
		printk("[FTRC] registering is failed: %d\n", ret);
		goto exit;
	}
exit:
	return ret;
}

void __exit ftrace_mod_exit(void){
	unregister_ftrace_function(&ops);
}

module_init(ftrace_mod_init);
module_exit(ftrace_mod_exit);

MODULE_LICENSE("GPL");
