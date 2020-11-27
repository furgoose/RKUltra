#ifndef __HIDEFS_H
#define __HIDEFS_H

#include "common.h"
#include "misc.h"

int hidefs_init(void);
int hidefs_clean(void);
void hide_proc(pid_t pid);
void unhide_proc(pid_t pid);
bool is_hidden_proc(pid_t pid);

#endif