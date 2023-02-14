#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "lib/dma.h"
#include "lib/memp.h"
#include "data.h"
#include "types.h"

#include "print.h"
#include "byteswap.h"

void texInit(void)
{
	/*extern u8 _textureslistSegmentRomStart;
	extern u8 _textureslistSegmentRomEnd;

	u32 len = ((&_textureslistSegmentRomEnd - &_textureslistSegmentRomStart) + 15) & -16;

	g_Textures = mempAlloc(len, MEMPOOL_PERMANENT);

	dmaExec(g_Textures, (romptr_t) &_textureslistSegmentRomStart, len);*/

	u32 _textureslistSegmentRomStart = 0x1ff7ca0;
	u32 _textureslistSegmentRomEnd = 0x1ffea20;

	u32 len = ((_textureslistSegmentRomEnd - _textureslistSegmentRomStart) + 15) & -16;

	/*
		struct texture {
			u32 soundsurfacetype : 4;
			u32 surfacetype : 4;
			u32 dataoffset : 24;
			u32 unk04_00 : 4;
			u32 unk04_04 : 4;
			u32 unk04_08 : 4;
			u32 unk04_0c : 4;
		};

		// Example of data loaded from ROM
		44 00 00 00 00 00 00 00 
		dd 00 01 98 00 00 00 00 
		00 00 01 a7 00 00 00 00 
		00 00 05 30 00 00 00 00

		98 01 00
		00 01 98
	*/

	g_Textures = mempAlloc(len, MEMPOOL_PERMANENT);

	dmaExec(g_Textures, (romptr_t)_textureslistSegmentRomStart, len);

	// Convert the data offset value from big endian to little endian
	u32 texturesCount = len / 8;

	debugPrint(PC_DBG_FLAG_TEX, "texInit(): loading %d textures from the textures list\n", texturesCount);

	// TODO: check the conversions here
	for (u32 i = 0; i < texturesCount; i++) 
	{
		u32 dataOffsetA = g_Textures[i].dataoffset;
		u32 dataOffsetB = dataOffsetA;

		u8* dataOffsetAPtr = (u8*)&dataOffsetA;
		u8* dataOffsetBPtr = (u8*)&dataOffsetB;

		dataOffsetAPtr[0] = dataOffsetBPtr[2];
		dataOffsetAPtr[1] = dataOffsetBPtr[1];
		dataOffsetAPtr[2] = dataOffsetBPtr[0];

		g_Textures[i].dataoffset = dataOffsetA;

		if (i > 0 && g_Textures[i].dataoffset == 0) {
			debugPrint(PC_DBG_FLAG_TEX, "tetInit(): ended dataoffset conversion at index %d\n", i);
			break;
		}

	}
}
