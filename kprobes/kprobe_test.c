#include <linux/kernel.h>
#include <linux/kern_levels.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/ptrace.h>

#define MAX_SYMBOL_LEN	64
static char symbol[MAX_SYMBOL_LEN] = "sys_openat";

static struct kprobe kp = {
	.symbol_name = symbol,
};

int pre_handler_kprobe_test(struct kprobe *p, struct pt_regs * regs);

int pre_handler_kprobe_test(struct kprobe *p, struct pt_regs * regs)
{
	/*if (ptregs)
		printk(KERN_INFO "[KPB] r0=%lu, r1=%lu, r2=%lu, r3=%lu, r4=%lu, r5=%lu, r6=%lu, r7=%lu, r8=%lu, r9=%lu, r10=%lu,"
			"r11=%lu, r12=%lu, r13=%lu, r14=%lu, r15=%lu, r16=%lu, r17=%lu, r18=%lu, r19=%lu, r20=%lu, r21=%lu, r22=%lu,"
			"r23=%lu, r24=%lu, r25=%lu, r26=%lu, r27=%lu, r28=%lu\n", ptregs->r0, ptregs->r1, ptregs->r2, ptregs->r3,
			ptregs->r4, ptregs->r5, ptregs->r6, ptregs->r7, ptregs->r8, ptregs->r9, ptregs->r10, ptregs->r11, ptregs->r12,
			ptregs->r13, ptregs->r14, ptregs->r15, ptregs->r16, ptregs->r17, ptregs->r18, ptregs->r19, ptregs->r20, ptregs->r21,
			ptregs->r22, ptregs->r23, ptregs->r24, ptregs->r25, ptregs->r26, ptregs->r27, ptregs->r28);
	else
		printk(KERN_INFO "pt_regs is null\n");*/
#ifdef CONFIG_X86
	pr_info("[KPB] <%s> pre_handler: p->addr = 0x%p, ip = %lx, flags = 0x%lx\n",
		p->symbol_name, p->addr, regs->ip, regs->flags);
#endif
#ifdef CONFIG_PPC
	pr_info("[KPB] <%s> pre_handler: p->addr = 0x%p, nip = 0x%lx, msr = 0x%lx\n",
		p->symbol_name, p->addr, regs->nip, regs->msr);
#endif
#ifdef CONFIG_MIPS
	pr_info("[KPB] <%s> pre_handler: p->addr = 0x%p, epc = 0x%lx, status = 0x%lx\n",
		p->symbol_name, p->addr, regs->cp0_epc, regs->cp0_status);
#endif
#ifdef CONFIG_ARM64
	pr_info("[KPB] <%s> pre_handler: p->addr = 0x%p, pc = 0x%lx,"
			" pstate = 0x%lx\n",
		p->symbol_name, p->addr, (long)regs->pc, (long)regs->pstate);
#endif
#ifdef CONFIG_S390
	pr_info("[KPB] <%s> pre_handler: p->addr, 0x%p, ip = 0x%lx, flags = 0x%lx\n",
		p->symbol_name, p->addr, regs->psw.addr, regs->flags);
#endif
	return 0;
}

int __init kprobes_mod_init(void)
{
	int ret = 0;
	kp.pre_handler = pre_handler_kprobe_test;
	kp.post_handler = NULL;
	kp.fault_handler = NULL;
	ret = register_kprobe(&kp);
	if (ret)
		printk(KERN_INFO "[KPB] register_kprobe failed with code %d\n", ret);
	return ret;
}

void __exit kprobes_mod_exit(void)
{
	unregister_kprobe(&kp);
}

module_init(kprobes_mod_init);
module_exit(kprobes_mod_exit);

MODULE_LICENSE("GPL");
