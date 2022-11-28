#ifndef _IN_BSS_H
#define _IN_BSS_H
#include <ultra64.h>
#include "constants.h"
#include "types.h"

extern OSThread g_MainThread;
extern OSMesgQueue g_SchedMesgQueue;
extern OSSched g_Sched;
extern OSViMode var8008dcc0[2];
extern OSViMode *var8008dd60[2];
extern s32 var8008de08;
extern s32 var8008de0c;
extern s32 var8008de10;
extern u8 g_SchedSpecialArtifactIndexes[3];
extern s32 g_SchedWriteArtifactsIndex;
extern OSPiHandle CartRomHandle;
extern OSPiHandle LeoDiskHandle;
extern OSTimer var80090ab0;
extern OSMesgQueue g_GbpakMesgQueue;
extern OSMesg var80090ae8;
extern u8 g_Is4Mb;
extern u32 g_VmNumTlbMisses;
extern u32 g_VmNumPageMisses;
extern u32 g_VmNumPageReplaces;
extern u8 *g_VmMarker;
extern u32 g_VmRamEnd;
extern u32 g_VmStateTableEnd;
extern AMAudioMgr g_AudioManager;
extern Mtx *var80092870;
extern s32 g_SndMaxFxBusses;
extern struct seqinstance g_SeqInstances[3];
extern ALHeap g_SndHeap;
extern u32 var80095200;
extern OSMesgQueue g_PiMesgQueue;
extern struct g_vars g_Vars;
extern u8 **var8009a874;
extern u8 **var8009a888;
extern union filedataptr g_TileFileData;
extern s32 g_TileNumRooms;
extern u32 *g_TileRooms;
extern struct geoblock g_CdSavedBlock;
extern u8 g_RdpDramStack[SP_DRAM_STACK_SIZE8];
extern N_ALSndPlayer var8009c2d0;
extern struct var8009c340 var8009c340;
extern u8 var8009c344[2];
extern u32 var8009c3e0;
extern struct asistream *g_AsiStream;
extern struct mp3decfourbytes *var8009c640;
extern f32 *var8009c644;
extern f32 *var8009c6d8;
extern f32 *var8009c6dc;
extern OSTimer var8009c760;
extern OSTime __osCurrentTime;
extern u32 __osBaseCounter;
extern u32 __osViIntrCount;
extern OSMesgQueue __osPiAccessQueue;
extern u8 __osContLastCmd;
extern u8 __osMaxControllers;
extern OSTimer __osEepromTimer;
extern OSMesgQueue __osEepromTimerQ;
extern OSMesg __osEepromTimerMsg;
extern u32 var8009ca84;
extern u16 *g_FrameBuffers[2];
extern u8 var8009caec;
extern u8 var8009caed;
extern u8 var8009caee;
extern u8 var8009caef;
extern u8 var8009caf0;
extern struct prop *g_DangerousProps[MAX_DANGEROUSPROPS];
extern u16 *g_WallhitCountsPerRoom;
extern s32 g_WallhitsMax;
extern u32 var8009cc48;
extern u32 g_WallhitsNumFree;
extern u32 g_WallhitsNumUsed;
extern u32 var8009cc54;
extern u32 var8009cc58;
extern s32 g_MinPropWallhits;
extern u32 g_MaxPropWallhits;
extern s32 g_MinBgWallhitsPerRoom;
extern s32 g_MaxBgWallhitsPerRoom;
extern s32 var8009cc70;
extern s32 var8009cc74;
extern f32 g_WallhitTargetBloodRatio;
extern Mtx *var8009cc80;
extern Mtx *var8009cc84;
extern Mtx *var8009cc88;
extern u8 *var8009cca0;
extern void *var8009ccc0[20];
extern s32 g_NumChrs;
extern s16 *g_Chrnums;
extern s16 *g_ChrIndexes;
extern s32 g_NumActiveHeadsPerGender;
extern u32 g_ActiveMaleHeads[8];
extern u32 g_ActiveFemaleHeads[8];
extern s16 *g_RoomPropListChunkIndexes;
extern struct roomproplistchunk *g_RoomPropListChunks;
extern struct prop *g_InteractProp;
extern s32 var8009cdac;
extern s32 var8009cdb0;
extern struct weaponobj *g_Proxies[30];
extern s32 g_MaxWeaponSlots;
extern s32 g_MaxHatSlots;
extern s32 g_MaxAmmoCrates;
extern s32 g_MaxDebrisSlots;
extern s32 g_MaxProjectiles;
extern s32 g_MaxEmbedments;
extern struct weaponobj *g_WeaponSlots;
extern struct hatobj *g_HatSlots;
extern struct ammocrateobj *g_AmmoCrates;
extern struct defaultobj *g_DebrisSlots;
extern struct projectile *g_Projectiles;
extern struct embedment *g_Embedments;
extern struct model *g_EmbedModel;
extern struct modelnode *g_EmbedNode;
extern struct coord var8009ce78;
extern struct coord var8009ce88;
extern struct tvscreen var8009ce98;
extern struct tvscreen var8009cf10;
extern struct tvscreen var8009cf88;
extern struct stagesetup g_StageSetup;
extern u8 *g_GeCreditsData;
extern struct objective *g_Objectives[MAX_OBJECTIVES];
extern u32 g_ObjectiveStatuses[MAX_OBJECTIVES];
extern struct tag *g_TagsLinkedList;
extern struct briefingobj *g_BriefingObjs;
extern struct criteria_roomentered *g_RoomEnteredCriterias;
extern struct criteria_throwinroom *g_ThrowInRoomCriterias;
extern struct criteria_holograph *g_HolographCriterias;
extern struct sndstate *g_CasingAudioHandles[2];
extern s32 var8009d0d8;
extern struct sndstate *g_BgunAudioHandles[4];
extern struct fireslot g_Fireslots[NUM_FIRESLOTS];
extern struct casing g_Casings[20];
extern struct boltbeam g_BoltBeams[8];
extern struct lasersight g_LaserSights[4];
extern struct model *g_ModelSlots;
extern struct anim *g_AnimSlots;
extern s32 g_ModelNumObjs;
extern s32 g_ModelNumChrs;
extern struct modelrwdatabinding *g_ModelRwdataBindings[3];
extern struct coord var8009dd20;
extern f32 var8009dd2c;
extern struct coord var8009dd30;
extern f32 var8009dd3c;
extern struct coord var8009dd40;
extern f32 var8009dd4c;
extern struct coord var8009dd50;
extern f32 var8009dd5c;
extern f32 var8009dd6c;
extern s32 g_DefaultWeapons[2];
extern s32 g_CutsceneCurAnimFrame60;

#if PAL
extern f32 var8009e388pf;
#endif

extern s16 g_CutsceneAnimNum;
extern s32 g_CutsceneFrameOverrun240;
extern bool g_CutsceneSkipRequested;
extern f32 g_CutsceneCurTotalFrame60f;
extern s16 g_SpawnPoints[MAX_SPAWNPOINTS];
extern s32 g_NumSpawnPoints;
extern s32 var8009de90;
extern s32 var8009de94;
extern s32 var8009de98;
extern s32 var8009de9c;
extern u8 g_MpSelectedPlayersForStats[4];
extern char g_CheatMarqueeString[];
extern u8 *g_BlurBuffer;
extern s32 var8009dfc0;
extern struct briefing g_Briefing;
extern struct missionconfig g_MissionConfig;
extern struct menu g_Menus[4];
extern struct menudata g_MenuData;
extern struct activemenu g_AmMenus[4];
extern s32 g_AmIndex;
extern u32 g_CheatsActiveBank0;
extern u32 g_CheatsActiveBank1;
extern u32 g_CheatsEnabledBank0;
extern u32 g_CheatsEnabledBank1;
extern s32 g_FilemgrLastPakError;
extern struct gamefile g_GameFile;
extern struct fileguid g_GameFileGuid;
extern u8 g_AltTitleUnlocked;
extern u8 g_AltTitleEnabled;
extern struct padsfileheader *g_PadsFile;
extern u16 *g_PadOffsets;
extern u16 *g_CoverFlags;
extern s32 *g_CoverRooms;
extern struct covercandidate *g_CoverCandidates;
extern u16 g_NumSpecialCovers;
extern u16 *g_SpecialCoverNums;
extern struct pak g_Paks[5];
extern OSPfs g_Pfses[4];
extern u32 g_SkyStageNum;
extern bool g_SkyLightningActive;
extern struct explosion *g_Explosions;
extern s32 g_MaxExplosions;
extern struct smoke *g_Smokes;
extern s32 g_MaxSmokes;
extern Mtx var800a3448;
extern Mtx var800a3488;
extern struct spark g_Sparks[100];
extern s32 g_NextSparkIndex;
extern struct sparkgroup g_SparkGroups[10];
extern s32 g_NextSparkGroupIndex;
extern u8 *var800a41a0;
extern struct wallhit *g_Wallhits;
extern struct wallhit *g_FreeWallhits;
extern struct wallhit *g_ActiveWallhits;
extern s32 g_MaxShards;
extern struct shard *g_Shards;
extern Gfx *var800a4634;
extern struct room *g_Rooms;
extern u8 *g_MpRoomVisibility;
extern struct bgroom *g_BgRooms;
extern struct bgportal *g_BgPortals;
extern struct var800a4ccc *var800a4ccc;
extern u8 *g_BgLightsFileData;
extern s16 *g_RoomPortals;
extern struct var800a4cf0 var800a4cf0;
extern f32 var800a6470[16 * 3];
extern struct var800a6538 var800a6538[16];
extern u32 g_BgRoomTestsDisabled;
extern struct screenbox var800a65c0;
extern struct screenbox g_PortalScreenBbox;
extern u32 g_BgCmdThrowing;
extern s32 g_BgNumLightsChecksum;
extern s32 g_BgLightsOffsetChecksum;
extern u32 g_FogEnabled;
extern u32 var800a65e4;
extern struct coord *var800a65e8;
extern u8 *g_RoomMtxAges;
extern s16 *g_RoomMtxLinkedRooms;
extern s16 *g_RoomMtxBaseRooms;
extern f32 *g_RoomMtxScales;
extern Mtxf *g_RoomMtxMatrices;
extern struct fileinfo g_FileInfo[NUM_FILES];
extern u8 *g_VtxBuffers[3];
extern u8 *g_GfxMemPos;
extern u8 g_GfxActiveBufferIndex;
extern s32 g_MusicStageNum;
extern struct musicevent g_MusicEventQueue[40];
extern struct var800aaa38 var800aaa38[3];
extern u32 g_AudioXReasonsActive[4];
extern s32 g_MusicXReasonMinDurations[4];
extern s32 g_MusicXReasonMaxDurations[4];
extern u32 *g_LangBanks[69];
extern struct jpncharpixels *g_JpnCharCachePixels;
extern struct jpncacheitem *g_JpnCacheCacheItems;
extern struct texture *g_Textures;
extern struct texpool g_TexSharedPool;
extern struct texcacheitem g_TexCacheItems[150];
extern s32 g_TexCacheCount;
extern u8 *var800ab540;
extern u32 var800ab544;
extern s32 var800ab548;
extern s32 g_TexNumConfigs;
extern struct tex **g_TexWords;
extern struct textureconfig *g_TexWallhitConfigs;
extern Gfx *g_TexGdl1;
extern Gfx *g_TexGdl2;
extern struct texturepair *g_ExplosionTexturePairs;
extern struct textureconfig *g_TexBeamConfigs;
extern struct textureconfig *g_TexLaserConfigs;
extern struct textureconfig *g_TexGroup03Configs;
extern struct textureconfig *g_TexGeCrosshairConfigs;
extern struct textureconfig *g_TexShadowConfigs;
extern struct textureconfig *g_TexShieldConfigs;
extern struct textureconfig *g_TexShardConfigs;
extern struct textureconfig *g_TexScreenConfigs;
extern struct textureconfig *g_TexWaterConfigs;
extern struct textureconfig *g_TexLightGlareConfigs;
extern struct textureconfig *g_TexSparkConfigs;
extern struct textureconfig *g_TexGeneralConfigs;
extern struct textureconfig *g_TexRadarConfigs;
extern struct scenariodata g_ScenarioData;
extern u32 var800ac4cc;
extern struct chrdata *g_MpAllChrPtrs[MAX_MPCHRS];
extern struct mpchrconfig *g_MpAllChrConfigPtrs[MAX_MPCHRS];
extern s32 g_MpNumChrs;
extern struct mpbotconfig g_BotConfigsArray[MAX_BOTS];
extern u8 g_MpSimulantDifficultiesPerNumPlayers[MAX_BOTS][4];
extern struct mpplayerconfig g_PlayerConfigsArray[6];
extern u8 g_AmBotCommands[16];
extern struct mpsetup g_MpSetup;
extern struct bossfile g_BossFile;
extern struct chrdata *g_MpBotChrPtrs[MAX_BOTS];
extern s32 g_JpnMaxCacheItems;
extern s32 var8009d370jf;

#endif
