
#ifndef _LAUNCH_H_
#define _LAUNCH_H_

#include <gccore.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define no_config_bytes 16

extern u8 config_bytes[no_config_bytes] ATTRIBUTE_ALIGN(32);

extern GXRModeObj *rmode;
extern u32 *xfb;

#define WII_MAGIC 0x5D1C9EA3
#define NGC_MAGIC 0xC2339F3D

enum discTypes
{
    IS_NGC_DISC=0,
    IS_WII_DISC,
    IS_UNK_DISC
};

extern u8 geckoattached;
extern int wifigecko;
extern u8 loaderhbc;
extern u8 identifysu;
extern char gameidbuffer[8];
extern u8 vidMode;
extern int patchVidModes;
extern u8 vipatchselect;
extern u8 countryString;
extern int aspectRatio;
extern GXRModeObj *vmode;

void prepare();
s32 rundisc();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LAUNCH_H_ */
