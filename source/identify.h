
#ifndef __IDENTIFY_H__
#define __IDENTIFY_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern u8 channelios;
extern u8 menuios;
extern u16 bootindex;
extern u32 bootid;
extern u32 bootentrypoint;
extern bool hookpatched;

void apply_pokevalues();
void apply_patch();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __Identify_H__

