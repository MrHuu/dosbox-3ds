/*
 *  Copyright (C) 2002-2021  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <malloc.h>
#include <sys/types.h>

#include "cross.h"
#include "SDL.h"

#include "dosbox.h"
#include "video.h"
#include "mouse.h"
#include "pic.h"
#include "timer.h"
#include "setup.h"
#include "support.h"
#include "debug.h"
#include "mapper.h"
#include "vga.h"
#include "keyboard.h"
#include "cpu.h"
#include "cross.h"
#include "control.h"
#include "render.h"

#include "ctr_system.h"
#include "ctr_input.h"

#include "../../gui/dosbox_splash.h"

#define DEFAULT_CONFIG_FILE "/dosbox.conf"
#define MAPPERFILE "mapper-" VERSION ".map"

#define STDOUT_FILE	TEXT("stdout.txt")
#define STDERR_FILE	TEXT("stderr.txt")

#define DISABLE_JOYSTICK

#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000

#ifdef CTR_GFXEND_THREADED
	LightLock mutex;
	Thread thread;
	volatile bool thread_started;
	volatile bool kill_thread;
#endif

static u32 *SOC_buffer = NULL;

extern ctr_settings_t ctr_settings;

void MountDOSBoxDir(char DriveLetter, const char *path);
aptHookCookie cookie;
void aptHookFunc(APT_HookType hookType, void *param)
{
	switch (hookType) {
		case APTHOOK_ONSUSPEND:
			ctr_backlight_enable(true,GSPLCD_SCREEN_BOTTOM);
		case APTHOOK_ONSLEEP:
			break;
		case APTHOOK_ONRESTORE:
		case APTHOOK_ONWAKEUP:
//			ctr_backlight_enable(false,GSPLCD_SCREEN_BOTTOM);
			break;
		default:
			break;
	}
}

struct SDL_Block {
	bool inited;
	bool active;
	bool updating;
	struct {
		Bit32u width;
		Bit32u height;
		Bit32u bpp;
		Bitu flags;
		double scalex,scaley;
		GFX_CallBack_t callback;
	} draw;
	bool wait_on_error;
	struct {
		struct {
			Bit16u width, height;
			bool fixed;
		} full;
		struct {
			Bit16u width, height;
		} window;
		Bit8u bpp;
	} desktop;
	struct {
		SDL_Surface * surface;
	} blit;
	SDL_Rect clip;
	SDL_Surface * surface;
	SDL_cond *cond;
	SDL_Rect updateRects[1024];
};

static SDL_Block sdl;

SDL_Surface* SDL_SetVideoMode_Wrap(int width,int height,int bpp,Bit32u flags){

	SDL_Surface* s = SDL_SetVideoMode(width,height,16,flags|
			((flags & SDL_BOTTOMSCR)?SDL_BOTTOMSCR:SDL_TOPSCR)|
			(ctr_settings.gfx_scale_to_fit?SDL_FITHEIGHT:NULL)|
			(ctr_settings.gfx_keep_aspect?NULL:SDL_FITWIDTH));

	return s;
}

//Globals for keyboard initialisation
bool startup_state_numlock=false;
bool startup_state_capslock=false;

void GFX_SetTitle(Bit32s cycles,int frameskip,bool paused){
	return;
}

void GFX_GetSize(int &width, int &height, bool &fullscreen) {
	width = sdl.draw.width;
	height = sdl.draw.height;
}

Bitu GFX_GetBestMode(Bitu flags) {
	return GFX_CAN_16;
}

void GFX_ResetScreen(void) {
	GFX_Stop();
	if (sdl.draw.callback)
		(sdl.draw.callback)( GFX_CallBackReset );
	GFX_Start();
	CPU_Reset_AutoAdjust();
}

void GFX_TearDown(void) {
	#ifndef CTR_GFXEND_THREADED
		if (sdl.updating)
			GFX_EndUpdate( 0 );
	#endif

	if (sdl.blit.surface) {
		SDL_FreeSurface(sdl.blit.surface);
		sdl.blit.surface=0;
	}
}

Bitu GFX_SetSize(Bitu width,Bitu height,Bitu flags,double scalex,double scaley,GFX_CallBack_t callback) {
	if (sdl.updating)
		GFX_EndUpdate( 0 );

	sdl.draw.width=width;
	sdl.draw.height=height;
	sdl.draw.callback=callback;
	sdl.draw.scalex=scalex;
	sdl.draw.scaley=scaley;

	int bpp=0;
	Bitu retFlags = 0;

	if (sdl.blit.surface) {
		SDL_FreeSurface(sdl.blit.surface);
		sdl.blit.surface=0;
	}

	bpp=16;

	sdl.clip.x=0;sdl.clip.y=0;
	sdl.surface=SDL_SetVideoMode_Wrap(width,height,bpp,(flags & GFX_CAN_RANDOM) ? SDL_SWSURFACE : SDL_HWSURFACE);
	if (sdl.surface == NULL)
		E_Exit("Could not set windowed video mode %ix%i-%i: %s",(int)width,(int)height,bpp,SDL_GetError());

	if (sdl.surface) {
		switch (sdl.surface->format->BitsPerPixel) {
		case 8:
			retFlags = GFX_CAN_8;
			break;
		case 15:
			retFlags = GFX_CAN_15;
			break;
		case 16:
			retFlags = GFX_CAN_16;
			break;
		case 32:
			retFlags = GFX_CAN_32;
			break;
		}
		if (retFlags && (sdl.surface->flags & SDL_HWSURFACE))
			retFlags |= GFX_HARDWARE;
		if (retFlags && (sdl.surface->flags & SDL_DOUBLEBUF)) {
			sdl.blit.surface=SDL_CreateRGBSurface(SDL_HWSURFACE,
				sdl.draw.width, sdl.draw.height,
				sdl.surface->format->BitsPerPixel,
				sdl.surface->format->Rmask,
				sdl.surface->format->Gmask,
				sdl.surface->format->Bmask,
			0);
			/* If this one fails be ready for some flickering... */
		}
	}

	if (retFlags)
		GFX_Start();

	return retFlags;
}

bool GFX_StartUpdate(Bit8u * & pixels,Bitu & pitch) {
	if (!sdl.active || sdl.updating)
		return false;

	if (sdl.blit.surface) {
		if (SDL_MUSTLOCK(sdl.blit.surface) && SDL_LockSurface(sdl.blit.surface))
			return false;
		pixels=(Bit8u *)sdl.blit.surface->pixels;
		pitch=sdl.blit.surface->pitch;
	} else {
		if (SDL_MUSTLOCK(sdl.surface) && SDL_LockSurface(sdl.surface))
			return false;
		pixels=(Bit8u *)sdl.surface->pixels;
		pixels+=sdl.clip.y*sdl.surface->pitch;
		pixels+=sdl.clip.x*sdl.surface->format->BytesPerPixel;
		pitch=sdl.surface->pitch;
	}
	
	sdl.updating=true;
	return true;
}

void GFX_EndUpdate_Thread() {
	thread_started = true;
	const Bit16u *changedLines = 0;
	while (!kill_thread)
	{
		sdl.updating=false;

		if (SDL_MUSTLOCK(sdl.surface)) {
			if (sdl.blit.surface) {
				SDL_UnlockSurface(sdl.blit.surface);
				int Blit = SDL_BlitSurface( sdl.blit.surface, 0, sdl.surface, &sdl.clip );
				LOG(LOG_MISC,LOG_WARN)("BlitSurface returned %d",Blit);
			} else {
				SDL_UnlockSurface(sdl.surface);
			}
			SDL_Flip(sdl.surface);
		} else if (changedLines) {
			Bitu y = 0, index = 0, rectCount = 0;
			while (y < sdl.draw.height) {
				if (!(index & 1)) {
					y += changedLines[index];
				} else {
					SDL_Rect *rect = &sdl.updateRects[rectCount++];
					rect->x = sdl.clip.x;
					rect->y = sdl.clip.y + y;
					rect->w = (Bit16u)sdl.draw.width;
					rect->h = changedLines[index];
						y += changedLines[index];
				}
				index++;
			}
			if (rectCount)
				SDL_UpdateRects( sdl.surface, rectCount, sdl.updateRects );
		}
	}
}

void GFX_EndUpdate( const Bit16u *changedLines ) {
	if (!RENDER_GetForceUpdate() && !sdl.updating)
		return;

	sdl.updating=false;

	if (SDL_MUSTLOCK(sdl.surface)) {
		if (sdl.blit.surface) {
			SDL_UnlockSurface(sdl.blit.surface);
			int Blit = SDL_BlitSurface( sdl.blit.surface, 0, sdl.surface, &sdl.clip );
			LOG(LOG_MISC,LOG_WARN)("BlitSurface returned %d",Blit);
		} else {
			SDL_UnlockSurface(sdl.surface);
		}
		SDL_Flip(sdl.surface);
	} else if (changedLines) {
		Bitu y = 0, index = 0, rectCount = 0;
		while (y < sdl.draw.height) {
			if (!(index & 1)) {
				y += changedLines[index];
			} else {
				SDL_Rect *rect = &sdl.updateRects[rectCount++];
				rect->x = sdl.clip.x;
				rect->y = sdl.clip.y + y;
				rect->w = (Bit16u)sdl.draw.width;
				rect->h = changedLines[index];
					y += changedLines[index];
			}
			index++;
		}
		if (rectCount)
			SDL_UpdateRects( sdl.surface, rectCount, sdl.updateRects );
	}
}

void GFX_SetPalette(Bitu start,Bitu count,GFX_PalEntry * entries) {
	#ifdef CTR_GFXEND_THREADED
		LightLock_Lock(&mutex);
	#endif
	/* I should probably not change the GFX_PalEntry :) */
	if (sdl.surface->flags & SDL_HWPALETTE) {
		if (!SDL_SetPalette(sdl.surface,SDL_PHYSPAL,(SDL_Color *)entries,start,count)) {
			E_Exit("SDL:Can't set palette");
		}
	} else {
		if (!SDL_SetPalette(sdl.surface,SDL_LOGPAL,(SDL_Color *)entries,start,count)) {
			E_Exit("SDL:Can't set palette");
		}
	}
	#ifdef CTR_GFXEND_THREADED
		LightLock_Unlock(&mutex);
	#endif
}

Bitu GFX_GetRGB(Bit8u red,Bit8u green,Bit8u blue) {
	return SDL_MapRGB(sdl.surface->format,red,green,blue);
}

void GFX_LosingFocus(void) {
}

void GFX_Events() {
	GetInput();
}

/* static variable to show wether there is not a valid stdout.
 * Fixes some bugs when -noconsole is used in a read only directory */
static bool no_stdout = false;
void GFX_ShowMsg(char const* format,...) {
	char buf[512];

	va_list msg;
	va_start(msg,format);
	vsnprintf(buf,sizeof(buf),format,msg);
	va_end(msg);

	buf[sizeof(buf) - 1] = '\0';
	if (!no_stdout) puts(buf); //Else buf is parsed again. (puts adds end of line)
}

void GFX_Stop() {
	#ifdef CTR_GFXEND_THREADED
		LightLock_Lock(&mutex);
	#endif
	if (sdl.updating)
		GFX_EndUpdate( 0 );
	sdl.active=false;
	#ifdef CTR_GFXEND_THREADED
		LightLock_Unlock(&mutex);
	#endif
}

void GFX_Start() {
	sdl.active=true;
	#ifdef CTR_GFXEND_THREADED
		LightLock_Init(&mutex);
		kill_thread=false;

		if (!RENDER_GetForceUpdate() && !sdl.updating)
			return;

		if (!thread_started)
		{
			//I think Core 1 at 70% should be plenty even for New 3DS Stuff.
			thread = threadCreate(GFX_EndUpdate_Thread, 0, 2 * 1024, 0x18, 1, true);
		}
	#endif
}

static void GUI_ShutDown(Section * /*sec*/) {
	GFX_Stop();
	if (sdl.draw.callback) (sdl.draw.callback)( GFX_CallBackStop );
	#ifdef CTR_GFXEND_THREADED
		kill_thread=true;
	#endif
}

static void GUI_StartUp(Section * sec) {
	sec->AddDestroyFunction(&GUI_ShutDown);
	Section_prop * section=static_cast<Section_prop *>(sec);
}

static void GUI_ShowSplash() {
	sdl.active=false;
	sdl.updating=false;

	/* Initialize screen for first time */
	sdl.surface=SDL_SetVideoMode_Wrap(640,400,0,0);
	if (sdl.surface == NULL) E_Exit("Could not initialize video: %s",SDL_GetError());
	sdl.desktop.bpp=sdl.surface->format->BitsPerPixel;

	GFX_Stop();

    Bit32u rmask = 0x000000ff;
    Bit32u gmask = 0x0000ff00;
    Bit32u bmask = 0x00ff0000;

	/* Please leave the Splash screen stuff in working order in DOSBox. We spend a lot of time making DOSBox. */
	SDL_Surface* splash_surf = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 400, 32, rmask, gmask, bmask, 0);
	if (splash_surf) {
		SDL_FillRect(splash_surf, NULL, SDL_MapRGB(splash_surf->format, 0, 0, 0));

		Bit8u* tmpbufp = new Bit8u[640*400*3];
		GIMP_IMAGE_RUN_LENGTH_DECODE(tmpbufp,gimp_image.rle_pixel_data,640*400,3);
		for (Bitu y=0; y<400; y++) {

			Bit8u* tmpbuf = tmpbufp + y*640*3;
			Bit32u * draw=(Bit32u*)(((Bit8u *)splash_surf->pixels)+((y)*splash_surf->pitch));
			for (Bitu x=0; x<640; x++) {
				*draw++ = tmpbuf[x*3+0]+tmpbuf[x*3+1]*0x100+tmpbuf[x*3+2]*0x10000+0x00000000;
			}
		}

		bool exit_splash = false;

		static Bitu max_splash_loop = 1000;
//		static Bitu splash_fade = 100;
//		static bool use_fadeout = true;

		for (Bit32u ct = 0,startticks = GetTicks();ct < max_splash_loop;ct = GetTicks()-startticks) {
			SDL_Event evt;
			while (SDL_PollEvent(&evt)) {
				if (evt.type == SDL_QUIT) {
					exit_splash = true;
					break;
				}
			}
			if (exit_splash) break;
/*
			if (ct<1) {*/
				SDL_FillRect(sdl.surface, NULL, SDL_MapRGB(sdl.surface->format, 0, 0, 0));
				SDL_SetAlpha(splash_surf, SDL_SRCALPHA,255);
				SDL_BlitSurface(splash_surf, NULL, sdl.surface, NULL);
				SDL_Flip(sdl.surface);/*
			} else if (ct>=max_splash_loop-splash_fade) {
				if (use_fadeout) {
					SDL_FillRect(sdl.surface, NULL, SDL_MapRGB(sdl.surface->format, 0, 0, 0));
					SDL_SetAlpha(splash_surf, SDL_SRCALPHA, (Bit8u)((max_splash_loop-1-ct)*255/(splash_fade-1)));
					SDL_BlitSurface(splash_surf, NULL, sdl.surface, NULL);
					SDL_Flip(sdl.surface);
				}
			}
			*/

		}
/*
		if (use_fadeout) {
			SDL_FillRect(sdl.surface, NULL, SDL_MapRGB(sdl.surface->format, 0, 0, 0));
			SDL_Flip(sdl.surface);
		}*/
			SDL_FillRect(sdl.surface, NULL, SDL_MapRGB(sdl.surface->format, 0, 0, 0));
			SDL_Flip(sdl.surface);


		SDL_FreeSurface(splash_surf);
		delete [] tmpbufp;

	}
}

bool mouselocked; //Global variable for mapper

void GFX_CaptureMouse(void) {
}

void Mouse_AutoLock(bool enable) {
}

void Config_Add_SDL() {
	Section_prop * sdl_sec=control->AddSection_prop("sdl",&GUI_StartUp);
	sdl_sec->AddInitFunction(&MAPPER_StartUp);
	Prop_bool* Pbool;
	Prop_string* Pstring;

	Pbool = sdl_sec->Add_bool("waitonerror",Property::Changeable::Always, true);
	Pbool->Set_help("Wait before closing the console if dosbox has an error.");

	Pstring = sdl_sec->Add_path("mapperfile",Property::Changeable::Always,MAPPERFILE);
	Pstring->Set_help("File used to load/save the key/event mappings from. Resetmapper only works with the default value.");

}

void shutdown_program() {
	throw 1;
}
void restart_program(std::vector<std::string> & parameters) {
	ctr_restart();
}

int main(int argc, char* argv[]) {

	aptHook(&cookie, aptHookFunc, NULL);
	APT_SetAppCpuTimeLimit(70);
	osSetSpeedupEnable(true);

	ctr_check_dsp();

	cfguInit();

	SOC_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
	socInit(SOC_buffer, SOC_BUFFERSIZE);

	try {
		CommandLine com_line(argc,argv);

		Config myconf(&com_line);
		control=&myconf;

		/* Init the configuration system and add default values */
		Config_Add_SDL();
		DOSBOX_Init();

		/* Init SDL */
		if ( SDL_Init( SDL_INIT_AUDIO|SDL_INIT_VIDEO ) < 0 ) E_Exit("Can't init SDL %s",SDL_GetError());
		sdl.inited = true;

		SDL_ShowCursor(SDL_DISABLE);

		GUI_ShowSplash();

		/* Parse configuration files */
		std::string config_file, config_path, config_combined;
		Cross::GetPlatformConfigDir(config_path);

		//First parse -userconf
		if(control->cmdline->FindExist("-userconf",true)){
			config_file.clear();
			Cross::GetPlatformConfigDir(config_path);
			Cross::GetPlatformConfigName(config_file);
			config_combined = config_path + config_file;
			control->ParseConfigFile(config_combined.c_str());
			if(!control->configfiles.size()) {
				//Try to create the userlevel configfile.
				config_file.clear();
				Cross::CreatePlatformConfigDir(config_path);
				Cross::GetPlatformConfigName(config_file);
				config_combined = config_path + config_file;
				if(control->PrintConfig(config_combined.c_str())) {
					LOG_MSG("CONFIG: Generating default configuration.\nWriting it to %s",config_combined.c_str());
					//Load them as well. Makes relative paths much easier
					control->ParseConfigFile(config_combined.c_str());
				}
			}
		}

		//Second parse -conf switches
		while(control->cmdline->FindString("-conf",config_file,true)) {
			if (!control->ParseConfigFile(config_file.c_str())) {
				// try to load it from the user directory
				if (!control->ParseConfigFile((config_path + config_file).c_str())) {
					LOG_MSG("CONFIG: Can't open specified config file: %s",config_file.c_str());
				}
			}
		}

		// if none found => user selection list of all .conf found in 'config' folder.
		if(!control->configfiles.size()) {ctr_conf_select();control->ParseConfigFile((char*)conf_path);}

		// if none found => parse localdir conf
		if(!control->configfiles.size()) control->ParseConfigFile("dosbox.conf");

		// if none found => parse userlevel conf
		if(!control->configfiles.size()) {
			config_file.clear();
			Cross::GetPlatformConfigName(config_file);
			control->ParseConfigFile((config_path + config_file).c_str());
		}

		if(!control->configfiles.size()) {
			//Try to create the userlevel configfile.
			config_file.clear();
			Cross::CreatePlatformConfigDir(config_path);
			Cross::GetPlatformConfigName(config_file);
			config_combined = config_path + config_file;
			if(control->PrintConfig(config_combined.c_str())) {
				LOG_MSG("CONFIG: Generating default configuration.\nWriting it to %s",config_combined.c_str());
				//Load them as well. Makes relative paths much easier
				control->ParseConfigFile(config_combined.c_str());
			} else {
				LOG_MSG("CONFIG: Using default settings. Create a configfile to change them");
			}
		}

		/* Init all the sections */
		control->Init();
		/* Some extra SDL Functions */

		/* Init the keyMapper */
		MAPPER_Init();
//		if (control->cmdline->FindExist("-startmapper")) MAPPER_RunInternal();
#if defined(CTR_ROMFS)
		romfsInit();
		MountDOSBoxDir('X', "romfs:/");
#endif
		/* Start up main machine */
		control->StartUp();
		/* Shutdown everything */
	} catch (char * error) {
		ctr_sys_error(false,error);
		gfxExit();
	}
	catch (int){
		; //nothing, pressed killswitch
	}
	catch(...){
		; // Unknown error, let's just exit.
	}
	SDL_Quit();//Let's hope sdl will quit as well when it catches an exception

#if defined(CTR_ROMFS)
	romfsExit();
#endif
	ctr_backlight_enable(true,GSPLCD_SCREEN_BOTTOM);
	socExit();
	cfguExit();

	return 0;
}


