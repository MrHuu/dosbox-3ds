#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <dirent.h>

#include "ctr_system.h"
#include "ctr_gfx.h"

typedef struct fileliststruct {
    char filename[64];
} fileliststruct;

static fileliststruct *filelist = NULL;
char conf_path[256]             = {0};

void ctr_backlight_enable(bool enable, u32 screen) {
	u8 device_model = 0xFF;
	CFGU_GetSystemModel(&device_model);
	if (device_model != CFG_MODEL_2DS)
	{
		gspLcdInit();
		enable ? GSPLCD_PowerOnBacklight(screen):GSPLCD_PowerOffBacklight(screen);
		gspLcdExit();
	}
}

void ctr_sys_error(bool fatal, const char* error) {
	if (!gspHasGpuRight())
		gfxInitDefault();

	errorConf msg;
	errorInit(&msg, ERROR_TEXT, CFG_LANGUAGE_EN);
	errorText(&msg, error);
	errorDisp(&msg);

	if (fatal)
		exit(0);

	return;
}

void ctr_check_dsp() {
	if (envIsHomebrew())
		return;

	FILE *dsp = fopen("sdmc:/3ds/dspfirm.cdc", "r");
	if (dsp == NULL) {
		fclose(dsp);
		ctr_sys_error(1, "Cannot find DSP firmware!\n\n\"sdmc:/3ds/dspfirm.cdc\"");
	}
	fclose(dsp);
}

void ctr_wait_for_input ()
{
	printf("Press a key to continue...\n");
	while (aptMainLoop())
	{
		hidScanInput();
		u32 kDown = hidKeysDown();

		if (kDown) break;

		gspWaitForVBlank();
	}
}

void ctr_restart() {
	Result res;
	u64 titleId;

	if (envIsHomebrew())
	{
		ctr_sys_error(false,"[.3dsx] Restart not supported.");
		return;
	}

	APT_GetAppletInfo((NS_APPID) envGetAptAppId(), &titleId, NULL, NULL, NULL, NULL);

	res = APT_PrepareToDoApplicationJump(0, titleId, 0x1);
	if (R_FAILED(res))
		ctr_sys_error(true,"CIA cant run, cant prepare.");

	res = APT_DoApplicationJump(NULL, 0, NULL);
	if (R_FAILED(res))
		ctr_sys_error(true,"CIA cant run, cant jump.");

	while(1);
}

static int compare_filenames(const void* a, const void* b) {
    const fileliststruct* file_a = (const fileliststruct*)a;
    const fileliststruct* file_b = (const fileliststruct*)b;
    return strcmp(file_a->filename, file_b->filename);
}

static int findConf(const char* confdir) {
    int i = 0;
    DIR* dp = NULL;
    struct dirent* ds;
    dp = opendir(confdir);
    if (dp != NULL) {
        while ((ds = readdir(dp)) != NULL) {
            if ((strstr(ds->d_name, ".conf")) || (strstr(ds->d_name, ".CONF"))) {
                fileliststruct *new_filelist;
                new_filelist = (fileliststruct*)realloc(filelist, (i + 1) * sizeof(fileliststruct));
                if (new_filelist == NULL) {
                    break;
                }
                filelist = new_filelist;
                memset(&filelist[i], 0, sizeof(fileliststruct));
                strcpy(filelist[i].filename, ds->d_name);
                i++;
            }
        }
        closedir(dp);

        qsort(filelist, i, sizeof(fileliststruct), compare_filenames);
    }
    return i;
}

void ctr_conf_select() {
	int listTotal=0;
	const char* scandir = "sdmc:/3ds/dosbox/config";

	listTotal = findConf(scandir);

	if(listTotal < 1)
		return;

	char listing[45]        = {""};
	int timeout             = 400;
	int listMaxDisplay      = 20;
	int listDisplay         = 0;
	int listCurrentPosition = 0;
	int listScrollPosition  = 0;
	u32 kHeldOld            = 0;
	bool redraw;

	while (aptMainLoop())
	{
		gfxDrawSprite(GFX_BOTTOM, GFX_LEFT, (u8*)ctr_bottom_disable_bgr, 240, 320, 0, 0);

		if (timeout > 100)
		{
			char str_timeout[32];
			sprintf(str_timeout, "Timeout: %i",timeout/100);
			gfxDrawText(GFX_BOTTOM, GFX_LEFT, NULL, str_timeout , 230, 220);
			redraw = true;
			timeout--;
		}
		else
		{
			if (timeout!=-1) break;
		}

		hidScanInput();

		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();

		if (kDown & KEY_START) break;

        if (kDown & KEY_DUP)
		{
			listCurrentPosition--;
			if((listCurrentPosition+listScrollPosition) < listScrollPosition)
			{
				listScrollPosition--;
				if(listScrollPosition < 0) listScrollPosition = 0;
			}
			if(listCurrentPosition < 0) listCurrentPosition = 0;
		}

		if (kDown & KEY_DDOWN)
		{
			listCurrentPosition++;
			if(listCurrentPosition > listTotal - 1) listCurrentPosition = listTotal - 1;
			if(listCurrentPosition > listMaxDisplay)
	        {
		        if((listCurrentPosition+listScrollPosition) < listTotal) listScrollPosition++;
			    listCurrentPosition = listMaxDisplay;
			}
		}

		if (kDown & KEY_DLEFT)
		{
						listCurrentPosition = listCurrentPosition - 5;
			if((listCurrentPosition+listScrollPosition) < listScrollPosition)
			{
				listScrollPosition = listScrollPosition - 5;
				if(listScrollPosition < 0) listScrollPosition = 0;
			}
			if(listCurrentPosition < 0) listCurrentPosition = 0;
		}

		if (kDown & KEY_DRIGHT)
		{
						listCurrentPosition = listCurrentPosition + 5;
			if(listCurrentPosition > listTotal - 5) listCurrentPosition = listTotal - 5;
			if(listCurrentPosition > listMaxDisplay)
	        {
		        if((listCurrentPosition+listScrollPosition) < listTotal)
				{
					listScrollPosition = listScrollPosition + 5;
				} else {
					listScrollPosition = listTotal-(listMaxDisplay+1);
				}
			    listCurrentPosition = listMaxDisplay;
			}
		}

		if (kHeld != kHeldOld || redraw )
		{

			if (kHeld != kHeldOld)
				timeout=-1;

			for(listDisplay = 0; listDisplay < listTotal; listDisplay++)
			{
				if(listDisplay <= 20)
				{
					int len = strlen(filelist[listDisplay+listScrollPosition].filename)-5;
					strncpy(listing, "",45);
					strncpy(listing, filelist[listDisplay+listScrollPosition].filename, len);

					if(listDisplay == listCurrentPosition)
						gfxDrawText(GFX_BOTTOM, GFX_LEFT, NULL, (char*) "--->" , (230-(10*listDisplay)), 10);

					gfxDrawText(GFX_BOTTOM, GFX_LEFT, NULL, listing , (230-(10*listDisplay)), 30);
				}
			}

			kHeldOld = kHeld;
			redraw   = false;

			gfxScreenSwapBuffers(GFX_BOTTOM,false);
		}
		gspWaitForVBlank();
	}

	snprintf(conf_path, sizeof(conf_path), "%s/%s",
			scandir, filelist[listCurrentPosition+listScrollPosition].filename);
	free(filelist);

	gfxDrawSprite(GFX_BOTTOM, GFX_LEFT, (u8*)ctr_bottom_load_bgr, 240, 320, 0, 0);
	gfxScreenSwapBuffers(GFX_BOTTOM,false);

	return;
}