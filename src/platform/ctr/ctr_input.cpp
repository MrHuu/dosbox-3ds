#include "ctr_input.h"

bool KEY_PRESSED_UP;
bool KEY_PRESSED_RIGHT;
bool KEY_PRESSED_DOWN;
bool KEY_PRESSED_LEFT;
bool KEY_PRESSED_A;
bool KEY_PRESSED_B;
bool KEY_PRESSED_X;
bool KEY_PRESSED_Y;
bool KEY_PRESSED_L;
bool KEY_PRESSED_R;
bool KEY_PRESSED_SELECT;
bool KEY_PRESSED_START;
bool KEY_PRESSED_ZL;
bool KEY_PRESSED_ZR;
bool KEY_PRESSED_ZL2;
bool KEY_PRESSED_ZR2;

bool keyHeld = false;
KBD_KEYS prevHeld = KBD_NONE;

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
		memcpy(&fbAdr[((x+xOffset)+(y+j)*fbWidth)*3], &spriteData[((xOffset)+(j)*width)*3], widthDrawn*3);
	}

}

s16 T_X;
s16 T_Y;

KBD_KEYS TouchKey(s16 T_X, s16 T_Y)
{
	if (T_Y < 34)
	{
		if (T_X > 1 && T_X < 30)
			return KBD_esc;
		else if (T_X > 30 && T_X < 55)
			return KBD_f1;
		else if (T_X > 54 && T_X < 79)
			return KBD_f2;
		else if (T_X > 78 && T_X < 103)
			return KBD_f3;
		else if (T_X > 102 && T_X < 127)
			return KBD_f4;
		else if (T_X > 126 && T_X < 151)
			return KBD_f5;
		else if (T_X > 150 && T_X < 175)
			return KBD_f6;
		else if (T_X > 174 && T_X < 199)
			return KBD_f7;
		else if (T_X > 198 && T_X < 223)
			return KBD_f8;		
		else if (T_X > 222 && T_X < 247)
			return KBD_f9;
		else if (T_X > 246 && T_X < 271)
			return KBD_f10;
		else if (T_X > 270 && T_X < 295)
			return KBD_f11;		
		else if (T_X > 294 && T_X < 319)
			return KBD_f12;
		else
			return KBD_NONE;
	}
	else if (T_Y > 33 && T_Y < 67)
	{
		if (T_X > 1 && T_X < 30)
			return KBD_1;
		else if (T_X > 29 && T_X < 59)
			return KBD_2;
		else if (T_X > 58 && T_X < 88)
			return KBD_3;
		else if (T_X > 87 && T_X < 117)
			return KBD_4;
		else if (T_X > 116 && T_X < 146)
			return KBD_5;
		else if (T_X > 145 && T_X < 175)
			return KBD_6;
		else if (T_X > 174 && T_X < 204)
			return KBD_7;
		else if (T_X > 203 && T_X < 233)
			return KBD_8;
		else if (T_X > 232 && T_X < 262)
			return KBD_9;
		else if (T_X > 261 && T_X < 291)
			return KBD_0;
		else if (T_X > 290 && T_X < 319)
			return KBD_backspace;
		else
			return KBD_NONE;
	}
	else if (T_Y > 66 && T_Y < 100)
	{
		if (T_X > 1 && T_X < 30)
			return KBD_q;
		else if (T_X > 29 && T_X < 59)
			return KBD_w;
		else if (T_X > 58 && T_X < 88)
			return KBD_e;
		else if (T_X > 87 && T_X < 117)
			return KBD_r;
		else if (T_X > 116 && T_X < 146)
			return KBD_t;
		else if (T_X > 145 && T_X < 175)
			return KBD_y;
		else if (T_X > 174 && T_X < 204)
			return KBD_u;
		else if (T_X > 203 && T_X < 233)
			return KBD_i;
		else if (T_X > 232 && T_X < 262)
			return KBD_o;
		else if (T_X > 261 && T_X < 291)
			return KBD_p;
		else if (T_X > 290 && T_X < 319)
			return KBD_enter;
		else
			return KBD_NONE;
	}
	else if (T_Y > 99 && T_Y < 133)
	{
		if (T_X > 1 && T_X < 16)
			return KBD_tab;
		else if (T_X > 15 && T_X < 45)
			return KBD_a;
		else if (T_X > 44 && T_X < 74)
			return KBD_s;
		else if (T_X > 73 && T_X < 103)
			return KBD_d;
		else if (T_X > 102 && T_X < 132)
			return KBD_f;
		else if (T_X > 131 && T_X < 161)
			return KBD_g;
		else if (T_X > 160 && T_X < 190)
			return KBD_h;
		else if (T_X > 189 && T_X < 219)
			return KBD_j;
		else if (T_X > 218 && T_X < 248)
			return KBD_k;
		else if (T_X > 247 && T_X < 277)
			return KBD_l;
		else if (T_X > 276 && T_X < 319)
			return KBD_enter;
		else
			return KBD_NONE;
	}
	else if (T_Y > 132 && T_Y < 166)
	{
		if (T_X > 1 && T_X < 37)
			return KBD_capslock;
		else if (T_X > 36 && T_X < 66)
			return KBD_z;
		else if (T_X > 65 && T_X < 95)
			return KBD_x;
		else if (T_X > 94 && T_X < 124)
			return KBD_c;
		else if (T_X > 123 && T_X < 153)
			return KBD_v;
		else if (T_X > 152 && T_X < 182)
			return KBD_b;
		else if (T_X > 181 && T_X < 211)
			return KBD_n;
		else if (T_X > 210 && T_X < 240)
			return KBD_m;
		else if (T_X > 239 && T_X < 319)
			return KBD_rightshift;
		else
			return KBD_NONE;
	}
	else if (T_Y > 165 && T_Y < 199)
	{
		if (T_X > 1 && T_X < 45)
			return KBD_NONE;
		else if (T_X > 44 && T_X < 89)
			return KBD_NONE;
		else if (T_X > 88 && T_X < 232)
			return KBD_space;
		else if (T_X > 231 && T_X < 276)
			return KBD_rightalt;
		else if (T_X > 275 && T_X < 319)
			return KBD_rightctrl;
		else
			return KBD_NONE;
	}

	else
		return KBD_NONE;

}

bool SetKey(KBD_KEYS key, bool pressed, bool kHeld)
{
	if (pressed && !kHeld)
	{
		kHeld = true;
		KEYBOARD_AddKey(key,true);
	}
	else if (!pressed && kHeld)
	{
		kHeld = false;
		KEYBOARD_AddKey(key,false);
	}

	return kHeld;
}

void GetInput()
{
		aptMainLoop();
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {

		case SDL_QUIT:
			throw(0);
			break;
		}
	}

	hidScanInput();

	touchPosition touch;
	hidTouchRead(&touch);

	if ((touch.px!=0 && touch.py!=0) && !keyHeld)
	{
		keyHeld = true;
		prevHeld = TouchKey(touch.px,touch.py);
		if (prevHeld != KBD_NONE) 
			KEYBOARD_AddKey(prevHeld ,true);
		gfxDrawSprite(GFX_BOTTOM, GFX_LEFT, (u8*)ctr_bottom_kbd_bgr, 240, 320, 0, 0);
		gfxScreenSwapBuffers(GFX_BOTTOM,false);
	}
	else if ((touch.px==0 && touch.py==0) && keyHeld )
	{
		keyHeld = false;
		if (prevHeld != KBD_NONE) 
			KEYBOARD_AddKey(prevHeld ,false);
	}

	u32 kDown = hidKeysHeld();

	KEY_PRESSED_UP     = SetKey( KBD_up        , (kDown & KEY_UP)     , KEY_PRESSED_UP     );
	KEY_PRESSED_RIGHT  = SetKey( KBD_right     , (kDown & KEY_RIGHT)  , KEY_PRESSED_RIGHT  );
	KEY_PRESSED_DOWN   = SetKey( KBD_down      , (kDown & KEY_DOWN)   , KEY_PRESSED_DOWN   );
	KEY_PRESSED_LEFT   = SetKey( KBD_left      , (kDown & KEY_LEFT)   , KEY_PRESSED_LEFT   );
	KEY_PRESSED_A      = SetKey( KBD_up        , (kDown & KEY_A)      , KEY_PRESSED_A      );
	KEY_PRESSED_B      = SetKey( KBD_space     , (kDown & KEY_B)      , KEY_PRESSED_B      );
	KEY_PRESSED_X      = SetKey( KBD_rightctrl , (kDown & KEY_X)      , KEY_PRESSED_X      );
	KEY_PRESSED_Y      = SetKey( KBD_rightalt  , (kDown & KEY_Y)      , KEY_PRESSED_Y      );
	KEY_PRESSED_L      = SetKey( KBD_up        , (kDown & KEY_L)      , KEY_PRESSED_L      );
	KEY_PRESSED_R      = SetKey( KBD_right     , (kDown & KEY_R)      , KEY_PRESSED_R      );
	KEY_PRESSED_SELECT = SetKey( KBD_esc       , (kDown & KEY_SELECT) , KEY_PRESSED_SELECT );
	KEY_PRESSED_START  = SetKey( KBD_enter     , (kDown & KEY_START)  , KEY_PRESSED_START  );
	KEY_PRESSED_ZL     = SetKey( KBD_rightctrl , (kDown & KEY_ZL)     , KEY_PRESSED_ZL     );
	KEY_PRESSED_ZL2    = SetKey( KBD_f11       , (kDown & KEY_ZL)     , KEY_PRESSED_ZL2    );
    KEY_PRESSED_ZR     = SetKey( KBD_f12       , (kDown & KEY_ZR)     , KEY_PRESSED_ZR     );
	KEY_PRESSED_ZR2    = SetKey( KBD_rightctrl , (kDown & KEY_ZR)     , KEY_PRESSED_ZR2    );


	return;
}