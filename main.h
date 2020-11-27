#ifndef __MAIN_H
#define __MAIN_H

#include "common.h"
#include "hidefs.h"
#include "syscalls.h"
#include "misc.h"

static int __init lkm_rootkit_init(void);
static void __exit lmk_rootkit_exit(void);

module_init(lkm_rootkit_init);
module_exit(lmk_rootkit_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fergus Longley");
MODULE_AUTHOR("Joshua Turner");
MODULE_DESCRIPTION("Rootkit");
MODULE_VERSION("0.0.1");

#endif