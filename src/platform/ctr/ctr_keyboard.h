#ifndef __CTR_KEYBOARD_H__
#define __CTR_KEYBOARD_H__

struct ctr_kbd_desc_t {
	const char* key_desc;
};

struct ctr_kbd_desc_t ctr_kbd_desc[] {
	"None",
	"1","2","3","4","5","6","7","8","9","0",		
	"q","w","e","r","t","y","u","i","o","p",	
	"a","s","d","f","g","h","j","k","l","z",
	"x","c","v","b","n","m",
	"F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12",

	"esc","tab","backspace","enter","space",
	"leftalt","rightalt","leftctrl","rightctrl","leftshift","rightshift",
	"capslock","scrolllock","numlock",
	
	"grave","minus","equals","backslash","leftbracket","rightbracket",
	"semicolon","quote","period","comma","slash","extra_lt_gt",

	"printscreen","pause",
	"insert","home","pageup","delete","end","pagedown",
	"left","up","down","right",

	"kp1","kp2","kp3","kp4","kp5","kp6","kp7","kp8","kp9","kp0",
	"kpdivide","kpmultiply","kpminus","kpplus","kpenter","kpperiod",
};

#endif //__CTR_KEYBOARD_H__