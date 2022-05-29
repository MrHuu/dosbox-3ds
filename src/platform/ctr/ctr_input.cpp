#include "ctr_input.h"
#include "text.h"
#include "ctr_keyboard.h"

KBD_KEYS prevHeld, tmp_btn_id = KBD_NONE;
u64 elapsed_tick;
bool KeyHeld, kbdHeld, isInit, btnMapping = false;
u8 CurrentKey;
u8 PreviousKey;

s16 GraphicID;
s16 GraphicX;
s16 GraphicY;

s16 T_X;
s16 T_Y;

enum KeyboardStates
{
	STATE_LOWER=0,
	STATE_UPPER,
	STATE_SYMBOL,
	STATE_NUMBER
};
KeyboardStates KeyboardState;

enum active_mode_enum { MODE_IDLE, MODE_KBD, MODE_MOUSE, MODE_MAPPER, MODE_SETTING};

typedef struct
{
	active_mode_enum active_mode;
	int32_t touch_x;
	int32_t touch_y;
	u64  idle_timestamp;
	u64  touch_timestamp;
	bool shouldCheckIdle;
	bool shouldCheckMenu;
	bool isAlt;
	bool isAltHeld;
	bool isCtrl;
	bool isCtrlHeld;
	bool isShift;
	bool isShiftHeld;
	bool isCaps;
	bool isCapsHeld;
} ctr_bottom_mode_t;
ctr_bottom_mode_t CurrentMode = { MODE_IDLE, 0, 0, 0, 0, false, false, false, false, false, false, false, false, false };

typedef struct
{
	bool bottom_init;
	bool bottom_enabled;
	bool bottom_idle;
} ctr_bottom_gfx_t;
ctr_bottom_gfx_t ctr_bottom_gfx = { false, false, false };

typedef struct
{
	int32_t mouse_x;
	int32_t mouse_y;
	int32_t mouse_x_delta;
	int32_t mouse_y_delta;
	int32_t mouse_x_rel;
	int32_t mouse_y_rel;
	int32_t mouse_x_origin;
	int32_t mouse_y_origin;
	int mouse_multiplier;
	u64  touch_timestamp;
	bool mouse_button_left;
	bool mouse_button_right;
	int32_t mouse_button_x_origin;
	int32_t mouse_button_y_origin;
	bool ShouldCheck;
} ctr_bottom_state_mouse_t;
ctr_bottom_state_mouse_t ctr_bottom_state_mouse = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, false, false, false };

struct ctr_bottom_kbd_gfx_t {
	const u8* gfxId;
	unsigned int w;
	unsigned int h;
	unsigned int x;
	unsigned int y;
};

struct ctr_bottom_kbd_gfx_t kbd_gfx[] = {
	{key_key_bgr,       31, 27,  0,  0},
	{key_fkey_bgr,      31, 22,  0,  0},
	{key_alt_bgr,       31, 42, 42,233},
	{key_ctrl_bgr,      31, 42, 42,277},
	{key_shift_bgr,     31, 77, 75,241},
	{key_caps_bgr,      31, 34, 75,  2},
	{key_enter_top_bgr, 33, 26,140,292},
	{key_enter_bot_bgr, 31, 40,109,278},
	{key_space_bgr,     31,141, 90,167},
	{key_tab_bgr,       31, 13,  2,101}
};

struct ctr_controller_t {
	unsigned int btn_id;
	const char* key_desc;
	unsigned int pos_desc_y;
	unsigned int pos_desc_x;
	bool isPressed;
};

struct ctr_controller_t ctr_controller[] = {
	{KEY_ZL,         "ZL",       214,  10, false},
	{KEY_L,          "L",        196,  10, false},
	{KEY_CPAD_UP,    "CP Up",    178,  10, false},
	{KEY_CPAD_DOWN,  "CP Down",  160,  10, false},
	{KEY_CPAD_LEFT,  "CP Left",  142,  10, false},
	{KEY_CPAD_RIGHT, "CP Right", 124,  10, false},
	{KEY_DUP,        "DP Up",    106,  10, false},
	{KEY_DDOWN,      "DP Down",   88,  10, false},
	{KEY_DLEFT,      "DP Left",   70,  10, false},
	{KEY_DRIGHT,     "DP Right",  52,  10, false},

	{KEY_ZR,         "ZR",       214, 232, false},
	{KEY_R,          "R",        196, 232, false},
	{KEY_A,          "A",        178, 232, false},
	{KEY_B,          "B",        160, 232, false},
	{KEY_X,          "X",        142, 232, false},
	{KEY_Y,          "Y",        124, 232, false},
	{KEY_START,      "Start",    106, 232, false},
	{KEY_SELECT,     "Select",    88, 232, false}
};

struct ctr_controller_map_t {
	KBD_KEYS map_id;
};

struct ctr_controller_map_t ctr_controller_map[] = {
	{KBD_NONE},
	{KBD_NONE},
	{KBD_up},
	{KBD_down},
	{KBD_left},
	{KBD_right},
	{KBD_up},
	{KBD_down},
	{KBD_left},
	{KBD_right},
	{KBD_NONE},
	{KBD_rightshift},
	{KBD_up},
	{KBD_space},
	{KBD_rightctrl},
	{KBD_rightalt},
	{KBD_enter},
	{KBD_esc}
};

void sysError(const char* error)
{
	errorConf msg;
	errorInit(&msg, ERROR_TEXT, CFG_LANGUAGE_EN);
	errorText(&msg, error);
	errorDisp(&msg);
	return;
}

void gfxDrawText(gfxScreen_t screen, gfx3dSide_t side, font_s* f, char* str, s16 x, s16 y)
{
	if(!str)return;
	if(!f)f=&fontDefault;

	u16 fbWidth, fbHeight;
	u8* fbAdr=gfxGetFramebuffer(screen, side, &fbWidth, &fbHeight);

	drawString(fbAdr, f, str, y, x, fbHeight, fbWidth);
}

void gfxDrawTextN(gfxScreen_t screen, gfx3dSide_t side, font_s* f, char* str, u16 length, s16 x, s16 y)
{
	if(!str)return;
	if(!f)f=&fontDefault;

	u16 fbWidth, fbHeight;
	u8* fbAdr=gfxGetFramebuffer(screen, side, &fbWidth, &fbHeight);

	drawStringN(fbAdr, f, str, length, y, x, fbHeight, fbWidth);
}

void gfxDrawSprite(gfxScreen_t screen, gfx3dSide_t side, u8* spriteData, u16 width, u16 height, s16 x, s16 y)
{
	if(!spriteData)return;

	u16 fbWidth, fbHeight;
	u8* fbAdr=gfxGetFramebuffer(screen, side, &fbWidth, &fbHeight);

	if(x+width<0 || x>=fbWidth)return;
	if(y+height<0 || y>=fbHeight)return;

	u16 xOffset=0, yOffset=0;
	u16 widthDrawn=width, heightDrawn=height;

	if(x<0)xOffset=-x;
	if(y<0)yOffset=-y;
	if(x+width>=fbWidth)widthDrawn=fbWidth-x;
	if(y+height>=fbHeight)heightDrawn=fbHeight-y;
	widthDrawn-=xOffset;
	heightDrawn-=yOffset;

	int j;
	for(j=yOffset; j<yOffset+heightDrawn; j++)
	{
		memcpy(&fbAdr[((x+xOffset)+(y+j)*fbWidth)*2], &spriteData[((xOffset)+(j)*width)*2], widthDrawn*2);
	}
}


void gfxDrawScreen(u8 KeyboadState, s16 T_X, s16 T_Y, u8 Key)
{
   switch (CurrentMode.active_mode)
   {
		case MODE_IDLE:
			gfxDrawSprite(GFX_BOTTOM, GFX_LEFT, (u8*)ctr_bottom_idle_bgr, 240, 320, 0, 0);
			break;

		case MODE_KBD:
			switch (KeyboardState)
			{
				case STATE_SYMBOL:
					if (CurrentMode.isShift)
						gfxDrawSprite(GFX_BOTTOM, GFX_LEFT, (u8*)ctr_bottom_kbd_sym_alt_bgr, 240, 320, 0, 0);
					else
						gfxDrawSprite(GFX_BOTTOM, GFX_LEFT, (u8*)ctr_bottom_kbd_sym_bgr, 240, 320, 0, 0);
					break;
				case STATE_NUMBER:
						gfxDrawSprite(GFX_BOTTOM, GFX_LEFT, (u8*)ctr_bottom_kbd_num_bgr, 240, 320, 0, 0);
					break;
				default:
					if (CurrentMode.isShift || CurrentMode.isCaps)
						gfxDrawSprite(GFX_BOTTOM, GFX_LEFT, (u8*)ctr_bottom_kbd_up_bgr, 240, 320, 0, 0);
					else
						gfxDrawSprite(GFX_BOTTOM, GFX_LEFT, (u8*)ctr_bottom_kbd_low_bgr, 240, 320, 0, 0);
					break;
			}
			if(kbdHeld && (Key != KBD_NONE))
				gfxDrawSprite(GFX_BOTTOM, GFX_LEFT,(u8*)kbd_gfx[GraphicID].gfxId, kbd_gfx[GraphicID].w, kbd_gfx[GraphicID].h, GraphicX, GraphicY);

			if(kbdHeld && (Key == KBD_enter))
			{
				gfxDrawSprite(GFX_BOTTOM, GFX_LEFT,(u8*)kbd_gfx[6].gfxId, kbd_gfx[6].w, kbd_gfx[6].h, kbd_gfx[6].x, kbd_gfx[6].y);
				gfxDrawSprite(GFX_BOTTOM, GFX_LEFT,(u8*)kbd_gfx[7].gfxId, kbd_gfx[7].w, kbd_gfx[7].h, kbd_gfx[7].x, kbd_gfx[7].y);
			}
			if (CurrentMode.isAlt)
				gfxDrawSprite(GFX_BOTTOM, GFX_LEFT,(u8*)kbd_gfx[2].gfxId, kbd_gfx[2].w, kbd_gfx[2].h, kbd_gfx[2].x, kbd_gfx[2].y);
			if (CurrentMode.isCtrl)
				gfxDrawSprite(GFX_BOTTOM, GFX_LEFT,(u8*)kbd_gfx[3].gfxId, kbd_gfx[3].w, kbd_gfx[3].h, kbd_gfx[3].x, kbd_gfx[3].y);
			if (CurrentMode.isShift)
				gfxDrawSprite(GFX_BOTTOM, GFX_LEFT,(u8*)kbd_gfx[4].gfxId, kbd_gfx[4].w, kbd_gfx[4].h, kbd_gfx[4].x, kbd_gfx[4].y);
			if (CurrentMode.isCaps)
				gfxDrawSprite(GFX_BOTTOM, GFX_LEFT,(u8*)kbd_gfx[5].gfxId, kbd_gfx[5].w, kbd_gfx[5].h, kbd_gfx[5].x, kbd_gfx[5].y);
			if(btnMapping)
				gfxDrawSprite(GFX_BOTTOM, GFX_LEFT, (u8*)btn_mapper_bgr, 40, 320, 0, 0);
			break;

		case MODE_MOUSE:
			if ( CurrentMode.active_mode == MODE_MOUSE && T_Y < 200 )
			{
				char str_multiply[2];
				sprintf(str_multiply, "%d", ctr_bottom_state_mouse.mouse_multiplier);

				gfxDrawSprite(GFX_BOTTOM, GFX_LEFT, (u8*)ctr_bottom_mouse_bgr, 240, 320, 0, 0);
				gfxDrawText(GFX_BOTTOM, GFX_LEFT, NULL, str_multiply, 240 - 185, 285);
			}
			else if ( CurrentMode.active_mode == MODE_MOUSE && T_Y > 200 )
				gfxDrawSprite(GFX_BOTTOM, GFX_LEFT, (u8*)ctr_bottom_mouse_bgr, 240, 320, 0, 0);
			break;

		case MODE_MAPPER:
			char str_btn_buf[32];

			gfxDrawSprite(GFX_BOTTOM, GFX_LEFT, (u8*)ctr_bottom_mapper_bgr, 240, 320, 0, 0);
			for (int i = 0; i < 18; i++) {
				sprintf(str_btn_buf,  "%s:  %s", ctr_controller[i].key_desc, ctr_kbd_desc[ctr_controller_map[i].map_id].key_desc);
				gfxDrawText(GFX_BOTTOM, GFX_LEFT, NULL, str_btn_buf , ctr_controller[i].pos_desc_y, ctr_controller[i].pos_desc_x);
			}
			if (Key == 255)
				gfxDrawSprite(GFX_BOTTOM, GFX_LEFT,(u8*)btn_save_bgr, 24, 80, 48, 120);
			break;

		case MODE_SETTING:
			gfxDrawSprite(GFX_BOTTOM, GFX_LEFT, (u8*)ctr_bottom_setting_bgr, 240, 320, 0, 0);

			char str_CPU_CycleMax[32];
			sprintf(str_CPU_CycleMax, "cycles: %d", CPU_CycleMax);

			char str_frameskip[32];
			sprintf(str_frameskip, "frameskip: %d", render.frameskip.max);

			gfxDrawText(GFX_BOTTOM, GFX_LEFT, NULL, str_CPU_CycleMax, 204, 180);
			gfxDrawText(GFX_BOTTOM, GFX_LEFT, NULL, str_frameskip , 156, 180);
			break;
	}
	gfxScreenSwapBuffers(GFX_BOTTOM,false);
}

static void MAPPER_SaveBinds(void)
{
	int i;
	FILE *fptr;

	if ((fptr = fopen(ctr_mapperFile.c_str(),"wb")) == NULL){
		sysError("Error writing mapper file:\n\nCannot create mapper file.");
		return;
	}

	for(i = 0; i < 18; ++i)
		fwrite(&ctr_controller_map[i], sizeof(struct ctr_controller_map_t), 1, fptr);
	fclose(fptr);

	return;
}

static bool MAPPER_LoadBinds(void)
{
	int i;
	FILE *fptr;

	if ((fptr = fopen(ctr_mapperFile.c_str(),"rb")) == NULL){
		sysError("Error reading mapper file:\n\nMapper file not found.");
		return -1;
	}

	for(i = 0; i < 18; ++i)
		fread(&ctr_controller_map[i], sizeof(struct ctr_controller_map_t), 1, fptr);
	fclose(fptr);

	return 0;
}

void MAPPER_SetBinds(unsigned int btn)
{
	tmp_btn_id = KBD_NONE;
	bool TkbdHeld = true;
	btnMapping = true;

	CurrentMode.active_mode = MODE_KBD;
	SetMod(0,true);

	while(aptMainLoop()) {

		hidScanInput();

		u32 tmp_kDown = hidKeysHeld();

		if (!tmp_kDown && TkbdHeld)
		{
			TkbdHeld = false;
			gfxDrawScreen(KeyboardState, T_X, T_Y, 0);
		}
		else if ((tmp_kDown & KEY_TOUCH)&&!TkbdHeld)
		{
			touchPosition tmp_touch;
			hidTouchRead(&tmp_touch);

			if ((tmp_touch.py > 200) && (tmp_touch.px > 256 && tmp_touch.px < 319))
			{
				CurrentMode.active_mode = MODE_MAPPER;
				btnMapping = false;
				return;
			}

			TkbdHeld = true;

			tmp_btn_id = TouchKey(tmp_touch.px,tmp_touch.py);
			kbdHeld = true;
			gfxDrawScreen(KeyboardState, T_X, T_Y, tmp_btn_id);
			kbdHeld = false;

			if (tmp_btn_id != KBD_NONE)
			{
				CurrentMode.active_mode = MODE_MAPPER;
				ctr_controller_map[btn].map_id = tmp_btn_id;
				btnMapping = false;
				return;

			}
		}
		gspWaitForVBlank();
	}
	return;
}

KBD_KEYS TouchKeyNum(s16 T_X, s16 T_Y)
{
	if (T_Y < 34)
	{
		if (T_X > 45 && T_X < 73)
			{GraphicID = 0; GraphicX = 207; GraphicY = 2; return KBD_home;}
		else if (T_X > 74 && T_X < 102)
			{GraphicID = 1; GraphicX = 207; GraphicY = 32; return KBD_end;}
		else if (T_X > 103 && T_X < 131)
			{GraphicID = 1; GraphicX = 207; GraphicY = 56; return KBD_pageup;}
		else if (T_X > 161 && T_X < 189)
			{GraphicID = 1; GraphicX = 207; GraphicY = 80; return KBD_NONE;} /* KBD_numlock */
		else if (T_X > 190 && T_X < 218)
			{GraphicID = 1; GraphicX = 207; GraphicY = 104; return KBD_kpdivide;}
		else if (T_X > 219 && T_X < 247)
			{GraphicID = 1; GraphicX = 207; GraphicY = 128; return KBD_kpmultiply;}
		else if (T_X > 248 && T_X < 276)
			{GraphicID = 1; GraphicX = 207; GraphicY = 152; return KBD_kpminus;}
		else
			return KBD_NONE;
	}
	else if (T_Y > 34 && T_Y < 66)
	{
		if (T_X > 45 && T_X < 73)
			{GraphicID = 0; GraphicX = 174; GraphicY = 2; return KBD_delete;}
		else if (T_X > 74 && T_X < 102)
			{GraphicID = 0; GraphicX = 174; GraphicY = 31; return KBD_insert;}
		else if (T_X > 103 && T_X < 131)
			{GraphicID = 0; GraphicX = 174; GraphicY = 60; return KBD_pagedown;}
		else if (T_X > 161 && T_X < 189)
			{GraphicID = 0; GraphicX = 174; GraphicY = 89; return KBD_kp7;}
		else if (T_X > 190 && T_X < 218)
			{GraphicID = 0; GraphicX = 174; GraphicY = 118; return KBD_kp8;}
		else if (T_X > 219 && T_X < 247)
			{GraphicID = 0; GraphicX = 174; GraphicY = 147; return KBD_kp9;}
		else if (T_X > 248 && T_X < 276)
			{GraphicID = 0; GraphicX = 174; GraphicY = 176; return KBD_kpplus;}
		else
			return KBD_NONE;
	}
	else if (T_Y > 67 && T_Y < 99)
	{
		if (T_X > 74 && T_X < 102)
			{GraphicID = 0; GraphicX = 141; GraphicY = 2; return KBD_up;}
		else if (T_X > 161 && T_X < 189)
			{GraphicID = 0; GraphicX = 141; GraphicY = 31; return KBD_kp4;}
		else if (T_X > 190 && T_X < 218)
			{GraphicID = 0; GraphicX = 141; GraphicY = 60; return KBD_kp5;}
		else if (T_X > 219 && T_X < 247)
			{GraphicID = 0; GraphicX = 141; GraphicY = 89; return KBD_kp6;}
		else if (T_X > 248 && T_X < 276)
			{GraphicID = 0; GraphicX = 141; GraphicY = 118; return KBD_kpplus;}
		else
			return KBD_NONE;
	}
	else if (T_Y > 99 && T_Y < 133)
	{
		if (T_X > 45 && T_X < 73)
			{GraphicID = 9; GraphicX = 108; GraphicY = 2; return KBD_left;}
		else if (T_X > 74 && T_X < 102)
			{GraphicID = 0; GraphicX = 108; GraphicY = 17; return KBD_down;}
		else if (T_X > 103 && T_X < 131)
			{GraphicID = 0; GraphicX = 108; GraphicY = 46; return KBD_right;}
		else if (T_X > 161 && T_X < 189)
			{GraphicID = 0; GraphicX = 108; GraphicY = 75; return KBD_kp1;}
		else if (T_X > 190 && T_X < 218)
			{GraphicID = 0; GraphicX = 108; GraphicY = 104; return KBD_kp2;}
		else if (T_X > 219 && T_X < 247)
			{GraphicID = 0; GraphicX = 108; GraphicY = 133; return KBD_kp3;}
		else if (T_X > 248 && T_X < 276)
			{GraphicID = 0; GraphicX = 108; GraphicY = 162; return KBD_kpenter;}
		else
			return KBD_NONE;
	}
	else if (T_Y > 133 && T_Y < 165)
	{
		if (T_X > 161 && T_X < 218)
			{GraphicID = 5; GraphicX = 75; GraphicY = 2; return KBD_kp0;}
		else if (T_X > 219 && T_X < 247)
			{GraphicID = 0; GraphicX = 75; GraphicY = 38; return KBD_kpperiod;}
		else if (T_X > 248 && T_X < 267)
			{GraphicID = 0; GraphicX = 75; GraphicY = 67; return KBD_kpenter;}
		else
			return KBD_NONE;
	}
	else if (T_Y > 165 && T_Y < 199)
	{
		if (T_X > 1 && T_X < 45)
		{
			KeyboardState = STATE_SYMBOL;
			return KBD_NONE;
		}
		else if (T_X > 44 && T_X < 89)
		{
			if (KeyboardState == STATE_NUMBER)
				KeyboardState = STATE_LOWER;
			else
				KeyboardState = STATE_NUMBER;
			return KBD_NONE;
		}
		else
			return KBD_NONE;
	}
	else
		return KBD_NONE;

}

KBD_KEYS TouchKey(s16 T_X, s16 T_Y)
{
	if (KeyboardState == STATE_NUMBER)
	{
		return TouchKeyNum(T_X, T_Y);
	}
	if (T_Y < 34)
	{
		if (T_X > 1 && T_X < 30)
			{GraphicID = 0; GraphicX = 207; GraphicY = 2; return KBD_esc;}
		else if (T_X > 30 && T_X < 55)
			{GraphicID = 1; GraphicX = 207; GraphicY = 32; return KBD_f1;}
		else if (T_X > 54 && T_X < 79)
			{GraphicID = 1; GraphicX = 207; GraphicY = 56; return KBD_f2;}
		else if (T_X > 78 && T_X < 103)
			{GraphicID = 1; GraphicX = 207; GraphicY = 80; return KBD_f3;}
		else if (T_X > 102 && T_X < 127)
			{GraphicID = 1; GraphicX = 207; GraphicY = 104; return KBD_f4;}
		else if (T_X > 126 && T_X < 151)
			{GraphicID = 1; GraphicX = 207; GraphicY = 128; return KBD_f5;}
		else if (T_X > 150 && T_X < 175)
			{GraphicID = 1; GraphicX = 207; GraphicY = 152; return KBD_f6;}
		else if (T_X > 174 && T_X < 199)
			{GraphicID = 1; GraphicX = 207; GraphicY = 176; return KBD_f7;}
		else if (T_X > 198 && T_X < 223)
			{GraphicID = 1; GraphicX = 207; GraphicY = 200; return KBD_f8;}
		else if (T_X > 222 && T_X < 247)
			{GraphicID = 1; GraphicX = 207; GraphicY = 224; return KBD_f9;}
		else if (T_X > 246 && T_X < 271)
			{GraphicID = 1; GraphicX = 207; GraphicY = 248; return KBD_f10;}
		else if (T_X > 270 && T_X < 295)
			{GraphicID = 1; GraphicX = 207; GraphicY = 272; return KBD_f11;}
		else if (T_X > 294 && T_X < 319)
			{GraphicID = 1; GraphicX = 207; GraphicY = 296; return KBD_f12;}
		else
			return KBD_NONE;
	}
	else if (T_Y > 33 && T_Y < 67)
	{
		if (T_X > 1 && T_X < 30)
			{GraphicID = 0; GraphicX = 174; GraphicY = 2; return KBD_1;}
		else if (T_X > 29 && T_X < 59)
			{GraphicID = 0; GraphicX = 174; GraphicY = 31; return KBD_2;}
		else if (T_X > 58 && T_X < 88)
			{GraphicID = 0; GraphicX = 174; GraphicY = 60; return KBD_3;}
		else if (T_X > 87 && T_X < 117)
			{GraphicID = 0; GraphicX = 174; GraphicY = 89; return KBD_4;}
		else if (T_X > 116 && T_X < 146)
			{GraphicID = 0; GraphicX = 174; GraphicY = 118; return KBD_5;}
		else if (T_X > 145 && T_X < 175)
			{GraphicID = 0; GraphicX = 174; GraphicY = 147; return KBD_6;}
		else if (T_X > 174 && T_X < 204)
			{GraphicID = 0; GraphicX = 174; GraphicY = 176; return KBD_7;}
		else if (T_X > 203 && T_X < 233)
			{GraphicID = 0; GraphicX = 174; GraphicY = 205; return KBD_8;}
		else if (T_X > 232 && T_X < 262)
			{GraphicID = 0; GraphicX = 174; GraphicY = 234; return KBD_9;}
		else if (T_X > 261 && T_X < 291)
			{GraphicID = 0; GraphicX = 174; GraphicY = 263; return KBD_0;}
		else if (T_X > 290 && T_X < 319)
			{GraphicID = 0; GraphicX = 174; GraphicY = 292; return KBD_backspace;}
		else
			return KBD_NONE;
	}
	else if (T_Y > 66 && T_Y < 100)
	{
		if (T_X > 1 && T_X < 30)
			{GraphicID = 0; GraphicX = 141; GraphicY = 2; return (KeyboardState == STATE_SYMBOL)?KBD_grave:KBD_q;}
		else if (T_X > 29 && T_X < 59)
			{GraphicID = 0; GraphicX = 141; GraphicY = 31; return (KeyboardState == STATE_SYMBOL)?KBD_NONE:KBD_w;}
		else if (T_X > 58 && T_X < 88)
			{GraphicID = 0; GraphicX = 141; GraphicY = 60; return (KeyboardState == STATE_SYMBOL)?KBD_NONE:KBD_e;}
		else if (T_X > 87 && T_X < 117)
			{GraphicID = 0; GraphicX = 141; GraphicY = 89; return (KeyboardState == STATE_SYMBOL)?KBD_NONE:KBD_r;}
		else if (T_X > 116 && T_X < 146)
			{GraphicID = 0; GraphicX = 141; GraphicY = 118; return (KeyboardState == STATE_SYMBOL)?KBD_NONE:KBD_t;}
		else if (T_X > 145 && T_X < 175)
			{GraphicID = 0; GraphicX = 141; GraphicY = 147; return (KeyboardState == STATE_SYMBOL)?KBD_minus:KBD_y;}
		else if (T_X > 174 && T_X < 204)
			{GraphicID = 0; GraphicX = 141; GraphicY = 176; return (KeyboardState == STATE_SYMBOL)?KBD_equals:KBD_u;}
		else if (T_X > 203 && T_X < 233)
			{GraphicID = 0; GraphicX = 141; GraphicY = 205; return (KeyboardState == STATE_SYMBOL)?KBD_backslash:KBD_i;}
		else if (T_X > 232 && T_X < 262)
			{GraphicID = 0; GraphicX = 141; GraphicY = 234; return (KeyboardState == STATE_SYMBOL)?KBD_leftbracket:KBD_o;}
		else if (T_X > 261 && T_X < 291)
			{GraphicID = 0; GraphicX = 141; GraphicY = 263; return (KeyboardState == STATE_SYMBOL)?KBD_rightbracket:KBD_p;}
		else if (T_X > 290 && T_X < 319)
			{GraphicID = 6; GraphicX = 141; GraphicY = 292; return KBD_enter;}
		else
			return KBD_NONE;
	}
	else if (T_Y > 99 && T_Y < 133)
	{
		if (T_X > 1 && T_X < 16)
			{GraphicID = 9; GraphicX = 108; GraphicY = 2; return KBD_tab;}
		else if (T_X > 15 && T_X < 45)
			{GraphicID = 0; GraphicX = 108; GraphicY = 17; return (KeyboardState == STATE_SYMBOL)?KBD_NONE:KBD_a;}
		else if (T_X > 44 && T_X < 74)
			{GraphicID = 0; GraphicX = 108; GraphicY = 46; return (KeyboardState == STATE_SYMBOL)?KBD_NONE:KBD_s;}
		else if (T_X > 73 && T_X < 103)
			{GraphicID = 0; GraphicX = 108; GraphicY = 75; return (KeyboardState == STATE_SYMBOL)?KBD_NONE:KBD_d;}
		else if (T_X > 102 && T_X < 132)
			{GraphicID = 0; GraphicX = 108; GraphicY = 104; return (KeyboardState == STATE_SYMBOL)?KBD_NONE:KBD_f;}
		else if (T_X > 131 && T_X < 161)
			{GraphicID = 0; GraphicX = 108; GraphicY = 133; return (KeyboardState == STATE_SYMBOL)?KBD_NONE:KBD_g;}
		else if (T_X > 160 && T_X < 190)
			{GraphicID = 0; GraphicX = 108; GraphicY = 162; return (KeyboardState == STATE_SYMBOL)?KBD_NONE:KBD_h;}
		else if (T_X > 189 && T_X < 219)
			{GraphicID = 0; GraphicX = 108; GraphicY = 191; return (KeyboardState == STATE_SYMBOL)?KBD_NONE:KBD_j;}
		else if (T_X > 218 && T_X < 248)
			{GraphicID = 0; GraphicX = 108; GraphicY = 220; return (KeyboardState == STATE_SYMBOL)?KBD_semicolon:KBD_k;}
		else if (T_X > 247 && T_X < 277)
			{GraphicID = 0; GraphicX = 108; GraphicY = 249; return (KeyboardState == STATE_SYMBOL)?KBD_quote:KBD_l;}
		else if (T_X > 276 && T_X < 319)
			{GraphicID = 7; GraphicX = 108; GraphicY = 278; return KBD_enter;}
		else
			return KBD_NONE;
	}
	else if (T_Y > 132 && T_Y < 166)
	{
		if (T_X > 1 && T_X < 37)
			{GraphicID = 5; GraphicX = 75; GraphicY = 2; return KBD_capslock;}
		else if (T_X > 36 && T_X < 66)
			{GraphicID = 0; GraphicX = 75; GraphicY = 38; return (KeyboardState == STATE_SYMBOL)?KBD_NONE:KBD_z;}
		else if (T_X > 65 && T_X < 95)
			{GraphicID = 0; GraphicX = 75; GraphicY = 67; return (KeyboardState == STATE_SYMBOL)?KBD_NONE:KBD_x;}
		else if (T_X > 94 && T_X < 124)
			{GraphicID = 0; GraphicX = 75; GraphicY = 96; return (KeyboardState == STATE_SYMBOL)?KBD_NONE:KBD_c;}
		else if (T_X > 123 && T_X < 153)
			{GraphicID = 0; GraphicX = 75; GraphicY = 125; return (KeyboardState == STATE_SYMBOL)?KBD_NONE:KBD_v;}
		else if (T_X > 152 && T_X < 182)
			{GraphicID = 0; GraphicX = 75; GraphicY = 154; return (KeyboardState == STATE_SYMBOL)?KBD_comma:KBD_b;}
		else if (T_X > 181 && T_X < 211)
			{GraphicID = 0; GraphicX = 75; GraphicY = 183; return (KeyboardState == STATE_SYMBOL)?KBD_period:KBD_n;}
		else if (T_X > 210 && T_X < 240)
			{GraphicID = 0; GraphicX = 75; GraphicY = 212; return (KeyboardState == STATE_SYMBOL)?KBD_slash:KBD_m;}
		else if (T_X > 239 && T_X < 319)
			{GraphicID = 4; GraphicX = 75; GraphicY = 241; return KBD_rightshift;}
		else
			return KBD_NONE;
	}
	else if (T_Y > 165 && T_Y < 199)
	{
		if (T_X > 1 && T_X < 45)
		{
			if (KeyboardState == STATE_SYMBOL)
				KeyboardState = STATE_LOWER;
			else
				KeyboardState = STATE_SYMBOL;
			return KBD_NONE;
		}
		else if (T_X > 44 && T_X < 89)
		{
			if (KeyboardState == STATE_NUMBER)
				KeyboardState = STATE_LOWER;
			else
				KeyboardState = STATE_NUMBER;
			return KBD_NONE;
		}
		else if (T_X > 88 && T_X < 232)
			{GraphicID = 8; GraphicX = 42; GraphicY = 90; return KBD_space;}
		else if (T_X > 231 && T_X < 276)
			{GraphicID = 2; GraphicX = 42; GraphicY = 233; return KBD_rightalt;}
		else if (T_X > 275 && T_X < 319)
			{GraphicID = 3; GraphicX = 42; GraphicY = 277; return KBD_rightctrl;}
		else
			return KBD_NONE;
	}

	else
		return KBD_NONE;

}

void SetMod(u8 Key, bool reset)
{
	if (!reset)
	{
		if (Key == KBD_rightshift)
			CurrentMode.isShift = !CurrentMode.isShift;
		else if (Key == KBD_capslock)
			CurrentMode.isCaps = !CurrentMode.isCaps;
		else if (Key == KBD_rightalt)
			CurrentMode.isAlt = !CurrentMode.isAlt;
		else if (Key == KBD_rightctrl)
			CurrentMode.isCtrl = !CurrentMode.isCtrl;

		if (CurrentMode.isShift && Key != KBD_NONE && Key != KBD_rightshift && Key != KBD_rightalt && Key != KBD_rightctrl)
		{
			CurrentMode.isShift = false;
		}
		if (CurrentMode.isAlt && Key != KBD_NONE && Key != KBD_rightshift && Key != KBD_rightalt && Key != KBD_rightctrl)
		{
			CurrentMode.isAlt   = false;
		}
		if (CurrentMode.isCtrl && Key != KBD_NONE && Key != KBD_rightshift && Key != KBD_rightalt && Key != KBD_rightctrl)
		{
			CurrentMode.isCtrl  = false;
		}
	}
	else
	{
		if (CurrentMode.isAltHeld)
			KEYBOARD_AddKey(KBD_rightalt,false);
		if (CurrentMode.isCtrlHeld)
			KEYBOARD_AddKey(KBD_rightctrl,false);
		if (CurrentMode.isShiftHeld)
			KEYBOARD_AddKey(KBD_rightshift,false);
		if (CurrentMode.isCapsHeld)
			KEYBOARD_AddKey(KBD_capslock,false);

		CurrentMode.isAltHeld   = false;
		CurrentMode.isCtrlHeld  = false;
		CurrentMode.isShiftHeld = false;
		CurrentMode.isCapsHeld  = false;

		CurrentMode.isAlt   = false;
		CurrentMode.isCtrl  = false;
		CurrentMode.isShift = false;
		CurrentMode.isCaps  = false;
	}
}

bool SetKey(KBD_KEYS key, bool pressed, bool kHeld)
{
	if (pressed && !kHeld)
	{
		kHeld = true;
		if (CurrentMode.isShift)
			KEYBOARD_AddKey(KBD_rightshift,true);
		KEYBOARD_AddKey(key,true);
	}
	else if (!pressed && kHeld)
	{
		kHeld = false;
		if (CurrentMode.isShift)
			KEYBOARD_AddKey(KBD_rightshift,false);
		KEYBOARD_AddKey(key,false);
	}

	return kHeld;
}

void SetMode(s16 T_X, s16 T_Y)
{
	if (T_Y > 200)
	{
		if (T_X < 63)
			CurrentMode.active_mode = MODE_KBD;
		else if (T_X > 64 && T_X < 127)
			CurrentMode.active_mode = MODE_MOUSE;
		else if (T_X > 128 && T_X < 191)
		{
			CurrentMode.touch_timestamp = svcGetSystemTick();
			CurrentMode.shouldCheckMenu = true;
			return;
		}
		else if (T_X > 192 && T_X < 255)
			CurrentMode.active_mode = MODE_MAPPER;
		else if (T_X > 256 && T_X < 320)
			CurrentMode.active_mode = MODE_SETTING;
	}
	return;
}

void touch_event()
{
	touchPosition touch;
	hidTouchRead(&touch);

	T_X = touch.px;
	T_Y = touch.py;

// -------------
// - Bottom Disabled / idle
// -------------
	if (!ctr_bottom_gfx.bottom_enabled)
	{
		if (!isInit)
		{
			if ( CurrentMode.idle_timestamp == 0 )
			{
				MAPPER_LoadBinds();
				isInit = true;
				CurrentMode.idle_timestamp  = svcGetSystemTick();
				CurrentMode.shouldCheckIdle = true;

				gfxDrawScreen(KeyboardState, 0, 0, 0);
				gfxDrawScreen(KeyboardState, 0, 0, 0);

				gfxScreenSwapBuffers(GFX_BOTTOM,false);
				return;
			}
		}
		if ( CurrentMode.shouldCheckIdle && !KeyHeld && !ctr_bottom_gfx.bottom_idle )
		{
			elapsed_tick = ( svcGetSystemTick() - CurrentMode.idle_timestamp );
			if ( elapsed_tick > 2000000000 )
			{
				ctr_bottom_gfx.bottom_idle = true;
				CurrentMode.shouldCheckIdle = false;
/*
				gspLcdInit();
				GSPLCD_PowerOffBacklight(GSPLCD_SCREEN_BOTTOM);
				gspLcdExit();
*/
				return;
			}
		}
		if ( CurrentMode.shouldCheckMenu && KeyHeld )
		{
			elapsed_tick = ( svcGetSystemTick() - CurrentMode.touch_timestamp );
			if ( elapsed_tick > 200000000)
			{
				CurrentMode.shouldCheckMenu   = false;
				ctr_bottom_gfx.bottom_enabled = true;
				CurrentMode.active_mode = MODE_KBD;
				gfxDrawScreen(KeyboardState, T_X, T_Y, 0);
/*
				gspLcdInit();
				GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTTOM);
				gspLcdExit();
*/
			}
		}
		else if ( CurrentMode.shouldCheckMenu && !KeyHeld )
		{
			CurrentMode.shouldCheckMenu   = false;
		}

		if (T_X == 0 && T_Y == 0)
		{
			if (KeyHeld)
			{
				KeyHeld = false;
				CurrentMode.idle_timestamp  = svcGetSystemTick();

				if ( !ctr_bottom_gfx.bottom_idle )
					CurrentMode.shouldCheckIdle = true;

				gfxDrawScreen(KeyboardState, T_X, T_Y, 0);
			}
			return;
		}
		else if (T_Y > 200 && T_X > 128 && T_X < 191)
		{
			if (!KeyHeld)
			{
				KeyHeld = true;
				CurrentMode.touch_timestamp = svcGetSystemTick();

				CurrentMode.shouldCheckMenu = true;
				CurrentMode.shouldCheckIdle = false;
				gfxDrawScreen(KeyboardState, T_X, T_Y, 0);
			}
		}
		else
		{
			if (!KeyHeld)
			{
				gfxDrawScreen(KeyboardState, T_X, T_Y, 0);
			}
			CurrentMode.shouldCheckMenu = false;
			CurrentMode.shouldCheckIdle = false;

			KeyHeld = true;
		}
		if ( ctr_bottom_gfx.bottom_idle )
		{
/*
				gspLcdInit();
				GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTTOM);
				gspLcdExit();
*/
		}
		ctr_bottom_gfx.bottom_idle = false;
		return;
	}
	else
	{
// ------------------
// - Bottom Enabled -
// ------------------

		if (T_X == 0 && T_Y == 0)
		{
			PreviousKey = 0;
			CurrentKey  = 0;

			if ( CurrentMode.shouldCheckMenu && KeyHeld)
			{
				elapsed_tick = ( svcGetSystemTick() - CurrentMode.touch_timestamp );
				if ( elapsed_tick < 200000000)
				{
// pause?
				}
				CurrentMode.shouldCheckMenu   = false;
			}
			if (KeyHeld)
			{
				KeyHeld = false;
				gfxDrawScreen(KeyboardState, T_X, T_Y, CurrentKey);

				ctr_bottom_state_mouse.mouse_button_left = false;
				ctr_bottom_state_mouse.mouse_button_right = false;
				Mouse_ButtonReleased(0);
				Mouse_ButtonReleased(1);
			}
		}
		else if (T_Y > 200 && !KeyHeld)
		{
			SetMode(T_X, T_Y);
			KeyHeld = true;
//			CurrentMode.shouldCheckMenu   = true;
			gfxDrawScreen(KeyboardState, T_X, T_Y, CurrentKey);
			return;
		}
		else if (T_Y > 200 && KeyHeld)
		{
			if ( CurrentMode.shouldCheckMenu )
			{
				elapsed_tick = ( svcGetSystemTick() - CurrentMode.touch_timestamp );
				if ( elapsed_tick > 200000000)
				{
					ctr_bottom_gfx.bottom_enabled = false;
					CurrentMode.shouldCheckMenu   = false;
					CurrentMode.active_mode       = MODE_IDLE;
					gfxDrawScreen(KeyboardState, T_X, T_Y, 0);
//					ctr_bottom_gfx.gfx_drawn      = true;
				}
			}
			else if ( CurrentMode.active_mode == MODE_MOUSE )
			{
				ctr_bottom_state_mouse.mouse_x_rel = 0;
				ctr_bottom_state_mouse.mouse_y_rel = 0;
			}
			return;
		}
		else if ( CurrentMode.active_mode == MODE_MOUSE )
		{
			if (T_X == 0 && T_Y == 0)
			{
				if ( ctr_bottom_state_mouse.ShouldCheck && elapsed_tick < 300000000 )
				{
					ctr_bottom_state_mouse.mouse_button_left = true;
					Mouse_ButtonPressed(0);
					ctr_bottom_state_mouse.ShouldCheck = false;
					KeyHeld = true;
				}
			}
			if (T_X > 0 || T_Y > 0)
			{
				elapsed_tick = ( svcGetSystemTick() - ctr_bottom_state_mouse.touch_timestamp );
				if (!KeyHeld)
				{
					if (T_X > 294 && T_X < 320)
					{
						if (T_Y > 158 && T_Y < 176)
						{
							if (ctr_bottom_state_mouse.mouse_multiplier != 9)
							{
								ctr_bottom_state_mouse.mouse_multiplier++;
								KeyHeld = true;
								return;
							}
						}
						else if (T_Y > 175 && T_Y < 193)
						{
							if (ctr_bottom_state_mouse.mouse_multiplier != 1)
							{
								ctr_bottom_state_mouse.mouse_multiplier--;
								KeyHeld = true;
								return;
							}
						}
					}
					ctr_bottom_state_mouse.mouse_x_origin = T_X;
					ctr_bottom_state_mouse.mouse_y_origin = T_Y;
					KeyHeld = true;

					if ( elapsed_tick < 100000000)
					{
						if(!ctr_bottom_state_mouse.ShouldCheck)
						{
							ctr_bottom_state_mouse.mouse_button_x_origin = ctr_bottom_state_mouse.mouse_x_origin;
							ctr_bottom_state_mouse.mouse_button_y_origin = ctr_bottom_state_mouse.mouse_y_origin;
						}
						ctr_bottom_state_mouse.ShouldCheck = true;
					}
					ctr_bottom_state_mouse.touch_timestamp = svcGetSystemTick();
				}
				else if( ctr_bottom_state_mouse.ShouldCheck && elapsed_tick > 300000000 )
				{
					ctr_bottom_state_mouse.mouse_button_right = true;
					Mouse_ButtonPressed(1);
					ctr_bottom_state_mouse.ShouldCheck = false;
				}
				else
				{
					if ( ctr_bottom_state_mouse.ShouldCheck && elapsed_tick < 300000000 )
					{
						int pos_x;
						int pos_y;
						if (ctr_bottom_state_mouse.mouse_button_x_origin < T_X)
							pos_x = T_X - ctr_bottom_state_mouse.mouse_button_x_origin;
						else
							pos_x = ctr_bottom_state_mouse.mouse_button_x_origin - T_X;

						if (ctr_bottom_state_mouse.mouse_button_y_origin < T_Y)
							pos_y = T_Y - ctr_bottom_state_mouse.mouse_button_y_origin;
						else
							pos_y = ctr_bottom_state_mouse.mouse_button_y_origin - T_Y;

						if (pos_x > 1 || pos_y > 1)
						{
							ctr_bottom_state_mouse.mouse_button_left = true;
							Mouse_ButtonPressed(0);
							ctr_bottom_state_mouse.ShouldCheck = false;
						}
					}
					if ( T_Y < ctr_bottom_state_mouse.mouse_y_origin )
						ctr_bottom_state_mouse.mouse_y_rel = -(( ctr_bottom_state_mouse.mouse_y_origin -  T_Y ) * (ctr_bottom_state_mouse.mouse_multiplier*2) );
					else
						ctr_bottom_state_mouse.mouse_y_rel = (( T_Y - ctr_bottom_state_mouse.mouse_y_origin ) * (ctr_bottom_state_mouse.mouse_multiplier*2) );

					if ( T_X < ctr_bottom_state_mouse.mouse_x_origin )
						ctr_bottom_state_mouse.mouse_x_rel = -(( ctr_bottom_state_mouse.mouse_x_origin -  T_X ) * (ctr_bottom_state_mouse.mouse_multiplier*2) );
					else
						ctr_bottom_state_mouse.mouse_x_rel = (( T_X - ctr_bottom_state_mouse.mouse_x_origin ) * (ctr_bottom_state_mouse.mouse_multiplier*2) );

					ctr_bottom_state_mouse.mouse_x_origin = T_X;
					ctr_bottom_state_mouse.mouse_y_origin = T_Y;
				}
				Mouse_CursorMoved(	(float)ctr_bottom_state_mouse.mouse_x_rel,
						(float)ctr_bottom_state_mouse.mouse_y_rel,
						(float)0/100.0f,
						(float)0/100.0f,
						true
				);
				return;
			}
		}
	}
	if ( CurrentMode.active_mode == MODE_KBD )
	{
		if ((touch.px!=0 && touch.py!=0) && !kbdHeld)
		{
			kbdHeld = true;
			prevHeld = TouchKey(touch.px,touch.py);
			if (prevHeld != KBD_NONE)
			{
				if (CurrentMode.isShift)
				{
					CurrentMode.isShiftHeld = true;
					KEYBOARD_AddKey(KBD_rightshift,true);
				}
				KEYBOARD_AddKey(prevHeld ,true);
			}

			gfxDrawScreen(STATE_LOWER,touch.px,touch.py,prevHeld);
			SetMod(prevHeld,false);
		}
		else if ((touch.px==0 && touch.py==0) && kbdHeld )
		{
			kbdHeld = false;
			if (prevHeld != KBD_NONE)
			{
				if (CurrentMode.isShiftHeld)
				{
					CurrentMode.isShiftHeld = false;
					KEYBOARD_AddKey(KBD_rightshift,false);
				}
				KEYBOARD_AddKey(prevHeld ,false);
			}
			gfxDrawScreen(STATE_LOWER,touch.px,touch.py,prevHeld);
		}
	}
	if ( CurrentMode.active_mode == MODE_MAPPER )
	{
		if ((touch.px==0 && touch.py==0) && KeyHeld )
		{
			KeyHeld = false;
			return;
		}
		else if ((touch.px==0 && touch.py==0) && !KeyHeld )
		{
			return;
		}
		else if (!KeyHeld)
		{
			KeyHeld = true;
			if (T_X > 5 && T_X < 92)
			{
				if (T_Y > 11 && T_Y < 27)
					MAPPER_SetBinds(0);
				else if (T_Y > 29 && T_Y < 45)
					MAPPER_SetBinds(1);
				else if (T_Y > 47 && T_Y < 63)
					MAPPER_SetBinds(2);
				else if (T_Y > 65 && T_Y < 81)
					MAPPER_SetBinds(3);
				else if (T_Y > 83 && T_Y < 99)
					MAPPER_SetBinds(4);
				else if (T_Y > 101 && T_Y < 117)
					MAPPER_SetBinds(5);
				else if (T_Y > 119 && T_Y < 135)
					MAPPER_SetBinds(6);
				else if (T_Y > 137 && T_Y < 153)
					MAPPER_SetBinds(7);
				else if (T_Y > 155 && T_Y < 171)
					MAPPER_SetBinds(8);
				else if (T_Y > 173 && T_Y < 189)
					MAPPER_SetBinds(9);
			}
			else if (T_X > 227 && T_X < 314)
			{
				if (T_Y > 11 && T_Y < 27)
					MAPPER_SetBinds(10);
				else if (T_Y > 29 && T_Y < 45)
					MAPPER_SetBinds(11);
				else if (T_Y > 47 && T_Y < 63)
					MAPPER_SetBinds(12);
				else if (T_Y > 65 && T_Y < 81)
					MAPPER_SetBinds(13);
				else if (T_Y > 83 && T_Y < 99)
					MAPPER_SetBinds(14);
				else if (T_Y > 101 && T_Y < 117)
					MAPPER_SetBinds(15);
				else if (T_Y > 119 && T_Y < 135)
					MAPPER_SetBinds(16);
				else if (T_Y > 137 && T_Y < 153)
					MAPPER_SetBinds(17);
				else if (T_Y > 173 && T_Y < 189)
				/* load / reset button? */
					MAPPER_LoadBinds();
			}
			/*save button */
			else if (T_X > 120 && T_X < 199)
			{
				if (T_Y > 168 && T_Y < 191)
				{
					gfxDrawScreen(0,0,0,255);
					MAPPER_SaveBinds();
					KeyHeld = true;
				}
			}
		}

		return;
	}
	if ( CurrentMode.active_mode == MODE_SETTING )
	{
		if (!KeyHeld)
		{
			if (T_X > 284 && T_X < 313)
			{
				if (T_Y > 3 && T_Y < 25)
				{
					CPU_CycleIncrease(true);
					KeyHeld = true;
					return;
				}
				else if (T_Y > 26 && T_Y < 49)
				{
					CPU_CycleDecrease(true);
					KeyHeld = true;
					return;
				}
				else if (T_Y > 52 && T_Y < 74)
				{
					if (render.frameskip.max<10) render.frameskip.max++;
					KeyHeld = true;
					return;
				}
				else if (T_Y > 75 && T_Y < 98)
				{
					if (render.frameskip.max>0) render.frameskip.max--;
					KeyHeld = true;
					return;
				}
			}
		}
	}
	return;
}

void button_event()
{
	int i;

	u32 kDown = hidKeysHeld();
	for(i = 0; i < 18; ++i)
	{
		if(ctr_controller_map[i].map_id != KBD_NONE)
			ctr_controller[i].isPressed = SetKey( ctr_controller_map[i].map_id, (kDown & ctr_controller[i].btn_id), ctr_controller[i].isPressed);
	}
	return;
}



void GetInput()
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {

		case SDL_QUIT:
			throw(0);
			break;
		}
	}

	aptMainLoop();
	hidScanInput();

	touch_event();
	button_event();

	return;
}
