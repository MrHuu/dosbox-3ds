#ifndef __CTR_INPUT_H__
#define __CTR_INPUT_H__

#include <3ds.h>
#include <sys/types.h>
#include "SDL.h"

#include "cpu.h"
#include "mouse.h"
#include "keyboard.h"
#include "render.h"

#include "../../../ctr/gfx/ctr_bottom_kbd_low_bgr.h"
#include "../../../ctr/gfx/ctr_bottom_kbd_up_bgr.h"
#include "../../../ctr/gfx/ctr_bottom_kbd_sym_bgr.h"
#include "../../../ctr/gfx/ctr_bottom_kbd_sym_alt_bgr.h"
#include "../../../ctr/gfx/ctr_bottom_kbd_num_bgr.h"
#include "../../../ctr/gfx/ctr_bottom_mouse_bgr.h"
#include "../../../ctr/gfx/ctr_bottom_mapper_bgr.h"
#include "../../../ctr/gfx/ctr_bottom_setting_bgr.h"
#include "../../../ctr/gfx/ctr_bottom_idle_bgr.h"

#include "../../../ctr/gfx/btn_save_bgr.h"
#include "../../../ctr/gfx/btn_mapper_bgr.h"

#include "../../../ctr/gfx/kbd/key_key_bgr.h"
#include "../../../ctr/gfx/kbd/key_fkey_bgr.h"
#include "../../../ctr/gfx/kbd/key_alt_bgr.h"
#include "../../../ctr/gfx/kbd/key_ctrl_bgr.h"
#include "../../../ctr/gfx/kbd/key_enter_top_bgr.h"
#include "../../../ctr/gfx/kbd/key_enter_bot_bgr.h"
#include "../../../ctr/gfx/kbd/key_shift_bgr.h"
#include "../../../ctr/gfx/kbd/key_caps_bgr.h"
#include "../../../ctr/gfx/kbd/key_space_bgr.h"
#include "../../../ctr/gfx/kbd/key_tab_bgr.h"

KBD_KEYS TouchKey(s16 T_X, s16 T_Y);

void GetInput();
bool SetKey(KBD_KEYS key, bool pressed, bool kHeld);
void SetMod(u8 Key, bool reset);

extern Render_t render;

extern void CPU_CycleIncrease(bool pressed);
extern void CPU_CycleDecrease(bool pressed);

#include <string>
extern std::string ctr_mapperFile;

#endif //__CTR_INPUT_H__
