#include <ultra64.h>
#include "constants.h"
#include "game/tex.h"
#include "game/texdecompress.h"
#include "bss.h"
#include "lib/rzip.h"
#include "lib/dma.h"
#include "lib/main.h"
#include "lib/memp.h"
#include "data.h"
#include "textureconfig.h"
#include "types.h"

#include "print.h"
#include "byteswap.h"

void texSetBitstring(u8 *arg0)
{
	var800ab540 = arg0;
	var800ab544 = 0;
	var800ab548 = 0;
}

s32 texReadBits(s32 arg0)
{
	while (var800ab548 < arg0) {
		var800ab544 = *var800ab540 | var800ab544 << 8;
		var800ab540++;
		var800ab548 += 8;
	}

	var800ab548 -= arg0;

	return var800ab544 >> var800ab548 & ((1 << arg0) - 1);
}

extern u8 *g_TextureConfigSegment;
extern u32 g_TexBase;
extern Gfx *g_TexGdl3;
extern struct textureconfig *g_TexRedLinesConfigs;
extern struct textureconfig *g_TexSkyConfigs;

u32 _textureconfigSegmentRomStart = 0x007eb270;
u32 _textureconfigSegmentStart = 0x007eb270;
u32 _textureconfigSegmentEnd = 0x007ebdc0;

void texReset(void)
{
	s32 stage;
	u32 len = _textureconfigSegmentEnd - _textureconfigSegmentStart;
	s32 i;

	debugPrint(PC_DBG_FLAG_TEX, "texReset()\n");

	g_TextureConfigSegment = mempAlloc(len, MEMPOOL_STAGE);
	dmaExec(g_TextureConfigSegment, (romptr_t)_textureconfigSegmentRomStart, len);

	//g_TexBase = (uintptr_t)g_TextureConfigSegment - ROM_SIZE * 1024 * 1024;
	//g_TexBase = (uintptr_t)g_TextureConfigSegment;
	
	// Note PC: do we really need to load from ROM? the structures are in textureconfig.c
	g_TexBase = 0;
	
	g_TexGdl1 = (Gfx *)(g_TexBase + (uintptr_t)g_TcGdl1);
	g_TexGdl2 = (Gfx *)(g_TexBase + (uintptr_t)g_TcGdl2);
	g_TexGdl3 = (Gfx *)(g_TexBase + (uintptr_t)g_TcGdl3);
	g_ExplosionTexturePairs = (void *)(g_TexBase + (uintptr_t)g_TcExplosionTexturePairs);
	g_TexWallhitConfigs = (struct textureconfig *)(g_TexBase + (uintptr_t)g_TcWallhitConfigs);
	g_TexBeamConfigs = (struct textureconfig *)(g_TexBase + (uintptr_t)g_TcBeamConfigs);
	g_TexLaserConfigs = (struct textureconfig *)(g_TexBase + (uintptr_t)g_TcLaserConfigs);
	g_TexGroup03Configs = (struct textureconfig *)(g_TexBase + (uintptr_t)g_TcGroup03Configs);
	g_TexGeCrosshairConfigs = (struct textureconfig *)(g_TexBase + (uintptr_t)g_TcGeCrosshairConfigs);
	g_TexRedLinesConfigs = (struct textureconfig *)(g_TexBase + (uintptr_t)g_TcRedLineConfigs);
	g_TexShadowConfigs = (struct textureconfig *)(g_TexBase + (uintptr_t)g_TcGroup06Configs);
	g_TexShieldConfigs = (struct textureconfig *)(g_TexBase + (uintptr_t)g_TcShieldConfigs);
	g_TexShardConfigs = (struct textureconfig *)(g_TexBase + (uintptr_t)g_TcShardConfigs);
	g_TexScreenConfigs = (struct textureconfig *)(g_TexBase + (uintptr_t)g_TcScreenConfigs);
	g_TexWaterConfigs = (struct textureconfig *)(g_TexBase + (uintptr_t)g_TcWaterConfigs);
	g_TexSkyConfigs = (struct textureconfig *)(g_TexBase + (uintptr_t)g_TcSkyConfigs);
	g_TexLightGlareConfigs = (struct textureconfig *)(g_TexBase + (uintptr_t)g_TcLightGlareConfigs);
	g_TexSparkConfigs = (struct textureconfig *)(g_TexBase + (uintptr_t)g_TcSparkConfigs);
	g_TexGeneralConfigs = (struct textureconfig *)(g_TexBase + (uintptr_t)g_TcGeneralConfigs);
	g_TexRadarConfigs = (struct textureconfig *)(g_TexBase + (uintptr_t)g_TcRadarConfigs);

	//g_TexNumConfigs = (len - (uintptr_t)&g_TcWallhitConfigs + ROM_SIZE * 1024 * 1024) / sizeof(struct textureconfig);
	g_TexNumConfigs = (len) / sizeof(struct textureconfig_load);
	g_TexWords = mempAlloc(ALIGN16(g_TexNumConfigs * 4), MEMPOOL_STAGE);

	debugPrint(PC_DBG_FLAG_TEX, "- g_TexNumConfigs: %x\n", g_TexNumConfigs);

	for (i = 0; i < g_TexNumConfigs; i++) {
		g_TexWords[i] = NULL;
	}

	for (i = 0; i < ARRAYCOUNT(g_TcExplosionTexturePairs); i++) {
		texLoad(&g_ExplosionTexturePairs[i].texturenum1, NULL, false, NULL);
		texLoad(&g_ExplosionTexturePairs[i].texturenum2, NULL, false, NULL);
	}

	texLoadFromDisplayList(g_TexGdl1, 0, 0);
	texLoadFromDisplayList(g_TexGdl3, 0, 0);

	stage = mainGetStageNum();

	if (IS4MB() && stage != STAGE_TITLE && stage != STAGE_CITRAINING && stage != STAGE_4MBMENU) {
		for (i = 0; i < ARRAYCOUNT(g_TcWallhitConfigs); i++) {
			texLoadFromConfig(&g_TexWallhitConfigs[i]);
		}

		for (i = 0; i < ARRAYCOUNT(g_TcBeamConfigs); i++) {
			texLoadFromConfig(&g_TexBeamConfigs[i]);
		}

		for (i = 0; i < ARRAYCOUNT(g_TcWaterConfigs); i++) {
			texLoadFromConfig(&g_TexWaterConfigs[i]);
		}

		for (i = 0; i < ARRAYCOUNT(g_TcGroup06Configs); i++) {
			texLoadFromConfig(&g_TexShadowConfigs[i]);
		}

		for (i = 0; i < ARRAYCOUNT(g_TcShieldConfigs); i++) {
			texLoadFromConfig(&g_TexShieldConfigs[i]);
		}

		for (i = 0; i < ARRAYCOUNT(g_TcRadarConfigs); i++) {
			texLoadFromConfig(&g_TexRadarConfigs[i]);
		}

		for (i = 0; i < ARRAYCOUNT(g_TcSparkConfigs); i++) {
			texLoadFromConfig(&g_TexSparkConfigs[i]);
		}
	}
}
