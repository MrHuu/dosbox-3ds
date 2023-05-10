#ifndef __CTR_SYSTEM_H__
#define __CTR_SYSTEM_H__

#include <3ds.h>

void ctr_check_dsp();
void ctr_backlight_enable(bool enable, u32 screen);
void ctr_sys_error(bool fatal,const char* error);
void ctr_restart();
//void ctr_restart_with_arg();
void ctr_wait_for_input();

//void __system_initArgv();

void ctr_conf_select();
//void ctr_check_conf(int argc, char* argv[]);

extern char conf_path[256];

/*
typedef struct
{
	u32 argc;
	char args[0x300];
}ciaParam;
*/
extern char *arg_data[0x300];
extern int arg_count;
#endif //__CTR_SYSTEM_H__
