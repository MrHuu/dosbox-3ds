#ifndef __CTR_GFX_H__
#define __CTR_GFX_H__

#include <3ds.h>

#include "text.h"

#include <cstring>

#include "../../../ctr/gfx/ctr_bottom_kbd_low_bgr.h"
#include "../../../ctr/gfx/ctr_bottom_kbd_up_bgr.h"
#include "../../../ctr/gfx/ctr_bottom_kbd_sym_bgr.h"
#include "../../../ctr/gfx/ctr_bottom_kbd_sym_alt_bgr.h"
#include "../../../ctr/gfx/ctr_bottom_kbd_num_bgr.h"
#include "../../../ctr/gfx/ctr_bottom_mouse_bgr.h"
#include "../../../ctr/gfx/ctr_bottom_mapper_bgr.h"
#include "../../../ctr/gfx/ctr_bottom_mapper_joy_bgr.h"
#include "../../../ctr/gfx/ctr_bottom_system_bgr.h"
#include "../../../ctr/gfx/ctr_bottom_idle_bgr.h"
#include "../../../ctr/gfx/ctr_bottom_disable_bgr.h"
#include "../../../ctr/gfx/ctr_bottom_load_bgr.h"

#include "../../../ctr/gfx/btn_save_bgr.h"
#include "../../../ctr/gfx/btn_mapper_bgr.h"

#include "../../../ctr/gfx/kbd/key_key_bgr.h"
#include "../../../ctr/gfx/kbd/key_fkey_bgr.h"
#include "../../../ctr/gfx/kbd/key_alt_bgr.h"
#include "../../../ctr/gfx/kbd/key_lalt_bgr.h"
#include "../../../ctr/gfx/kbd/key_ralt_bgr.h"
#include "../../../ctr/gfx/kbd/key_ctrl_bgr.h"
#include "../../../ctr/gfx/kbd/key_lctrl_bgr.h"
#include "../../../ctr/gfx/kbd/key_rctrl_bgr.h"
#include "../../../ctr/gfx/kbd/key_enter_top_bgr.h"
#include "../../../ctr/gfx/kbd/key_enter_bot_bgr.h"
#include "../../../ctr/gfx/kbd/key_shift_bgr.h"
#include "../../../ctr/gfx/kbd/key_lshift_bgr.h"
#include "../../../ctr/gfx/kbd/key_rshift_bgr.h"
#include "../../../ctr/gfx/kbd/key_caps_bgr.h"
#include "../../../ctr/gfx/kbd/key_space_bgr.h"
#include "../../../ctr/gfx/kbd/key_tab_bgr.h"

void gfxDrawText(gfxScreen_t screen, gfx3dSide_t side, font_s* f, char* str, s16 x, s16 y);
void gfxDrawTextN(gfxScreen_t screen, gfx3dSide_t side, font_s* f, char* str, u16 length, s16 x, s16 y);
void gfxDrawSprite(gfxScreen_t screen, gfx3dSide_t side, u8* spriteData, u16 width, u16 height, s16 x, s16 y);

#endif //__CTR_GFX_H__
