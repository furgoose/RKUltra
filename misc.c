#include "misc.h"

// https://stackoverflow.com/a/60564037
static inline void force_write_cr0(unsigned long cr0)
{
    asm volatile("mov %0, %%cr0"
                 : "+r"(cr0), "+m"(__force_order));
}

inline void disable_write_protect(void)
{
    unsigned long cr0 = read_cr0();
    clear_bit(16, &cr0);
    force_write_cr0(cr0);
}

inline void enable_write_protect(void)
{
    unsigned long cr0 = read_cr0();
    set_bit(16, &cr0);
    force_write_cr0(cr0);
}