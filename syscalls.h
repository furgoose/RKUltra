#ifndef __SYSCALLS_H
#define __SYSCALLS_H

#include "common.h"
#include "hidefs.h"

unsigned long *find_syscall_table(void);
int sys_call_init(void);
int sys_call_clean(void);

#endif