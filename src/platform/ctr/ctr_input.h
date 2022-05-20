
#include <3ds.h>
#include <sys/types.h>
#include "SDL.h"

#include "keyboard.h"
#include "../../../ctr/gfx/ctr_bottom_kbd_bgr.h"


KBD_KEYS TouchKey(s16 T_X, s16 T_Y);

void GetInput();
bool SetKey(KBD_KEYS key, bool pressed, bool kHeld);