#ifndef __CTR_INPUT_H__
#define __CTR_INPUT_H__

#include <string>

#include <3ds.h>
#include <sys/types.h>
#include "SDL.h"

#include "cpu.h"
#include "mouse.h"
#include "keyboard.h"
#include "joystick.h"
#include "render.h"


void backlightEnable(bool enable, u32 screen);

KBD_KEYS TouchKey(s16 T_X, s16 T_Y);

void GetInput();
void SetMod(u8 Key, bool reset);

bool SetKey(KBD_KEYS key, bool pressed, bool kHeld);

extern Render_t render;
extern std::string ctr_mapperFile;

extern void CPU_CycleIncrease(bool pressed);
extern void CPU_CycleDecrease(bool pressed);

void sysError(const char* error);

typedef struct
{
	bool gfx_scale_to_fit;
	bool gfx_keep_aspect;
} ctr_settings_t;

#endif //__CTR_INPUT_H__
