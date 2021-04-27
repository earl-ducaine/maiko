#ifndef INITKBDDEFS_H
#define INITKBDDEFS_H 1
void set_kbd_iopointers(void);
void keyboardtype(int fd);
void init_keyboard(int flg);
void device_before_exit(void);
void set_kbd_iopointers(void);
void keyboardtype(int fd);
int get_keycode_from_keysym(uint keysym);
#endif
