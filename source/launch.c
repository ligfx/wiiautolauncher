
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <gccore.h>
#include <time.h>
#include <sdcard/wiisd_io.h>

#include "patchcode.h"
#include "disc.h"
#include "fst.h"
#include "launch.h"
#include "identify.h"
#include "sd.h"
#include "wdvd.h"
#include "apploader.h"

u8 loaderhbc = 0;
u8 identifysu = 0;
u32 GameIOS = 0;
u32 vmode_reg = 0;
GXRModeObj *vmode = NULL;
u32 AppEntrypoint = 0;
char gameidbuffer[8];
u8 vidMode = 0;
int patchVidModes = 0x0;
u8 vipatchselect = 0;
u8 countryString = 0;
int aspectRatio = -1;

extern void __exception_closeall();


s32 rundisc() {

	Disc_SetLowMemPre();
	WDVD_Init();
	if(Disc_Open() != 0) {
		printf("Cannot open disc\n");
		while(1);
		return 0;
	}
	
	if(disc_type == IS_UNK_DISC) {
		printf("Unknown disc\n");
		while(1);
		return 0;
	}
	if(disc_type == IS_NGC_DISC) {
		printf("Launch GameCube disc\n");
		printf("Cannot use cheat codes and other features\n");
	
		WII_LaunchTitle(0x100000100LL);
		return 0;
	}
	memset(gameidbuffer, 0, 8);
	memcpy(gameidbuffer, (char*)0x80000000, 6);
	printf("launch_rundisc: gameidbuffer value = %s\n", gameidbuffer);
	if (config_bytes[7] == 0x00)
		codelist = (u8 *) 0x800022A8;
	else
		codelist = (u8 *) 0x800028B8;
	codelistend = (u8 *) 0x80003000;
	
	if (config_bytes[2] != 0x00) {
		if(config_bytes[3] == 0x01)
			sd_copy_patch(gameidbuffer);		
		sd_copy_gameconfig(gameidbuffer);	
		if(config_bytes[4] == 0x01)
			sd_copy_codes(gameidbuffer);
	}
		
	switch (codes_state) {
        case 0:
            break;
        case 1:
            printf("Game ID: %s\n", gameidbuffer);
            printf("No SD Codes Found\n");
            break;
        case 2:
            printf("Game ID: %s\n", gameidbuffer);
            printf("SD Codes Found. Applying\n");
            break;
        case 3:
            printf("Game ID: %s\n", gameidbuffer);
            printf("No Hook, Not Applying Codes\n");
            break;
        case 4:
            printf("Game ID: %s\n", gameidbuffer);
            printf("Codes Error: Too Many Codes\n");
            printf("%u Lines Found, %u Lines Allowed\n", (codelistsize / 8) - 2, ((codelistend - codelist) / 8) - 2);
            while(1);
            break;
    }
   __io_wiisd.shutdown();
	printf("\n");
	
	configbytes[0] = config_bytes[0];
	printf("launch_rundisc: configbytes[0] value = %d\n", configbytes[0]);
	configbytes[1] = config_bytes[1];
	printf("launch_rundisc: configbytes[1] value = %d\n", configbytes[1]);
	hooktype = config_bytes[2];
	printf("launch_rundisc: hooktype value = %d\n", hooktype);
	debuggerselect = config_bytes[7];
	printf("launch_rundisc: debuggerselect value = %d\n", debuggerselect);
	u8 codesselect = config_bytes[4];
	printf("launch_rundisc: codesselect value = %d\n", codesselect);
	if(codesselect)
		ocarina_set_codes(codelist, codelistend, tempcodelist, codelistsize);
	countryString = config_bytes[13];
	printf("launch_rundisc: countryString value = %d\n", countryString);	
	switch(config_bytes[1]) {
		case 0:
			vidMode = 0;
		break;
		case 1:
			vidMode = 3;
		break;
		case 2:
			vidMode = 2;
		break;
		case 3:
			vidMode = 4;
		break;
	}
	printf("launch_rundisc: vidMode value = %d\n", vidMode);
	switch(config_bytes[12]) {	
		case 0:			
		break;
		case 1:
			vipatchselect = 1;
		break;
		case 2:
			patchVidModes = 0;
		break;
		case 3:
			patchVidModes = 1;
		break;
		case 4:
			patchVidModes = 2;
		break;
	}
	printf("launch_rundisc: vipatchselect value = %d\n", vipatchselect);
	printf("launch_rundisc: patchVidModes value = %d\n", patchVidModes);
	if(config_bytes[14] > 0)
		aspectRatio = (int)config_bytes[14] - 1;
	printf("launch_rundisc: aspectRatio value = %d\n", aspectRatio);
	
	u32 offset = 0;
	Disc_FindPartition(&offset);
	WDVD_OpenPartition(offset, &GameIOS);
	printf("launch_rundisc: GameIOS value: %i\n", GameIOS);
	vmode = Disc_SelectVMode(vidMode, &vmode_reg);
	AppEntrypoint = Apploader_Run(vidMode, vmode, vipatchselect, countryString,
					patchVidModes, aspectRatio);
	load_handler();
	if(config_bytes[5] == 1) {
		*(u32*)(*(u32*)0x80001808) = 0x1;
		DCFlushRange((void*)(*(u32*)(*(u32*)0x80001808)), 4);
	}
	if(hooktype != 0 && hookpatched) 
		ocarina_do_code();
	
	if (config_bytes[2] != 0x00) {
		apply_pokevalues();

		if(config_bytes[3] == 0x01 && patch_state == 1)
			apply_patch();
	}
	
	printf("launch_rundisc: AppEntrypoint value = %08X\n", AppEntrypoint);
	WDVD_Close();

	Disc_SetLowMem(GameIOS);
	Disc_SetTime();
	Disc_SetVMode(vmode, vmode_reg);

	u32 level = IRQ_Disable();
	__IOS_ShutdownSubsystems();
	__exception_closeall();

	*(vu32*)0xCC003024 = 1;

 	if(AppEntrypoint == 0x3400)
	{
 		if(hooktype)
 		{
			asm volatile (
				"lis %r3, returnpointdisc@h\n"
				"ori %r3, %r3, returnpointdisc@l\n"
				"mtlr %r3\n"
				"lis %r3, 0x8000\n"
				"ori %r3, %r3, 0x18A8\n"
				"nop\n"
				"mtctr %r3\n"
				"bctr\n"
				"returnpointdisc:\n"
				"bl DCDisable\n"
				"bl ICDisable\n"
				"li %r3, 0\n"
				"mtsrr1 %r3\n"
				"lis %r4, AppEntrypoint@h\n"
				"ori %r4,%r4,AppEntrypoint@l\n"
				"lwz %r4, 0(%r4)\n"
				"mtsrr0 %r4\n"
				"rfi\n"
			);
 		}
 		else
 		{
 			asm volatile (
 				"isync\n"
				"lis %r3, AppEntrypoint@h\n"
				"ori %r3, %r3, AppEntrypoint@l\n"
 				"lwz %r3, 0(%r3)\n"
 				"mtsrr0 %r3\n"
 				"mfmsr %r3\n"
 				"li %r4, 0x30\n"
 				"andc %r3, %r3, %r4\n"
 				"mtsrr1 %r3\n"
 				"rfi\n"
 			);
 		}
	}
 	else if(hooktype)
	{
		asm volatile (
			"lis %r3, AppEntrypoint@h\n"
			"ori %r3, %r3, AppEntrypoint@l\n"
			"lwz %r3, 0(%r3)\n"
			"mtlr %r3\n"
			"lis %r3, 0x8000\n"
			"ori %r3, %r3, 0x18A8\n"
			"nop\n"
			"mtctr %r3\n"
			"bctr\n"
		);
	}
	else
	{
		asm volatile (
			"lis %r3, AppEntrypoint@h\n"
			"ori %r3, %r3, AppEntrypoint@l\n"
			"lwz %r3, 0(%r3)\n"
			"mtlr %r3\n"
			"blr\n"
		);
	}
	IRQ_Restore(level);

	return 1;
}


GXRModeObj *rmode = NULL;
u32 *xfb;

u8 config_bytes[no_config_bytes] ATTRIBUTE_ALIGN(32);

void prepare() {
	VIDEO_Init();
	
	rmode = VIDEO_GetPreferredMode(NULL);
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	
	VIDEO_SetBlack(FALSE);

	VIDEO_Flush();

	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

    int x = 24, y = 32, w, h;
    w = rmode->fbWidth - (32);
    h = rmode->xfbHeight - (48);
	CON_InitEx(rmode, x, y, w, h);

    VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);
	
	printf("\x1b[2J");
	printf("WII AUTO LAUNCHER 2015- ligfx\n");
	printf("Based on WiiLauncher v0.2 (c) 2013, 2014 conanac\n");
	printf("Thanks to developers of: geckoos, wiiflow, tinyload, ftpii, wiixplorer,\n");
	printf("cleanrip, wiibrowser. And others whose codes are reused in this wiibrew\n");
	printf("\n\n");
	
	if (*((u32 *) 0x80001804) == 0x53545542 && 
		(
		*((u32 *) 0x80001808) == 0x48415858 ||
		*((u32 *) 0x80001808) == 0x4A4F4449 ||
		*((u32 *) 0x80001808) == 0xAF1BF516
		)
		)
		loaderhbc = 1;
	printf("launch_prepare: loaderhbc value = %d\n", loaderhbc);
	
	sd_refresh();
	printf("launch_prepare: sd_found value = %d\n", sd_found);
}

