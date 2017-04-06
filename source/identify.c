
#include <stdio.h>
#include <string.h>
#include <ogcsys.h>

#include "identify.h"
#include "launch.h"
#include "sd.h"

u8 channelios;
u8 menuios;
u16 bootindex;
u32 bootid;
u32 bootentrypoint;
u8 channelidentified = 0;
u8 menuidentified = 0;
bool hookpatched = false;


void apply_pokevalues() {
	
	u32 i, *codeaddr, *codeaddr2, *addrfound = NULL;
	
	if (gameconfsize != 0)
	{
		for (i = 0; i < (gameconfsize / 4); i++)
		{
			if (*(gameconf + i) == 0)
			{
				if (((u32 *) (*(gameconf + i + 1))) == NULL ||
					*((u32 *) (*(gameconf + i + 1))) == *(gameconf + i + 2))
				{
					*((u32 *) (*(gameconf + i + 3))) = *(gameconf + i + 4);
					DCFlushRange((void *) *(gameconf + i + 3), 4);
					if (((u32 *) (*(gameconf + i + 1))) == NULL)
						printf("identify_apply_pokevalues: poke ");
					else
						printf("identify_apply_pokevalues: pokeifequal ");
					printf("0x%08X = %08X\n",
						(u32)(*(gameconf + i + 3)), (u32)(*(gameconf + i + 4)));
				}
				i += 4;
			}
			else
			{
				codeaddr = (u32 *)(*(gameconf + i + *(gameconf + i) + 1));
				codeaddr2 = (u32 *)(*(gameconf + i + *(gameconf + i) + 2));
				if (codeaddr == 0 && addrfound != NULL)
					codeaddr = addrfound;
				else if (codeaddr == 0 && codeaddr2 != 0)
					codeaddr = (u32 *) ((((u32) codeaddr2) >> 28) << 28);
				else if (codeaddr == 0 && codeaddr2 == 0)
				{
					i += *(gameconf + i) + 4;
					continue;
				}
				if (codeaddr2 == 0)
					codeaddr2 = codeaddr + *(gameconf + i);
				addrfound = NULL;
				while (codeaddr <= (codeaddr2 - *(gameconf + i)))
				{
					if (memcmp(codeaddr, gameconf + i + 1, (*(gameconf + i)) * 4) == 0)
					{
						*(codeaddr + ((*(gameconf + i + *(gameconf + i) + 3)) / 4)) = *(gameconf + i + *(gameconf + i) + 4);
						if (addrfound == NULL) addrfound = codeaddr;
						DCFlushRange((void *) (codeaddr + ((*(gameconf + i + *(gameconf + i) + 3)) / 4)), 4);
						printf("identify_apply_pokevalues: searchandpoke 0x%08X = %08X\n",
							(u32)((codeaddr + ((*(gameconf + i + *(gameconf + i) + 3)) / 4))), 
							(u32)(*(gameconf + i + *(gameconf + i) + 4)));
					}
					codeaddr++;
				}
				i += *(gameconf + i) + 4;
			}
		}
	}
}

u32 be32(const u8 *p) {
	return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

void apply_patch() {

	int i;

	u8 *filebuff = (u8*)sdbuffer;
	u8 no_patches;
	u32 patch_dest;
	u32 patch_size;

	no_patches = filebuff[0];

	filebuff += 1;

	for(i=0;i<no_patches;i++)
	{
		patch_dest = be32(filebuff);
		patch_size = be32(filebuff+4);

		memcpy((u8*)patch_dest, filebuff+8, patch_size);
		DCFlushRange((u8*)patch_dest, patch_size);
		printf("identify_apply_patch: i = %d, patch_dest = %08X, patch_size = %08X\n",
			i, patch_dest, patch_size);
		filebuff = filebuff + patch_size + 8;
	}
}


