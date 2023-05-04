#include <ultra64.h>
#include "constants.h"
#include "game/cheats.h"
#include "game/game_00b820.h"
#include "game/setup.h"
#include "game/objectives.h"
#include "game/playerreset.h"
#include "game/botmgr.h"
#include "game/chr.h"
#include "game/chrmgr.h"
#include "game/body.h"
#include "game/prop.h"
#include "game/setuputils.h"
#include "game/atan2f.h"
#include "game/inv.h"
#include "game/playermgr.h"
#include "game/bg.h"
#include "game/stagetable.h"
#include "game/file.h"
#include "game/lv.h"
#include "game/mplayer/scenarios.h"
#include "game/challenge.h"
#include "game/lang.h"
#include "game/mplayer/mplayer.h"
#include "game/pad.h"
#include "game/propobj.h"
#include "game/chrai.h"
#include "bss.h"
#include "lib/args.h"
#include "lib/memp.h"
#include "lib/model.h"
#include "lib/path.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "lib/ailist.h"
#include "lib/anim.h"
#include "lib/collision.h"
#include "data.h"
#include "types.h"

#include "print.h"
#include "byteswap.h"
#include "gfx/hashmap.h"
#include "offsetsmap.h"
#include "native_functions.h"
#include <string.h> 

s32 g_SetupCurMpLocation;
u32 var8009cc34;
u32 var8009cc38;
u32 var8009cc3c;

struct tvscreen var80061a80 = {
	g_TvCmdlist00, // cmdlist
	0,           // offset
	0xffff,      // pause60
	0,           // tconfig
	0,           // rot
	1,           // xscale
	0,           // xscalefrac
	0,           // xscaleinc
	1,           // xscaleold
	1,           // xscalenew
	1,           // yscale
	0,           // yscalefrac
	0,           // yscaleinc
	1,           // yscaleold
	1,           // yscalenew
	0.5,         // xmid
	0,           // xmidfrac
	0,           // xmidinc
	0.5,         // xmidold
	0.5,         // xmidnew
	0.5,         // ymid
	0,           // ymidfrac
	0,           // ymidinc
	0.5,         // ymidold
	0.5,         // ymidnew
	0xff,        // red
	0xff,        // redold
	0xff,        // rednew
	0xff,        // green
	0xff,        // greenold
	0xff,        // greennew
	0xff,        // blue
	0xff,        // blueold
	0xff,        // bluenew
	0xff,        // alpha
	0xff,        // alphaold
	0xff,        // alphanew
	1,           // colfrac
	0,           // colinc
};

struct tvscreen var80061af4 = {
	var8006aaa0, // cmdlist
	0,           // offset
	0xffff,      // pause60
	0,           // tconfig
	0,           // rot
	1,           // xscale
	0,           // xscalefrac
	0,           // xscaleinc
	1,           // xscaleold
	1,           // xscalenew
	1,           // yscale
	0,           // yscalefrac
	0,           // yscaleinc
	1,           // yscaleold
	1,           // yscalenew
	0.5,         // xmid
	0,           // xmidfrac
	0,           // xmidinc
	0.5,         // xmidold
	0.5,         // xmidnew
	0.5,         // ymid
	0,           // ymidfrac
	0,           // ymidinc
	0.5,         // ymidold
	0.5,         // ymidnew
	0xff,        // red
	0xff,        // redold
	0xff,        // rednew
	0xff,        // green
	0xff,        // greenold
	0xff,        // greennew
	0xff,        // blue
	0xff,        // blueold
	0xff,        // bluenew
	0xff,        // alpha
	0xff,        // alphaold
	0xff,        // alphanew
	1,           // colfrac
	0,           // colinc
};

struct tvscreen var80061b68 = {
	var8006aae4, // cmdlist
	0,           // offset
	0xffff,      // pause60
	0,           // tconfig
	0,           // rot
	1,           // xscale
	0,           // xscalefrac
	0,           // xscaleinc
	1,           // xscaleold
	1,           // xscalenew
	1,           // yscale
	0,           // yscalefrac
	0,           // yscaleinc
	1,           // yscaleold
	1,           // yscalenew
	0.5,         // xmid
	0,           // xmidfrac
	0,           // xmidinc
	0.5,         // xmidold
	0.5,         // xmidnew
	0.5,         // ymid
	0,           // ymidfrac
	0,           // ymidinc
	0.5,         // ymidold
	0.5,         // ymidnew
	0xff,        // red
	0xff,        // redold
	0xff,        // rednew
	0xff,        // green
	0xff,        // greenold
	0xff,        // greennew
	0xff,        // blue
	0xff,        // blueold
	0xff,        // bluenew
	0xff,        // alpha
	0xff,        // alphaold
	0xff,        // alphanew
	1,           // colfrac
	0,           // colinc
};

u32 var80061bdc = 0x00000000;
f32 g_DoorScale = 1;
u32 var80061be4 = 0x00000000;
u32 var80061be8 = 0x00000000;
u32 var80061bec = 0x00000000;

void propsReset(void)
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_Lifts); i++) {
		g_Lifts[i] = NULL;
	}

	g_MaxWeaponSlots = 50;
	g_MaxHatSlots = 10;
	g_MaxAmmoCrates = 20;
	g_MaxDebrisSlots = 15;
	g_MaxProjectiles = IS4MB() ? 20 : 100;
	g_MaxEmbedments = IS4MB() ? 40 : 80;

	if (g_Vars.stagenum >= STAGE_TITLE) {
		g_MaxWeaponSlots = 0;
		g_MaxHatSlots = 0;
		g_MaxAmmoCrates = 0;
		g_MaxDebrisSlots = 0;
		g_MaxProjectiles = 0;
		g_MaxEmbedments = 0;
	}

	setupReset0f00cc8c();
	setupResetProxyMines();

	g_AlarmTimer = 0;
	g_AlarmAudioHandle = NULL;
	g_AlarmSpeakerWeight = 64;

	g_GasReleaseTimer240 = 0;
	g_GasReleasing = false;
	g_GasPos.x = 0;
	g_GasPos.y = 0;
	g_GasPos.z = 0;
	g_GasLastCough60 = 0;
	g_GasSoundTimer240 = 0;
	g_GasAudioHandle = NULL;

	g_CountdownTimerOff = COUNTDOWNTIMERREASON_AI;
	g_CountdownTimerRunning = false;
	g_CountdownTimerValue60 = 0;

	g_PlayersDetonatingMines = 0;
	g_TintedGlassEnabled = false;

	if (g_MaxWeaponSlots == 0) {
		g_WeaponSlots = NULL;
	} else {
		g_WeaponSlots = mempAlloc(ALIGN16(g_MaxWeaponSlots * sizeof(struct weaponobj)), MEMPOOL_STAGE);

		for (i = 0; i < g_MaxWeaponSlots; i++) {
			g_WeaponSlots[i].base.prop = NULL;
		}

		g_NextWeaponSlot = 0;
	}

	if (g_MaxHatSlots == 0) {
		g_HatSlots = NULL;
	} else {
		g_HatSlots = mempAlloc(ALIGN16(g_MaxHatSlots * sizeof(struct hatobj)), MEMPOOL_STAGE);

		for (i = 0; i < g_MaxHatSlots; i++) {
			g_HatSlots[i].base.prop = NULL;
		}

		g_NextHatSlot = 0;
	}

	if (g_MaxAmmoCrates == 0) {
		g_AmmoCrates = NULL;
	} else {
		g_AmmoCrates = mempAlloc(ALIGN16(g_MaxAmmoCrates * sizeof(struct ammocrateobj)), MEMPOOL_STAGE);

		for (i = 0; i < g_MaxAmmoCrates; i++) {
			g_AmmoCrates[i].base.prop = NULL;
		}
	}

	if (g_MaxDebrisSlots == 0) {
		g_DebrisSlots = NULL;
	} else {
		g_DebrisSlots = mempAlloc(ALIGN16(g_MaxDebrisSlots * sizeof(struct defaultobj)), MEMPOOL_STAGE);

		for (i = 0; i < g_MaxDebrisSlots; i++) {
			g_DebrisSlots[i].prop = NULL;
		}
	}

	if (g_MaxProjectiles == 0) {
		g_Projectiles = NULL;
	} else {
		g_Projectiles = mempAlloc(ALIGN16(g_MaxProjectiles * sizeof(struct projectile)), MEMPOOL_STAGE);

		for (i = 0; i < g_MaxProjectiles; i++) {
			g_Projectiles[i].flags = PROJECTILEFLAG_FREE;
		}
	}

	if (g_MaxEmbedments == 0) {
		g_Embedments = NULL;
	} else {
		g_Embedments = mempAlloc(ALIGN16(g_MaxEmbedments * sizeof(struct embedment)), MEMPOOL_STAGE);

		for (i = 0; i < g_MaxEmbedments; i++) {
			g_Embedments[i].flags = EMBEDMENTFLAG_FREE;
		}
	}

	g_LiftDoors = NULL;
	g_PadlockedDoors = NULL;
	g_SafeItems = NULL;
	g_LinkedScenery = NULL;
	g_BlockedPaths = NULL;

	g_EmbedProp = NULL;
	g_EmbedHitPart = -1;
	g_CctvWaitScale = 1;
	g_CctvDamageRxScale = 1;
	g_AutogunAccuracyScale = 1;
	g_AutogunDamageTxScale = 1;
	g_AutogunDamageRxScale = 1;
	g_AmmoQuantityScale = 1;

	g_MaxThrownLaptops = g_Vars.normmplayerisrunning ? 12 : PLAYERCOUNT();

	g_ThrownLaptops = mempAlloc(ALIGN16(g_MaxThrownLaptops * sizeof(struct autogunobj)), MEMPOOL_STAGE);
	g_ThrownLaptopBeams = mempAlloc(ALIGN16(g_MaxThrownLaptops * sizeof(struct beam)), MEMPOOL_STAGE);

	for (i = 0; i < g_MaxThrownLaptops; i++) {
		g_ThrownLaptops[i].base.prop = NULL;
	}
}

void setupCreateLiftDoor(struct linkliftdoorobj *link)
{
	link->next = g_LiftDoors;
	g_LiftDoors = link;
}

void setupCreatePadlockedDoor(struct padlockeddoorobj *link)
{
	link->next = g_PadlockedDoors;
	g_PadlockedDoors = link;
}

void setupCreateSafeItem(struct safeitemobj *link)
{
	link->next = g_SafeItems;
	g_SafeItems = link;
}

void setupCreateConditionalScenery(struct linksceneryobj *link)
{
	link->next = g_LinkedScenery;
	g_LinkedScenery = link;
}

void setupCreateBlockedPath(struct blockedpathobj *blockedpath)
{
	blockedpath->next = g_BlockedPaths;
	g_BlockedPaths = blockedpath;
}

void setupReset0f00cc8c(void)
{
	struct tvscreen tmp1;
	struct tvscreen tmp2;
	struct tvscreen tmp3;

	tmp1 = var80061a80;
	var8009ce98 = tmp1;

	tmp2 = var80061af4;
	var8009cf10 = tmp2;

	tmp3 = var80061b68;
	var8009cf88 = tmp3;
}

void setupResetProxyMines(void)
{
	s32 i;

	for (i = 0; i < ARRAYCOUNT(g_Proxies); i++) {
		g_Proxies[i] = NULL;
	}
}

s32 setupCountCommandType(u32 type)
{
	struct defaultobj *obj = (struct defaultobj *)g_StageSetup.props;
	s32 count = 0;

	if (obj) {
		while (obj->type != OBJTYPE_END) {
			if (obj->type == (u8)type) {
				count++;
			}

			obj = (struct defaultobj *)((u32 *)obj + setupGetCmdLength((u32 *)obj));
		}
	}

	return count;
}

void setupCreateObject(struct defaultobj *obj, s32 cmdindex)
{
	f32 f0;
	s32 modelnum;
	struct pad pad;
	Mtxf mtx;
	struct coord centre;
	f32 scale;
	struct coord pos;
	s16 rooms[8];
	struct prop *prop2;
	u32 flag40;
	u32 stack;
	struct chrdata *chr;
	struct prop *prop;

	modelnum = obj->modelnum;
	setupLoadModeldef(modelnum);
	scale = obj->extrascale * (1.0f / 256.0f);

	if (g_Vars.normmplayerisrunning || g_Vars.lvmpbotlevel) {
		obj->hidden2 |= OBJH2FLAG_CANREGEN;
	}

	if (obj->flags & OBJFLAG_INSIDEANOTHEROBJ) {
		if (obj->type == OBJTYPE_WEAPON) {
			func0f08ae0c((struct weaponobj *)obj, g_ModelStates[modelnum].modeldef);
		} else {
			objInitWithModelDef(obj, g_ModelStates[modelnum].modeldef);
		}

		modelSetScale(obj->model, obj->model->scale * scale);
		return;
	}

	if (obj->flags & OBJFLAG_ASSIGNEDTOCHR) {
		chr = chrFindByLiteralId(obj->pad);

		if (chr && chr->prop && chr->model) {
			if (obj->type == OBJTYPE_WEAPON) {
				prop = func0f08ae0c((struct weaponobj *)obj, g_ModelStates[modelnum].modeldef);
			} else {
				prop = objInitWithModelDef(obj, g_ModelStates[modelnum].modeldef);
			}

			modelSetScale(obj->model, obj->model->scale * scale);
			propReparent(prop, chr->prop);
		}
	} else {
		if (obj->pad < 0) {
			if (obj->type == OBJTYPE_WEAPON) {
				func0f08ae0c((struct weaponobj *)obj, g_ModelStates[modelnum].modeldef);
			} else {
				objInitWithModelDef(obj, g_ModelStates[modelnum].modeldef);
			}

			modelSetScale(obj->model, obj->model->scale * scale);
			return;
		}

		padUnpack(obj->pad, PADFIELD_POS | PADFIELD_LOOK | PADFIELD_UP | PADFIELD_BBOX | PADFIELD_ROOM, &pad);

		if (pad.room > 0) {
			mtx00016d58(&mtx, 0, 0, 0, -pad.look.x, -pad.look.y, -pad.look.z, pad.up.x, pad.up.y, pad.up.z);

			pos.x = pad.pos.x;
			pos.y = pad.pos.y;
			pos.z = pad.pos.z;

			rooms[0] = pad.room;
			rooms[1] = -1;

			if (!padHasBboxData(obj->pad)) {
				if (obj->flags & OBJFLAG_00000002) {
					centre.x = pad.pos.x;
					centre.y = pad.pos.y;
					centre.z = pad.pos.z;
				} else {
					centre.x = pad.pos.x;
					centre.y = pad.pos.y;
					centre.z = pad.pos.z;
				}
			} else {
				padGetCentre(obj->pad, &centre);
				centre.x += (pad.bbox.ymin - pad.bbox.ymax) * 0.5f * pad.up.x;
				centre.y += (pad.bbox.ymin - pad.bbox.ymax) * 0.5f * pad.up.y;
				centre.z += (pad.bbox.ymin - pad.bbox.ymax) * 0.5f * pad.up.z;
			}

			if (obj->type == OBJTYPE_WEAPON) {
				prop2 = func0f08ae0c((struct weaponobj *)obj, g_ModelStates[modelnum].modeldef);
			} else {
				prop2 = objInitWithAutoModel(obj);
			}

			if (padHasBboxData(obj->pad)) {
				struct modelrodata_bbox *bbox = objFindBboxRodata(obj);

				if (bbox != NULL) {
					f32 xscale = 1.0f;
					f32 yscale = 1.0f;
					f32 zscale = 1.0f;
					f32 minscale;
					f32 maxscale;

					flag40 = OBJFLAG_YTOPADBOUNDS;

					if (obj->flags & OBJFLAG_XTOPADBOUNDS) {
						if (bbox->xmin < bbox->xmax) {
							if (obj->flags & OBJFLAG_00000002) {
								xscale = (pad.bbox.xmax - pad.bbox.xmin) / ((bbox->xmax - bbox->xmin) * obj->model->scale);
							} else {
								xscale = (pad.bbox.xmax - pad.bbox.xmin) / ((bbox->xmax - bbox->xmin) * obj->model->scale);
							}
						}
					}

					if (obj->flags & flag40) {
						if (bbox->ymin < bbox->ymax) {
							if (obj->flags & OBJFLAG_00000002) {
								zscale = (pad.bbox.zmax - pad.bbox.zmin) / ((bbox->ymax - bbox->ymin) * obj->model->scale);
							} else {
								yscale = (pad.bbox.ymax - pad.bbox.ymin) / ((bbox->ymax - bbox->ymin) * obj->model->scale);
							}
						}
					}

					if (obj->flags & OBJFLAG_ZTOPADBOUNDS) {
						if (bbox->zmin < bbox->zmax) {
							if (obj->flags & OBJFLAG_00000002) {
								yscale = (pad.bbox.ymax - pad.bbox.ymin) / ((bbox->zmax - bbox->zmin) * obj->model->scale);
							} else {
								zscale = (pad.bbox.zmax - pad.bbox.zmin) / ((bbox->zmax - bbox->zmin) * obj->model->scale);
							}
						}
					}

					minscale = xscale;

					if (yscale < minscale) {
						minscale = yscale;
					}

					if (zscale < minscale) {
						minscale = zscale;
					}

					maxscale = xscale;

					if (yscale > maxscale) {
						maxscale = yscale;
					}

					if (zscale > maxscale) {
						maxscale = zscale;
					}

					if ((obj->flags & OBJFLAG_XTOPADBOUNDS) == 0) {
						if (obj->flags & OBJFLAG_00000002) {
							if (bbox->xmax == bbox->xmin) {
								xscale = maxscale;
							}
						} else if (bbox->xmax == bbox->xmin) {
							xscale = maxscale;
						}
					}

					if ((u32)(obj->flags & flag40) == 0) {
						if (obj->flags & OBJFLAG_00000002) {
							if (bbox->ymax == bbox->ymin) {
								zscale = maxscale;
							}
						} else if (bbox->ymax == bbox->ymin) {
							yscale = maxscale;
						}
					}

					if ((obj->flags & OBJFLAG_ZTOPADBOUNDS) == 0) {
						if (obj->flags & OBJFLAG_00000002) {
							if (bbox->zmax == bbox->zmin) {
								yscale = maxscale;
							}
						} else if (bbox->zmax == bbox->zmin) {
							zscale = maxscale;
						}
					}

					xscale /= maxscale;
					yscale /= maxscale;
					zscale /= maxscale;

					if (xscale <= 0.000001f || yscale <= 0.000001f || zscale <= 0.000001f) {
						xscale = yscale = zscale = 1;
					}

					mtx00015e24(xscale, &mtx);
					mtx00015e80(yscale, &mtx);
					mtx00015edc(zscale, &mtx);

					modelSetScale(obj->model, obj->model->scale * maxscale);
				}
			}

			modelSetScale(obj->model, obj->model->scale * scale);
			mtx00015f04(obj->model->scale, &mtx);

			if (obj->flags2 & OBJFLAG2_DONTPAUSE) {
				prop2->flags |= PROPFLAG_DONTPAUSE;
			}

			if (obj->flags & OBJFLAG_00000002) {
				func0f06ab60(obj, &pos, &mtx, rooms, &centre);
			} else {
				func0f06a730(obj, &pos, &mtx, rooms, &centre);
			}

			if (obj->hidden & OBJHFLAG_00008000) {
				propActivateThisFrame(prop2);
			} else {
				propActivate(prop2);
			}

			propEnable(prop2);
		}
	}
}

/**
 * Assigns a weapon to its home.
 *
 * Its home is a chr's hand or a pad, as defined in the stage's setup file.
 *
 * The Marquis of Queensbury Rules (everyone unarmed) and Enemy Rockets cheats
 * are implemented here.
 */
void setupPlaceWeapon(struct weaponobj *weapon, s32 cmdindex)
{
	if (weapon->base.flags & OBJFLAG_ASSIGNEDTOCHR) {
		u32 stack[2];
		struct chrdata *chr = chrFindByLiteralId(weapon->base.pad);

		if (chr && chr->prop && chr->model) {
			if (cheatIsActive(CHEAT_MARQUIS)) {
				// NTSC 1.0 and newer simplifies the Marquis logic
#if VERSION >= VERSION_NTSC_1_0
				weapon->base.flags &= ~OBJFLAG_DEACTIVATED;
				weapon->base.flags |= OBJFLAG_WEAPON_AICANNOTUSE;
				modelmgrLoadProjectileModeldefs(weapon->weaponnum);
				func0f08b25c(weapon, chr);
#else
				if (g_Vars.stagenum == STAGE_INVESTIGATION
						&& lvGetDifficulty() == DIFF_PA
						&& weapon->weaponnum == WEAPON_K7AVENGER) {
					modelmgrLoadProjectileModeldefs(weapon->weaponnum);
					func0f08b25c(weapon, chr);
				} else if (g_Vars.stagenum == STAGE_ATTACKSHIP) {
					weapon->base.flags &= ~OBJFLAG_DEACTIVATED;
					weapon->base.flags |= OBJFLAG_WEAPON_AICANNOTUSE;
					modelmgrLoadProjectileModeldefs(weapon->weaponnum);
					func0f08b25c(weapon, chr);
				} else {
					weapon->weaponnum = WEAPON_NONE;
				}
#endif
			} else {
				if (cheatIsActive(CHEAT_ENEMYROCKETS)) {
					switch (weapon->weaponnum) {
					case WEAPON_FALCON2:
					case WEAPON_FALCON2_SILENCER:
					case WEAPON_FALCON2_SCOPE:
					case WEAPON_MAGSEC4:
					case WEAPON_MAULER:
					case WEAPON_PHOENIX:
					case WEAPON_DY357MAGNUM:
					case WEAPON_DY357LX:
					case WEAPON_CMP150:
					case WEAPON_CYCLONE:
					case WEAPON_CALLISTO:
					case WEAPON_RCP120:
					case WEAPON_LAPTOPGUN:
					case WEAPON_DRAGON:
					case WEAPON_AR34:
					case WEAPON_SUPERDRAGON:
					case WEAPON_SHOTGUN:
					case WEAPON_REAPER:
					case WEAPON_SNIPERRIFLE:
					case WEAPON_FARSIGHT:
					case WEAPON_DEVASTATOR:
					case WEAPON_ROCKETLAUNCHER:
					case WEAPON_SLAYER:
					case WEAPON_COMBATKNIFE:
					case WEAPON_CROSSBOW:
					case WEAPON_TRANQUILIZER:
					case WEAPON_GRENADE:
					case WEAPON_NBOMB:
					case WEAPON_TIMEDMINE:
					case WEAPON_PROXIMITYMINE:
					case WEAPON_REMOTEMINE:
						weapon->weaponnum = WEAPON_ROCKETLAUNCHER;
						weapon->base.modelnum = MODEL_CHRDYROCKET;
						weapon->base.extrascale = 256;
						break;
					case WEAPON_K7AVENGER:
						// Don't replace the K7 guard's weapon in Investigation
						// because it would make an objective impossible.
						// @bug: It's still replaced on PD mode difficulty.
						if (g_Vars.stagenum != STAGE_INVESTIGATION || lvGetDifficulty() != DIFF_PA) {
							weapon->weaponnum = WEAPON_ROCKETLAUNCHER;
							weapon->base.modelnum = MODEL_CHRDYROCKET;
							weapon->base.extrascale = 256;
						}
						break;
					}
				}

				modelmgrLoadProjectileModeldefs(weapon->weaponnum);
				func0f08b25c(weapon, chr);
			}
		}
	} else {
		bool createweapon = true;

		if (g_Vars.normmplayerisrunning || g_Vars.lvmpbotlevel) {
			struct mpweapon *mpweapon;
			s32 locationindex;

			g_SetupCurMpLocation = -1;

			switch (weapon->weaponnum) {
			case WEAPON_MPLOCATION00:
			case WEAPON_MPLOCATION01:
			case WEAPON_MPLOCATION02:
			case WEAPON_MPLOCATION03:
			case WEAPON_MPLOCATION04:
			case WEAPON_MPLOCATION05:
			case WEAPON_MPLOCATION06:
			case WEAPON_MPLOCATION07:
			case WEAPON_MPLOCATION08:
			case WEAPON_MPLOCATION09:
			case WEAPON_MPLOCATION10:
			case WEAPON_MPLOCATION11:
			case WEAPON_MPLOCATION12:
			case WEAPON_MPLOCATION13:
			case WEAPON_MPLOCATION14:
			case WEAPON_MPLOCATION15:
				locationindex = weapon->weaponnum - WEAPON_MPLOCATION00;
				mpweapon = mpGetMpWeaponByLocation(locationindex);
				g_SetupCurMpLocation = locationindex;
				weapon->weaponnum = mpweapon->weaponnum;
				weapon->base.modelnum = mpweapon->model;
				weapon->base.extrascale = mpweapon->extrascale;
				createweapon = mpweapon->hasweapon;

				if (mpweapon->weaponnum == WEAPON_MPSHIELD) {
					struct shieldobj *shield = (struct shieldobj *)weapon;
					shield->base.modelnum = MODEL_CHRSHIELD;
					shield->base.type = OBJTYPE_SHIELD;
					shield->base.flags |= OBJFLAG_01000000 | OBJFLAG_INVINCIBLE;
					shield->base.flags2 |= OBJFLAG2_IMMUNETOEXPLOSIONS | OBJFLAG2_IMMUNETOGUNFIRE;
					shield->initialamount = 1;
					shield->amount = 1;
					setupCreateObject(&shield->base, cmdindex);
					createweapon = false;
				}
				break;
			}
		}

		if (weapon->weaponnum != WEAPON_NONE && createweapon) {
			modelmgrLoadProjectileModeldefs(weapon->weaponnum);
			setupCreateObject(&weapon->base, cmdindex);
		}
	}
}

void setupCreateHat(struct hatobj *hat, s32 cmdindex)
{
	if (hat->base.flags & OBJFLAG_ASSIGNEDTOCHR) {
		struct chrdata *chr = chrFindByLiteralId(hat->base.pad);

		if (chr && chr->prop && chr->model) {
			hatAssignToChr(hat, chr);
		}
	} else {
		setupCreateObject(&hat->base, cmdindex);
	}
}

void setupCreateKey(struct keyobj *key, s32 cmdindex)
{
	setupCreateObject(&key->base, cmdindex);
}

void setupCreateMine(struct mineobj *mine, s32 cmdindex)
{
	mine->base.type = OBJTYPE_WEAPON;

	setupCreateObject(&mine->base, cmdindex);

	if (g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0) {
		mine->base.hidden = (mine->base.hidden & 0x0fffffff) | (2 << 28);
	}

	mine->base.prop->forcetick = true;
}

void setupCreateCctv(struct cctvobj *cctv, s32 cmdindex)
{
	struct defaultobj *obj = &cctv->base;

	setupCreateObject(obj, cmdindex);

	if (cctv->lookatpadnum >= 0) {
		struct coord lenspos;
		union modelrodata *lens = modelGetPartRodata(obj->model->definition, MODELPART_CCTV_CASING);
		struct pad pad;
		f32 xdiff;
		f32 ydiff;
		f32 zdiff;

		padUnpack(cctv->lookatpadnum, PADFIELD_POS, &pad);

		lenspos.x = lens->position.pos.x;
		lenspos.y = lens->position.pos.y;
		lenspos.z = lens->position.pos.z;

		mtx00016208(obj->realrot, &lenspos);

		lenspos.x += obj->prop->pos.x;
		lenspos.y += obj->prop->pos.y;
		lenspos.z += obj->prop->pos.z;

		xdiff = lenspos.x - pad.pos.x;
		ydiff = lenspos.y - pad.pos.y;
		zdiff = lenspos.z - pad.pos.z;

		if (ydiff) {
			// empty
		}

		mtx00016d58(&cctv->camrotm, 0.0f, 0.0f, 0.0f, xdiff, ydiff, zdiff, 0.0f, 1.0f, 0.0f);
		mtx00015f04(obj->model->scale, &cctv->camrotm);

		cctv->toleft = 0;
		cctv->yleft = *(s32 *)&cctv->yleft * M_BADTAU / 65536.0f;
		cctv->yright = *(s32 *)&cctv->yright * M_BADTAU / 65536.0f;
		cctv->yspeed = 0.0f;
		cctv->ymaxspeed = *(s32 *)&cctv->ymaxspeed * M_BADTAU / 65536.0f;
		cctv->maxdist = *(s32 *)&cctv->maxdist;
		cctv->yrot = cctv->yleft;

		cctv->yzero = atan2f(xdiff, zdiff);
		cctv->xzero = M_BADTAU - atan2f(ydiff, sqrtf(xdiff * xdiff + zdiff * zdiff));

		if (xdiff || zdiff) {
			// empty
		}

		cctv->seebondtime60 = 0;
	}
}

void setupCreateAutogun(struct autogunobj *autogun, s32 cmdindex)
{
	setupCreateObject(&autogun->base, cmdindex);

	autogun->maxspeed = *(s32 *)&autogun->maxspeed * PALUPF(M_BADTAU) / 65536.0f;
	autogun->aimdist = *(s32 *)&autogun->aimdist * 100.0f / 65536.0f;
	autogun->ymaxleft = *(s32 *)&autogun->ymaxleft * M_BADTAU / 65536.0f;
	autogun->ymaxright = *(s32 *)&autogun->ymaxright * M_BADTAU / 65536.0f;

	autogun->firecount = 0;
	autogun->lastseebond60 = -1;
	autogun->lastaimbond60 = -1;
	autogun->allowsoundframe = -1;
	autogun->yrot = 0;
	autogun->yspeed = 0;
	autogun->yzero = 0;
	autogun->xrot = 0;
	autogun->xspeed = 0;
	autogun->xzero = 0;
	autogun->barrelspeed = 0;
	autogun->barrelrot = 0;
	autogun->beam = mempAlloc(ALIGN16(sizeof(struct beam)), MEMPOOL_STAGE);
	autogun->beam->age = -1;
	autogun->firing = false;
	autogun->ammoquantity = 255;
	autogun->shotbondsum = 0;

	if (autogun->targetpad >= 0) {
		u32 stack1;
		f32 xdiff;
		f32 ydiff;
		f32 zdiff;
		u32 stack2;
		struct pad pad;

		padUnpack(autogun->targetpad, PADFIELD_POS, &pad);

		xdiff = pad.pos.x - autogun->base.prop->pos.x;
		ydiff = pad.pos.y - autogun->base.prop->pos.y;
		zdiff = pad.pos.z - autogun->base.prop->pos.z;

		autogun->yzero = atan2f(xdiff, zdiff);
		autogun->xzero = atan2f(ydiff, sqrtf(xdiff * xdiff + zdiff * zdiff));
	} else if (autogun->base.modelnum == MODEL_CETROOFGUN) {
		// Deep Sea roofgun
		autogun->xzero = -1.5705462694168f;
	}
}

void setupCreateHangingMonitors(struct hangingmonitorsobj *monitors, s32 cmdindex)
{
	setupCreateObject(&monitors->base, cmdindex);
}

void setupCreateSingleMonitor(struct singlemonitorobj *monitor, s32 cmdindex)
{
	u32 stack[2];

	monitor->screen = var8009ce98;
	tvscreenSetImageByNum(&monitor->screen, monitor->imagenum);

	// In GE, monitors with a negative pad are hanging TVs which attach to a
	// hangingmonitors object, which is actually just the mount. In PD, hanging
	// monitors do not exist in the setup files so this code is unused.
	if (monitor->base.pad < 0 && (monitor->base.flags & OBJFLAG_INSIDEANOTHEROBJ) == 0) {
		s32 modelnum = monitor->base.modelnum;
		struct defaultobj *owner = (struct defaultobj *)setupGetCmdByIndex(cmdindex + monitor->owneroffset);
		struct prop *prop;
		f32 scale;
		struct coord spa4;
		Mtxf sp64;
		Mtxf sp24;

		setupLoadModeldef(modelnum);

		scale = monitor->base.extrascale * (1.0f / 256.0f);

		if (g_Vars.normmplayerisrunning || g_Vars.lvmpbotlevel) {
			monitor->base.hidden2 |= OBJH2FLAG_CANREGEN;
		}

		prop = objInitWithAutoModel(&monitor->base);
		monitor->base.embedment = embedmentAllocate();

		if (prop && monitor->base.embedment) {
			monitor->base.hidden |= OBJHFLAG_EMBEDDED;
			modelSetScale(monitor->base.model, monitor->base.model->scale * scale);
			monitor->base.model->attachedtomodel = owner->model;

			if (monitor->ownerpart == MODELPART_0000) {
				monitor->base.model->attachedtonode = modelGetPart(owner->model->definition, MODELPART_0000);
			} else if (monitor->ownerpart == MODELPART_0001) {
				monitor->base.model->attachedtonode = modelGetPart(owner->model->definition, MODELPART_0001);
			} else if (monitor->ownerpart == MODELPART_0002) {
				monitor->base.model->attachedtonode = modelGetPart(owner->model->definition, MODELPART_0002);
			} else {
				monitor->base.model->attachedtonode = modelGetPart(owner->model->definition, MODELPART_0003);
			}

			propReparent(prop, owner->prop);
			mtx4LoadXRotation(0.3664608001709f, &sp64);
			mtx00015f04(monitor->base.model->scale / owner->model->scale, &sp64);
			modelGetRootPosition(monitor->base.model, &spa4);

			spa4.x = -spa4.x;
			spa4.y = -spa4.y;
			spa4.z = -spa4.z;

			mtx4LoadTranslation(&spa4, &sp24);
			mtx00015be4(&sp64, &sp24, &monitor->base.embedment->matrix);
		}
	} else {
		setupCreateObject(&monitor->base, cmdindex);
	}

	if (monitor->base.prop && (monitor->base.flags & OBJFLAG_MONITOR_RENDERPOSTBG)) {
		monitor->base.prop->flags |= PROPFLAG_RENDERPOSTBG;
	}
}

void setupCreateMultiMonitor(struct multimonitorobj *monitor, s32 cmdindex)
{
	monitor->screens[0] = var8009ce98;
	tvscreenSetImageByNum(&monitor->screens[0], monitor->imagenums[0]);

	monitor->screens[1] = var8009ce98;
	tvscreenSetImageByNum(&monitor->screens[1], monitor->imagenums[1]);

	monitor->screens[2] = var8009ce98;
	tvscreenSetImageByNum(&monitor->screens[2], monitor->imagenums[2]);

	monitor->screens[3] = var8009ce98;
	tvscreenSetImageByNum(&monitor->screens[3], monitor->imagenums[3]);

	setupCreateObject(&monitor->base, cmdindex);
}

s32 setupGetPortalByPad(s32 padnum)
{
	f32 mult;
	struct coord centre;
	struct coord coord;
	u32 stack;
	struct pad pad;

	padGetCentre(padnum, &centre);
	padUnpack(padnum, PADFIELD_BBOX | PADFIELD_UP, &pad);

	mult = (pad.bbox.ymax - pad.bbox.ymin) * 0.5f + 10;

	coord.x = pad.up.x * mult + centre.x;
	coord.y = pad.up.y * mult + centre.y;
	coord.z = pad.up.z * mult + centre.z;

	centre.x = centre.x - pad.up.x * mult;
	centre.y = centre.y - pad.up.y * mult;
	centre.z = centre.z - pad.up.z * mult;

	return bg0f164e8c(&centre, &coord);
}

s32 setupGetPortalByDoorPad(s32 padnum)
{
	f32 mult;
	struct coord centre;
	struct coord coord;
	u32 stack;
	struct pad pad;

	padGetCentre(padnum, &centre);
	padUnpack(padnum, PADFIELD_BBOX | PADFIELD_NORMAL, &pad);

	mult = (pad.bbox.xmax - pad.bbox.xmin) * 0.5f + 10;

	coord.x = pad.normal.x * mult + centre.x;
	coord.y = pad.normal.y * mult + centre.y;
	coord.z = pad.normal.z * mult + centre.z;

	centre.x = centre.x - pad.normal.x * mult;
	centre.y = centre.y - pad.normal.y * mult;
	centre.z = centre.z - pad.normal.z * mult;

	return bg0f164e8c(&centre, &coord);
}

void setupCreateDoor(struct doorobj *door, s32 cmdindex)
{
	f32 scale;
	s32 modelnum = door->base.modelnum;
	s32 portalnum = -1;
	struct pad pad;

	setupLoadModeldef(modelnum);

	if (door->doorflags & DOORFLAG_ROTATEDPAD) {
		padRotateForDoor(door->base.pad);
	}

	if (door->base.flags & OBJFLAG_DOOR_HASPORTAL) {
		portalnum = setupGetPortalByDoorPad(door->base.pad);
	}

	padUnpack(door->base.pad, PADFIELD_POS | PADFIELD_LOOK | PADFIELD_UP | PADFIELD_NORMAL | PADFIELD_BBOX | PADFIELD_ROOM, &pad);

	if (g_DoorScale != 1) {
		pad.bbox.xmin *= g_DoorScale;
		pad.bbox.xmax *= g_DoorScale;

		// If the door has a portal, adjust the pad's bbox to match the portal's dimensions
		if (portalnum >= 0) {
			struct var800a4ccc *ptr = &var800a4ccc[portalnum];
			f32 f0 = pad.pos.f[0] * ptr->coord.f[0] + pad.pos.f[1] * ptr->coord.f[1] + pad.pos.f[2] * ptr->coord.f[2];
			f32 min = ptr->min;
			struct coord sp150;
			f0 = (f0 - min) * (g_DoorScale - 1);

			sp150.x = ptr->coord.x * f0;
			sp150.y = ptr->coord.y * f0;
			sp150.z = ptr->coord.z * f0;

			f0 = sp150.f[0] * pad.normal.f[0] + sp150.f[1] * pad.normal.f[1] + sp150.f[2] * pad.normal.f[2];
			pad.bbox.xmin += f0;
			pad.bbox.xmax += f0;

			f0 = sp150.f[0] * pad.up.f[0] + sp150.f[1] * pad.up.f[1] + sp150.f[2] * pad.up.f[2];
			pad.bbox.ymin += f0;
			pad.bbox.ymax += f0;

			f0 = sp150.f[0] * pad.look.f[0] + sp150.f[1] * pad.look.f[1] + sp150.f[2] * pad.look.f[2];
			pad.bbox.zmin += f0;
			pad.bbox.zmax += f0;
		}

		// Write the modified bbox into the pad file data
		padCopyBboxFromPad(door->base.pad, &pad);
	}

	if (pad.room > 0) {
		Mtxf sp110;
		struct prop *prop;
		s32 siblingcmdindex;
		struct coord pos;
		s16 rooms[8];
		Mtxf finalmtx;
		struct coord centre;
		Mtxf zrotmtx;
		struct coord sp54;
		f32 xscale;
		f32 yscale;
		f32 zscale;
		struct modelrodata_bbox *bbox;

		bbox = modeldefFindBboxRodata(g_ModelStates[modelnum].modeldef);

		mtx00016d58(&sp110, 0, 0, 0,
				-pad.look.x, -pad.look.y, -pad.look.z,
				pad.up.x, pad.up.y, pad.up.z);
		mtx4LoadXRotation(1.5705462694168f, &finalmtx);
		mtx4LoadZRotation(1.5705462694168f, &zrotmtx);
		mtx4MultMtx4InPlace(&zrotmtx, &finalmtx);
		mtx4MultMtx4InPlace(&sp110, &finalmtx);

		padGetCentre(door->base.pad, &centre);

		xscale = (pad.bbox.ymax - pad.bbox.ymin) / (bbox->xmax - bbox->xmin);
		yscale = (pad.bbox.zmax - pad.bbox.zmin) / (bbox->ymax - bbox->ymin);
		zscale = (pad.bbox.xmax - pad.bbox.xmin) / (bbox->zmax - bbox->zmin);

		if (xscale <= 0.000001f || yscale <= 0.000001f || zscale <= 0.000001f) {
			xscale = yscale = zscale = 1;
		}

		mtx00015e24(xscale, &finalmtx);
		mtx00015e80(yscale, &finalmtx);
		mtx00015edc(zscale, &finalmtx);

		pos.x = pad.pos.x;
		pos.y = pad.pos.y;
		pos.z = pad.pos.z;

		rooms[0] = pad.room;
		rooms[1] = -1;

		if (door->doortype == DOORTYPE_VERTICAL || door->doortype == DOORTYPE_FALLAWAY) {
			sp54.x = pad.look.f[0] * (pad.bbox.zmax - pad.bbox.zmin);
			sp54.y = pad.look.f[1] * (pad.bbox.zmax - pad.bbox.zmin);
			sp54.z = pad.look.f[2] * (pad.bbox.zmax - pad.bbox.zmin);
		} else {
			sp54.x = pad.up.f[0] * (pad.bbox.ymin - pad.bbox.ymax);
			sp54.y = pad.up.f[1] * (pad.bbox.ymin - pad.bbox.ymax);
			sp54.z = pad.up.f[2] * (pad.bbox.ymin - pad.bbox.ymax);
		}

		// These values are stored in the setup files as integers, but at
		// runtime they are floats. Hence reading a "float" as an integer,
		// converting it to a float and writing it back to the same property.
		door->maxfrac = *(s32 *) &door->maxfrac / 65536.0f;
		door->perimfrac = *(s32 *) &door->perimfrac / 65536.0f;
		door->accel = PALUPF(*(s32 *) &door->accel) / 65536000.0f;
		door->decel = PALUPF(*(s32 *) &door->decel) / 65536000.0f;
		door->maxspeed = PALUPF(*(s32 *) &door->maxspeed) / 65536.0f;

		// The sibling door is stored as a relative command number,
		// but at runtime it's a pointer.
		if (door->sibling) {
			siblingcmdindex = *(s32 *) &door->sibling + cmdindex;
			door->sibling = (struct doorobj *) setupGetCmdByIndex(siblingcmdindex);
		}

		prop = doorInit(door, &pos, &finalmtx, rooms, &sp54, &centre);

		if (door->base.flags & OBJFLAG_DOOR_HASPORTAL) {
			door->portalnum = portalnum;

			if (door->portalnum >= 0 && door->frac == 0) {
				doorDeactivatePortal(door);
			}
		}

		if (door->base.model) {
			scale = xscale;

			if (yscale > scale) {
				scale = yscale;
			}

			if (zscale > scale) {
				scale = zscale;
			}

			modelSetScale(door->base.model, door->base.model->scale * scale);
		}

		propActivate(prop);
		propEnable(prop);
	} else {
		door->base.prop = NULL;
	}
}

void setupCreateHov(struct defaultobj *obj, struct hov *hov)
{
	hov->unk04 = 0;
	hov->unk08 = 0;
	hov->unk0c = 0;
	hov->unk10 = atan2f(obj->realrot[2][0], obj->realrot[2][2]);
	hov->unk14 = 0;
	hov->unk18 = 0;
	hov->unk1c = 0;
	hov->unk20 = 0;
	hov->unk24 = 0;
	hov->unk28 = 0;
	hov->unk2c = 0;
	hov->unk30 = 0;
	hov->ground = 0;
	hov->nexttick60 = -1;
	hov->prevtick60 = -1;
}

void setupLoadBriefing(s32 stagenum, u8 *buffer, s32 bufferlen, struct briefing *briefing)
{
	if (stagenum < STAGE_TITLE) {
		s32 stageindex = stageGetIndex(stagenum);
		struct defaultobj *start;
		u16 setupfilenum;
		s32 setupfilesize;
		struct objective *objective;
		struct briefingobj *briefingobj;
		s32 i;
		u8 *langbuffer;
		s32 langbufferlen;
		struct stagesetup *setup;

		if (stageindex < 0) {
			stageindex = 0;
		}

		setupfilenum = g_Stages[stageindex].setupfileid;
		g_LoadType = LOADTYPE_LANG;

		fileLoadToAddr(setupfilenum, FILELOADMETHOD_DEFAULT, buffer, bufferlen);

		setup = (struct stagesetup *)buffer;
		setupfilesize = fileGetLoadedSize(setupfilenum);
		langbuffer = &buffer[setupfilesize];
		langbufferlen = bufferlen - setupfilesize;

		briefing->langbank = langGetLangBankIndexFromStagenum(stagenum);

		langLoadToAddr(briefing->langbank, langbuffer, langbufferlen);

		start = (struct defaultobj *)((uintptr_t)setup + (uintptr_t)setup->props);

		if (start != NULL) {
			struct defaultobj *obj;
			s32 wanttype = BRIEFINGTYPE_TEXT_PA;

			if (lvGetDifficulty() == DIFF_A) {
				wanttype = BRIEFINGTYPE_TEXT_A;
			}

			if (lvGetDifficulty() == DIFF_SA) {
				wanttype = BRIEFINGTYPE_TEXT_SA;
			}

			for (i = 0; (u32)(i < 6); i++) {
				briefing->objectivenames[i] = 0;
			}

			briefing->briefingtextnum = L_MISC_042; // "No briefing for this mission"

			obj = start;

			while (obj->type != OBJTYPE_END) {
				if (1);
				switch (obj->type) {
				case OBJTYPE_BRIEFING:
					briefingobj = (struct briefingobj *) obj;

					if (briefingobj->type == BRIEFINGTYPE_TEXT_PA) {
						briefing->briefingtextnum = briefingobj->text;
					}

					if (briefingobj->type == wanttype) {
						briefing->briefingtextnum = briefingobj->text;
					}
					break;
				case OBJTYPE_BEGINOBJECTIVE:
					objective = (struct objective *) obj;

					if (objective->index < 7U) {
						briefing->objectivenames[objective->index] = objective->text;
						briefing->objectivedifficulties[objective->index] = objective->difficulties;
					}
					break;
				}

				obj = (struct defaultobj *)((u32 *)obj + setupGetCmdLength((u32 *)obj));
			}
		}
	}
}

struct cmd32 {
	s32 type;
	s32 param1;
	s32 param2;
	s32 param3;
};


u32 setupCalculateIntroSize(s32* start)
{
	s32 *cmd = start;

	if (cmd) {
		while (swap_uint32(cmd[0]) != INTROCMD_END) {
			switch (swap_uint32(cmd[0])) {
			case INTROCMD_SPAWN:
				cmd += 3;
				break;
			case INTROCMD_CASE:
			case INTROCMD_CASERESPAWN:
				cmd += 3;
				break;
			case INTROCMD_HILL:
				cmd += 2;
				break;
			case INTROCMD_WEAPON:
				cmd += 4;
				break;
			case INTROCMD_AMMO:
				cmd += 4;
				break;
			case INTROCMD_3:
				cmd += 8;
				break;
			case INTROCMD_4:
				cmd += 2;
				break;
			case INTROCMD_OUTFIT:
				cmd += 2;
				break;
			case INTROCMD_6:
				cmd += 10;
				break;
			case INTROCMD_WATCHTIME:
				cmd += 3;
				break;
			case INTROCMD_CREDITOFFSET:
				cmd += 2;
				break;
			default:
				cmd++;
				break;
			}
		}
	}

	return cmd - start;
}

void setupConvertIntroCmd(s32* start)
{
	s32 *cmd = start;

	if (cmd) {
		while (cmd[0] != INTROCMD_END) {
			switch (cmd[0]) {
			case INTROCMD_SPAWN:
				cmd += 3;
				break;
			case INTROCMD_CASE:
			case INTROCMD_CASERESPAWN:
				cmd += 3;
				break;
			case INTROCMD_HILL:
				cmd += 2;
				break;
			case INTROCMD_WEAPON: {
				cmd += 4;
				break;
			}
			case INTROCMD_AMMO:
				cmd += 4;
				break;
			case INTROCMD_3:
				cmd += 8;
				break;
			case INTROCMD_4:
				cmd += 2;
				break;
			case INTROCMD_OUTFIT:
				cmd += 2;
				break;
			case INTROCMD_6:
				cmd += 10;
				break;
			case INTROCMD_WATCHTIME:
				cmd += 3;
				break;
			case INTROCMD_CREDITOFFSET:
				cmd += 2;
				break;
			default:
				cmd++;
				break;
			}
		}
	}
}

void convertDefaultobj(struct defaultobj* dst, struct defaultobj_load* src)
{
	dst->extrascale = swap_uint16(src->extrascale);
	dst->hidden2 = src->hidden2;
	dst->type = src->type;
	dst->modelnum = swap_int16(src->modelnum);
	dst->pad = swap_int16(src->pad);
	dst->flags = swap_uint32(src->flags);
	dst->flags2 = swap_uint32(src->flags2);
	dst->flags3 = swap_uint32(src->flags3);
	dst->prop = (struct prop*)(uintptr_t)swap_uint32(src->prop);
	dst->model = (struct model*)(uintptr_t)swap_uint32(src->model);
	for (u32 i = 0; i < 3; i++)
		for (u32 j = 0; j < 3; j++)
			dst->realrot[i][j] = swap_f32(src->realrot[i][j]);
	dst->hidden = swap_uint32(src->hidden);
	dst->geotilef = (struct geotilef*)(uintptr_t)swap_uint32(src->geotilef);
	dst->projectile = (struct projectile*)(uintptr_t)swap_uint32(src->projectile);
	dst->damage = swap_int16(src->damage);
	dst->maxdamage = swap_int16(src->maxdamage);
	for (u32 i = 0; i < 4; i++)
		dst->shadecol[i] = src->shadecol[i];
	for (u32 i = 0; i < 4; i++)
		dst->nextcol[i] = src->nextcol[i];
	dst->floorcol = swap_uint16(src->floorcol);
	dst->geocount = src->geocount; 
}

void convertTvscreenobj(struct tvscreen* dst, struct tvscreen_load* src)
{
	dst->cmdlist = (u32*)(uintptr_t)swap_uint32(src->cmdlist);
	dst->offset = swap_uint16(src->offset);
	dst->pause60 = swap_int16(src->pause60);
	dst->tconfig = (struct textureconfig*)(uintptr_t)swap_uint32(src->tconfig);
	dst->rot = swap_f32(src->rot);
	dst->xscale = swap_f32(src->xscale);
	dst->xscalefrac = swap_f32(src->xscalefrac);
	dst->xscaleinc = swap_f32(src->xscaleinc);
	dst->xscaleold = swap_f32(src->xscaleold);
	dst->xscalenew = swap_f32(src->xscalenew);
	dst->yscale = swap_f32(src->yscale);
	dst->yscalefrac = swap_f32(src->yscalefrac);
	dst->yscaleinc = swap_f32(src->yscaleinc);
	dst->yscaleold = swap_f32(src->yscaleold);
	dst->yscalenew = swap_f32(src->yscalenew);
	dst->xmid = swap_f32(src->xmid);
	dst->xmidfrac = swap_f32(src->xmidfrac);
	dst->xmidinc = swap_f32(src->xmidinc);
	dst->xmidold = swap_f32(src->xmidold);
	dst->xmidnew = swap_f32(src->xmidnew);
	dst->ymid = swap_f32(src->ymid);
	dst->ymidfrac = swap_f32(src->ymidfrac);
	dst->ymidinc = swap_f32(src->ymidinc);
	dst->ymidold = swap_f32(src->ymidold);
	dst->ymidnew = swap_f32(src->ymidnew);
	dst->red = src->red;
	dst->redold = src->redold;
	dst->rednew = src->rednew;
	dst->green = src->green;
	dst->greenold = src->greenold;
	dst->greennew = src->greennew;
	dst->blue = src->blue;
	dst->blueold = src->blueold;
	dst->bluenew = src->bluenew;
	dst->alpha = src->alpha;
	dst->alphaold = src->alphaold;
	dst->alphanew = src->alphanew;
	dst->colfrac = swap_f32(src->colfrac);
	dst->colinc = swap_f32(src->colinc);
}

void convertHovobj(struct hov* dst, struct hov* src)
{
	dst->type = src->type;
	dst->flags = src->flags;
	dst->unk04 = swap_f32(src->unk04);
	dst->unk08 = swap_f32(src->unk08);
	dst->unk0c = swap_f32(src->unk0c);
	dst->unk10 = swap_f32(src->unk10);
	dst->unk14 = swap_f32(src->unk14);
	dst->unk18 = swap_f32(src->unk18);
	dst->unk1c = swap_f32(src->unk1c);
	dst->unk20 = swap_f32(src->unk20);
	dst->unk24 = swap_f32(src->unk24);
	dst->unk28 = swap_f32(src->unk28);
	dst->unk2c = swap_f32(src->unk2c);
	dst->unk30 = swap_f32(src->unk30);
	dst->ground = swap_f32(src->ground);
	dst->nexttick60 = swap_uint32(src->nexttick60);
	dst->prevtick60 = swap_uint32(src->prevtick60);
}

void convertChraiCommand(u32 type, u8* cmd)
{
	switch(type) {
		default: {
			print("convertChraiCommand: unhandled ai cmd: %04x\n", type);
		}
	}
}

/*
	Similar to modeldef, we need to load the N64 structure to a PC structure with different offsets
*/
void setupLoadFiles(s32 stagenum)
{
	s32 i;
	s32 j;
	struct ailist tmp;
	s32 numchrs = 0;
	s32 numobjs = 0;
	s32 extra;
	struct stagesetup *setup;
	u16 filenum;
	bool modified;

	g_PadEffects = NULL;
	g_LastPadEffectIndex = -1;

	g_DoorScale = 1;

	for (i = 0; i < NUM_MODELS; i++) {
		g_ModelStates[i].modeldef = NULL;
	}

	print("setupLoadFiles %x\n", stagenum);

	if (stagenum < STAGE_TITLE) {
		if (g_Vars.normmplayerisrunning) {
			filenum = g_Stages[g_StageIndex].mpsetupfileid;
		} else {
			filenum = g_Stages[g_StageIndex].setupfileid;
		}

		g_LoadType = LOADTYPE_SETUP;

		// Note PC: using fileLoadToNew, the loaded struct will be too small to hold our larger buffer
		// TODO: need to allocate a buffer to store the new file, however we cannot calculate it's size in advance...
		g_GeCreditsData = (u8 *)fileLoadToNew(filenum, FILELOADMETHOD_DEFAULT);
		u32 allocsize = fileGetAllocationSize(filenum);
		u32 loadedsize = fileGetLoadedSize(filenum);
		print("allocsize %x loadedsize %x\n", allocsize, loadedsize);

		// HACK: reserve 2x more size for this file
		// In the future, we need to compute the exact size of the converted assets
		fileSetSize(filenum, g_GeCreditsData, allocsize * 2, true);

		u8* originalFileData = nativeMalloc(loadedsize);
		memcpy(originalFileData, g_GeCreditsData, loadedsize);

		struct stagesetup_load* setup_load = (struct stagesetup_load*)originalFileData;
		setup = (struct stagesetup *)g_GeCreditsData;
		langLoad(langGetLangBankIndexFromStagenum(stagenum));

		// Convert the N64 struct to a PC struct
		memset(setup, 0, loadedsize);
		u32 fileWriteOffset = 0;
		initOffsetsContext();

		print("- g_StageSetup.waypoints %x\n", swap_uint32(setup_load->waypoints));
		print("- g_StageSetup.waygroups %x\n", swap_uint32(setup_load->waygroups));
		print("- g_StageSetup.cover %x\n", swap_uint32(setup_load->cover));
		print("- g_StageSetup.intro %x\n", swap_int32(setup_load->intro));
		print("- g_StageSetup.props %x\n", swap_uint32(setup_load->props));
		print("- g_StageSetup.paths %x\n", swap_uint32(setup_load->paths));
		print("- g_StageSetup.ailists %x\n", swap_uint32(setup_load->ailists));
		print("- g_StageSetup.padfiledata %x\n", swap_uint32(setup_load->padfiledata));

		fileWriteOffset += sizeof(struct stagesetup);

		/*
			props
			first we must iterate the props as they were written in the original file format
		*/
		u32 objOffset = swap_uint32(setup_load->props);

		if (objOffset != 0)
		{
			addOffsetGlobal(objOffset, fileWriteOffset, 0);

			struct defaultobj_load* obj = (struct defaultobj_load*)((uintptr_t)setup_load + (uintptr_t)objOffset);

			while (obj->type != OBJTYPE_END) {
				//print("Converting OBJTYPE: %02x\n", obj->type);
				// Note: some fields are read from the file, some others are generated but it's not explicit
				// so lets convert all of them, theyll be overwritten anyway
				switch (obj->type) 
				{
					case OBJTYPE_DOOR: {
						struct doorobj_load* src = (struct doorobj_load*)(obj);
						struct doorobj dst;
						convertDefaultobj(&dst.base, obj);
						dst.maxfrac = swap_f32(src->maxfrac);
						dst.perimfrac = swap_f32(src->perimfrac);
						dst.accel = swap_f32(src->accel);
						dst.decel = swap_f32(src->decel);
						dst.maxspeed = swap_f32(src->maxspeed);
						dst.doorflags = swap_uint16(src->doorflags);
						dst.doortype = swap_uint16(src->doortype);
						dst.keyflags = swap_uint32(src->keyflags);
						dst.autoclosetime = swap_int32(src->autoclosetime);

						// Converting fields after this, probably not necessary
						dst.frac = swap_f32(src->frac);
						dst.fracspeed = swap_f32(src->fracspeed);
						dst.mode = (src->mode);
						dst.glasshits = (src->glasshits);
						dst.fadealpha = swap_int16(src->fadealpha);
						dst.xludist = swap_int16(src->xludist);
						dst.opadist = swap_int16(src->opadist);
						dst.startpos = src->startpos;
						swap_coord(&dst.startpos);
						// no need to set mtx98
						// Note: src->sibling can be 0xffffffff and interpreted as -1, use swap int32 here
						dst.sibling = (struct doorobj*)(uintptr_t)swap_int32(src->sibling);
						dst.lastopen60 = swap_int32(src->lastopen60);
						dst.portalnum = swap_int16(src->portalnum);
						dst.soundtype = src->soundtype;
						dst.fadetime60 = src->fadetime60;
						dst.lastcalc60 = swap_int32(src->lastcalc60);
						dst.laserfade = src->laserfade;
						for (u32 i = 0; i < 3; i++) dst.unusedmaybe[i] = src->unusedmaybe[i];
						for (u32 i = 0; i < 4; i++) dst.shadeinfo1[i] = src->shadeinfo1[i];
						for (u32 i = 0; i < 4; i++) dst.shadeinfo2[i] = src->shadeinfo2[i];
						dst.actual1 = src->actual1;
						dst.actual2 = src->actual2;
						dst.extra1 = src->extra1;
						dst.extra2 = src->extra2;

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct doorobj));
						fileWriteOffset += sizeof(struct doorobj);
						break;
					}

					case OBJTYPE_TAG: {
						struct tag_load* src = (struct tag_load*)(obj);
						struct tag dst;

						//dst.identifier = swap_uint32(src->identifier);
						dst.identifier = src->identifier;
						dst.tagnum = swap_uint16(src->tagnum);
						dst.cmdoffset = swap_int16(src->cmdoffset);
						dst.next = (struct tag*)(uintptr_t)swap_uint32(src->next);
						dst.obj = (struct defaultobj*)(uintptr_t)swap_uint32(src->obj);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct tag));
						fileWriteOffset += sizeof(struct tag);
						break;
					}

					case OBJTYPE_BASIC: {
						struct defaultobj_load* src = (struct defaultobj_load*)(obj);
						struct defaultobj dst;
						convertDefaultobj(&dst, obj);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct defaultobj));
						fileWriteOffset += sizeof(struct defaultobj);
						break;
					}

					case OBJTYPE_GLASS: {
						struct glassobj_load* src = (struct glassobj_load*)(obj);
						struct glassobj dst;
						convertDefaultobj(&dst.base, obj);
						dst.portalnum = swap_int16(src->portalnum);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct glassobj));
						fileWriteOffset += sizeof(struct glassobj);
						break;
					}

					case OBJTYPE_LIFT: {
						struct liftobj_load* src = (struct liftobj_load*)(obj);
						struct liftobj dst;
						convertDefaultobj(&dst.base, obj);

						for (u32 i = 0; i < 4; i++) dst.pads[i] = swap_int16(src->pads[i]);
						for (u32 i = 0; i < 4; i++) dst.doors[i] = (struct doorobj*)(uintptr_t)swap_uint32(src->doors[i]);
						dst.dist = swap_f32(src->dist);
						dst.speed = swap_f32(src->speed);
						dst.accel = swap_f32(src->accel);
						dst.maxspeed = swap_f32(src->maxspeed);
						dst.soundtype = src->soundtype;
						dst.levelcur = src->levelcur;
						dst.levelaim = src->levelaim;
						dst.prevpos = src->prevpos;
						swap_coord(&dst.prevpos);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct liftobj));
						fileWriteOffset += sizeof(struct liftobj);
						break;
					}

					case OBJTYPE_LINKLIFTDOOR: {
						struct linkliftdoorobj_load* src = (struct linkliftdoorobj_load*)(obj);
						struct linkliftdoorobj dst;

						dst.unk00 = src->unk00;
						dst.door = (struct prop *)(uintptr_t)swap_uint32(src->door);
						dst.lift = (struct prop *)(uintptr_t)swap_uint32(src->lift);
						dst.next = (struct linkliftdoorobj *)(uintptr_t)swap_uint32(src->next);
						dst.stopnum = swap_int32(src->stopnum);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct linkliftdoorobj));
						fileWriteOffset += sizeof(struct linkliftdoorobj);
						break;
					}
					
					case OBJTYPE_SINGLEMONITOR: {
						struct singlemonitorobj_load* src = (struct singlemonitorobj_load*)(obj);
						struct singlemonitorobj dst;
						convertDefaultobj(&dst.base, obj);
						convertTvscreenobj(&dst.screen, &src->screen);
						
						dst.owneroffset = swap_int16(src->owneroffset);
						dst.ownerpart = src->ownerpart;
						dst.imagenum = src->imagenum;

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct singlemonitorobj));
						fileWriteOffset += sizeof(struct singlemonitorobj);
						break;
					}

					case OBJECTIVETYPE_HOLOGRAPH: {
						struct criteria_holograph* src = (struct criteria_holograph*)(obj);
						struct criteria_holograph dst;

						dst.unk00 = src->unk00;
						dst.obj = swap_uint32(src->obj);
						dst.status = swap_uint32(src->status);
						//struct criteria_holograph *next;

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct criteria_holograph));
						fileWriteOffset += sizeof(struct criteria_holograph);
						break;
					}

					case OBJTYPE_CHR: {
						struct packedchr* src = (struct packedchr*)(obj);
						struct packedchr dst;
						
						dst.chrindex = swap_int16(src->chrindex);
						dst.unk02 = src->unk02;
						dst.typenum = src->typenum;
						dst.spawnflags = swap_uint32(src->spawnflags);
						dst.chrnum = swap_int16(src->chrnum);
						dst.padnum = swap_uint16(src->padnum);
						dst.bodynum = src->bodynum;
						dst.headnum = src->headnum;
						dst.ailistnum = swap_uint16(src->ailistnum);
						dst.padpreset = swap_uint16(src->padpreset);
						dst.chrpreset = swap_uint16(src->chrpreset);
						dst.hearscale = swap_uint16(src->hearscale);
						dst.viewdist = swap_uint16(src->viewdist);
						dst.flags = swap_uint32(src->flags);
						dst.flags2 = swap_uint32(src->flags2);
						dst.team = src->team;
						dst.squadron = src->squadron;
						dst.chair = swap_int16(src->chair);
						dst.convtalk = swap_uint32(src->convtalk);
						dst.tude = src->tude;
						dst.naturalanim = src->naturalanim;
						dst.yvisang = src->yvisang;
						dst.teamscandist = src->teamscandist;

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct packedchr));
						fileWriteOffset += sizeof(struct packedchr);
						break;
					}

					case OBJTYPE_WEAPON: {
						struct weaponobj* src = (struct weaponobj*)(obj);
						struct weaponobj dst;
						convertDefaultobj(&dst.base, obj);
						dst.weaponnum = src->weaponnum;
						dst.unk5d = src->unk5d;
						dst.unk5e = src->unk5e;
						dst.gunfunc = src->gunfunc;
						dst.fadeouttimer60 = src->fadeouttimer60;
						dst.dualweaponnum = src->dualweaponnum;
						dst.timer240 = swap_int16(src->timer240);
						dst.team = swap_int16(src->team);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct weaponobj));
						fileWriteOffset += sizeof(struct weaponobj);
						break;
					}

					case OBJTYPE_MULTIMONITOR: {
						struct multimonitorobj_load* src = (struct multimonitorobj_load*)(obj);
						struct multimonitorobj dst;
						convertDefaultobj(&dst.base, obj);
						for (u32 i = 0; i < 4; i++) convertTvscreenobj(&dst.screens[i], &src->screens[i]);
						for (u32 i = 0; i < 4; i++) dst.imagenums[i] = src->imagenums[i];

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct multimonitorobj));
						fileWriteOffset += sizeof(struct multimonitorobj);
						break;
					}

					case OBJTYPE_CCTV: {
						struct cctvobj_load* src = (struct cctvobj_load*)(obj);
						struct cctvobj dst;
						convertDefaultobj(&dst.base, obj);

						dst.lookatpadnum = swap_int16(src->lookatpadnum);
						dst.toleft = swap_int16(src->toleft);
						//Mtxf camrotm;
						
						dst.yzero = swap_f32(src->yzero);
						dst.yrot = swap_f32(src->yrot);
						dst.yleft = swap_f32(src->yleft);
						dst.yright = swap_f32(src->yright);
						dst.yspeed = swap_f32(src->yspeed);
						dst.ymaxspeed = swap_f32(src->ymaxspeed);
						dst.seebondtime60 = swap_int32(src->seebondtime60);
						dst.maxdist = swap_f32(src->maxdist);
						dst.xzero = swap_f32(src->xzero);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct cctvobj));
						fileWriteOffset += sizeof(struct cctvobj);
						break;
					}

					case OBJTYPE_HOVERPROP: {
						struct hoverpropobj_load* src = (struct hoverpropobj_load*)(obj);
						struct hoverpropobj dst;
						convertDefaultobj(&dst.base, obj);
						convertHovobj(&dst.hov, &src->hov);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct hoverpropobj));
						fileWriteOffset += sizeof(struct hoverpropobj);
						break;
					}

					case OBJTYPE_HOVERBIKE: {
						struct hoverbikeobj_load* src = (struct hoverbikeobj_load*)(obj);
						struct hoverbikeobj dst;
						convertDefaultobj(&dst.base, obj);
						convertHovobj(&dst.hov, &src->hov);

						for (u32 i = 0; i < 2; i++) dst.speed[i] = swap_f32(src->speed[i]);
						for (u32 i = 0; i < 2; i++) dst.prevpos[i] = swap_f32(src->prevpos[i]);
						dst.w = swap_f32(src->w);
						for (u32 i = 0; i < 2; i++) dst.rels[i] = swap_f32(src->rels[i]);
						dst.exreal = swap_f32(src->exreal);
						dst.ezreal = swap_f32(src->ezreal);
						dst.ezreal2 = swap_f32(src->ezreal2);
						dst.leanspeed = swap_f32(src->leanspeed);
						dst.leandiff = swap_f32(src->leandiff);
						dst.maxspeedtime240 = swap_uint32(src->maxspeedtime240);
						for (u32 i = 0; i < 2; i++) dst.speedabs[i] = swap_f32(src->speedabs[i]);
						for (u32 i = 0; i < 2; i++) dst.speedrel[i] = swap_f32(src->speedrel[i]);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct hoverbikeobj));
						fileWriteOffset += sizeof(struct hoverbikeobj);
						break;
					}

					case OBJTYPE_AUTOGUN: {
						struct autogunobj_load* src = (struct autogunobj_load*)(obj);
						struct autogunobj dst;
						convertDefaultobj(&dst.base, obj);

						dst.targetpad = swap_int16(src->targetpad);
						dst.firing = src->firing;
						dst.firecount = src->firecount;

						dst.yzero = swap_f32(src->yzero);
						dst.ymaxleft = swap_f32(src->ymaxleft);
						dst.ymaxright = swap_f32(src->ymaxright);
						dst.yrot = swap_f32(src->yrot);
						dst.yspeed = swap_f32(src->yspeed);
						dst.xzero = swap_f32(src->xzero);
						dst.xrot = swap_f32(src->xrot);
						dst.xspeed = swap_f32(src->xspeed);
						dst.maxspeed = swap_f32(src->maxspeed);
						dst.aimdist = swap_f32(src->aimdist);
						dst.barrelspeed = swap_f32(src->barrelspeed);
						dst.barrelrot = swap_f32(src->barrelrot);

						dst.lastseebond60 = swap_int32(src->lastseebond60);
						dst.lastaimbond60 = swap_int32(src->lastaimbond60);
						dst.allowsoundframe = swap_int32(src->allowsoundframe);
						dst.beam = (struct beam*)(uintptr_t)swap_uint32(src->beam);
						dst.shotbondsum = swap_f32(src->shotbondsum);
						dst.target = (struct prop*)(uintptr_t)swap_uint32(src->target);
						dst.targetteam = src->targetteam;
						dst.ammoquantity = src->ammoquantity;
						dst.nextchrtest = swap_int16(src->nextchrtest);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct autogunobj));
						fileWriteOffset += sizeof(struct autogunobj);
						break;
					}

					case OBJTYPE_FAN: {
						struct fanobj_load* src = (struct fanobj_load*)(obj);
						struct fanobj dst;
						convertDefaultobj(&dst.base, obj);

						dst.yrot = swap_f32(src->yrot);
						dst.yrotprev = swap_f32(src->yrotprev);
						dst.ymaxspeed = swap_f32(src->ymaxspeed);
						dst.yspeed = swap_f32(src->yspeed);
						dst.yaccel = swap_f32(src->yaccel);
						dst.on = src->on;

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct fanobj));
						fileWriteOffset += sizeof(struct fanobj);
						break;
					}

					case OBJTYPE_SHIELD: {
						struct shieldobj_load* src = (struct shieldobj_load*)(obj);
						struct shieldobj dst;
						convertDefaultobj(&dst.base, obj);

						dst.initialamount = swap_f32(src->initialamount);
						dst.amount = swap_f32(src->amount);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct shieldobj));
						fileWriteOffset += sizeof(struct shieldobj);
						break;
					}

					case OBJTYPE_TINTEDGLASS: {
						struct tintedglassobj_load* src = (struct tintedglassobj_load*)(obj);
						struct tintedglassobj dst;
						convertDefaultobj(&dst.base, obj);

						dst.xludist = swap_int16(src->xludist);
						dst.opadist = swap_int16(src->opadist);
						dst.opacity = swap_int16(src->opacity);
						dst.portalnum = swap_int16(src->portalnum);
						dst.unk64 = swap_f32(src->unk64);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct tintedglassobj));
						fileWriteOffset += sizeof(struct tintedglassobj);
						break;
					}
					
					case OBJTYPE_LINKGUNS: {
						struct linkgunsobj* src = (struct linkgunsobj*)(obj);
						struct linkgunsobj dst;

						dst.unk00 = src->unk00;
						dst.offset1 = swap_int16(src->offset1);
						dst.offset2 = swap_int16(src->offset2);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct linkgunsobj));
						fileWriteOffset += sizeof(struct linkgunsobj);
						break;
					}

					case OBJTYPE_HOVERCAR: {
						struct hovercarobj_load* src = (struct hovercarobj_load*)(obj);
						struct hovercarobj dst;
						convertDefaultobj(&dst.base, obj);

						dst.ailist = (u8*)(uintptr_t)swap_uint32(src->ailist);
						dst.aioffset = swap_uint16(src->aioffset);
						dst.aireturnlist = swap_int16(src->aireturnlist);
						dst.speed = swap_f32(src->speed);
						dst.speedaim = swap_f32(src->speedaim);
						dst.speedtime60 = swap_f32(src->speedtime60);
						dst.turnyspeed60 = swap_f32(src->turnyspeed60);
						dst.turnxspeed60 = swap_f32(src->turnxspeed60);
						dst.turnrot60 = swap_f32(src->turnrot60);
						dst.roty = swap_f32(src->roty);
						dst.rotx = swap_f32(src->rotx);
						dst.rotz = swap_f32(src->rotz);
						dst.path = (struct path *)(uintptr_t)swap_uint32(src->path);
						dst.nextstep = swap_int32(src->nextstep);
						dst.status = swap_int16(src->status);
						dst.dead = swap_int16(src->dead);
						dst.deadtimer60 = swap_int16(src->deadtimer60);
						dst.sparkstimer60 = swap_int16(src->sparkstimer60);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct hovercarobj));
						fileWriteOffset += sizeof(struct hovercarobj);
						break;
					}

					case OBJTYPE_CONDITIONALSCENERY: {
						struct linksceneryobj_load* src = (struct linksceneryobj_load*)(obj);
						struct linksceneryobj dst;

						// Note: do not swap unk00, it's already in the right order because it's the first fields of defaultobj
						// not a single u32
						dst.unk00 = src->unk00;
						dst.trigger = (struct defaultobj *)(uintptr_t)swap_uint32(src->trigger);
						dst.unexp = (struct defaultobj *)(uintptr_t)swap_uint32(src->unexp);
						dst.exp = (struct defaultobj *)(uintptr_t)swap_uint32(src->exp);
						dst.next = (struct linksceneryobj *)(uintptr_t)swap_uint32(src->next);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct linksceneryobj));
						fileWriteOffset += sizeof(struct linksceneryobj);
						break;
					}

					case OBJTYPE_BRIEFING: {
						struct briefingobj_load* src = (struct briefingobj_load*)(obj);
						struct briefingobj dst;

						dst.unk00 = src->unk00;
						dst.type = swap_uint32(src->type);
						dst.text = swap_uint32(src->text);
						dst.next = (struct briefingobj*)(uintptr_t)swap_uint32(src->next);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct briefingobj));
						fileWriteOffset += sizeof(struct briefingobj);
						break;
					}

					case OBJTYPE_RENAMEOBJ: {
						struct textoverride_load* src = (struct textoverride_load*)(obj);
						struct textoverride dst;

						dst.unk00 = src->unk00;
						dst.objoffset = swap_int32(src->objoffset);
						dst.weapon = swap_int32(src->weapon);
						dst.obtaintext = swap_uint32(src->obtaintext);
						dst.ownertext = swap_uint32(src->ownertext);
						dst.inventorytext = swap_uint32(src->inventorytext);
						dst.inventory2text = swap_uint32(src->inventory2text);
						dst.pickuptext = swap_uint32(src->pickuptext);
						dst.next = (struct textoverride *)(uintptr_t)swap_uint32(src->next);
						dst.obj = (struct defaultobj *)(uintptr_t)swap_uint32(src->obj);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct textoverride));
						fileWriteOffset += sizeof(struct textoverride);
						break;
					}

					case OBJTYPE_KEY: {
						struct keyobj_load* src = (struct keyobj_load*)(obj);
						struct keyobj dst;
						convertDefaultobj(&dst.base, obj);
						dst.keyflags = swap_uint32(src->keyflags);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct keyobj));
						fileWriteOffset += sizeof(struct keyobj);
						break;
					}

					case OBJTYPE_AMMOCRATE: {
						struct ammocrateobj_load* src = (struct ammocrateobj_load*)(obj);
						struct ammocrateobj dst;
						convertDefaultobj(&dst.base, obj);
						dst.ammotype = swap_int32(src->ammotype);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct ammocrateobj));
						fileWriteOffset += sizeof(struct ammocrateobj);
						break;
					}

					case OBJTYPE_CAMERAPOS: {
						struct cameraposobj* src = (struct cameraposobj*)(obj);
						struct cameraposobj dst;

						dst.type = swap_uint32(src->type);
						dst.x = swap_f32(src->x);
						dst.y = swap_f32(src->y);
						dst.z = swap_f32(src->z);
						dst.theta = swap_f32(src->theta);
						dst.verta = swap_f32(src->verta);
						dst.pad = swap_int32(src->pad);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct cameraposobj));
						fileWriteOffset += sizeof(struct cameraposobj);
						break;
					}

					case OBJTYPE_BEGINOBJECTIVE: {
						struct objective* src = (struct objective*)(obj);
						struct objective dst;
						
						dst.unk00 = src->unk00;
						dst.index = swap_int32(src->index);
						dst.text = swap_uint32(src->text);
						dst.unk0c = swap_uint16(src->unk0c);
						dst.flags = src->flags;
						dst.difficulties = src->difficulties;

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(struct objective));
						fileWriteOffset += sizeof(struct objective);
						break;
					}

					case OBJECTIVETYPE_COMPFLAGS:
					case OBJECTIVETYPE_FAILFLAGS:
					case OBJECTIVETYPE_COLLECTOBJ: {
						/*
							0x00: type
							0x04: flag
						*/
						u32* src = (u32*)(obj);
						u32 dst[2];

						dst[0] = src[0];
						dst[1] = swap_uint32(src[1]);

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(u32) * 2);
						fileWriteOffset += sizeof(u32) * 2;
						break;
					}

					case OBJTYPE_ENDOBJECTIVE: {
						/*
							0x00: type
						*/
						u32* src = (u32*)(obj);
						u32 dst[1];

						dst[0] = src[0];

						u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
						memcpy(writePtr, &dst, sizeof(u32));
						fileWriteOffset += sizeof(u32);
						break;
					}

					default: {
						print("Unhandled OBJTYPE: %02x\n", obj->type);
						fatalExit();
					}
				}


				obj = (struct defaultobj_load *)((uintptr_t)obj + (uintptr_t)setupGetCmdLengthLoad((u32*)obj) * 4);
			}

			{
				// Write objtype end
				struct defaultobj* writePtr = (struct defaultobj*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
				writePtr->type = OBJTYPE_END;
				fileWriteOffset += sizeof(u32);
			}
		}

		/*
			intro
			intro is a pointer to a list of 32 bits commands, calculate the length of the command list
			and copy it in the new structure. We can walk the commands list to find out its size
		*/
		{
			s32* introPtr = (s32 *)((uintptr_t)setup_load + (u32)swap_int32(setup_load->intro));
			u32 introSize = setupCalculateIntroSize(introPtr);
			print("intro size %x bytes\n", introSize * 4);

			addOffsetGlobal(swap_int32(setup_load->intro), fileWriteOffset, 0);
			s32* intro = (s32*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);

			u32* cmd = introPtr;
			for (u32 k = 0; k < introSize; k++)
			{
				s32* writePtr = (s32*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
				*writePtr = swap_int32(*cmd);

				fileWriteOffset += 4;
				cmd++;
			}

			// Write intro end
			{
				s32* writePtr = (s32*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
				*writePtr = INTROCMD_END;
				fileWriteOffset += 4;
			}

			// Then we need to convert the actual content of the intro cmds
			//print("setupConvertIntroCmd introSize: %x\n", introSize);
			//setupConvertIntroCmd(intro);
		}

		g_StageSetup.intro = (s32*)((uintptr_t)setup + (uintptr_t)replaceOffsetGlobal(swap_int32(setup_load->intro)));
		print("- g_StageSetup.intro %llx\n", g_StageSetup.intro);

		g_StageSetup.props = (u32*)((uintptr_t)setup + (uintptr_t)replaceOffsetGlobal(objOffset));
		print("- g_StageSetup.props %llx\n", g_StageSetup.props);

		// Iterate objects again to test if the conversion worked
		{
			struct defaultobj* obj = (struct defaultobj*)(g_StageSetup.props);

			while (obj->type != OBJTYPE_END) {
				//print("Reading OBJTYPE: %02x\n", obj->type);
				obj = (struct defaultobj *)((uintptr_t)obj + (uintptr_t)setupGetCmdLength((u32*)obj) * 4);
			}
		}

		/*
			paths
		*/
		if (setup_load->paths)
		{
			u32 pathsOffset = swap_uint32(setup_load->paths);
			print("pathsOffset: %x\n", pathsOffset);

			addOffsetGlobal(pathsOffset, fileWriteOffset, 0);

			// Parse the list once to copy the path structure
			struct path_load* srcPathList = (struct path_load*)((uintptr_t)setup_load + pathsOffset);
			for (u32 i = 0; swap_uint32(srcPathList[i].pads) != 0; i++) {
				struct path dst;

				dst.pads = (s32*)(uintptr_t)swap_uint32(srcPathList[i].pads);
				dst.id = srcPathList[i].id;
				dst.flags = srcPathList[i].flags;
				dst.len = swap_uint16(srcPathList[i].len);

				u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
				memcpy(writePtr, &dst, sizeof(struct path));
				fileWriteOffset += sizeof(struct path);
			}

			// End the list with a path ending the structure
			{
				struct path dst = { NULL, 0, 0, 0 };
				u8* writePtr = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
				memcpy(writePtr, &dst, sizeof(struct path));
				fileWriteOffset += sizeof(struct path);
			}

			struct path* paths = (struct path*)((uintptr_t)setup + (uintptr_t)replaceOffsetGlobal(pathsOffset));
			for (u32 i = 0; paths[i].pads != NULL; i++) {
				// Copy the pads
				//print("pads: %x\n", paths[i].pads);
				s32* padsSrc = (s32*)((uintptr_t)setup_load + (uintptr_t)paths[i].pads);
				s32* padsDst = (s32*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
				u32 padsLength = 0;

				for (u32 j = 0; swap_int32(padsSrc[j]) >= 0; j++) {
					//print(" %x\n", swap_int32(padsSrc[j]));
					padsDst[j] = swap_int32(padsSrc[j]);
					padsLength++;
				}

				if (padsLength > 0) 
				{
					//addOffsetGlobal(dst.pads, fileWriteOffset, 0);
					paths[i].pads = (s32*)(uintptr_t)fileWriteOffset;
					//print("pads written at: %x\n", paths[i].pads);

					padsDst[padsLength] = -1;
					fileWriteOffset += (padsLength + 1) * sizeof(s32);
				}


			}
			
			g_StageSetup.paths = (struct path*)((uintptr_t)setup + (uintptr_t)replaceOffsetGlobal(pathsOffset));

			// TODO: need to copy the pads as well?
		}

		/*
			ailists
		*/
		print("setup_load->ailists: %x\n", swap_uint32(setup_load->ailists));
		if (setup_load->ailists)
		{
			// First iterate and copy the list
			struct ailist_load* listSrc = (struct ailist_load*)((uintptr_t)setup_load + (uintptr_t)swap_uint32(setup_load->ailists));
			struct ailist* listDst = (struct ailist*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);

			//addOffsetGlobal(swap_uint32(setup_load->ailists), fileWriteOffset, 0);
			g_StageSetup.ailists = (struct ailist *)((uintptr_t)setup + (uintptr_t)fileWriteOffset);

			for (u32 i = 0; listSrc[i].list != NULL; i++) {
				struct ailist ailist = { (u8*)(uintptr_t)swap_uint32(listSrc[i].list), swap_uint32(listSrc[i].id) };
				//print("ailist: %x id %x\n", ailist.list, ailist.id);
				listDst[i] = ailist;
				fileWriteOffset += sizeof(struct ailist);
			}

			{
				// Write last element
				struct ailist ailist = { 0, 0 };
				struct ailist* dst = (struct ailist*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);
				*dst = ailist;
				fileWriteOffset += sizeof(struct ailist);
			}

			// Then we iterate again to copy the actual ailist commands
			for (u32 i = 0; g_StageSetup.ailists[i].list != NULL; i++) {
				//print("ailist2: %x id %x\n", g_StageSetup.ailists[i].list, g_StageSetup.ailists[i].id);

				/*
					We should parse the list of commands and copy it to our new structure
					However, the structure is written in big endian. But the size of each AI cmd fields
					is not explicit. We must refer to the code that uses it to convert each field (see chraicommands.c)
				*/
				u8 *cmd = (u8*)((uintptr_t)setup_load + (uintptr_t)g_StageSetup.ailists[i].list);
				u8 *cmdDst = (u8*)((uintptr_t)setup + (uintptr_t)fileWriteOffset);

				g_StageSetup.ailists[i].list = (u8*)(uintptr_t)fileWriteOffset;

				while (true) {
					s32 type = (cmd[0] << 8) + cmd[1];
					u32 len = chraiGetCommandLength(cmd, 0);
					//print(" - cmd: %04x len %x\n", type, len);

					for (u32 j = 0; j < len; j++) {
						cmdDst[j] = cmd[j];
					}

					// Here we copy the commands but we don't convert the content
					// It's a lot to convert and it has to be done manually...
					//convertChraiCommand(type, cmdDst);

					cmd += len;
					cmdDst += len;
					fileWriteOffset += len;

					if (type == AICMD_END) {
						break;
					}
				}
			}
		}

		nativeFree(originalFileData);
		clearOffsetsContext();

		print("allocsize %x loadedsize %x new size %x\n", allocsize, loadedsize, fileWriteOffset);
		//fileSetSize(filenum, setup, fileWriteOffset, true);

		// --- end of convert

		g_LoadType = LOADTYPE_PADS;

		// TODO: convert padfiledata...
		g_StageSetup.padfiledata = fileLoadToNew(g_Stages[g_StageIndex].padsfileid, FILELOADMETHOD_DEFAULT);

		g_StageSetup.waypoints = NULL;
		g_StageSetup.waygroups = NULL;
		g_StageSetup.cover = NULL;

		// Convert ailist pointers from file-local to proper pointers
		if (g_StageSetup.ailists) {
			for (i = 0; g_StageSetup.ailists[i].list != NULL; i++) {
				g_StageSetup.ailists[i].list = (u8 *)((uintptr_t)setup + (uintptr_t)g_StageSetup.ailists[i].list);
			}
		}

		// Sort the global AI lists by ID asc
		do {
			modified = false;

			for (i = 0; g_GlobalAilists[i + 1].list != NULL; i++) {
				if (g_GlobalAilists[i + 1].id < g_GlobalAilists[i].id) {
					// Swap them
					tmp = g_GlobalAilists[i];
					g_GlobalAilists[i] = g_GlobalAilists[i + 1];
					g_GlobalAilists[i + 1] = tmp;

					modified = true;
				}
			}
		} while (modified);

		// Sort the stage AI lists by ID asc
		do {
			modified = false;

			for (i = 0; g_StageSetup.ailists[i + 1].list != NULL; i++) {
				if (g_StageSetup.ailists[i + 1].id < g_StageSetup.ailists[i].id) {
					// Swap them
					tmp = g_StageSetup.ailists[i];
					g_StageSetup.ailists[i] = g_StageSetup.ailists[i + 1];
					g_StageSetup.ailists[i + 1] = tmp;

					modified = true;
				}
			}
		} while (modified);

		// Count the AI lists
		for (g_NumGlobalAilists = 0; g_GlobalAilists[g_NumGlobalAilists].list != NULL; g_NumGlobalAilists++);
		for (g_NumLvAilists = 0; g_StageSetup.ailists[g_NumLvAilists].list != NULL; g_NumLvAilists++);

		// Convert path pad pointers from file-local to proper pointers
		// and calculate the path lengths
		if (g_StageSetup.paths) {
			for (i = 0; g_StageSetup.paths[i].pads != NULL; i++) {
				g_StageSetup.paths[i].pads = (s32 *)((uintptr_t)g_StageSetup.paths[i].pads + (uintptr_t)setup);

				for (j = 0; g_StageSetup.paths[i].pads[j] >= 0; j++);

				g_StageSetup.paths[i].len = j;
			}
		}

		// Count the number of chrs and objects so enough model slots can be allocated
		numchrs += setupCountCommandType(OBJTYPE_CHR);

		if (!g_Vars.normmplayerisrunning && g_MissionConfig.iscoop && g_Vars.numaibuddies > 0) {
			// @bug? The Hotshot buddy has two guns, but only one is counted here.
			numchrs += g_Vars.numaibuddies;
			numobjs += g_Vars.numaibuddies; // the buddy's weapon
		}

		numobjs += setupCountCommandType(OBJTYPE_WEAPON);
		numobjs += setupCountCommandType(OBJTYPE_KEY);
		numobjs += setupCountCommandType(OBJTYPE_HAT);
		numobjs += setupCountCommandType(OBJTYPE_DOOR);
		numobjs += setupCountCommandType(OBJTYPE_CCTV);
		numobjs += setupCountCommandType(OBJTYPE_AUTOGUN);
		numobjs += setupCountCommandType(OBJTYPE_HANGINGMONITORS);
		numobjs += setupCountCommandType(OBJTYPE_SINGLEMONITOR);
		numobjs += setupCountCommandType(OBJTYPE_MULTIMONITOR);
		numobjs += setupCountCommandType(OBJTYPE_SHIELD);
		numobjs += setupCountCommandType(OBJTYPE_BASIC);
		numobjs += setupCountCommandType(OBJTYPE_DEBRIS);
		numobjs += setupCountCommandType(OBJTYPE_GLASS);
		numobjs += setupCountCommandType(OBJTYPE_TINTEDGLASS);
		numobjs += setupCountCommandType(OBJTYPE_SAFE);
		numobjs += setupCountCommandType(OBJTYPE_29);
		numobjs += setupCountCommandType(OBJTYPE_GASBOTTLE);
		numobjs += setupCountCommandType(OBJTYPE_ALARM);
		numobjs += setupCountCommandType(OBJTYPE_AMMOCRATE);
		numobjs += setupCountCommandType(OBJTYPE_MULTIAMMOCRATE);
		numobjs += setupCountCommandType(OBJTYPE_TRUCK);
		numobjs += setupCountCommandType(OBJTYPE_TANK);
		numobjs += setupCountCommandType(OBJTYPE_LIFT);
		numobjs += setupCountCommandType(OBJTYPE_HOVERBIKE);
		numobjs += setupCountCommandType(OBJTYPE_HOVERPROP);
		numobjs += setupCountCommandType(OBJTYPE_FAN);
		numobjs += setupCountCommandType(OBJTYPE_HOVERCAR);
		numobjs += setupCountCommandType(OBJTYPE_CHOPPER);
		numobjs += setupCountCommandType(OBJTYPE_HELI);
		numobjs += setupCountCommandType(OBJTYPE_ESCASTEP);

		if (g_Vars.normmplayerisrunning) {
			numobjs += scenarioNumProps();
		}

		modelmgrAllocateSlots(numobjs, numchrs);
	} else {
		// cover isn't set to NULL here... I guess it's not important
		g_StageSetup.waypoints = NULL;
		g_StageSetup.waygroups = NULL;
		g_StageSetup.intro = 0;
		g_StageSetup.props = 0;
		g_StageSetup.paths = NULL;
		g_StageSetup.ailists = NULL;
		g_StageSetup.padfiledata = NULL;

		modelmgrAllocateSlots(0, 0);
	}

	if (IS4MB()) {
		extra = 40;
	} else {
		extra = 60;
	}

	if (IS4MB());

	g_Vars.maxprops = numobjs + numchrs + extra + 40;
}

void setupCreateProps(s32 stagenum)
{
	s32 withchrs = !argFindByPrefix(1, "-nochr") && !argFindByPrefix(1, "-noprop");
	s32 withobjs = !argFindByPrefix(1, "-noobj") && !argFindByPrefix(1, "-noprop");
	s32 withhovercars;
	s32 escstepx;
	s32 escstepy;
	struct defaultobj *obj;
	s32 i;
	s32 j;

	withhovercars = !(stagenum == STAGE_EXTRACTION || stagenum == STAGE_DEFECTION)
		|| !(g_Vars.coopplayernum >= 0 || g_Vars.antiplayernum >= 0);

	escstepx = 0;
	escstepy = 0;
	g_Vars.textoverrides = NULL;

	for (j = 0; j != 6; j++) {
		g_Briefing.objectivenames[j] = 0;
		g_Briefing.objectivedifficulties[j] = 0;
	}

	g_Briefing.briefingtextnum = L_MISC_042; // "No briefing for this mission"

	if (stagenum < STAGE_TITLE) {
		if (g_StageSetup.padfiledata) {
			setupPreparePads();
		}

		setupLoadWaypoints();

		if (withchrs) {
			s32 numchrs = 0;

			numchrs += setupCountCommandType(OBJTYPE_CHR);

			if (g_Vars.normmplayerisrunning == false
					&& g_MissionConfig.iscoop
					&& g_Vars.numaibuddies > 0) {
				numchrs += g_Vars.numaibuddies;
			}

			chrmgrConfigure(numchrs);
		} else {
			chrmgrConfigure(0);
		}

		for (j = 0; j < PLAYERCOUNT(); j++) {
			setCurrentPlayerNum(j);
			invInit(setupCountCommandType(OBJTYPE_LINKGUNS));
		}

		if (g_StageSetup.props) {
			u32 diffflag = 0;
			s32 index;

			diffflag |= 1 << (lvGetDifficulty() + 4);

			if (g_Vars.mplayerisrunning) {
				if (PLAYERCOUNT() == 2) {
					diffflag |= OBJFLAG2_EXCLUDE_2P;
				} else if (PLAYERCOUNT() == 3) {
					diffflag |= OBJFLAG2_EXCLUDE_3P;
				} else if (PLAYERCOUNT() == 4) {
					diffflag |= OBJFLAG2_EXCLUDE_4P;
				}
			}

			botmgrRemoveAll();
			index = 0;

			obj = (struct defaultobj *)g_StageSetup.props;

			while (obj->type != OBJTYPE_END) {
				switch (obj->type) {
				case OBJTYPE_GRENADEPROB:
					{
						struct grenadeprobobj *grenadeprob = (struct grenadeprobobj *)obj;
						u8 probability = grenadeprob->probability;
						struct chrdata *chr = chrFindByLiteralId(grenadeprob->chrnum);

						if (chr && chr->prop && chr->model) {
							chr->grenadeprob = probability;
						}
					}
					break;
				case OBJTYPE_CHR:
					if (withchrs) {
						bodyAllocateChr(stagenum, (struct packedchr *) obj, index);
					}
					break;
				case OBJTYPE_DOOR:
					if (withobjs && (obj->flags2 & diffflag) == 0) {
						setupCreateDoor((struct doorobj *)obj, index);
					}
					break;
				case OBJTYPE_DOORSCALE:
					{
						struct doorscaleobj *scale = (struct doorscaleobj *)obj;
						g_DoorScale = scale->scale / 65536.0f;
					}
					break;
				case OBJTYPE_WEAPON:
					if (withchrs && (obj->flags2 & diffflag) == 0) {
						setupPlaceWeapon((struct weaponobj *)obj, index);
					}
					break;
				case OBJTYPE_KEY:
					if (withchrs && (obj->flags2 & diffflag) == 0) {
						setupCreateKey((struct keyobj *)obj, index);
					}
					break;
				case OBJTYPE_HAT:
					if (withchrs && (obj->flags2 & diffflag) == 0) {
						setupCreateHat((struct hatobj *)obj, index);
					}
					break;
				case OBJTYPE_CCTV:
					if (withobjs && (obj->flags2 & diffflag) == 0) {
						setupCreateCctv((struct cctvobj *)obj, index);
					}
					break;
				case OBJTYPE_AUTOGUN:
					if (withobjs && (obj->flags2 & diffflag) == 0) {
						setupCreateAutogun((struct autogunobj *)obj, index);
					}
					break;
				case OBJTYPE_HANGINGMONITORS:
					if (withobjs && (obj->flags2 & diffflag) == 0) {
						setupCreateHangingMonitors((struct hangingmonitorsobj *)obj, index);
					}
					break;
				case OBJTYPE_SINGLEMONITOR:
					if (withobjs && (obj->flags2 & diffflag) == 0) {
						setupCreateSingleMonitor((struct singlemonitorobj *)obj, index);
					}
					break;
				case OBJTYPE_MULTIMONITOR:
					if (withobjs && (obj->flags2 & diffflag) == 0) {
						setupCreateMultiMonitor((struct multimonitorobj *)obj, index);
					}
					break;
				case OBJTYPE_SHIELD:
					if (withobjs) {
#if VERSION >= VERSION_JPN_FINAL
						if ((obj->flags2 & diffflag) == 0)
#else
						if ((obj->flags2 & diffflag) == 0 || g_Jpn)
#endif
						{
							struct shieldobj *shield = (struct shieldobj *)obj;
							shield->initialamount = *(s32 *)&shield->initialamount / 65536.0f;
							shield->amount = shield->initialamount;
							setupCreateObject(obj, index);
						}
					}
					break;
				case OBJTYPE_TINTEDGLASS:
					if (withobjs && (obj->flags2 & diffflag) == 0) {
						if (obj->flags & OBJFLAG_GLASS_HASPORTAL) {
							struct tintedglassobj *glass = (struct tintedglassobj *)obj;
							glass->portalnum = setupGetPortalByPad(obj->pad);
							glass->unk64 = *(s32 *)&glass->unk64 / 65536.0f;
						}

						setupCreateObject(obj, index);
					}
					break;
				case OBJTYPE_LIFT:
					if (withobjs && (obj->flags2 & diffflag) == 0) {
						struct liftobj *lift = (struct liftobj *)obj;
						struct modelstate *modelstate;
						s32 modelnum = obj->modelnum;
						struct prop *prop;
						s32 i;

						lift->accel = PALUPF(*(s32 *)&lift->accel) / 65536.0f;
						lift->maxspeed = PALUPF(*(s32 *)&lift->maxspeed) / 65536.0f;
						lift->dist = 0;
						lift->speed = 0;
						lift->levelcur = 0;
						lift->levelaim = 0;

						for (i = 0; i < 4; i++) {
							if (lift->doors[i]) {
								lift->doors[i] = (struct doorobj *)setupGetCmdByIndex(index + *(s32*)&lift->doors[i]);
							}
						}

						obj->geocount = 1;
						setupLoadModeldef(modelnum);
						modelstate = &g_ModelStates[modelnum];

						if (modelstate->modeldef) {
							if (modelGetPartRodata(modelstate->modeldef, MODELPART_LIFT_WALL1)) {
								obj->geocount++;
							}
							if (modelGetPartRodata(modelstate->modeldef, MODELPART_LIFT_WALL2)) {
								obj->geocount++;
							}
							if (modelGetPartRodata(modelstate->modeldef, MODELPART_LIFT_WALL3)) {
								obj->geocount++;
							}
							if (modelGetPartRodata(modelstate->modeldef, MODELPART_LIFT_DOORBLOCK)) {
								obj->geocount++;
							}
							if (modelGetPartRodata(modelstate->modeldef, MODELPART_LIFT_FLOORNONRECT2)) {
								obj->geocount++;
							}
						}

						obj->flags &= ~OBJFLAG_00000100;

						setupCreateObject(obj, index);

						prop = obj->prop;

						if (prop) {
							lift->prevpos.x = prop->pos.x;
							lift->prevpos.y = prop->pos.y;
							lift->prevpos.z = prop->pos.z;

							liftUpdateTiles(lift, true);
						}
					}
					break;
				case OBJTYPE_HOVERPROP:
					if (withobjs && (obj->flags2 & diffflag) == 0) {
						struct hoverpropobj *hoverprop = (struct hoverpropobj *)obj;

						setupCreateObject(obj, index);
						setupCreateHov(obj, &hoverprop->hov);
					}
					break;
				case OBJTYPE_HOVERBIKE:
					if (withobjs && (obj->flags2 & diffflag) == 0) {
						struct hoverbikeobj *bike = (struct hoverbikeobj *)obj;

						setupCreateObject(obj, index);
						setupCreateHov(obj, &bike->hov);

						bike->speed[0] = 0;
						bike->speed[1] = 0;
						bike->w = 0;
						bike->rels[0] = 0;
						bike->rels[1] = 0;
						bike->exreal = 0;
						bike->ezreal = 0;
						bike->ezreal2 = 0;
						bike->leanspeed = 0;
						bike->leandiff = 0;
						bike->maxspeedtime240 = 0;
						bike->speedabs[0] = 0;
						bike->speedabs[1] = 0;
						bike->speedrel[0] = 0;
						bike->speedrel[1] = 0;
					}
					break;
				case OBJTYPE_FAN:
					if (withobjs && (obj->flags2 & diffflag) == 0) {
						struct fanobj *fan = (struct fanobj *)obj;

						fan->yrot = 0;
						fan->ymaxspeed = PALUPF(*(s32 *)&fan->ymaxspeed) / 65536.0f;
						fan->yaccel = PALUPF(*(s32 *)&fan->yaccel) / 65536.0f;

						setupCreateObject(obj, index);
					}
					break;
				case OBJTYPE_GLASS:
					if (withobjs && (obj->flags2 & diffflag) == 0) {
						if (obj->flags & OBJFLAG_GLASS_HASPORTAL) {
							struct glassobj *glass = (struct glassobj *)obj;
							glass->portalnum = setupGetPortalByPad(obj->pad);
						}

						setupCreateObject(obj, index);
					}
					break;
				case OBJTYPE_ESCASTEP:
					if (withobjs && (obj->flags2 & diffflag) == 0) {
						struct escalatorobj *step = (struct escalatorobj *)obj;
						struct prop *prop;

#ifdef AVOID_UB
						Mtxf sp1a8;
#else
						// TODO: There is a stack problem here that should be
						// resolved. sp1a8 is really an Mtxf which doesn't fit
						// in its current location in the stack.
						f32 sp1a8[12];
#endif
						f32 sp184[3][3];

						setupCreateObject(obj, index);

						prop = obj->prop;

						if (prop) {
							step->prevpos.x = prop->pos.x;
							step->prevpos.y = prop->pos.y;
							step->prevpos.z = prop->pos.z;
						}

						if (obj->flags & OBJFLAG_ESCSTEP_ZALIGNED) {
							step->frame = escstepy;
							escstepy += 40;
							mtx4LoadYRotation(4.7116389274597f, (Mtxf *) &sp1a8);
							mtx4ToMtx3((Mtxf *) &sp1a8, sp184);
							mtx00016110(sp184, obj->realrot);
						} else {
							step->frame = escstepx;
							escstepx += 40;
							mtx4LoadYRotation(M_BADPI, (Mtxf *) &sp1a8);
							mtx4ToMtx3((Mtxf *) &sp1a8, sp184);
							mtx00016110(sp184, obj->realrot);
						}
					}
					break;
				case OBJTYPE_BASIC:
				case OBJTYPE_ALARM:
				case OBJTYPE_AMMOCRATE:
				case OBJTYPE_DEBRIS:
				case OBJTYPE_GASBOTTLE:
				case OBJTYPE_29:
				case OBJTYPE_SAFE:
					if (withobjs && (obj->flags2 & diffflag) == 0) {
						setupCreateObject(obj, index);
					}
					break;
				case OBJTYPE_MULTIAMMOCRATE:
					{
						struct multiammocrateobj *crate = (struct multiammocrateobj *)obj;
						s32 ammoqty = 1;
						s32 i;

						if (g_Vars.normmplayerisrunning && g_SetupCurMpLocation >= 0) {
							struct mpweapon *mpweapon = mpGetMpWeaponByLocation(g_SetupCurMpLocation);
							ammoqty = mpweapon->priammoqty;

							if (mpweapon->priammotype > 0 && mpweapon->priammotype < 20) {
								crate->slots[mpweapon->priammotype - 1].quantity = ammoqty;
							}

							if (mpweapon->secammotype > 0 && mpweapon->secammotype < 20) {
								crate->slots[mpweapon->secammotype - 1].quantity = mpweapon->secammoqty;
							}
						}

						if (ammoqty > 0 && withobjs && (obj->flags2 & diffflag) == 0) {
							for (i = 0; i < ARRAYCOUNT(crate->slots); i++) {
								if (crate->slots[i].quantity > 0 && crate->slots[i].modelnum != 0xffff) {
									setupLoadModeldef(crate->slots[i].modelnum);
								}
							}

							setupCreateObject(obj, index);
						}
					}
					break;
				case OBJTYPE_TRUCK:
					if (withobjs && (obj->flags2 & diffflag) == 0) {
						struct truckobj *truck = (struct truckobj *)obj;

						setupCreateObject(obj, index);

						if (obj->model) {
							struct modelnode *node = modelGetPart(obj->model->definition, MODELPART_TRUCK_0005);

							if (node) {
								// The truck model doesn't exist in PD, so I'm assuming this is a toggle node
								union modelrwdata *rwdata = modelGetNodeRwData(obj->model, node);
								rwdata->toggle.visible = ((obj->flags & OBJFLAG_DEACTIVATED) == 0);
							}
						}

						truck->speed = 0;
						truck->wheelxrot = 0;
						truck->wheelyrot = 0;
						truck->speedaim = 0;
						truck->speedtime60 = -1;
						truck->turnrot60 = 0;
						truck->roty = 0;
						truck->ailist = ailistFindById(truck->ailist);
						truck->aioffset = 0;
						truck->aireturnlist = -1;
						truck->path = NULL;
						truck->nextstep = 0;
					}
					break;
				case OBJTYPE_HOVERCAR:
					if (withhovercars && withobjs && (obj->flags2 & diffflag) == 0) {
						struct hovercarobj *car = (struct hovercarobj *)obj;
						struct prop *prop;

						setupCreateObject(obj, index);

						prop = obj->prop;

						car->speed = 0;
						car->speedaim = 0;
						car->turnrot60 = 0;
						car->roty = 0;
						car->rotx = 0;
						car->speedtime60 = -1;
						car->ailist = ailistFindById(car->ailist);
						car->aioffset = 0;
						car->aireturnlist = -1;
						car->path = NULL;
						car->nextstep = 0;

						if (obj->flags & OBJFLAG_CHOPPER_INACTIVE) {
							prop->pos.y = cdFindFloorYColourTypeAtPos(&prop->pos, prop->rooms, NULL, 0) + 30;
						}

						prop->forcetick = true;
					}
					break;
				case OBJTYPE_CHOPPER:
					if (withobjs && (obj->flags2 & diffflag) == 0) {
						struct chopperobj *chopper = (struct chopperobj *)obj;

						setupCreateObject(obj, index);

						obj->flags |= OBJFLAG_CHOPPER_INIT;
						obj->prop->forcetick = true;

						chopper->turnrot60 = 0;
						chopper->roty = 0;
						chopper->rotx = 0;
						chopper->gunroty = 0;
						chopper->gunrotx = 0;
						chopper->barrelrot = 0;
						chopper->barrelrotspeed = 0;
						chopper->ailist = ailistFindById(chopper->ailist);
						chopper->aioffset = 0;
						chopper->aireturnlist = -1;
						chopper->path = NULL;
						chopper->nextstep = 0;
						chopper->target = -1;
						chopper->targetvisible = false;
						chopper->attackmode = CHOPPERMODE_PATROL;
						chopper->vz = 0;
						chopper->vy = 0;
						chopper->vx = 0;
						chopper->otz = 0;
						chopper->oty = 0;
						chopper->otx = 0;
						chopper->power = 0;
						chopper->bob = 0;
						chopper->bobstrength = 0.05f;
						chopper->timer60 = 0;
						chopper->patroltimer60 = 0;
						chopper->cw = 0;
						chopper->weaponsarmed = true;
						chopper->fireslotthing = mempAlloc(sizeof(struct fireslotthing), MEMPOOL_STAGE);
						chopper->fireslotthing->beam = mempAlloc(ALIGN16(sizeof(struct beam)), MEMPOOL_STAGE);
						chopper->fireslotthing->beam->age = -1;
						chopper->fireslotthing->unk08 = -1;
						chopper->fireslotthing->unk00 = 0;
						chopper->fireslotthing->unk01 = 0;
						chopper->fireslotthing->unk0c = 0.85f;
						chopper->fireslotthing->unk10 = 0.2f;
						chopper->fireslotthing->unk14 = 0;
						chopper->dead = false;
					}
					break;
				case OBJTYPE_HELI:
					if (withobjs && (obj->flags2 & diffflag) == 0) {
						struct heliobj *heli = (struct heliobj *)obj;

						setupCreateObject(obj, index);

						heli->speed = 0;
						heli->speedaim = 0;
						heli->rotoryrot = 0;
						heli->rotoryspeed = 0;
						heli->rotoryspeedaim = 0;
						heli->yrot = 0;
						heli->speedtime60 = -1;
						heli->rotoryspeedtime = -1;
						heli->ailist = ailistFindById(heli->ailist);
						heli->aioffset = 0;
						heli->aireturnlist = -1;
						heli->path = NULL;
						heli->nextstep = 0;
					}
					break;
				case OBJTYPE_TAG:
					{
						struct tag *tag = (struct tag *)obj;
						struct defaultobj *taggedobj = setupGetObjByCmdIndex(index + tag->cmdoffset);
						tag->obj = taggedobj;

						if (taggedobj) {
							taggedobj->hidden |= OBJHFLAG_TAGGED;
						}

						tagInsert(tag);
					}
					break;
				case OBJTYPE_RENAMEOBJ:
					{
						struct textoverride *override = (struct textoverride *)obj;
						struct defaultobj *targetobj = setupGetObjByCmdIndex(override->objoffset + index);
						override->obj = targetobj;

						if (targetobj) {
							targetobj->hidden |= OBJHFLAG_HASTEXTOVERRIDE;
						}

						invInsertTextOverride(override);
					}
					break;
				case OBJTYPE_BRIEFING:
					{
						struct briefingobj *briefing = (struct briefingobj *)obj;
						s32 wanttype = BRIEFINGTYPE_TEXT_PA;

						briefingInsert(briefing);

						if (lvGetDifficulty() == DIFF_A) {
							wanttype = BRIEFINGTYPE_TEXT_A;
						}

						if (lvGetDifficulty() == DIFF_SA) {
							wanttype = BRIEFINGTYPE_TEXT_SA;
						}

						if (briefing->type == wanttype) {
							g_Briefing.briefingtextnum = briefing->text;
						}
					}
					break;
				case OBJTYPE_CAMERAPOS:
					{
						struct cameraposobj *camera = (struct cameraposobj *)obj;
						camera->x = *(s32 *)&camera->x / 100.0f;
						camera->y = *(s32 *)&camera->y / 100.0f;
						camera->z = *(s32 *)&camera->z / 100.0f;
						camera->theta = *(s32 *)&camera->theta / 65536.0f;
						camera->verta = *(s32 *)&camera->verta / 65536.0f;
					}
					break;
				case OBJTYPE_BEGINOBJECTIVE:
					{
						struct objective *objective = (struct objective *)obj;

						objectiveInsert(objective);

						if ((u32)objective->index < 7) {
							g_Briefing.objectivenames[objective->index] = objective->text;
							g_Briefing.objectivedifficulties[objective->index] = objective->difficulties;
						}
					}
					break;
				case OBJECTIVETYPE_ENTERROOM:
					objectiveAddRoomEnteredCriteria((struct criteria_roomentered *)obj);
					break;
				case OBJECTIVETYPE_THROWINROOM:
					objectiveAddThrowInRoomCriteria((struct criteria_throwinroom *)obj);
					break;
				case OBJECTIVETYPE_HOLOGRAPH:
					objectiveAddHolographCriteria((struct criteria_holograph *)obj);
					break;
				case OBJTYPE_PADEFFECT:
					{
						struct padeffectobj *padeffect = (struct padeffectobj *)obj;
						if (g_LastPadEffectIndex == -1) {
							g_PadEffects = padeffect;
						}
						g_LastPadEffectIndex++;
					}
					break;
				case OBJTYPE_MINE:
					if (withobjs && (obj->flags2 & diffflag) == 0) {
						setupCreateMine((struct mineobj *)obj, index);
					}
					break;
				}

				obj = (struct defaultobj *)((u32 *)obj + setupGetCmdLength((u32 *)obj));
				index++;
			}

			index = 0;

			if (g_Vars.normmplayerisrunning && mpHasSimulants()) {
				u32 stack[4];
				s32 i;
				s32 slotsdone[MAX_BOTS];
				s32 chrnum = 0;
				s32 maxsimulants;
				s32 slotnum;

				if (challengeIsFeatureUnlocked(MPFEATURE_8BOTS)) {
					maxsimulants = MAX_BOTS;
				} else {
					maxsimulants = 4;
				}

				for (i = 0; i < MAX_BOTS; i++) {
					slotsdone[i] = false;
				}

				for (i = 0; i < maxsimulants; i++) {
					slotnum = random() % maxsimulants;

					while (slotsdone[slotnum]) {
						slotnum = (slotnum + 1) % maxsimulants;
					}

					if ((g_MpSetup.chrslots & (1 << (slotnum + 4)))
							&& mpIsSimSlotEnabled(slotnum)) {
						botmgrAllocateBot(chrnum, slotnum);
						chrnum++;
					}

					slotsdone[slotnum] = true;
				}
			}

			if (g_Vars.normmplayerisrunning) {
				scenarioInitProps();
			}

			obj = (struct defaultobj *)g_StageSetup.props;

			while (obj->type != OBJTYPE_END) {
				switch (obj->type) {
				case OBJTYPE_BASIC:
				case OBJTYPE_KEY:
				case OBJTYPE_AMMOCRATE:
				case OBJTYPE_WEAPON:
				case OBJTYPE_SINGLEMONITOR:
				case OBJTYPE_DEBRIS:
				case OBJTYPE_MULTIAMMOCRATE:
				case OBJTYPE_SHIELD:
				case OBJTYPE_GASBOTTLE:
				case OBJTYPE_29:
				case OBJTYPE_GLASS:
				case OBJTYPE_SAFE:
				case OBJTYPE_TINTEDGLASS:
					if (obj->prop && (obj->flags & OBJFLAG_INSIDEANOTHEROBJ)) {
						s32 offset = obj->pad;
						struct defaultobj *owner = setupGetObjByCmdIndex(index + offset);

						if (owner && owner->prop) {
							obj->hidden |= OBJHFLAG_HASOWNER;
							modelSetScale(obj->model, obj->model->scale);
							propReparent(obj->prop, owner->prop);
						}
					}
					break;
				case OBJTYPE_LINKGUNS:
					{
						struct linkgunsobj *link = (struct linkgunsobj *)obj;
						struct weaponobj *gun1 = (struct weaponobj *)setupGetCmdByIndex(link->offset1 + index);
						struct weaponobj *gun2 = (struct weaponobj *)setupGetCmdByIndex(link->offset2 + index);

						if (gun1 && gun2
								&& gun1->base.type == OBJTYPE_WEAPON
								&& gun2->base.type == OBJTYPE_WEAPON) {
							propweaponSetDual(gun1, gun2);
						}
					}
					break;
				case OBJTYPE_LINKLIFTDOOR:
					{
						struct linkliftdoorobj *link = (struct linkliftdoorobj *)obj;
						s32 dooroffset = (s32)(uintptr_t)link->door;
						s32 liftoffset = (s32)(uintptr_t)link->lift;
						struct defaultobj *door = setupGetObjByCmdIndex(index + dooroffset);
						struct defaultobj *lift = setupGetObjByCmdIndex(index + liftoffset);

						if (door && door->prop && lift && lift->prop) {
							link->door = door->prop;
							link->lift = lift->prop;

							setupCreateLiftDoor(link);

							door->hidden |= OBJHFLAG_LIFTDOOR;
						}
					}
					break;
				case OBJTYPE_SAFEITEM:
					{
						struct safeitemobj *link = (struct safeitemobj *)obj;
						s32 itemoffset = (s32)(uintptr_t)link->item;
						s32 safeoffset = (s32)(uintptr_t)link->safe;
						s32 dooroffset = (s32)(uintptr_t)link->door;
						struct defaultobj *item = setupGetObjByCmdIndex(index + itemoffset);
						struct defaultobj *safe = setupGetObjByCmdIndex(index + safeoffset);
						struct defaultobj *door = setupGetObjByCmdIndex(index + dooroffset);

						if (item && item->prop
								&& safe && safe->prop && safe->type == OBJTYPE_SAFE
								&& door && door->prop && door->type == OBJTYPE_DOOR) {
							link->item = item;
							link->safe = (struct safeobj *)safe;
							link->door = (struct doorobj *)door;

							setupCreateSafeItem(link);

							item->flags2 |= OBJFLAG2_LINKEDTOSAFE;
							door->flags2 |= OBJFLAG2_LINKEDTOSAFE;
						}
					}
					break;
				case OBJTYPE_PADLOCKEDDOOR:
					{
						struct padlockeddoorobj *link = (struct padlockeddoorobj *)obj;
						s32 dooroffset = (s32)(uintptr_t)link->door;
						s32 lockoffset = (s32)(uintptr_t)link->lock;
						struct defaultobj *door = setupGetObjByCmdIndex(index + dooroffset);
						struct defaultobj *lock = setupGetObjByCmdIndex(index + lockoffset);

						if (door && door->prop && lock && lock->prop
								&& door->type == OBJTYPE_DOOR) {
							link->door = (struct doorobj *)door;
							link->lock = lock;

							setupCreatePadlockedDoor(link);

							door->hidden |= OBJHFLAG_PADLOCKEDDOOR;
						}
					}
					break;
				case OBJTYPE_CONDITIONALSCENERY:
					{
						struct linksceneryobj *link = (struct linksceneryobj *)obj;
						s32 triggeroffset = (s32)(uintptr_t)link->trigger;
						s32 unexpoffset = (s32)(uintptr_t)link->unexp;
						s32 expoffset = (s32)(uintptr_t)link->exp;
						struct defaultobj *trigger = setupGetObjByCmdIndex(index + triggeroffset);
						struct defaultobj *unexp = NULL;
						struct defaultobj *exp = NULL;
						s32 alwayszero = 0;

						if (unexpoffset) {
							unexp = setupGetObjByCmdIndex(index + unexpoffset);
						}

						if (expoffset) {
							exp = setupGetObjByCmdIndex(index + expoffset);
						}

						if (trigger && trigger->prop
								&& (unexpoffset == 0 || (unexp && unexp->prop))
								&& (expoffset == 0 || (exp && exp->prop))) {
							link->trigger = trigger;
							link->unexp = unexp;
							link->exp = exp;

							setupCreateConditionalScenery(link);

							trigger->hidden |= OBJHFLAG_CONDITIONALSCENERY;

							if (unexpoffset) {
								unexp->hidden |= OBJHFLAG_CONDITIONALSCENERY;
							}

							// This gets optimised out but makes v0 unavailable
							// for storing OBJHFLAG_CONDITIONALSCENERY, which is required
							// for a match. Any function call would work.
							if (alwayszero) {
								random();
							}

							if (expoffset) {
								exp->hidden |= OBJHFLAG_CONDITIONALSCENERY;
								exp->flags2 |= OBJFLAG2_INVISIBLE;
							}

							if (trigger->hidden & OBJHFLAG_BLOCKEDPATH) {
								objSetBlockedPathUnblocked(trigger, false);
							}
						}
					}
					break;
				case OBJTYPE_BLOCKEDPATH:
					{
						struct blockedpathobj *blockedpath = (struct blockedpathobj *)obj;
						s32 objoffset = (s32)(uintptr_t)blockedpath->blocker;
						struct defaultobj *blocker = setupGetObjByCmdIndex(index + objoffset);

						if (blocker && blocker->prop) {
							blockedpath->blocker = blocker;

							setupCreateBlockedPath(blockedpath);

							blocker->hidden |= OBJHFLAG_BLOCKEDPATH;

							if (blocker->hidden & OBJHFLAG_CONDITIONALSCENERY) {
								objSetBlockedPathUnblocked(blocker, false);
							}
						}
					}
					break;
				}

				obj = (struct defaultobj *)((u32 *)obj + setupGetCmdLength((u32 *)obj));
				index++;
			}
		}
	} else {
		chrmgrConfigure(0);
	}

	stageAllocateBgChrs();
}
