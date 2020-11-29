#ifndef __SYSCALLS_H
#define __SYSCALLS_H

#include "common.h"
#include "hidefs.h"

unsigned long *find_syscall_table(void);

typedef asmlinkage long (*sys_call_stub)(const struct pt_regs *);

asmlinkage long rk_access(const struct pt_regs *);
asmlinkage long rk_clone(unsigned long, unsigned long, int __user *, unsigned long, int __user *);
asmlinkage long rk_fork(void);
asmlinkage long rk_exit(int);
asmlinkage long rk_kill(const struct pt_regs *);

extern sys_call_stub orig_access;
extern asmlinkage long (*orig_clone)(unsigned long, unsigned long, int __user *, unsigned long, int __user *);
extern asmlinkage long (*orig_fork)(void);
extern asmlinkage long (*orig_exit)(int);
extern sys_call_stub orig_kill;

#endif