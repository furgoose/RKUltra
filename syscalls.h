#ifndef __SYSCALLS_H
#define __SYSCALLS_H

#include "common.h"

unsigned long *find_syscall_table(void);

asmlinkage long rk_access(const struct pt_regs *pt_regs);

typedef asmlinkage long (*sys_call_ptr_t)(const struct pt_regs *);
extern sys_call_ptr_t orig_access;

#endif