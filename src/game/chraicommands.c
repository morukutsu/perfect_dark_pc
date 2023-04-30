#include <ultra64.h>
#include "constants.h"
#include "constants.h"
#include "game/bondmove.h"
#include "game/cheats.h"
#include "game/chraction.h"
#include "game/chrai.h"
#include "game/chraicommands.h"
#include "game/debug.h"
#include "game/dlights.h"
#include "game/playerreset.h"
#include "game/title.h"
#include "game/chr.h"
#include "game/prop.h"
#include "game/setuputils.h"
#include "game/propsnd.h"
#include "game/objectives.h"
#include "game/atan2f.h"
#include "game/bondgun.h"
#include "game/game_0b0fd0.h"
#include "game/modelmgr.h"
#include "game/player.h"
#include "game/hudmsg.h"
#include "game/inv.h"
#include "game/playermgr.h"
#include "game/explosions.h"
#include "game/smoke.h"
#include "game/weather.h"
#include "game/bg.h"
#include "game/stagetable.h"
#include "game/env.h"
#include "game/lv.h"
#include "game/music.h"
#include "game/training.h"
#include "game/gamefile.h"
#include "game/lang.h"
#include "game/pad.h"
#include "game/options.h"
#include "game/propobj.h"
#include "game/mpstats.h"
#include "bss.h"
#include "lib/main.h"
#include "lib/model.h"
#include "lib/snd.h"
#include "lib/music.h"
#include "lib/rng.h"
#include "lib/mtx.h"
#include "lib/ailist.h"
#include "lib/anim.h"
#include "lib/lib_317f0.h"
#include "lib/libc/ll.h"
#include "data.h"
#include "types.h"

/**
 * @cmd 0000
 */
bool aiGoToNext(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	osSyncPrintf(" (%d)\n", cmd[2]);

	return false;
}

/**
 * @cmd 0001
 */
bool aiGoToFirst(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, 0, cmd[2]);
	osSyncPrintf(" (%d)\n", cmd[2]);

	return false;
}

/**
 * @cmd 0002
 */
bool aiLabel(void)
{
	g_Vars.aioffset += 3;
	return false;
}

/**
 * @cmd 0003
 */
bool aiYield(void)
{
	g_Vars.aioffset += 2;

	if (g_Vars.chrdata) {
		g_Vars.chrdata->ailist = g_Vars.ailist;
		g_Vars.chrdata->aioffset = g_Vars.aioffset;
	} else if (g_Vars.truck) {
		g_Vars.truck->ailist = g_Vars.ailist;
		g_Vars.truck->aioffset = g_Vars.aioffset;
	} else if (g_Vars.heli) {
		g_Vars.heli->ailist = g_Vars.ailist;
		g_Vars.heli->aioffset = g_Vars.aioffset;
	} else if (g_Vars.hovercar) {
		g_Vars.hovercar->ailist = g_Vars.ailist;
		g_Vars.hovercar->aioffset = g_Vars.aioffset;
	}

	return true;
}

/**
 * @cmd 0005
 */
bool aiSetList(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 ailistid = cmd[4] | (cmd[3] << 8);
	u8 *ailist = ailistFindById(ailistid & 0xffff);

	struct chrdata *chr = g_Vars.chrdata;

	if ((cmd[2] & 0xff) == CHR_SELF) {
		g_Vars.ailist = ailist;
		g_Vars.aioffset = 0;
	} else {
		chr = chrFindById(chr, cmd[2]);

		if (chr) {
			chr->ailist = ailist;
			chr->aioffset = 0;
			chr->sleep = 0;
		}

		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 0006
 */
bool aiSetReturnList(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 ailistid = cmd[4] | (cmd[3] << 8);
	struct chrdata *chr;

	if (g_Vars.chrdata) {
		if (cmd[2] == CHR_SELF) {
			g_Vars.chrdata->aireturnlist = ailistid;
		} else {
			chr = chrFindById(g_Vars.chrdata, cmd[2]);

			if (chr) {
				chr->aireturnlist = ailistid;
			}
		}
	} else if (g_Vars.truck) {
		g_Vars.truck->aireturnlist = ailistid;
	} else if (g_Vars.heli) {
		g_Vars.heli->aireturnlist = ailistid;
	} else if (g_Vars.hovercar) {
		g_Vars.hovercar->aireturnlist = ailistid;
	}

	g_Vars.aioffset += 5;

	return false;
}

/**
 * @cmd 0007
 */
bool aiSetShotList(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 ailistid = cmd[3] | (cmd[2] << 8);

	if (g_Vars.chrdata) {
		g_Vars.chrdata->aishotlist = ailistid;
	} else if (g_Vars.truck) {
		osSyncPrintf("set shot list(void) doesn't work for g_Vars.CommandInfo.truck!\n");
	} else if (g_Vars.heli) {
		osSyncPrintf("set shot list(void) doesn't work for g_Vars.CommandInfo.heli!\n");
	} else if (g_Vars.hovercar) {
		osSyncPrintf("set shot list(void) doesn't work for g_Vars.CommandInfo.hovercar!\n");
	}

	g_Vars.aioffset += 4;

	return false;
}

const char var7f1a9524[] = "BOND IN ROOM \n";
const char var7f1a9534[] = "bond not in room \n";
const char var7f1a9548[] = "aTimer%s%s: %f\n";
const char var7f1a9558[] = "";
const char var7f1a955c[] = "";
const char var7f1a9560[] = "bTimer:%s%s %f\n";
const char var7f1a9570[] = "";
const char var7f1a9574[] = "";
const char var7f1a9578[] = "ai_ifbondintank: tank code has been removed.\n";
const char var7f1a95a8[] = "AI_BONDENABLECONTROL\n";
const char var7f1a95c0[] = "ai(void) enery tune on (%d, %d, %d)\n";
const char var7f1a95e8[] = "ai(void) enery tune off (%d)\n";
const char var7f1a9608[] = "%s%sAI_PRINT(void) height = %f ptop =  %f \n\n";
const char var7f1a9638[] = "";
const char var7f1a963c[] = "";
const char var7f1a9640[] = "Preset character = %d, my target = %d \n";
const char var7f1a9668[] = "characters target is %d \n";
const char var7f1a9684[] = "(DGD)chrai.c(void) aiIfChrsTargetIsChr does not support CHR_BOND as chr1!\n";
const char var7f1a96d0[] = "(DGD)chrai.c(void) aiIfChrsTargetIsChr does not support CHR_BOND as chr1!\n";
const char var7f1a971c[] = "DGD(void) (chrai.c) WARNING! attempt to use invalid CHR_PRESET1 - defaulting to JO\n";
const char var7f1a9770[] = "RISK CHECK = %d \n";
const char var7f1a9784[] = "aiIfOpportunityFireJumpF() called with no botdata!\n";
const char var7f1a97b8[] = "TARGET %d PASSED THE DISTANCE 2 TESTS \n";
const char var7f1a97e0[] = "%d I NOTICE THE TARGET %d\n";
const char var7f1a97fc[] = "%d I NOTICE THE TARGET %d\n";
const char var7f1a9818[] = "%d OPPORTUNITY FIRE A CHARACTER = %d \n";
const char var7f1a9840[] = "%d  CANNOT OPPORTUNITY FIRE A TARGET \n";
const char var7f1a9868[] = " RISK CHECK = %d \n";
const char var7f1a987c[] = "dist = %d \n";
const char var7f1a9888[] = "USING VOICE NUMBER %d \n";
const char var7f1a98a0[] = "SETORDERS COMMAND  \n";
const char var7f1a98b8[] = "SQUADRON MEMBER CHR NUMBER = %d \n";
const char var7f1a98dc[] = "DOING DIST CHECK \n";
const char var7f1a98f0[] = "%d SYNCHED WITH A CHARACTER %d \n";
const char var7f1a9914[] = "synched = %d \n";
const char var7f1a9924[] = "LEADER'S GOING TO COVER \n";
const char var7f1a9940[] = "LEADER IS BREAKING COVER \n";
const char var7f1a995c[] = " I AM SAFE TO BREAK COVER \n";
const char var7f1a9978[] = "not safe to break cover \n";
const char var7f1a9994[] = "LEADER IS COVER SEEN \n";
const char var7f1a99ac[] = "LEADER IS FLANKING LEFT \n";
const char var7f1a99c8[] = "LEADER IS FLANKING RIGHT \n";
const char var7f1a99e4[] = "LEADER IS DODGING \n";
const char var7f1a99f8[] = "LEADER IS THROWING GRENADE \n";
const char var7f1a9a18[] = "LEADER HAS SEEN FROM WAIT \n";
const char var7f1a9a34[] = "UNKNOWN LEADER ACTION \n";
const char var7f1a9a4c[] = "ORDERS CARRIED OUT  ";
const char var7f1a9a64[] = "WITHDRAWING!!!!!!!!!!!\n";
const char var7f1a9a7c[] = "CHECKING MY ORDERS  ";
const char var7f1a9a94[] = "ACTING ON MY ORDERS  ";
const char var7f1a9aac[] = "DOING DIST CHECK VS CHAR %d\n";
const char var7f1a9acc[] = "I'm just carrying out my ORDERS \n";
const char var7f1a9af0[] = "SQUAD IS DOING THAT ACTION\n";
const char var7f1a9b0c[] = "SQUAD NOT DOING ACTION\n";
const char var7f1a9b24[] = "g_Vars.CommandInfo.chrdata ->soundtimer = %d, g_Vars.CommandInfo.chrdata->talkgap = %d , soundgap = %d\n";
const char var7f1a9b8c[] = "RESETTING THE LISTENING \n";
const char var7f1a9ba8[] = "couldn't go to cover from danger !\n";
const char var7f1a9bcc[] = "couldn't find cover from danger !\n";
const char var7f1a9bf0[] = "Randnum = %d,    Alarmrandnum = %d \n";
const char var7f1a9c18[] = "IVE FOUND MY PAD %d \n";

/**
 * @cmd 01c1
 */
bool aiSetPunchDodgeList(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 ailistid = cmd[3] | (cmd[2] << 8);

	if (g_Vars.chrdata) {
		g_Vars.chrdata->aipunchdodgelist = ailistid;
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 01c2
 */
bool aiSetShootingAtMeList(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 ailistid = cmd[3] | (cmd[2] << 8);

	if (g_Vars.chrdata) {
		g_Vars.chrdata->aishootingatmelist = ailistid;
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 01c3
 */
bool aiSetDarkRoomList(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 ailistid = cmd[3] | (cmd[2] << 8);

	if (g_Vars.chrdata) {
		g_Vars.chrdata->aidarkroomlist = ailistid;
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 01c4
 */
bool aiSetPlayerDeadList(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 ailistid = cmd[3] | (cmd[2] << 8);

	if (g_Vars.chrdata) {
		g_Vars.chrdata->aiplayerdeadlist = ailistid;
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 0008
 */
bool aiReturn(void)
{
	u8 *ailist = NULL;

	if (g_Vars.chrdata) {
		ailist = ailistFindById(g_Vars.chrdata->aireturnlist);
	} else if (g_Vars.truck) {
		ailist = ailistFindById(g_Vars.truck->aireturnlist);
	} else if (g_Vars.heli) {
		ailist = ailistFindById(g_Vars.heli->aireturnlist);
	} else if (g_Vars.hovercar) {
		ailist = ailistFindById(g_Vars.hovercar->aireturnlist);
	}

	g_Vars.ailist = ailist;
	g_Vars.aioffset = 0;

	return false;
}

/**
 * @cmd 0004
 */
bool aiEndList(void)
{
	s32 ailistid;
	bool is_global;
	ailistid = chraiGetListIdByList(g_Vars.ailist, &is_global);

	return true;
}

/**
 * @cmd 0009
 */
bool aiStop(void)
{
	if (g_Vars.chrdata) {
		chrTryStop(g_Vars.chrdata);
	} else if (g_Vars.hovercar) {
		chopperStop(g_Vars.hovercar);
	}

	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 000a
 */
bool aiKneel(void)
{
	chrTryKneel(g_Vars.chrdata);
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 000b
 */
bool aiChrDoAnimation(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 anim_id = cmd[3] | (cmd[2] << 8);
	s32 startframe = cmd[5] | (cmd[4] << 8);
	s32 endframe = cmd[7] | (cmd[6] << 8);
	struct chrdata *chr = NULL;
	f32 fstartframe;
	f32 fendframe;

	if (g_Vars.chrdata) {
		chr = chrFindById(g_Vars.chrdata, cmd[10]);
	}

	if (startframe == 0xffff) {
		fstartframe = 0;
	} else if (startframe == 0xfffe) {
		fstartframe = animGetNumFrames(anim_id) - 1;
	} else {
		fstartframe = startframe;
	}

	if (endframe == 0xffff) {
		fendframe = -1.0f;
	} else {
		fendframe = endframe;
	}

	if (chr && chr->model) {
		f32 speed = 1.0f / (s32)cmd[11];

		if (g_Vars.in_cutscene) {
			if (startframe != 0xfffe) {
#if PAL
				fstartframe += var8009e388pf * speed;
#else
				fstartframe += g_CutsceneFrameOverrun240 * speed * 0.25f;
#endif
			}

			chr->prop->propupdate240 = 0;
		}

		chrTryStartAnim(chr, anim_id, fstartframe, fendframe, cmd[8], cmd[9], speed);

		if (startframe == 0xfffe) {
			chr0f0220ec(chr, 1, true);

			if (chr->prop->type == PROPTYPE_PLAYER) {
				u32 playernum = playermgrGetPlayerNumByProp(chr->prop);
				struct player *player = g_Vars.players[playernum];
				player->vv_ground = chr->ground;
				player->vv_manground = chr->ground;
			}
		}
	}

	g_Vars.aioffset += 12;

	return false;
}

/**
 * @cmd 000c
 */
bool aiIfIdle(void)
{
	if (g_Vars.chrdata->actiontype == ACT_ANIM) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

bool func0f04e418(void)
{
	return false;
}

/**
 * @cmd 000d
 */
bool aiBeSurprisedOneHand(void)
{
	chrTrySurprisedOneHand(g_Vars.chrdata);
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 000e
 */
bool aiBeSurprisedLookAround(void)
{
	chrTrySurprisedLookAround(g_Vars.chrdata);
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 0032
 */
bool aiIfStopped(void)
{
	if (chrIsStopped(g_Vars.chrdata)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0033
 */
bool aiIfChrDead(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if ((!chr || !chr->prop || chr->prop->type != PROPTYPE_PLAYER) && (!chr || !chr->model || chrIsDead(chr))) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0034
 */
bool aiIfChrDeathAnimationFinished(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);
	bool pass;

	if (!chr || !chr->prop) {
		pass = true;
	} else {
		if (chr->prop->type == PROPTYPE_PLAYER) {
			u32 playernum = playermgrGetPlayerNumByProp(chr->prop);
			pass = g_Vars.players[playernum]->isdead;
		} else {
			pass = (chr->actiontype == ACT_DEAD);
		}
	}

	if (pass) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 017b
 */
bool aiIfChrKnockedOut(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if ((!chr || !chr->prop || chr->prop->type != PROPTYPE_PLAYER) &&
			(!chr || !chr->model || chr->actiontype == ACT_DRUGGEDKO || chr->actiontype == ACT_DRUGGEDDROP || chr->actiontype == ACT_DRUGGEDCOMINGUP)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0035
 */
bool aiIfCanSeeTarget(void)
{
	if (chrCheckCanSeeTarget(g_Vars.chrdata)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 000f
 */
bool aiTrySidestep(void)
{
	if (chrTrySidestep(g_Vars.chrdata)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0010
 */
bool aiTryJumpOut(void)
{
	if (chrTryJumpOut(g_Vars.chrdata)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0011
 */
bool aiTryRunSideways(void)
{
	if (chrTryRunSideways(g_Vars.chrdata)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0012
 */
bool aiTryAttackWalk(void)
{
	if (chrTryAttackWalk(g_Vars.chrdata)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0013
 */
bool aiTryAttackRun(void)
{
	if (chrTryAttackRun(g_Vars.chrdata)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0014
 */
bool aiTryAttackRoll(void)
{
	if (chrTryAttackRoll(g_Vars.chrdata)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0015
 */
bool aiTryAttackStand(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 thingid = cmd[5] | (cmd[4] << 8);
	u32 thingtype = cmd[3] | (cmd[2] << 8);

	if (chrTryAttackStand(g_Vars.chrdata, thingtype, thingid)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[6]);
	} else {
		g_Vars.aioffset += 7;
	}

	return false;
}

/**
 * @cmd 0016
 */
bool aiTryAttackKneel(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 thingid = cmd[5] | (cmd[4] << 8);
	u32 thingtype = cmd[3] | (cmd[2] << 8);

	if (chrTryAttackKneel(g_Vars.chrdata, thingtype, thingid)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[6]);
	} else {
		g_Vars.aioffset += 7;
	}

	return false;
}

/**
 * @cmd 01ba
 */
bool aiTryAttackLie(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 thingid = cmd[5] | (cmd[4] << 8);
	u32 thingtype = cmd[3] | (cmd[2] << 8);

	if (chrTryAttackLie(g_Vars.chrdata, thingtype, thingid)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[6]);
	} else {
		g_Vars.aioffset += 7;
	}

	return false;
}

/**
 * @cmd 00f0
 */
bool ai00f0(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (g_Vars.chrdata->actiontype == ACT_ATTACK &&
			!g_Vars.chrdata->act_attack.reaim &&
			g_Vars.chrdata->act_attack.flags & ATTACKFLAG_DONTTURN) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 00f1
 */
bool aiIfAttacking(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (g_Vars.chrdata->actiontype == ACT_ATTACK) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0017
 */
bool aiTryModifyAttack(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 thingid = cmd[5] | (cmd[4] << 8);
	u32 thingtype = cmd[3] | (cmd[2] << 8);

	if ((g_Vars.chrdata && chrTryModifyAttack(g_Vars.chrdata, thingtype, thingid)) ||
			(g_Vars.hovercar && chopperAttack(g_Vars.hovercar))) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[6]);
	} else {
		g_Vars.aioffset += 7;
	}

	return false;
}

/**
 * @cmd 0018
 */
bool aiFaceEntity(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 thingid = cmd[5] | (cmd[4] << 8);
	u32 thingtype = cmd[3] | (cmd[2] << 8);

	if (chrFaceEntity(g_Vars.chrdata, thingtype, thingid)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[6]);
	} else {
		g_Vars.aioffset += 7;
	}

	return false;
}

/**
 * @cmd 0019
 */
bool ai0019(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);
	struct coord pos = {0, 0, 0};

	if (chr && chr->prop) {
		f32 damage = gsetGetDamage((struct gset *)&cmd[4]);
		chrDamageByImpact(chr, damage, &pos, (struct gset *)&cmd[4], NULL, (s8)cmd[3]);
	}

	g_Vars.aioffset += 8;

	return false;
}

/**
 * @cmd 001a
 */
bool aiChrDamageChr(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr1 = chrFindById(g_Vars.chrdata, cmd[2]);
	struct chrdata *chr2 = chrFindById(g_Vars.chrdata, cmd[3]);

	if (chr1 && chr2 && chr1->prop && chr2->prop) {
		struct prop *prop = chrGetHeldUsableProp(chr1, HAND_RIGHT);
		f32 damage;
		struct coord vector = {0, 0, 0};
		struct weaponobj *weapon;

		if (!prop) {
			prop = chrGetHeldUsableProp(chr1, HAND_LEFT);
		}

		if (prop) {
			vector.x = chr2->prop->pos.x - chr1->prop->pos.x;
			vector.y = chr2->prop->pos.y - chr1->prop->pos.y;
			vector.z = chr2->prop->pos.z - chr1->prop->pos.z;
			guNormalize(&vector.x, &vector.y, &vector.z);
			weapon = prop->weapon;
			damage = gsetGetDamage(&weapon->gset);
			chrDamageByImpact(chr2, damage, &vector, &weapon->gset, chr1->prop, (s8)cmd[4]);
		}
	}

	g_Vars.aioffset += 5;

	return false;
}

/**
 * @cmd 001b
 */
bool aiConsiderGrenadeThrow(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 value2 = cmd[5] | (cmd[4] << 8);
	u32 value1 = cmd[3] | (cmd[2] << 8);

	if (chrConsiderGrenadeThrow(g_Vars.chrdata, value1, value2)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[6]);
	} else {
		g_Vars.aioffset += 7;
	}

	return false;
}

/**
 * @cmd 001c
 */
bool aiDropItem(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 modelnum = cmd[3] | (cmd[2] << 8);

	if (chrDropItem(g_Vars.chrdata, modelnum & 0xffff, cmd[4] & 0xff)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[5]);
	} else {
		g_Vars.aioffset += 6;
	}

	return false;
}

/**
 * @cmd 0024
 */
bool aiSurrender(void)
{
	chrTrySurrender(g_Vars.chrdata);
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 0025
 */
bool aiFadeOut(void)
{
	chrFadeOut(g_Vars.chrdata);
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 0026
 */
bool aiRemoveChr(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop) {
		chr->hidden |= 0x20;
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0027
 */
bool aiTryStartAlarm(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 pad_id = cmd[3] | (cmd[2] << 8);

	if (chrTryStartAlarm(g_Vars.chrdata, pad_id)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 0028
 */
bool aiActivateAlarm(void)
{
	alarmActivate();
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 0029
 */
bool aiDeactivateAlarm(void)
{
	alarmDeactivate();
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 002a
 */
bool aiTryRunFromTarget(void)
{
	if (chrTryRunFromTarget(g_Vars.chrdata)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 002b
 */
bool aiTryJogToTargetProp(void)
{
	if (chrGoToTarget(g_Vars.chrdata, GOPOSFLAG_JOG)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 002c
 */
bool aiTryWalkToTargetProp(void)
{
	if (chrGoToTarget(g_Vars.chrdata, GOPOSFLAG_WALK)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 002d
 */
bool aiTryRunToTargetProp(void)
{
	if (chrGoToTarget(g_Vars.chrdata, GOPOSFLAG_RUN)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 002e
 */
bool aiTryGoToCoverProp(void)
{
	if (chrGoToCoverProp(g_Vars.chrdata)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist,g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 002f
 */
bool aiTryJogToChr(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (chrGoToChr(g_Vars.chrdata, cmd[2], GOPOSFLAG_JOG)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0030
 */
bool aiTryWalkToChr(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (chrGoToChr(g_Vars.chrdata, cmd[2], GOPOSFLAG_WALK)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0031
 */
bool aiTryRunToChr(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (chrGoToChr(g_Vars.chrdata, cmd[2], GOPOSFLAG_RUN)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0036
 */
bool aiRandom(void)
{
	g_Vars.chrdata->random = random() & 0xff;
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 0037
 */
bool aiIfRandomLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if ((g_Vars.chrdata && g_Vars.chrdata->random < cmd[2]) ||
			(g_Vars.hovercar && ((u8)random()) < cmd[2])) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0038
 */
bool aiIfRandomGreaterThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if ((g_Vars.chrdata && g_Vars.chrdata->random > cmd[2]) ||
			(g_Vars.hovercar && ((u8)random()) > cmd[2])) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 001d
 */
bool aiJogToPad(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 pad = cmd[3] | (cmd[2] << 8);

	chrGoToPad(g_Vars.chrdata, pad, GOPOSFLAG_JOG);
	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 001e
 */
bool aiGoToPadPreset(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	switch (cmd[2]) {
	case 0:
		chrGoToPad(g_Vars.chrdata, g_Vars.chrdata->padpreset1, GOPOSFLAG_WALK);
		break;
	case 1:
		chrGoToPad(g_Vars.chrdata, g_Vars.chrdata->padpreset1, GOPOSFLAG_JOG);
		break;
	default:
		chrGoToPad(g_Vars.chrdata, g_Vars.chrdata->padpreset1, GOPOSFLAG_RUN);
		break;
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 001f
 */
bool aiWalkToPad(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 pad = cmd[3] | (cmd[2] << 8);

	chrGoToPad(g_Vars.chrdata, pad, GOPOSFLAG_WALK);
	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 0020
 */
bool aiRunToPad(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 pad = cmd[3] | (cmd[2] << 8);

	chrGoToPad(g_Vars.chrdata, pad, GOPOSFLAG_RUN);
	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 0021
 */
bool aiSetPath(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	chrSetPath(g_Vars.chrdata, cmd[2]);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0022
 */
bool aiStartPatrol(void)
{
	chrTryStartPatrol(g_Vars.chrdata);
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 0039
 */
bool aiIfCanHearAlarm(void)
{
	if (chrCanHearAlarm(g_Vars.chrdata)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0023
 */
bool aiIfPatrolling(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (g_Vars.chrdata->actiontype == ACT_PATROL
			|| (g_Vars.chrdata->actiontype == ACT_GOPOS && g_Vars.chrdata->act_gopos.flags & GOPOSFLAG_FORPATHSTART)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 003a
 */
bool aiIfAlarmActive(void)
{
	if (alarmIsActive()) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 003b
 */
bool aiIfGasActive(void)
{
	if (gasIsActive()) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 003c
 */
bool aiIfHearsTarget(void)
{
	if (chrIsHearingTarget(g_Vars.chrdata)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 003d
 */
bool aiIfSawInjury(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (chrSawInjury(g_Vars.chrdata, cmd[2])) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 003e
 */
bool aiIfSawDeath(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (chrSawDeath(g_Vars.chrdata, cmd[2])) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 003f
 */
bool aiIfLosToTarget(void)
{
	if ((g_Vars.chrdata && chrHasLosToTarget(g_Vars.chrdata)) ||
			(g_Vars.hovercar && chopperCheckTargetInFov(g_Vars.hovercar, 64) && chopperCheckTargetInSight(g_Vars.hovercar))) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 017a
 */
bool aiIfLosToAttackTarget(void)
{
	if ((g_Vars.chrdata && g_Vars.chrdata->prop && chrHasLosToAttackTarget(g_Vars.chrdata, &g_Vars.chrdata->prop->pos, g_Vars.chrdata->prop->rooms, true))
			|| (g_Vars.hovercar && chopperCheckTargetInFov(g_Vars.hovercar, 64) && chopperCheckTargetInSight(g_Vars.hovercar))) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0040
 */
bool aiIfTargetNearlyInSight(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 distance = (cmd[3] << 16) | (cmd[4] << 8) | cmd[5] | (cmd[2] << 24);

	if (chrIsTargetNearlyInSight(g_Vars.chrdata, distance)) {
		cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[6]);
	} else {
		g_Vars.aioffset += 7;
	}

	return false;
}

/**
 * @cmd 0041
 */
bool aiIfNearlyInTargetsSight(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 distance = (cmd[3] << 16) | (cmd[4] << 8) | cmd[5] | (cmd[2] << 24);

	if (chrIsNearlyInTargetsSight(g_Vars.chrdata, distance)) {
		cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[6]);
	} else {
		g_Vars.aioffset += 7;
	}

	return false;
}

/**
 * @cmd 0042
 */
bool aiSetPadPresetToPadOnRouteToTarget(void)
{
	if (chrSetPadPresetToPadOnRouteToTarget(g_Vars.chrdata)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0043
 */
bool aiIfSawTargetRecently(void)
{
	if (chrSawTargetRecently(g_Vars.chrdata)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0044
 */
bool aiIfHeardTargetRecently(void)
{
	if (chrHeardTargetRecently(g_Vars.chrdata)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0045
 */
bool aiIfLosToChr(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && chrHasLosToPos(g_Vars.chrdata, &chr->prop->pos, chr->prop->rooms)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0046
 */
bool aiIfNeverBeenOnScreen(void)
{
	if ((g_Vars.chrdata->chrflags & CHRCFLAG_EVERONSCREEN) == 0) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0047
 */
bool aiIfOnScreen(void)
{
	if (g_Vars.chrdata->prop->flags & (PROPFLAG_ONTHISSCREENTHISTICK | PROPFLAG_ONANYSCREENTHISTICK | PROPFLAG_ONANYSCREENPREVTICK)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0048
 */
bool aiIfChrInOnScreenRoom(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);
	u8 pass = false;
	s32 i;

	if (chr && chr->prop) {
		for (i = 0; chr->prop->rooms[i] != -1; i++) {
			if (roomIsOnscreen(chr->prop->rooms[i])) {
				pass = true;
			}
		}
	}

	if (pass) {
		cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0049
 */
bool aiIfRoomIsOnScreen(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 pad_id = cmd[3] | (cmd[2] << 8);
	s32 room_id = chrGetPadRoom(g_Vars.chrdata, pad_id);

	if (room_id >= 0 && roomIsOnscreen(room_id)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 004a
 */
bool aiIfTargetAimingAtMe(void)
{
	if (chrIsTargetAimingAtMe(g_Vars.chrdata)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 004b
 */
bool aiIfNearMiss(void)
{
	if (chrResetNearMiss(g_Vars.chrdata)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 004c
 */
bool aiIfSeesSuspiciousItem(void)
{
	s16 *ptr;
	ubool pass = false;
	struct prop *prop;
	struct defaultobj *obj;
	s16 propnums[256];
	struct prop *chrprop = g_Vars.chrdata->prop;

	roomGetProps(chrprop->rooms, &propnums[0], 256);

	ptr = &propnums[0];

	while (*ptr >= 0) {
		prop = &g_Vars.props[*ptr];
		obj = prop->obj;

		if (prop->type == PROPTYPE_WEAPON) {
			if ((obj->hidden & OBJHFLAG_SUSPICIOUS) && chrHasLosToProp(g_Vars.chrdata, prop)) {
				pass = true;
			}
		} else if (prop->type == PROPTYPE_OBJ) {
			if (((obj->hidden & OBJHFLAG_SUSPICIOUS) || !objIsHealthy(obj))
					&& chrHasLosToProp(g_Vars.chrdata, prop)) {
				pass = true;
			}
		} else if (prop->type == PROPTYPE_EXPLOSION) {
			if (chrHasLosToProp(g_Vars.chrdata, prop)) {
				pass = true;
			}
		}

		ptr++;
	}

	if (pass) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 004e
 */
bool aiIfCheckFovWithTarget(void)
{
	bool pass;
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (cmd[4] == 0) {
		if (cmd[3]) {
			pass = chrIsInTargetsFovX(g_Vars.chrdata, cmd[2]);
		} else {
			pass = chrIsVerticalAngleToTargetWithin(g_Vars.chrdata, cmd[2]);
		}
	} else {
		pass = g_Vars.chrdata->yvisang && chrIsVerticalAngleToTargetWithin(g_Vars.chrdata, g_Vars.chrdata->yvisang) == 0;
	}

	if (pass) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[5]);
	} else {
		g_Vars.aioffset += 6;
	}

	return false;
}

/**
 * @cmd 004d
 */
bool aiIfTargetInFovLeft(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (chrGetAngleToTarget(g_Vars.chrdata) < cmd[2] * M_BADTAU * 0.00390625f) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 004f
 */
bool aiIfTargetOutOfFovLeft(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (chrGetAngleToTarget(g_Vars.chrdata) > cmd[2] * M_BADTAU * 0.00390625f) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0050
 */
bool aiIfTargetInFov(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (chrIsTargetInFov(g_Vars.chrdata, cmd[2], 0)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0051
 */
bool aiIfTargetOutOfFov(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (!chrIsTargetInFov(g_Vars.chrdata, cmd[2], 0)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0052
 */
bool aiIfDistanceToTargetLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 distance = (cmd[3] | (cmd[2] << 8)) * (f32)10;

	if (chrGetDistanceToTarget(g_Vars.chrdata) < distance) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 0053
 */
bool aiIfDistanceToTargetGreaterThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 distance = (cmd[3] | (cmd[2] << 8)) * (f32)10;

	if (chrGetDistanceToTarget(g_Vars.chrdata) > distance) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 0054
 */
bool aiIfChrDistanceToPadLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

#if VERSION >= VERSION_NTSC_1_0
	s32 padnum = (cmd[6] | (cmd[5] << 8)) & 0xffff;
	f32 value = (cmd[4] | (cmd[3] << 8)) * 10.0f;
	s32 realpadnum = padnum;

	if (padnum == 9000) {
		realpadnum = (u16) g_Vars.chrdata->padpreset1;
	}

	if (chr && realpadnum < 9000 && chrGetDistanceToPad(chr, realpadnum) < value) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[7]);
	} else {
		g_Vars.aioffset += 8;
	}
#else
	u16 padnum = cmd[6] | (cmd[5] << 8);
	f32 value = (cmd[4] | (cmd[3] << 8)) * 10.0f;

	if (padnum == 9000) {
		padnum = g_Vars.chrdata->padpreset1;
	}

	if (chr && chrGetDistanceToPad(chr, padnum) < value) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[7]);
	} else {
		g_Vars.aioffset += 8;
	}
#endif

	return false;
}

#if VERSION >= VERSION_NTSC_1_0
/**
 * @cmd 01df
 */
bool aiIfChrSameFloorDistanceToPadLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);
	u16 padnum = cmd[6] | (cmd[5] << 8);
	f32 distance = (cmd[4] | (cmd[3] << 8)) * 10.0f;
	s32 padnum2;

	if (padnum == 9000) {
		padnum = g_Vars.chrdata->padpreset1;
	}

	padnum2 = padnum;

	if (chr && chrGetSameFloorDistanceToPad(chr, padnum2 & 0xffffffff) < distance) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[7]);
	} else {
		g_Vars.aioffset += 8;
	}

	return false;
}
#endif

/**
 * @cmd 0055
 */
bool aiIfChrDistanceToPadGreaterThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);
	u16 padnum = cmd[6] | (cmd[5] << 8);
	f32 distance = (cmd[4] | (cmd[3] << 8)) * 10.0f;

	if (padnum == 9000) {
		padnum = g_Vars.chrdata->padpreset1;
	}

#if VERSION >= VERSION_NTSC_1_0
	if (chr && padnum < 9000 && chrGetDistanceToPad(chr, padnum) > distance)
#else
	if (chr && chrGetDistanceToPad(chr, padnum) > distance)
#endif
	{
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[7]);
	} else {
		g_Vars.aioffset += 8;
	}

	return false;
}

/**
 * @cmd 0056
 */
bool aiIfDistanceToChrLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 cutoff = (cmd[3] | (cmd[2] << 8)) * 10.0f;

	if (chrGetDistanceToChr(g_Vars.chrdata, cmd[4]) < cutoff) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[5]);
	} else {
		g_Vars.aioffset += 6;
	}

	return false;
}

/**
 * @cmd 0057
 */
bool aiIfDistanceToChrGreaterThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 cutoff = (cmd[3] | (cmd[2] << 8)) * 10.0f;

	if (chrGetDistanceToChr(g_Vars.chrdata, cmd[4]) > cutoff) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[5]);
	} else {
		g_Vars.aioffset += 6;
	}

	return false;
}

/**
 * @cmd 0058
 */
bool ai0058(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 distance = (cmd[3] | (cmd[2] << 8)) * 10.0f;

	if (chrSetChrPresetToAnyChrNearSelf(g_Vars.chrdata, distance)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 0059
 */
bool aiIfDistanceFromTargetToPadLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 pad = cmd[5] | (cmd[4] << 8);
	f32 value = (cmd[3] | (cmd[2] << 8)) * 10.0f;

	if (chrGetDistanceFromTargetToPad(g_Vars.chrdata, pad) < value) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[6]);
	} else {
		g_Vars.aioffset += 7;
	}

	return false;
}

/**
 * @cmd 005a
 */
bool aiIfDistanceFromTargetToPadGreaterThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 pad = cmd[5] | (cmd[4] << 8);
	f32 value = (cmd[3] | (cmd[2] << 8)) * 10.0f;

	if (chrGetDistanceFromTargetToPad(g_Vars.chrdata, pad) > value) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[6]);
	} else {
		g_Vars.aioffset += 7;
	}

	return false;
}

/**
 * @cmd 005b
 */
bool aiIfChrInRoom(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);
	u16 pad_id = cmd[5] | (cmd[4] << 8);
	s32 room = chrGetPadRoom(g_Vars.chrdata, pad_id);

	if ((cmd[3] == 0 && room >= 0 && chr && chr->prop && chr->prop->rooms[0] == room)
			|| (cmd[3] == 1 && chr && chr->prop && chr->prop->rooms[0] == g_Vars.chrdata->roomtosearch)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[6]);
		return false;
	}

	if (cmd[3] == 2 && stageGetIndex(g_Vars.stagenum) == STAGEINDEX_G5BUILDING) {
		s32 i;
		bool pass = false;

		for (i = 0; i < PLAYERCOUNT(); i++) {
			if (g_Vars.players[i]->eyespy && g_Vars.players[i]->eyespy->prop
#if VERSION >= VERSION_NTSC_1_0
					&& chrGetDistanceToPad(g_Vars.players[i]->eyespy->prop->chr, pad_id) < 150.0f
#else
					&& chrGetDistanceToPad(g_Vars.players[i]->eyespy->prop->chr, 0x4d) < 150.0f
#endif
					) {
				pass = true;
			}
		}

		if (pass == true) {
			g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[6]);
		} else {
			g_Vars.aioffset += 7;
		}

		return false;
	}

	g_Vars.aioffset += 7;

	return false;
}

/**
 * @cmd 005c
 */
bool aiIfTargetInRoom(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct prop *prop = chrGetTargetProp(g_Vars.chrdata);
	u16 pad_id = cmd[3] | (cmd[2] << 8);
	s32 room_id = chrGetPadRoom(g_Vars.chrdata, pad_id);

	if (room_id >= 0 && prop && room_id == prop->rooms[0]) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 005d
 */
bool aiIfChrHasObject(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[3]);
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);
	s32 hasprop = false;

	if (obj && obj->prop && chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		s32 prevplayernum = g_Vars.currentplayernum;
		setCurrentPlayerNum(playermgrGetPlayerNumByProp(chr->prop));
		hasprop = invHasProp(obj->prop);
		setCurrentPlayerNum(prevplayernum);
	}

	if (hasprop) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 005e
 */
bool aiIfWeaponThrown(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (weaponFindLanded(cmd[2])) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 005f
 */
bool aiIfWeaponThrownOnObject(void)
{
	struct bytelist *cmd = (struct bytelist *)(g_Vars.ailist + g_Vars.aioffset);
	struct defaultobj *obj = objFindByTagId(cmd->b3);
	bool pass = false;

	if (obj && obj->prop) {
		struct prop *prop = obj->prop->child;

		while (prop) {
			if (prop->type == PROPTYPE_WEAPON) {
				struct weaponobj *weapon = prop->weapon;

				if (weapon->weaponnum == cmd->b2) {
					pass = true;
				}
			}

			prop = prop->next;
		}
	}

	if (pass) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd->b4);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 0060
 */
bool aiIfChrHasWeaponEquipped(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);
	bool passes = false;

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		u32 prevplayernum = g_Vars.currentplayernum;
		u32 playernum = playermgrGetPlayerNumByProp(chr->prop);
		setCurrentPlayerNum(playernum);

		if (bgunGetWeaponNum(HAND_RIGHT) == cmd[3]) {
			passes = true;
		}

		setCurrentPlayerNum(prevplayernum);
	}

	if (passes) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 0061
 */
bool aiIfGunUnclaimed(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (cmd[3] == 0) {
		struct defaultobj *obj = objFindByTagId(cmd[2]);

		if (obj && obj->prop) {
			g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
		} else {
			g_Vars.aioffset += 5;
		}
	} else {
		struct weaponobj *weapon = g_Vars.chrdata->gunprop->weapon;

		if (weapon && weapon->base.prop) {
			weapon->base.flags |= OBJFLAG_FORCENOBOUNCE;
			g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
		} else {
			g_Vars.aioffset += 5;
		}
	}

	return false;
}

/**
 * @cmd 0062
 */
bool aiIfObjectHealthy(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop && objIsHealthy(obj)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0063
 */
bool aiIfChrActivatedObject(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[3]);
	u32 stack[1];
	bool pass = false;

	if (obj && obj->prop) {
		if (cmd[2] == CHR_ANY) {
			if (obj->hidden & (OBJHFLAG_ACTIVATED_BY_BOND | OBJHFLAG_ACTIVATED_BY_COOP)) {
				pass = true;
				obj->hidden &= ~(OBJHFLAG_ACTIVATED_BY_BOND | OBJHFLAG_ACTIVATED_BY_COOP);
			}
		} else {
			struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

			if (chr && chr->prop) {
				if (chr->prop == g_Vars.bond->prop && (obj->hidden & OBJHFLAG_ACTIVATED_BY_BOND)) {
					pass = true;
					obj->hidden &= ~OBJHFLAG_ACTIVATED_BY_BOND;
				} else if (g_Vars.coopplayernum >= 0 && chr->prop == g_Vars.coop->prop && (obj->hidden & OBJHFLAG_ACTIVATED_BY_COOP)) {
					pass = true;
					obj->hidden &= ~OBJHFLAG_ACTIVATED_BY_COOP;
				}
			}
		}
	}

	if (pass) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 0065
 */
bool aiObjInteract(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop) {
		if (obj->prop->type == PROPTYPE_DOOR) {
			doorsActivate(obj->prop, false);
		} else if (obj->prop->type == PROPTYPE_OBJ || obj->prop->type == PROPTYPE_WEAPON) {
			propobjInteract(obj->prop);
		}
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0066
 */
bool aiDestroyObject(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop && objGetDestroyedLevel(obj) == 0) {
		struct defaultobj *entity = obj->prop->obj;

		if (entity->modelnum == MODEL_ELVIS_SAUCER) {
			obj->flags = (obj->flags & ~OBJFLAG_FORCEMORTAL) | OBJFLAG_INVINCIBLE;
			explosionCreateSimple(entity->prop, &entity->prop->pos, entity->prop->rooms, EXPLOSIONTYPE_LAPTOP, 0);
			smokeCreateAtProp(entity->prop, SMOKETYPE_UFO);
		} else {
			f32 damage = ((obj->maxdamage - obj->damage) + 1) / 250.0f;
			objDamage(obj, damage, &obj->prop->pos, WEAPON_REMOTEMINE, -1);
		}
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0067
 */
bool ai0067(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop && obj->prop->parent && obj->prop->parent->type == PROPTYPE_CHR) {
		struct chrdata *chr = obj->prop->parent->chr;
		objSetDropped(obj->prop, DROPTYPE_SURRENDER);
		chr->hidden |= CHRHFLAG_DROPPINGITEM;
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0068
 */
bool aiChrDropItems(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop) {
		chrDropConcealedItems(chr);
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0069
 */
bool aiChrDropWeapon(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		u32 prevplayernum = g_Vars.currentplayernum;
		u32 playernum = playermgrGetPlayerNumByProp(chr->prop);
		u32 weaponnum;
		setCurrentPlayerNum(playernum);
		weaponnum = bgunGetWeaponNum(HAND_RIGHT);
		invRemoveItemByNum(weaponnum);
		bgunCycleBack();
		setCurrentPlayerNum(prevplayernum);
	} else if (chr && chr->prop) {
		if (chr->weapons_held[0]) {
			objSetDropped(chr->weapons_held[0], DROPTYPE_DEFAULT);
			chr->hidden |= CHRHFLAG_DROPPINGITEM;
		}

		if (chr->weapons_held[1]) {
			objSetDropped(chr->weapons_held[1], DROPTYPE_DEFAULT);
			chr->hidden |= CHRHFLAG_DROPPINGITEM;
		}
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 006a
 */
bool aiGiveObjectToChr(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[3]);

	if (obj && obj->prop && chr && chr->prop) {
		if (chr->prop->type == PROPTYPE_PLAYER) {
			u32 something;
			u32 prevplayernum = g_Vars.currentplayernum;
			struct defaultobj *obj2 = obj->prop->obj;
			u32 playernum = playermgrGetPlayerNumByProp(chr->prop);
			setCurrentPlayerNum(playernum);

#if VERSION >= VERSION_NTSC_1_0
			if (obj->prop->parent) {
				objDetach(obj->prop);
				objFreeEmbedmentOrProjectile(obj->prop);
				propActivate(obj->prop);
			}
#endif

			something = propPickupByPlayer(obj->prop, 0);
			propExecuteTickOperation(obj->prop, something);
			playernum = playermgrGetPlayerNumByProp(chr->prop);
			obj2->hidden = (playernum << 28) | (obj2->hidden & 0x0fffffff);
			setCurrentPlayerNum(prevplayernum);
		} else {
			if (obj->prop->parent) {
				objDetach(obj->prop);
			} else {
				propDeregisterRooms(obj->prop);
				propDelist(obj->prop);
				propDisable(obj->prop);
			}

			if (obj->type != OBJTYPE_WEAPON || chrEquipWeapon((struct weaponobj *) obj, chr) == 0) {
				propReparent(obj->prop, chr->prop);
			}
		}
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 006b
 */
bool aiObjectMoveToPad(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);
	u16 padnum = cmd[4] | (cmd[3] << 8);
	Mtxf matrix;
	struct pad pad;
	s16 rooms[2];

	if (obj && obj->prop) {
		padUnpack(padnum, PADFIELD_POS | PADFIELD_LOOK | PADFIELD_UP | PADFIELD_ROOM, &pad);
		mtx00016d58(&matrix,
				0, 0, 0,
				-pad.look.x, -pad.look.y, -pad.look.z,
				pad.up.x, pad.up.y, pad.up.z);

		if (obj->model) {
			mtx00015f04(obj->model->scale, &matrix);
		}

		rooms[0] = pad.room;
		rooms[1] = -1;
		func0f06a730(obj, &pad.pos, &matrix, rooms, &pad.pos);
	}

	g_Vars.aioffset += 5;

	return false;
}

/**
 * @cmd 006c
 */
bool aiOpenDoor(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop && obj->prop->type == PROPTYPE_DOOR) {
		if (!doorCallLift(obj->prop, false)) {
			struct doorobj *door = (struct doorobj *) obj;
			doorsRequestMode(door, DOORMODE_OPENING);
		}
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 006d
 */
bool aiCloseDoor(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop && obj->prop->type == PROPTYPE_DOOR) {
		struct doorobj *door = (struct doorobj *) obj;
		doorsRequestMode(door, DOORMODE_CLOSING);
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 006e
 */
bool aiIfDoorState(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);
	bool pass = false;

	if (obj && obj->prop && obj->type == OBJTYPE_DOOR) {
		struct doorobj *door = (struct doorobj *) obj;

		if (door->mode == DOORMODE_IDLE) {
			if (door->frac <= 0) {
				pass = (cmd[3] & DOORSTATE_CLOSED) != 0;
			} else {
				pass = (cmd[3] & DOORSTATE_OPEN) != 0;
			}
		} else if (door->mode == DOORMODE_OPENING || door->mode == DOORMODE_WAITING) {
			pass = (cmd[3] & DOORSTATE_OPENING) != 0;
		} else if (door->mode == DOORMODE_CLOSING) {
			pass = (cmd[3] & DOORSTATE_CLOSING) != 0;
		}
	}

	if (pass) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 006f
 */
bool aiIfObjectIsDoor(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop && obj->type == OBJTYPE_DOOR && (obj->hidden & 0x200)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0070
 */
bool aiLockDoor(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop && obj->prop->type == PROPTYPE_DOOR) {
		struct doorobj *door = (struct doorobj *) obj;
		u8 bits = cmd[3];
		door->keyflags = door->keyflags | bits;
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 0071
 */
bool aiUnlockDoor(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop && obj->prop->type == PROPTYPE_DOOR) {
		struct doorobj *door = (struct doorobj *) obj;
		u8 bits = cmd[3];
		door->keyflags = door->keyflags & ~bits;
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 0072
 */
bool aiIfDoorLocked(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);
	bool pass = false;

	if (obj && obj->prop && obj->prop->type == PROPTYPE_DOOR) {
		struct doorobj *door = (struct doorobj *) obj;
		u32 bits = cmd[3];
		u32 keyflags = door->keyflags;

		if ((keyflags & bits) == bits) {
			pass = true;
		}
	}

	if (pass) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 0073
 */
bool aiIfObjectiveComplete(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (cmd[2] < objectiveGetCount() &&
			objectiveCheck(cmd[2]) == OBJECTIVE_COMPLETE &&
			objectiveGetDifficultyBits(cmd[2]) & (1 << lvGetDifficulty())) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0074
 */
bool aiIfObjectiveFailed(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (cmd[2] < objectiveGetCount() &&
			objectiveCheck(cmd[2]) == OBJECTIVE_FAILED &&
			objectiveGetDifficultyBits(cmd[2]) & (1 << lvGetDifficulty())) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0075
 */
bool ai0075(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (func0f04a4ec(g_Vars.chrdata, cmd[2])) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0076
 */
bool aiSetPadPresetToTargetQuadrant(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (chrSetPadPresetToWaypointWithinTargetQuadrant(g_Vars.chrdata, cmd[2])) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 007d
 */
bool aiIfNumArghsLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (chrGetNumArghs(g_Vars.chrdata) < cmd[2]) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 007e
 */
bool aiIfNumArghsGreaterThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (chrGetNumArghs(g_Vars.chrdata) > cmd[2]) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 007f
 */
bool aiIfNumCloseArghsLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (chrGetNumCloseArghs(g_Vars.chrdata) < cmd[2]) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0080
 */
bool aiIfNumCloseArghsGreaterThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (chrGetNumCloseArghs(g_Vars.chrdata) > cmd[2]) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0081
 */
bool aiIfChrHealthGreaterThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 value = cmd[3] * 0.1f;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);
	u32 pass = false;

	if (chr && chr->prop) {
		if (chr->prop->type == PROPTYPE_PLAYER) {
			u32 playernum = playermgrGetPlayerNumByProp(chr->prop);

			pass = (value > g_Vars.players[playernum]->bondhealth * 8.0f);
		} else {
			pass = (value > chr->maxdamage - chr->damage);
		}
	}

	if (pass) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 0082
 */
bool aiIfChrHealthLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 value = cmd[3] * 0.1f;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);
	u32 pass = false;

	if (chr && chr->prop) {
		if (chr->prop->type == PROPTYPE_PLAYER) {
			u32 playernum = playermgrGetPlayerNumByProp(chr->prop);

			pass = (value < g_Vars.players[playernum]->bondhealth * 8.0f);
		} else {
			pass = (value < chr->maxdamage - chr->damage);
		}
	}

	if (pass) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 010f
 */
bool aiIfChrShieldLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 value = (cmd[4] | (cmd[3] << 8)) * 0.1f;
	struct chrdata *chr = chrFindById(g_Vars.chrdata,cmd[2]);

	if (chr && chrGetShield(chr) < value) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[5]);
	} else {
		g_Vars.aioffset = g_Vars.aioffset + 6;
	}

	return false;
}

/**
 * @cmd 0110
 */
bool aiIfChrShieldGreaterThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 value = (cmd[4] | (cmd[3] << 8)) * 0.1f;
	struct chrdata *chr = chrFindById(g_Vars.chrdata,cmd[2]);

	if (chr && chrGetShield(chr) > value) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[5]);
	} else {
		g_Vars.aioffset = g_Vars.aioffset + 6;
	}

	return false;
}

/**
 * @cmd 0083
 */
bool aiIfInjured(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && (chr->chrflags & CHRCFLAG_JUST_INJURED)) {
		chr->chrflags &= ~CHRCFLAG_JUST_INJURED;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset = g_Vars.aioffset + 4;
	}

	return false;
}

/**
 * @cmd 0168
 */
bool aiIfShieldDamaged(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && (chr->chrflags & CHRCFLAG_SHIELDDAMAGED)) {
		chr->chrflags &= ~CHRCFLAG_SHIELDDAMAGED;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset = g_Vars.aioffset + 4;
	}

	return false;
}

/**
 * @cmd 0077
 */
bool aiIfDifficultyLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (lvGetDifficulty() < cmd[2]) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0078
 */
bool aiIfDifficultyGreaterThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (lvGetDifficulty() > cmd[2]) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0079
 */
bool aiIfStageTimerLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 target = (f32)(cmd[3] | (cmd[2] << 8));
	f32 time = lvGetStageTimeInSeconds();

	if (time < target) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 007a
 */
bool aiIfStageTimerGreaterThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 target = (f32)(cmd[3] | (cmd[2] << 8));
	f32 time = lvGetStageTimeInSeconds();

	if (time > target) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 007b
 */
bool aiIfStageIdLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (cmd[2] > mainGetStageNum()) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 007c
 */
bool aiIfStageIdGreaterThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (mainGetStageNum() > cmd[2]) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0084
 */
bool aiSetMorale(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	g_Vars.chrdata->morale = cmd[2];
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0085
 */
bool aiAddMorale(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	incrementByte(&g_Vars.chrdata->morale, cmd[2]);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0086
 */
bool aiChrAddMorale(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[3]);
	incrementByte(&chr->morale, cmd[2]);
	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 0087
 */
bool aiSubtractMorale(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	decrementByte(&g_Vars.chrdata->morale, cmd[2]);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0088
 */
bool aiIfMoraleLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (g_Vars.chrdata->morale < cmd[2]) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0089
 */
bool aiIfMoraleLessThanRandom(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (g_Vars.chrdata->morale < g_Vars.chrdata->random) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 008a
 */
bool aiSetAlertness(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	g_Vars.chrdata->alertness = cmd[2];
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 008b
 */
bool aiAddAlertness(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	incrementByte(&g_Vars.chrdata->alertness, cmd[2]);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 008c
 */
bool aiChrAddAlertness(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[3]);

	if (chr && chr->prop) {
		incrementByte(&chr->alertness, cmd[2]);
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 008d
 */
bool aiSubtractAlertness(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	decrementByte(&g_Vars.chrdata->alertness, cmd[2]);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 008e
 */
bool aiIfAlertness(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if ((g_Vars.chrdata->alertness < cmd[2] && cmd[3] == 0) ||
			(cmd[2] < g_Vars.chrdata->alertness && cmd[3] == 1)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 008f
 */
bool aiIfChrAlertnessLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[3]);

	if (chr && chr->alertness < cmd[2]) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 0090
 */
bool aiIfAlertnessLessThanRandom(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (g_Vars.chrdata->alertness < g_Vars.chrdata->random) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0092
 */
bool aiSetHearDistance(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 distance = (cmd[3] | (cmd[2] << 8)) / 1000.0f;
	g_Vars.chrdata->hearingscale = distance;

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 0093
 */
bool aiSetViewDistance(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (!cheatIsActive(CHEAT_PERFECTDARKNESS)) {
		g_Vars.chrdata->visionrange = cmd[2];
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0094
 */
bool aiSetGrenadeProbability(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	g_Vars.chrdata->grenadeprob = cmd[2];
	g_Vars.aioffset += + 3;

	return false;
}

/**
 * @cmd 0095
 */
bool aiSetChrNum(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	chrSetChrnum(g_Vars.chrdata, cmd[2]);
	g_Vars.chrdata->chrnum = cmd[2];
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0096
 */
bool aiSetMaxDamage(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 maxdamage = (cmd[4] | (cmd[3] << 8)) * 0.1f;

	if (g_Vars.hovercar) {
		chopperSetMaxDamage(g_Vars.hovercar, maxdamage);
	} else {
		struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

		if (chr && chr->prop && !chrIsDead(chr)
					&& chr->actiontype != ACT_DEAD
					&& chr->actiontype != ACT_DIE
					&& chr->actiontype != ACT_DRUGGEDKO
					&& chr->actiontype != ACT_DRUGGEDDROP
					&& chr->actiontype != ACT_DRUGGEDCOMINGUP) {
			chrSetMaxDamage(chr, maxdamage);
		}
	}

	g_Vars.aioffset += 5;

	return false;
}

/**
 * @cmd 0097
 */
bool aiAddHealth()
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 amount = (cmd[3] | (cmd[2] << 8)) * 0.1f;

	chrAddHealth(g_Vars.chrdata, amount);

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 010e
 */
bool aiSetShield(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 amount = (cmd[3] | (cmd[2] << 8)) * 0.1f;

	if (cheatIsActive(CHEAT_ENEMYSHIELDS)) {
		amount = amount < 8 ? 8 : amount;
	}

	chrSetShield(g_Vars.chrdata, amount);

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 0098
 */
bool aiSetReactionSpeed(void)
{
	s8 *cmd = (s8 *)g_Vars.ailist + g_Vars.aioffset;

	g_Vars.chrdata->speedrating = cmd[2];
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0099
 */
bool aiSetRecoverySpeed(void)
{
	s8 *cmd = (s8 *)g_Vars.ailist + g_Vars.aioffset;

	g_Vars.chrdata->arghrating = cmd[2];
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 009a
 */
bool aiSetAccuracy(void)
{
	s8 *cmd = (s8 *)g_Vars.ailist + g_Vars.aioffset;

	g_Vars.chrdata->accuracyrating = cmd[2];
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 01c6
 */
bool aiSetDodgeRating(void)
{
	s8 *cmd = (s8 *)g_Vars.ailist + g_Vars.aioffset;

	if (cmd[2] == 0) {
		g_Vars.chrdata->dodgerating = cmd[3];
	} else if (cmd[2] == 1) {
		g_Vars.chrdata->maxdodgerating = cmd[3];
	} else {
		g_Vars.chrdata->dodgerating = cmd[3];
		g_Vars.chrdata->maxdodgerating = cmd[3];
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 01c7
 */
bool aiSetUnarmedDodgeRating(void)
{
	s8 *cmd = (s8 *)g_Vars.ailist + g_Vars.aioffset;

	g_Vars.chrdata->unarmeddodgerating = cmd[2];
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 009b
 */
bool aiSetFlag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[3] << 16) | (cmd[4] << 8) | cmd[5] | (cmd[2] << 24);
	chrSetFlags(g_Vars.chrdata, flags, cmd[6]);

	g_Vars.aioffset += 7;

	return false;
}

/**
 * @cmd 009c
 */
bool aiUnsetFlag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[3] << 16) | (cmd[4] << 8) | cmd[5] | (cmd[2] << 24);
	chrUnsetFlags(g_Vars.chrdata, flags, cmd[6]);

	g_Vars.aioffset += 7;

	return false;
}

/**
 * @cmd 009d
 */
bool aiIfHasFlag(void)
{
	bool result;
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[3] << 16) | (cmd[4] << 8) | cmd[5] | (cmd[2] << 24);

	result = chrHasFlag(g_Vars.chrdata, flags, cmd[7]);

	if (cmd[6] == 0) {
		result = !result;
	}

	if (result) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[8]);
	} else {
		g_Vars.aioffset += 9;
	}

	return false;
}

/**
 * @cmd 009e
 */
bool aiChrSetFlag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[4] << 16) | (cmd[5] << 8) | cmd[6] | (cmd[3] << 24);
	chrSetFlagsById(g_Vars.chrdata, cmd[2], flags, cmd[7]);
	g_Vars.aioffset += 8;

	return false;
}

/**
 * @cmd 009f
 */
bool aiChrUnsetFlag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[4] << 16) | (cmd[5] << 8) | cmd[6] | (cmd[3] << 24);
	chrUnsetFlagsById(g_Vars.chrdata, cmd[2], flags, cmd[7]);
	g_Vars.aioffset += 8;

	return false;
}

/**
 * @cmd 00a0
 */
bool aiIfChrHasFlag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[4] << 16) | (cmd[5] << 8) | cmd[6] | (cmd[3] << 24);

	if (chrHasFlagById(g_Vars.chrdata, cmd[2], flags, cmd[7])) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[8]);
	} else {
		g_Vars.aioffset += 9;
	}

	return false;
}

/**
 * @cmd 00a1
 */
bool aiSetStageFlag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[3] << 16) | (cmd[4] << 8) | cmd[5] | (cmd[2] << 24);
	chrSetStageFlag(g_Vars.chrdata, flags);
	g_Vars.aioffset += 6;

	return false;
}

/**
 * @cmd 00a2
 */
bool aiUnsetStageFlag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[3] << 16) | (cmd[4] << 8) | cmd[5] | (cmd[2] << 24);
	chrUnsetStageFlag(g_Vars.chrdata, flags);
	g_Vars.aioffset += 6;

	return false;
}

/**
 * @cmd 00a3
 */
bool aiIfStageFlagEq(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[3] << 16) | (cmd[4] << 8) | cmd[5] | (cmd[2] << 24);

	if ((chrHasStageFlag(g_Vars.chrdata, flags) && cmd[6] == 1) ||
			(!chrHasStageFlag(g_Vars.chrdata, flags) && cmd[6] == 0)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[7]);
	} else {
		g_Vars.aioffset += 8;
	}

	return false;
}

/**
 * @cmd 00a4
 */
bool aiSetChrflag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[3] << 16) | (cmd[4] << 8) | cmd[5] | (cmd[2] << 24);

	g_Vars.chrdata->chrflags |= flags;
	g_Vars.aioffset += 6;

	return false;
}

/**
 * @cmd 00a5
 */
bool aiUnsetChrflag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[3] << 16) | (cmd[4] << 8) | cmd[5] | (cmd[2] << 24);

	g_Vars.chrdata->chrflags &= ~flags;
	g_Vars.aioffset += 6;

	return false;
}

/**
 * @cmd 00a6
 */
bool aiIfHasChrflag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[3] << 16) | (cmd[4] << 8) | cmd[5] | (cmd[2] << 24);

	if ((g_Vars.chrdata->chrflags & flags) == flags) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[6]);
	} else {
		g_Vars.aioffset += 7;
	}

	return false;
}

/**
 * @cmd 00a7
 */
bool aiChrSetChrflag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[4] << 16) | (cmd[5] << 8) | cmd[6] | (cmd[3] << 24);
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr) {
		chr->chrflags |= flags;
	}

	g_Vars.aioffset += 7;

	return false;
}

/**
 * @cmd 00a8
 */
bool aiChrUnsetChrflag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[4] << 16) | (cmd[5] << 8) | cmd[6] | (cmd[3] << 24);
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr) {
		chr->chrflags &= ~flags;
	}

	g_Vars.aioffset += 7;

	return false;
}

/**
 * @cmd 00a9
 */
bool aiIfChrHasChrflag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[4] << 16) | (cmd[5] << 8) | cmd[6] | (cmd[3] << 24);
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && (chr->chrflags & flags) == flags) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[7]);
	} else {
		g_Vars.aioffset += 8;
	}

	return false;
}

/**
 * @cmd 011b
 */
bool aiChrSetHiddenFlag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[4] << 16) | (cmd[5] << 8) | cmd[6] | (cmd[3] << 24);
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr) {
		chr->hidden |= flags;
	}

	g_Vars.aioffset += 7;

	return false;
}

/**
 * @cmd 011c
 */
bool aiChrUnsetHiddenFlag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[4] << 16) | (cmd[5] << 8) | cmd[6] | (cmd[3] << 24);
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr) {
		chr->hidden &= ~flags;
	}

	g_Vars.aioffset += 7;

	return false;
}

/**
 * @cmd 011d
 */
bool aiIfChrHasHiddenFlag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[4] << 16) | (cmd[5] << 8) | cmd[6] | (cmd[3] << 24);
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && (chr->hidden & flags) == flags) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[7]);
	} else {
		g_Vars.aioffset += 8;
	}

	return false;
}

/**
 * @cmd 00aa
 */
bool aiSetObjFlag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[4] << 16) | (cmd[5] << 8) | cmd[6] | (cmd[3] << 24);
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop) {
		obj->flags |= flags;
	}

	g_Vars.aioffset += 7;

	return false;
}

/**
 * @cmd 00ab
 */
bool aiUnsetObjFlag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[4] << 16) | (cmd[5] << 8) | cmd[6] | (cmd[3] << 24);
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop) {
		obj->flags &= ~flags;
	}

	g_Vars.aioffset += 7;

	return false;
}

/**
 * @cmd 00ac
 */
bool aiIfObjHasFlag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[4] << 16) | (cmd[5] << 8) | cmd[6] | (cmd[3] << 24);
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop && (obj->flags & flags) == flags) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[7]);
	} else {
		g_Vars.aioffset += 8;
	}

	return false;
}

/**
 * @cmd 00ad
 */
bool aiSetObjFlag2(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[4] << 16) | (cmd[5] << 8) | cmd[6] | (cmd[3] << 24);
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop) {
		obj->flags2 |= flags;
	}

	g_Vars.aioffset += 7;

	return false;
}

/**
 * @cmd 00ae
 */
bool aiUnsetObjFlag2(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[4] << 16) | (cmd[5] << 8) | cmd[6] | (cmd[3] << 24);
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop) {
		obj->flags2 &= ~flags;
	}

	g_Vars.aioffset += 7;

	return false;
}

/**
 * @cmd 00af
 */
bool aiIfObjHasFlag2(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[4] << 16) | (cmd[5] << 8) | cmd[6] | (cmd[3] << 24);
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop && (obj->flags2 & flags) == flags) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[7]);
	} else {
		g_Vars.aioffset += 8;
	}

	return false;
}

/**
 * @cmd 0118
 */
bool aiSetObjFlag3(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[4] << 16) | (cmd[5] << 8) | cmd[6] | (cmd[3] << 24);
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop) {
		obj->flags3 |= flags;
	}

	g_Vars.aioffset += 7;

	return false;
}

/**
 * @cmd 0119
 */
bool aiUnsetObjFlag3(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[4] << 16) | (cmd[5] << 8) | cmd[6] | (cmd[3] << 24);
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop) {
		obj->flags3 &= ~flags;
	}

	g_Vars.aioffset += 7;

	return false;
}

/**
 * @cmd 011a
 */
bool aiIfObjHasFlag3(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[4] << 16) | (cmd[5] << 8) | cmd[6] | (cmd[3] << 24);
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop && (obj->flags3 & flags) == flags) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[7]);
	} else {
		g_Vars.aioffset += 8;
	}

	return false;
}

/**
 * @cmd 00b0
 */
bool aiSetChrPreset(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	chrSetChrPreset(g_Vars.chrdata, cmd[2]);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 00b1
 */
bool aiSetChrTarget(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	chrSetChrPresetByChrnum(g_Vars.chrdata, cmd[2], cmd[3]);
	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 00b2
 */
bool aiSetPadPreset(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 pad_id = cmd[3] | (cmd[2] << 8);

	if (g_Vars.chrdata) {
		chrSetPadPreset(g_Vars.chrdata, pad_id);
	} else if (g_Vars.heli) {
		g_Vars.heli->base.pad = pad_id;
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 00b3
 */
bool aiChrSetPadPreset(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 pad_id = cmd[4] | (cmd[3] << 8);

	chrSetPadPresetByChrnum(g_Vars.chrdata, cmd[2], pad_id);

	g_Vars.aioffset += 5;

	return false;
}

/**
 * @cmd 00b4
 */
bool aiChrCopyPadPreset(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chrsrc = chrFindById(g_Vars.chrdata, cmd[2]);
	struct chrdata *chrdst = chrFindById(g_Vars.chrdata, cmd[3]);

	chrdst->padpreset1 = chrsrc->padpreset1;
	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 00b5
 *
 * The weirdness to do with result is required for a match.
 * The original source likely had something similar and probably used ifdefs.
 */
bool aiPrint(void)
{
	u32 len;
	u32 result = dprint();

	if (result) {
		result = 2;
	}

	if (result == 2) {
		// empty
	}

	len = chraiGetCommandLength(g_Vars.ailist, g_Vars.aioffset);

	g_Vars.aioffset += len;

	return false;
}

/**
 * @cmd 0091
 */
bool aiNoOp0091(void)
{
	g_Vars.aioffset += 2;
	return false;
}

/**
 * @cmd 00b6
 */
bool aiRestartTimer(void)
{
	if (g_Vars.chrdata) {
		chrRestartTimer(g_Vars.chrdata);
	} else if (g_Vars.hovercar) {
		chopperRestartTimer(g_Vars.hovercar);
	}

	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 00b7
 */
bool aiResetTimer(void)
{
	g_Vars.chrdata->timer60 = 0;
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 00b8
 */
bool aiPauseTimer(void)
{
	g_Vars.chrdata->hidden &= ~CHRHFLAG_TIMER_RUNNING;
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 00b9
 */
bool aiResumeTimer(void)
{
	g_Vars.chrdata->hidden |= CHRHFLAG_TIMER_RUNNING;
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 00ba
 */
bool aiIfTimerStopped(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if ((g_Vars.chrdata->hidden & CHRHFLAG_TIMER_RUNNING) == 0) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 00bb
 */
bool aiIfTimerGreaterThanRandom(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 timer = chrGetTimer(g_Vars.chrdata);

	if (g_Vars.chrdata->random < timer) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 00bc
 */
bool aiIfTimerLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 value = (u32)((cmd[3] << 8) | cmd[4] | (cmd[2] << 16)) / 60.0f;

	if ((g_Vars.chrdata && chrGetTimer(g_Vars.chrdata) < value) ||
			(g_Vars.hovercar && chopperGetTimer(g_Vars.hovercar) < value)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[5]);
	} else {
		g_Vars.aioffset += 6;
	}

	return false;
}

/**
 * @cmd 00bd
 */
bool aiIfTimerGreaterThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 value = (u32)((cmd[3] << 8) | cmd[4] | (cmd[2] << 16)) / 60.0f;

	// These two function calls were likely used in a debug print statement
	if (g_Vars.chrdata) {
		chrGetTimer(g_Vars.chrdata);
	}

	if (g_Vars.hovercar) {
		chopperGetTimer(g_Vars.hovercar);
	}

	if ((g_Vars.chrdata && chrGetTimer(g_Vars.chrdata) > value) ||
			(g_Vars.hovercar && chopperGetTimer(g_Vars.hovercar) > value)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[5]);
	} else {
		g_Vars.aioffset += 6;
	}

	return false;
}

/**
 * @cmd 00be
 */
bool aiShowCountdownTimer(void)
{
	countdownTimerSetVisible(COUNTDOWNTIMERREASON_AI, true);
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 00bf
 */
bool aiHideCountdownTimer(void)
{
	countdownTimerSetVisible(COUNTDOWNTIMERREASON_AI, false);
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 00c0
 */
bool aiSetCountdownTimerValue(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 seconds = cmd[3] | (cmd[2] << 8);

	countdownTimerSetValue60(seconds * 60);
	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 00c1
 */
bool aiStopCountdownTimer(void)
{
	countdownTimerSetRunning(false);
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 00c2
 */
bool aiStartCountdownTimer(void)
{
	countdownTimerSetRunning(true);
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 00c3
 */
bool aiIfCountdownTimerStopped(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (!countdownTimerIsRunning()) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 00c4
 */
bool aiIfCountdownTimerLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 value = cmd[3] | (cmd[2] << 8);

	if (countdownTimerGetValue60() < value * 60) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 00c5
 */
bool aiIfCountdownTimerGreaterThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 value = cmd[3] | (cmd[2] << 8);

	if (countdownTimerGetValue60() > value * 60) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 00c6
 */
bool aiSpawnChrAtPad(void)
{
	struct bytelist *cmd = (struct bytelist *)(g_Vars.ailist + g_Vars.aioffset);
	u16 pad = cmd->b4 << 8 | cmd->b5;
	u32 spawnflags = cmd->b8 << 24 | cmd->b9 << 16 | cmd->b10 << 8 | cmd->b11;
	u16 ailistid = cmd->b6 << 8 | cmd->b7;
	u8 *ailist = ailistFindById(ailistid);

	if (spawnflags);

	if (chrSpawnAtPad(g_Vars.chrdata, cmd->b2, (s8)cmd->b3, pad, ailist, spawnflags)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd->b12);
	} else {
		g_Vars.aioffset += 13;
	}

	return false;
}

/**
 * @cmd 00c7
 */
bool aiSpawnChrAtChr(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 spawnflags = (cmd[8] << 16) | (cmd[9] << 8) | cmd[10] | (cmd[7] << 24);
	u16 ailistid = cmd[6] | (cmd[5] << 8);
	u8 *ailist = ailistFindById(ailistid);

	if (chrSpawnAtChr(g_Vars.chrdata, cmd[2], (s8)cmd[3], cmd[4], ailist, spawnflags)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[11]);
	} else {
		g_Vars.aioffset += 12;
	}

	return false;
}

/**
 * @cmd 00c8
 */
bool aiTryEquipWeapon(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[6] << 16) | (cmd[7] << 8) | cmd[8] | (cmd[5] << 24);
	u32 model = cmd[3] | (cmd[2] << 8);
	struct prop *prop = NULL;

	if (g_Vars.chrdata && g_Vars.chrdata->prop && g_Vars.chrdata->model) {
		// If the Marqis cheat is active, don't give guns to chrs,
		// except where required for objectives.
#if VERSION < VERSION_NTSC_1_0
		// On NTSC beta, Marquis is disabled entirely on MBR, and the K7 guard
		// on Investigation PA is excluded here (ie. is given his weapon).
		if (cheatIsActive(CHEAT_MARQUIS) && g_Vars.stagenum != STAGE_MBR) {
			if (g_Vars.stagenum == STAGE_INVESTIGATION
					&& lvGetDifficulty() == DIFF_PA
					&& cmd[4] == WEAPON_K7AVENGER) {
				prop = chrGiveWeapon(g_Vars.chrdata, model, cmd[4], flags);
			}
		}
#elif VERSION < VERSION_PAL_BETA
		// NTSC final enables Marquis for MBR, but fails to realise why it was
		// disabled in the first place (Cass needs to equip her Falcon).
		// Additionally, NTSC Final changes the logic so the chrs are given
		// their weapon but they cannot equip them. This means the K7 guard
		// no longer needs to be handled.
		if (cheatIsActive(CHEAT_MARQUIS)) {
			flags &= ~OBJFLAG_WEAPON_LEFTHANDED;
			flags |= OBJFLAG_WEAPON_AICANNOTUSE;

			prop = chrGiveWeapon(g_Vars.chrdata, model, cmd[4], flags);
		}
#else
		// PAL fixes Cass on MBR by allowing her to equip her weapon
		if (cheatIsActive(CHEAT_MARQUIS)) {
			if (g_Vars.chrdata->bodynum != BODY_CASSANDRA || mainGetStageNum() != STAGE_MBR) {
				flags &= ~OBJFLAG_WEAPON_LEFTHANDED;
				flags |= OBJFLAG_WEAPON_AICANNOTUSE;
			}

			prop = chrGiveWeapon(g_Vars.chrdata, model, cmd[4], flags);
		}
#endif
		else if (cheatIsActive(CHEAT_ENEMYROCKETS)) {
			switch (cmd[4]) {
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
				prop = chrGiveWeapon(g_Vars.chrdata, MODEL_CHRDYROCKET, WEAPON_ROCKETLAUNCHER, flags);
				break;
			case WEAPON_K7AVENGER:
				if (g_Vars.stagenum == STAGE_INVESTIGATION && lvGetDifficulty() == DIFF_PA) {
					prop = chrGiveWeapon(g_Vars.chrdata, model, cmd[4], flags);
				} else {
					prop = chrGiveWeapon(g_Vars.chrdata, MODEL_CHRDYROCKET, WEAPON_ROCKETLAUNCHER, flags);
				}
				break;
			default:
				prop = chrGiveWeapon(g_Vars.chrdata, model, cmd[4], flags);
				break;
			}
		} else {
			prop = chrGiveWeapon(g_Vars.chrdata, model, cmd[4], flags);
		}
	}

	if (prop) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[9]);
	} else {
		g_Vars.aioffset += 10;
	}

	return false;
}

/**
 * @cmd 00c9
 */
bool aiTryEquipHat(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 flags = (cmd[5] << 16) | (cmd[6] << 8) | cmd[7] | (cmd[4] << 24);
	u32 modelnum = cmd[3] | (cmd[2] << 8);
	struct prop *prop = NULL;

	if (g_Vars.chrdata && g_Vars.chrdata->prop && g_Vars.chrdata->model) {
		prop = hatCreateForChr(g_Vars.chrdata, modelnum, flags);
	}

	if (prop) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[8]);
	} else {
		g_Vars.aioffset += 9;
	}

	return false;
}

/**
 * @cmd 00ca
 */
bool aiDuplicateChr(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 spawnflags = (cmd[6] << 16) | (cmd[7] << 8) | cmd[8] | (cmd[5] << 24);
	u16 ailistid = cmd[4] | (cmd[3] << 8);
	u8 *ailist = ailistFindById(ailistid);
	bool pass = false;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);
	struct chrdata *clone = NULL;
	struct weaponobj *srcweapon1 = NULL;
	struct prop *cloneprop = NULL;
	struct weaponobj *srcweapon0 = NULL;
	struct prop *srcweapon0prop = NULL;
	struct prop *cloneweapon0prop = NULL;
	struct prop *srcweapon1prop = NULL;
	struct weaponobj *cloneweapon1 = NULL;
	struct weaponobj *cloneweapon0 = NULL;
	struct prop *cloneweapon1prop = NULL;

	if (chr && (chr->chrflags & CHRCFLAG_CLONEABLE)) {
		cloneprop = chrSpawnAtChr(g_Vars.chrdata, chr->bodynum, -1, chr->chrnum, ailist, spawnflags);

		if (cloneprop) {
			clone = cloneprop->chr;
			chrSetChrnum(clone, chrsGetNextUnusedChrnum());
			chr->chrdup = clone->chrnum;

			srcweapon0prop = chrGetHeldProp(chr, 0);

			if (srcweapon0prop) {
				srcweapon0 = srcweapon0prop->weapon;
				cloneweapon0prop = chrGiveWeapon(clone, srcweapon0->base.modelnum, srcweapon0->weaponnum, 0);

				if (cloneweapon0prop) {
					cloneweapon0 = cloneweapon0prop->weapon;
				}
			}

			srcweapon1prop = chrGetHeldProp(chr, 1);

			if (srcweapon1prop) {
				srcweapon1 = srcweapon1prop->weapon;
				cloneweapon1prop = chrGiveWeapon(clone, srcweapon1->base.modelnum, srcweapon1->weaponnum, OBJFLAG_WEAPON_LEFTHANDED);

				if (cloneweapon1prop) {
					cloneweapon1 = cloneweapon1prop->weapon;
				}
			}

			if (srcweapon1 && srcweapon0
					&& cloneweapon1 && cloneweapon0
					&& srcweapon0 == srcweapon1->dualweapon && srcweapon1 == srcweapon0->dualweapon) {
				propweaponSetDual(cloneweapon1, cloneweapon0);
			}

			if (chr->weapons_held[2]) {
				struct defaultobj *obj = chr->weapons_held[2]->obj;
				hatCreateForChr(clone, obj->modelnum, 0);
			}

			clone->flags = chr->flags;
			clone->flags2 = chr->flags2;
			clone->padpreset1 = chr->padpreset1;

			if (g_Vars.normmplayerisrunning == false
					&& g_MissionConfig.iscoop
					&& g_Vars.numaibuddies > 0) {
				clone->flags |= CHRFLAG0_AIVSAI;
			}

			if (spawnflags & SPAWNFLAG_HIDDEN) {
				clone->chrflags &= CHRCFLAG_HIDDEN;
			}

			clone->team = chr->team;
			clone->squadron = chr->squadron;
			clone->voicebox = chr->voicebox;

			rebuildTeams();
			rebuildSquadrons();

			pass = true;
		}
	}

	if (pass) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[9]);
	} else {
		g_Vars.aioffset += 10;
	}

	return false;
}

/**
 * @cmd 00cb
 */
bool aiShowHudmsg(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	char *text = langGet(cmd[4] | (cmd[3] << 8));
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	u32 prevplayernum = g_Vars.currentplayernum;
	u32 playernum = g_Vars.currentplayernum;

	if (chr && chr->prop && (chr->prop->type & 0xff) == PROPTYPE_PLAYER) {
		playernum = playermgrGetPlayerNumByProp(chr->prop);
	}

	setCurrentPlayerNum(playernum);
	hudmsgCreate(text, HUDMSGTYPE_DEFAULT);
	setCurrentPlayerNum(prevplayernum);

	g_Vars.aioffset += 5;

	return false;
}

/**
 * @cmd 01a4
 */
bool aiShowHudmsgMiddle(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (cmd[2] == 0) {
		u32 text_id = cmd[5] | (cmd[4] << 8);
		char *text = langGet(text_id);
		hudmsgCreateWithColour(text, HUDMSGTYPE_7, cmd[3]);
	} else if (cmd[2] == 1) {
		u32 text_id = cmd[5] | (cmd[4] << 8);
		char *text = langGet(text_id);
		hudmsgCreateWithColour(text, HUDMSGTYPE_8, cmd[3]);
	} else {
		hudmsgRemoveAll();
	}

	g_Vars.aioffset += 6;

	return false;
}

/**
 * @cmd 00cc
 */
bool aiShowHudmsgTopMiddle(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	char *text = langGet(cmd[4] | (cmd[3] << 8));
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	u32 prevplayernum = g_Vars.currentplayernum;
	u32 playernum = g_Vars.currentplayernum;

	if (chr && chr->prop && (chr->prop->type & 0xff) == PROPTYPE_PLAYER) {
		playernum = playermgrGetPlayerNumByProp(chr->prop);
	}

	setCurrentPlayerNum(playernum);
	hudmsgCreateWithColour(text, HUDMSGTYPE_INGAMESUBTITLE, cmd[5]);
	setCurrentPlayerNum(prevplayernum);

	g_Vars.aioffset += 6;

	return false;
}

/**
 * @cmd 00cd
 */
bool aiSpeak(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);
	s16 audio_id = cmd[6] | (cmd[5] << 8);
	s16 text_id = cmd[4] | (cmd[3] << 8);
	s32 prevplayernum = g_Vars.currentplayernum;
	s32 playernum = prevplayernum;
	u32 channelnum;
	char *text = text_id >= 0 ? langGet(cmd[4] | (cmd[3] << 8)) : NULL;

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		playernum = playermgrGetPlayerNumByProp(chr->prop);
	}

	setCurrentPlayerNum(playernum);

	if (text && cmd[2] != CHR_P1P2) {
		func0f0926bc(g_Vars.chrdata->prop, 9, 0xffff);
	}

	if (cmd[2] == CHR_P1P2) {
		channelnum = audioPlayFromProp((s8)cmd[7], audio_id, 0, g_Vars.chrdata->prop, 0, 512);
	} else {
		channelnum = audioPlayFromProp((s8)cmd[7], audio_id, 0, g_Vars.chrdata->prop, 9, 512);
	}

	if (text && !sndIsFiltered(audio_id)) {
		hudmsgCreateAsSubtitle(text, HUDMSGTYPE_INGAMESUBTITLE, cmd[8], channelnum);
	}

	setCurrentPlayerNum(prevplayernum);

	g_Vars.aioffset += 9;

	return false;
}

/**
 * @cmd 00ce
 */
bool aiPlaySound(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s16 audio_id = cmd[3] | (cmd[2] << 8);

	audioPlayFromProp((s8)cmd[4], audio_id, 0, NULL, 0, 0);

	g_Vars.aioffset += 5;

	return false;
}

/**
 * @cmd 017c
 */
bool aiAssignSound(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s16 audio_id = cmd[3] | (cmd[2] << 8);

	audioPlayFromProp((s8)cmd[4], audio_id, -1, NULL, 11, 0);

	g_Vars.aioffset += 5;

	return false;
}

/**
 * @cmd 00d3
 */
bool aiAudioMuteChannel(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s8 channel = (s8)cmd[2];

	audioMuteChannel(channel);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0138
 */
bool aiIfChannelIdle(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s8 channel = (s8) cmd[2];

	if (audioIsChannelIdle(channel)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 00d1
 */
bool ai00d1(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s16 audio_id = cmd[4] | (cmd[3] << 8);
	u16 thing = cmd[6] | (cmd[5] << 8);

	audioPlayFromProp2((s8)cmd[2], audio_id, -1, NULL, thing, 2500, 3000, 0);

	g_Vars.aioffset += 7;

	return false;
}

/**
 * @cmd 00d2
 */
bool ai00d2(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 thing1 = cmd[4] | (cmd[3] << 8);
	u16 thing2 = cmd[6] | (cmd[5] << 8);
	s32 audio_id = func0f0927d4(thing1, 400, 2500, 3000, 32767);

	audioPlayFromProp2((s8)cmd[2], audio_id, -1, NULL, thing2, 2500, 3000, 0);

	g_Vars.aioffset += 7;

	return false;
}

/**
 * @cmd 00cf
 */
bool ai00cf(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[3]);
	u16 thing = cmd[5] | (cmd[4] << 8);

	if (obj && obj->prop) {
		audioPlayFromProp2((s8)cmd[2], -1, -1, obj->prop, thing, 2500, 3000, 0);
	}

	g_Vars.aioffset += 6;

	return false;
}

/**
 * @cmd 016b
 */
bool ai016b(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[3]);
	u16 thing1 = cmd[5] | (cmd[4] << 8);
	u16 thing2 = cmd[7] | (cmd[6] << 8);
	u16 thing3 = cmd[9] | (cmd[8] << 8);

	if (obj && obj->prop) {
		s32 thing1again;

		if (thing1 == 0) {
			thing1again = -1;
		} else {
			thing1again = thing1;
		}

		audioPlayFromProp2((s8)cmd[2], -1, -1, obj->prop, thing1again, thing2, thing3, 2);
	}

	g_Vars.aioffset += 10;

	return false;
}

/**
 * @cmd 0179
 */
bool ai0179(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 thing1 = cmd[5] | (cmd[4] << 8);
	u16 thing2 = cmd[7] | (cmd[6] << 8);
	u16 thing3 = cmd[9] | (cmd[8] << 8);

	if (cmd[10] == 0) {
		struct defaultobj *obj = objFindByTagId(cmd[3]);

		if (obj && obj->prop) {
			audioPlayFromProp2((s8)cmd[2], -1, -1, obj->prop, thing1, thing2, thing3, 0);
		}
	} else {
		struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[3]);

		if (chr && chr->prop) {
			audioPlayFromProp2((s8)cmd[2], -1, -1, chr->prop, thing1, thing2, thing3, 0);
		}
	}

	g_Vars.aioffset += 11;

	return false;
}

/**
 * @cmd 00d0
 */
bool ai00d0(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s16 padnum = cmd[4] | (cmd[3] << 8);
	s16 sound = cmd[6] | (cmd[5] << 8);

	propsnd0f0939f8(0, NULL, sound, padnum, -1, 2, 0, 0, 0, -1, 0, -1, -1, -1, -1);

	g_Vars.aioffset += 7;

	return false;
}

/**
 * @cmd 00d4
 */
bool ai00d4(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s16 thing = cmd[4] | (cmd[3] << 8);

	if (channelGetUnk06((s8)cmd[2]) < thing) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[5]);
	} else {
		g_Vars.aioffset += 6;
	}

	return false;
}

/**
 * @cmd 00d5
 */
bool aiHovercarBeginPath(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct path *path = pathFindById(cmd[2]);

	if (g_Vars.truck) {
		g_Vars.truck->path = path;
		g_Vars.truck->nextstep = 0;
	}

	if (g_Vars.hovercar) {
		struct chopperobj *chopper = chopperFromHovercar(g_Vars.hovercar);
		g_Vars.hovercar->path = path;
		g_Vars.hovercar->nextstep = 0;
		g_Vars.hovercar->path->flags |= PATHFLAG_INUSE;

		if (chopper) {
			chopper->targetvisible = false;
			chopper->attackmode = CHOPPERMODE_PATROL;
			chopper->turnrot60 = 0;
			chopper->roty = 0;
			chopper->rotx = 0;
			chopper->gunroty = 0;
			chopper->gunrotx = 0;
			chopper->barrelrot = 0;
			chopper->barrelrotspeed = 0;
			chopper->vz = 0;
			chopper->vy = 0;
			chopper->vx = 0;
			chopper->otz = 0;
			chopper->oty = 0;
			chopper->otx = 0;
			chopper->power = 0;
			chopper->bob = 0;
			chopper->bobstrength = 0.05;
			chopper->timer60 = 0;
			chopper->patroltimer60 = 0;
			chopper->cw = 0;
			chopper->weaponsarmed = true;
			chopper->base.flags |= OBJFLAG_CHOPPER_INIT;
		} else {
			g_Vars.hovercar->weaponsarmed = false;
		}
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 00d6
 */
bool aiSetVehicleSpeed(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 speedtime = cmd[5] | (cmd[4] << 8);
	f32 speedaim = (cmd[3] | (cmd[2] << 8)) * 100.0f / 15360.0f;

	if (g_Vars.truck) {
		g_Vars.truck->speedaim = speedaim;
		g_Vars.truck->speedtime60 = speedtime;
	}

	if (g_Vars.hovercar) {
		g_Vars.hovercar->speedaim = speedaim;
		g_Vars.hovercar->speedtime60 = speedtime;
	}

	g_Vars.aioffset += 6;

	return false;
}

/**
 * @cmd 00d7
 */
bool aiSetRotorSpeed(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 speedtime = cmd[5] | (cmd[4] << 8);
	f32 speedaim = (cmd[3] | (cmd[2] << 8)) * M_BADTAU / 3600;

	if (g_Vars.heli) {
		g_Vars.heli->rotoryspeedaim = speedaim;
		g_Vars.heli->rotoryspeedtime = speedtime;
	}

	g_Vars.aioffset += 6;

	return false;
}

/**
 * @cmd 00d8
 */
bool aiNoOp00d8(void)
{
	g_Vars.aioffset += 3;
	return false;
}

/**
 * @cmd 00d9
 */
bool aiNoOp00d9(void)
{
	g_Vars.aioffset += 3;
	return false;
}

/**
 * @cmd 00da
 */
bool aiSetObjImage(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop) {
		if (obj->type == OBJTYPE_SINGLEMONITOR) {
			struct singlemonitorobj *sm = (struct singlemonitorobj *) obj;
			tvscreenSetImageByNum(&sm->screen, cmd[4]);
		} else if (obj->type == OBJTYPE_MULTIMONITOR) {
			u8 slot = cmd[3];
			if (slot < 4) {
				struct multimonitorobj *mm = (struct multimonitorobj *) obj;
				tvscreenSetImageByNum(&mm->screens[slot], cmd[4]);
			}
		}
	}

	g_Vars.aioffset += 5;

	return false;
}

/**
 * @cmd 00db
 */
bool aiNoOp00db(void)
{
	g_Vars.aioffset += 3;
	return false;
}

/**
 * @cmd 00dc
 */
bool aiEndLevel(void)
{
	if (debugAllowEndLevel()) {
		if (g_IsTitleDemo) {
			mainChangeToStage(STAGE_TITLE);
		} else if (g_Vars.autocutplaying) {
			g_Vars.autocutfinished = true;
		} else {
			func0000e990();
		}
	}

	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 00dd
 */
bool ai00dd(void)
{
	playerEndCutscene();
	g_Vars.aioffset += 2;
	return false;
}

/**
 * @cmd 00de
 */
bool aiWarpJoToPad(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 pad_id = cmd[3] | (cmd[2] << 8);
	playerPrepareWarpType1(pad_id);

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 010d
 */
bool aiNoOp010d(void)
{
	g_Vars.aioffset += 2;
	return false;
}

/**
 * @cmd 0111
 */
bool aiSetCameraAnimation(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s16 anim_id = cmd[3] | (cmd[2] << 8);

	playerStartCutscene(anim_id);

	if (g_Vars.currentplayer->haschrbody == false) {
		return true;
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 0113
 */
bool aiIfInCutscene(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (g_Vars.in_cutscene) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0174
 */
bool aiIfCutsceneButtonPressed(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if ((g_Vars.in_cutscene && g_CutsceneSkipRequested) ||
			(g_Vars.stagenum == STAGE_CITRAINING && var80087260 > 0)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0175
 */
bool ai0175(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	playerReorientForCutsceneStop(cmd[2]);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0112
 */
bool aiObjectDoAnimation(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s16 anim_id = cmd[3] | (cmd[2] << 8);
	struct defaultobj *obj = NULL;
	f32 thing;
	s32 startframe = cmd[7] | (cmd[6] << 8);
	f32 fstartframe;

	if (startframe == 0xffff) {
		fstartframe = 0;
	} else if (startframe == 0xfffe) {
		fstartframe = animGetNumFrames(anim_id) - 2;

		if (fstartframe < 0) {
			fstartframe = 0;
		}
	} else {
		fstartframe = startframe;
	}

	if (cmd[4] == 255) {
		if (g_Vars.chrdata && g_Vars.chrdata->myspecial >= 0) {
			obj = objFindByTagId(g_Vars.chrdata->myspecial);
		}
	} else {
		obj = objFindByTagId(cmd[4]);
	}

	if (obj && obj->prop) {
		struct anim *anim = obj->model->anim;

		if (obj->model->anim == NULL) {
			obj->model->anim = modelmgrInstantiateAnim();
		}

		if (obj->model->anim) {
			thing = 1.0f / (s32)cmd[5];

			if (g_Vars.in_cutscene && startframe != 0xfffe) {
#if PAL
				fstartframe += var8009e388pf * thing;
#else
				fstartframe += g_CutsceneFrameOverrun240 * thing * 0.25f;
#endif
			}

			animInit(obj->model->anim);
#if VERSION >= VERSION_JPN_FINAL
			modelSetAnimPlaySpeed(obj->model, 1, 0);
#elif VERSION >= VERSION_PAL_BETA
			modelSetAnimPlaySpeed(obj->model, 1.2, 0);
#endif
			modelSetAnimation(obj->model, anim_id, 0, fstartframe, thing, 0);
			modelSetAnimScale(obj->model, func0f15c888() * obj->model->scale * 100.0f);
		}
	}

	g_Vars.aioffset += 8;

	return false;
}

/**
 * @cmd 0114
 */
bool aiEnableChr(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && chr->model) {
		propActivate(chr->prop);
		propEnable(chr->prop);
		chr0f0220ac(chr);
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0115
 */
bool aiDisableChr(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && chr->model) {
		propDeregisterRooms(chr->prop);
		propDelist(chr->prop);
		propDisable(chr->prop);
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0116
 */
bool aiEnableObj(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop && obj->model) {
		propActivate(obj->prop);
		propEnable(obj->prop);

		if (g_Vars.currentplayer->eyespy == NULL && obj->type == OBJTYPE_WEAPON) {
			struct weaponobj *weapon = (struct weaponobj *) obj;

			if (weapon->weaponnum == WEAPON_EYESPY) {
				playerInitEyespy();
			}
		}
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0117
 */
bool aiDisableObj(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop && obj->model) {
#if VERSION >= VERSION_PAL_FINAL
		if (g_Vars.autocutplaying
				&& mainGetStageNum() == STAGE_AIRFORCEONE
				&& (obj->modelnum == MODEL_AIRFORCE1 || obj->modelnum == MODEL_SK_SHUTTLE)) {
			// ignore
		} else {
			if (obj->prop->parent) {
				objDetach(obj->prop);
			} else {
				propDeregisterRooms(obj->prop);
				propDelist(obj->prop);
				propDisable(obj->prop);
			}
		}
#else
		if (obj->prop->parent) {
			objDetach(obj->prop);
		} else {
			propDeregisterRooms(obj->prop);
			propDelist(obj->prop);
			propDisable(obj->prop);
		}
#endif
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 00df
 */
bool ai00df(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct tag *tag = tagFindById(cmd[2]);

	if (tag) {
		s32 cmdindex = setupGetCmdIndexByTag(tag);

		if (cmdindex >= 0) {
			struct warpparams *params = (struct warpparams *) setupGetCmdByIndex(cmdindex + tag->cmdoffset);
			playerPrepareWarpType2(params, cmd[4] | (cmd[3] << 8), cmd[6] | (cmd[5] << 8));
		}
	}

	g_Vars.aioffset += 7;

	return false;
}

/**
 * @cmd 00e0
 */
bool aiRevokeControl(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		u32 prevplayernum = g_Vars.currentplayernum;
		u32 playernum = playermgrGetPlayerNumByProp(chr->prop);
		setCurrentPlayerNum(playernum);
		bgunSetSightVisible(GUNSIGHTREASON_NOCONTROL, false);
		bgunSetGunAmmoVisible(GUNAMMOREASON_NOCONTROL, false);

		if ((cmd[3] & 2) == 0) {
			hudmsgsSetOff(HUDMSGREASON_NOCONTROL);
		}

		if ((cmd[3] & 4) == 0) {
			countdownTimerSetVisible(COUNTDOWNTIMERREASON_NOCONTROL, false);
		}

		g_PlayersWithControl[g_Vars.currentplayernum] = false;
		setCurrentPlayerNum(prevplayernum);
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 00e1
 */
bool aiGrantControl(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		u32 prevplayernum = g_Vars.currentplayernum;
		setCurrentPlayerNum(playermgrGetPlayerNumByProp(chr->prop));
		bgunSetSightVisible(GUNSIGHTREASON_NOCONTROL, true);
		bgunSetGunAmmoVisible(GUNAMMOREASON_NOCONTROL, true);
		hudmsgsSetOn(HUDMSGREASON_NOCONTROL);
		countdownTimerSetVisible(COUNTDOWNTIMERREASON_NOCONTROL, true);
		g_PlayersWithControl[g_Vars.currentplayernum] = true;
		setCurrentPlayerNum(prevplayernum);
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 00e2
 */
bool aiChrMoveToPad(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
#if VERSION < VERSION_NTSC_1_0
	s32 padnum = cmd[4] | (cmd[3] << 8);
#endif
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);
	bool pass = false;
	f32 theta;
	struct pad pad;
	s16 rooms[2];
	struct chrdata *chr2;

	if (chr && chr->prop) {
#if VERSION >= VERSION_NTSC_1_0
		if (cmd[5] == 88) {
			chr2 = chrFindById(g_Vars.chrdata, cmd[4]);

			if (chr2 && chr2->prop) {
				theta = chrGetInverseTheta(chr2);
				pass = chrMoveToPos(chr, &chr2->prop->pos, chr2->prop->rooms, theta, false);
			}
		} else {
			s32 padnum = cmd[4] | (cmd[3] << 8);
			padnum = chrResolvePadId(chr, padnum);

			if (padnum >= 0) {
				padUnpack(padnum, PADFIELD_POS | PADFIELD_LOOK | PADFIELD_ROOM, &pad);
				theta = atan2f(pad.look.x, pad.look.z);

				rooms[0] = pad.room;
				rooms[1] = -1;
				pass = chrMoveToPos(chr, &pad.pos, rooms, theta, cmd[5]);
			}
		}
#else
		padnum = chrResolvePadId(chr, padnum);

		padUnpack(padnum, PADFIELD_POS | PADFIELD_LOOK | PADFIELD_ROOM, &pad);
		theta = atan2f(pad.look.x, pad.look.z);

		rooms[0] = pad.room;
		rooms[1] = -1;
		pass = chrMoveToPos(chr, &pad.pos, rooms, theta, cmd[5]);
#endif
	}

	if (pass) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[6]);
	} else {
		g_Vars.aioffset += 7;
	}

	return false;
}

/**
 * @cmd 00e3
 */
bool ai00e3(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		u32 prevplayernum = g_Vars.currentplayernum;
		u32 playernum = playermgrGetPlayerNumByProp(chr->prop);
		setCurrentPlayerNum(playernum);

		if (var8007074c != 2) {
			playerSetFadeColour(0, 0, 0, 0);
			playerSetFadeFrac(60, 1);
		}

		setCurrentPlayerNum(prevplayernum);
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 00e4
 */
bool ai00e4(void)
{
	s32 playernum;
	u32 prevplayernum = g_Vars.currentplayernum;

	for (playernum = 0; playernum < PLAYERCOUNT(); playernum++) {
		setCurrentPlayerNum(playernum);

		if (var8007074c != 2) {
			playerSetFadeColour(0, 0, 0, 1);
			playerSetFadeFrac(60, 0);
		}
	}

	setCurrentPlayerNum(prevplayernum);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 00e5
 */
bool aiIfColourFadeComplete(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	bool pass = false;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		u32 playernum = playermgrGetPlayerNumByProp(chr->prop);

		if (g_Vars.players[playernum]->colourfadetimemax60 < 0) {
			pass = true;
		}
	}

	if (pass) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 00e8
 */
bool aiSetDoorOpen(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop) {
		struct doorobj *door = (struct doorobj *) obj;
		door->frac = door->maxfrac;
		door->fracspeed = 0;
		door->lastopen60 = g_Vars.lvframe60;
		door->mode = 0;
		doorUpdateTiles(door);
		doorActivatePortal(door);
		func0f0926bc(door->base.prop, 1, 0xffff);
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 00e9
 */
bool ai00e9(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr) {
		weaponDeleteFromChr(chr, cmd[3]);
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 00ea
 */
bool aiIfNumPlayersLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if ((s8)cmd[2] > PLAYERCOUNT()) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 00eb
 */
bool aiIfChrAmmoQuantityLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);
	bool passes = false;

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		u32 prevplayernum = g_Vars.currentplayernum;
		u32 playernum = playermgrGetPlayerNumByProp(chr->prop);
		setCurrentPlayerNum(playernum);

		if (bgunGetAmmoCount((s8)cmd[3]) < (s8)cmd[4]) {
			passes = true;
		}

		setCurrentPlayerNum(prevplayernum);
	}

	if (passes) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[5]);
	} else {
		g_Vars.aioffset += 6;
	}

	return false;
}

/**
 * @cmd 00ec
 */
bool aiChrDrawWeapon(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		u32 prevplayernum = g_Vars.currentplayernum;
		u32 playernum = playermgrGetPlayerNumByProp(chr->prop);
		setCurrentPlayerNum(playernum);
		bgunEquipWeapon2(0, (s8)cmd[3]);
		bgunEquipWeapon2(1, 0);
		setCurrentPlayerNum(prevplayernum);
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 00ed
 */
bool aiChrDrawWeaponInCutscene(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		u32 prevplayernum = g_Vars.currentplayernum;
		u32 playernum = playermgrGetPlayerNumByProp(chr->prop);
		setCurrentPlayerNum(playernum);
		bgunEquipWeapon((s8)cmd[3]);
		setCurrentPlayerNum(prevplayernum);
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 00ee
 */
bool ai00ee(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		u32 prevplayernum = g_Vars.currentplayernum;
		u32 playernum = playermgrGetPlayerNumByProp(chr->prop);
		setCurrentPlayerNum(playernum);

		g_Vars.currentplayer->bondforcespeed.x = (s8)cmd[3];
		g_Vars.currentplayer->bondforcespeed.y = 0;
		g_Vars.currentplayer->bondforcespeed.z = (s8)cmd[4];

		setCurrentPlayerNum(prevplayernum);
	}

	g_Vars.aioffset += 5;

	return false;
}

/**
 * @cmd 00ef
 */
bool aiIfObjInRoom(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);
	u16 room_id = cmd[4] | (cmd[3] << 8);
	s32 room_something = chrGetPadRoom(g_Vars.chrdata, room_id);

	if (room_something >= 0 && obj && obj->prop && room_something == obj->prop->rooms[0]) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[5]);
	} else {
		g_Vars.aioffset += 6;
	}

	return false;
}

/**
 * @cmd 00f2
 */
bool aiSwitchToAltSky(void)
{
	envApplyTransitionFrac(1);
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 00f3
 */
bool aiChrSetInvincible(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		u32 prevplayernum = g_Vars.currentplayernum;
		u32 playernum = playermgrGetPlayerNumByProp(chr->prop);
		setCurrentPlayerNum(playernum);
		g_PlayerInvincible = true;
		setCurrentPlayerNum(prevplayernum);
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 00f4
 */
bool ai00f4(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	s32 range = cmd[3] | (cmd[2] << 8);
	s16 height1 = cmd[5] | (cmd[4] << 8);
	s16 rotangle = cmd[7] | (cmd[6] << 8);
	s32 padnum = cmd[9] | (cmd[8] << 8);
	s16 height2 = cmd[11] | (cmd[10] << 8);
	s32 posangle = cmd[13] | (cmd[12] << 8);

	playerPrepareWarpType3(posangle * M_BADTAU / 65536, rotangle * M_BADTAU / 65536, range, height1, height2, padnum);

	g_Vars.aioffset += 14;

	return false;
}

/**
 * @cmd 00f5
 */
bool ai00f5(void)
{
	var8007073c = 1;
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 00f6
 */
bool ai00f6(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (var8007073c == 2) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 00f7
 */
bool aiIfAllObjectivesComplete(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (objectiveIsAllComplete()) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset = g_Vars.aioffset + 3;
	}

	return false;
}

/**
 * @cmd 00f8
 */
bool aiIfPlayerIsInvincible(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	bool pass = false;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		u32 prevplayernum = g_Vars.currentplayernum;
		u32 playernum = playermgrGetPlayerNumByProp(chr->prop);
		setCurrentPlayerNum(playernum);
		pass = g_PlayerInvincible;
		setCurrentPlayerNum(prevplayernum);
	}

	if (pass) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 00f9
 */
bool aiPlayXTrack(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	g_Vars.aioffset += 5;
	musicSetXReason((s8)cmd[2], cmd[3], cmd[4]);

	return false;
}

/**
 * @cmd 00fa
 */
bool aiStopXTrack(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	g_Vars.aioffset += 3;
	musicUnsetXReason((s8)cmd[2]);

	return false;
}

/**
 * @cmd 015b
 */
bool aiPlayTrackIsolated(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (cmd[2] == MUSIC_CI_TRAINING) {
		u16 volume = optionsGetMusicVolume();
		musicPlayTrackIsolated(cmd[2]);
		optionsSetMusicVolume(volume);
	} else {
		musicPlayTrackIsolated(cmd[2]);
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 015c
 */
bool aiPlayDefaultTracks(void)
{
	g_Vars.aioffset += 2;
	musicPlayDefaultTracks();

	return false;
}

/**
 * @cmd 017d
 */
bool aiPlayCutsceneTrack(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	musicStartCutscene(cmd[2]);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 017e
 */
bool aiStopCutsceneTrack(void)
{
	g_Vars.aioffset += 2;
	musicEndCutscene();

	return false;
}

/**
 * @cmd 017f
 */
bool aiPlayTemporaryTrack(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	musicStartTemporaryAmbient(cmd[2]);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0180
 */
bool aiStopAmbientTrack(void)
{
	g_Vars.aioffset += 2;
	musicEndTemporaryAmbient();

	return false;
}

/**
 * @cmd 00fb
 */
bool aiChrExplosions(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		u32 prevplayernum = g_Vars.currentplayernum;
		u32 playernum = playermgrGetPlayerNumByProp(chr->prop);
		setCurrentPlayerNum(playernum);
		playerSurroundWithExplosions(0);
		setCurrentPlayerNum(prevplayernum);
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 00fc
 */
bool aiIfKillCountGreaterThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (g_Vars.killcount > cmd[2]) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 01ab
 */
bool aiIfNumKnockedOutChrs(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (cmd[2] < mpstatsGetTotalKnockoutCount() && cmd[3] == 0) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else if (mpstatsGetTotalKnockoutCount() < cmd[2] && cmd[3] == 1) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 00fd
 */
bool ai00fd(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && (chr->chrflags & CHRCFLAG_TRIGGERSHOTLIST)) {
		chr->chrflags &= ~CHRCFLAG_TRIGGERSHOTLIST;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 00fe
 */
bool aiKillBond(void)
{
	g_Vars.bond->isdead = true;
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 00ff
 */
bool aiBeSurprisedSurrender(void)
{
	chrTrySurprisedSurrender(g_Vars.chrdata);
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 0100
 */
bool aiNoOp0100(void)
{
	g_Vars.aioffset += 3;
	return false;
}

/**
 * @cmd 0101
 */
bool aiNoOp0101(void)
{
	g_Vars.aioffset += 3;
	return false;
}

/**
 * @cmd 0102
 */
bool aiSetLights(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 padnum = cmd[3] | (cmd[2] << 8);
	s32 roomnum = chrGetPadRoom(g_Vars.chrdata, padnum);

	if (roomnum >= 0) {
		switch (cmd[4]) {
		case LIGHTOP_TURNOFF:
			roomSetLightsOn(roomnum, false);
			break;
		case LIGHTOP_TURNON:
			roomSetLightsOn(roomnum, true);
			break;
		default:
			roomSetLightOp(roomnum, cmd[4], cmd[5], cmd[6], TICKS(cmd[7]));
			break;
		}
	}

	g_Vars.aioffset += 11;

	return false;
}

/**
 * @cmd 0103
 */
bool aiIfPropPresetIsBlockingSightToTarget(void)
{
	if (chrIsPropPresetBlockingSightToTarget(g_Vars.chrdata)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0104
 */
bool aiRemoveObjectAtPropPreset(void)
{
	if (g_Vars.chrdata->proppreset1 >= 0) {
		struct defaultobj *obj = (g_Vars.props + g_Vars.chrdata->proppreset1)->obj;
		obj->hidden &= ~OBJHFLAG_OCCUPIEDCHAIR;
	}

	g_Vars.chrdata->proppreset1 = -1;
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 0105
 */
bool aiIfPropPresetHeightLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct prop *prop = &g_Vars.props[g_Vars.chrdata->proppreset1];
	f32 value = cmd[3] | (cmd[2] << 8);
	f32 ymax;
	f32 ymin;
	f32 radius;

	propGetBbox(prop, &radius, &ymax, &ymin);

	if (ymax - ymin < value) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 0106
 */
bool aiSetTarget(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (g_Vars.chrdata) {
		s16 newtarget;

		if (!cmd[3] && !cmd[4]) {
			newtarget = propGetIndexByChrId(g_Vars.chrdata, cmd[2]);
		} else if (!cmd[4]) {
			struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);
			newtarget = chr->target;
		}

		if (newtarget != g_Vars.chrdata->target) {
			g_Vars.chrdata->lastvisibletarget60 = 0;
			g_Vars.chrdata->lastseetarget60 = 0;
			g_Vars.chrdata->lastheartarget60 = 0;

			// @bug: Unsetting these flags here causes guards to become deaf in
			// co-op mode. This is because their AI scripting toggles their
			// target between the two players on each frame, so this flag is
			// cleared before it is read. A suitable fix would be to only unset
			// these flags if either the old or new target is not a player.
			g_Vars.chrdata->hidden &= ~CHRHFLAG_IS_HEARING_TARGET;
			g_Vars.chrdata->chrflags &= ~CHRCFLAG_NEAR_MISS;

			g_Vars.chrdata->target = newtarget;
		}
	} else if (g_Vars.hovercar) {
		chopperSetTarget(g_Vars.hovercar, cmd[2]);
	}

	g_Vars.aioffset += 5;

	return false;
}

/**
 * @cmd 0107
 */
bool aiIfPresetsTargetIsNotMyTarget(void)
{
	s32 mypresetchrstarget;
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (g_Vars.chrdata->chrpreset1 != -1) {
		mypresetchrstarget = propGetIndexByChrId(g_Vars.chrdata, g_Vars.chrdata->chrpreset1);
	}

	if (g_Vars.chrdata->target != -1 && mypresetchrstarget != g_Vars.chrdata->target) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0108
 */
bool aiIfChrTarget(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);
	bool pass = false;

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		// empty
	} else if (cmd[2] != CHR_BOND) {
		if (cmd[4] == 0) {
			struct chrdata *chr2 = chrFindById(g_Vars.chrdata, cmd[3]);

			if (chr2 && chr2->prop && chrGetTargetProp(chr) == chr2->prop) {
				pass = true;
			}
		} else {
			if (chr->target != -1 && chr && chr->prop) {
				pass = true;
			}
		}
	}

	if (pass) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[5]);
	} else {
		g_Vars.aioffset += 6;
	}

	return false;
}

/**
 * @cmd 0109
 */
bool aiSetChrPresetToChrNearSelf(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 distance = (cmd[4] | (cmd[3] << 8)) * 10.0f;

	if (chrSetChrPresetToChrNearSelf(cmd[2], g_Vars.chrdata, distance)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[5]);
	} else {
		g_Vars.aioffset += 6;
	}

	return false;
}

/**
 * @cmd 010a
 */
bool aiSetChrPresetToChrNearPad(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 distance = (cmd[4] | (cmd[3] << 8)) * 10.0f;
	u16 padnum = cmd[6] | (cmd[5] << 8);

	if (chrSetChrPresetToChrNearPad(cmd[2], g_Vars.chrdata, distance, padnum)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[7]);
	} else {
		g_Vars.aioffset += 8;
	}

	return false;
}

/**
 * @cmd 010b
 */
bool aiChrSetTeam(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr) {
		chr->team = cmd[3];
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 010c
 */
bool aiIfCompareChrPresetsTeam(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, CHR_PRESET);

	if (!chr || (!chr->model && chr->prop->type != PROPTYPE_PLAYER)) {
		chrSetChrPreset(g_Vars.chrdata, CHR_BOND);
		chr = chrFindById(g_Vars.chrdata, CHR_PRESET);
	}

	if (chrCompareTeams(chr, g_Vars.chrdata, cmd[2])) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 011e
 */
bool aiIfHuman(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && CHRRACE(chr) == RACE_HUMAN) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 011f
 */
bool aiIfSkedar(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && CHRRACE(chr) == RACE_SKEDAR) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0120
 */
bool aiIfSafety2LessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u8 score;
	u8 numnearby;
	s16 *chrnums = teamGetChrIds(g_Vars.chrdata->team);

	score = 6;
	numnearby = 0;

	if (chrGetNumArghs(g_Vars.chrdata) > 0) {
		score -= 2;
	}

	switch (bgunGetWeaponNum(HAND_RIGHT)) {
	case WEAPON_FALCON2:
	case WEAPON_FALCON2_SILENCER:
	case WEAPON_FALCON2_SCOPE:
	case WEAPON_MAGSEC4:
	case WEAPON_MAULER:
	case WEAPON_PHOENIX:
	case WEAPON_DY357MAGNUM:
	case WEAPON_DY357LX:
	case WEAPON_CROSSBOW:
		break;
	case WEAPON_CMP150:
	case WEAPON_CYCLONE:
	case WEAPON_CALLISTO:
	case WEAPON_RCP120:
	case WEAPON_LAPTOPGUN:
	case WEAPON_DRAGON:
	case WEAPON_K7AVENGER:
	case WEAPON_AR34:
	case WEAPON_SUPERDRAGON:
	case WEAPON_SHOTGUN:
	case WEAPON_SNIPERRIFLE:
		score--;
		break;
	case WEAPON_REAPER:
	case WEAPON_FARSIGHT:
	case WEAPON_DEVASTATOR:
	case WEAPON_ROCKETLAUNCHER:
	case WEAPON_SLAYER:
		score -= 2;
		break;
	default:
		score++;
		break;
	}

	while (*chrnums != -2) {
		struct chrdata *chr = chrFindByLiteralId(*chrnums);

		if (chr && chr->model
				&& !chrIsDead(chr)
				&& chr->actiontype != ACT_DEAD
				&& chr->alertness > 100
				&& g_Vars.chrdata->squadron == chr->squadron
				&& g_Vars.chrdata->chrnum != chr->chrnum
				&& chrGetDistanceToChr(g_Vars.chrdata, chr->chrnum) < 3500) {
			numnearby++;
		}

		chrnums++;
	}

	if (numnearby == 0) {
		score -= 2;
	} else if (numnearby == 1) {
		score--;
	}

	if (score < 3 && numnearby != 0) {
		score = 3;
	}

	if (score < cmd[2]) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0121
 */
bool aiFindCover(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 criteria = cmd[3] | (cmd[2] << 8);

	if (g_Vars.chrdata && g_Vars.chrdata->prop && chrAssignCoverByCriteria(g_Vars.chrdata, criteria, 0) != -1) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 0122
 */
bool aiFindCoverWithinDist(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 criteria = cmd[3] | (cmd[2] << 8);
	u32 flags = (cmd[5] << 16) | (cmd[6] << 8) | cmd[7] | (cmd[4] << 24);

	if (g_Vars.chrdata && g_Vars.chrdata->prop && chrAssignCoverByCriteria(g_Vars.chrdata, criteria, flags) != -1) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[8]);
	} else {
		g_Vars.aioffset += 9;
	}

	return false;
}

/**
 * @cmd 0123
 */
bool aiFindCoverOutsideDist(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 criteria = cmd[3] | (cmd[2] << 8);
	u32 flags = (cmd[5] << 16) | (cmd[6] << 8) | cmd[7] | (cmd[4] << 24);

	if (g_Vars.chrdata && g_Vars.chrdata->prop && chrAssignCoverByCriteria(g_Vars.chrdata, criteria, -flags) != -1) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[8]);
	} else {
		g_Vars.aioffset += 9;
	}

	return false;
}

/**
 * @cmd 0124
 */
bool aiGoToCover(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	chrGoToCover(g_Vars.chrdata, cmd[2]);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0125
 */
bool aiCheckCoverOutOfSight(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (chrCheckCoverOutOfSight(g_Vars.chrdata, g_Vars.chrdata->cover, false)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0126
 */
bool aiIfPlayerUsingCmpOrAr34(void)
{
	u32 hand = HAND_RIGHT;
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	switch (bgunGetWeaponNum(hand)) {
		case WEAPON_CMP150:
		case WEAPON_AR34:
			g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
			break;
		default:
			g_Vars.aioffset += 3;
			break;
	}

	return false;
}

/**
 * @cmd 0127
 */
bool aiDetectEnemyOnSameFloor(void)
{ \
	s32 team = 0;
	f32 closestdist = 9999.9;
	f32 distance;
	u32 stack[2];
	f32 y;
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 scandist;
	s16 *chrnums = teamGetChrIds(1);
	struct chrdata *chr;
	s16 newtarget = -1;

	if (g_Vars.chrdata->teamscandist == 0) {
		scandist = 1500;
	} else if (g_Vars.chrdata->teamscandist == 255) {
		scandist = 9999;
	} else {
		scandist = g_Vars.chrdata->teamscandist * 40.0f;
	}

	y = g_Vars.chrdata->prop->pos.y;

	while (team < 8) {
		chr = chrFindByLiteralId(*chrnums);

		if (*chrnums != -2) {
			if (chr && chr->prop
					&& chr->team != TEAM_NONCOMBAT
					&& !chrIsDead(chr)
					&& chr->actiontype != ACT_DEAD
					&& chr->actiontype != ACT_DRUGGEDKO
					&& chr->actiontype != ACT_DRUGGEDDROP
					&& chr->actiontype != ACT_DRUGGEDCOMINGUP
					&& chrCompareTeams(g_Vars.chrdata, chr, COMPARE_ENEMIES)
					&& (chr->hidden & CHRHFLAG_CLOAKED) == 0
					&& (chr->chrflags & CHRCFLAG_HIDDEN) == 0
					&& (chr->hidden & CHRHFLAG_ANTINONINTERACTABLE) == 0
					&& y - chr->prop->pos.y > -200
					&& y - chr->prop->pos.y < 200
					&& ((g_Vars.chrdata->hidden & CHRHFLAG_PSYCHOSISED) == 0
						|| (chr->hidden & CHRHFLAG_ANTINONINTERACTABLE) == 0
						|| (chr->hidden & CHRHFLAG_DONTSHOOTME))
					&& g_Vars.chrdata->chrnum != chr->chrnum) {
				distance = chrGetDistanceToChr(g_Vars.chrdata, chr->chrnum);

				if (distance < closestdist) {
					if (distance < scandist || stageGetIndex(g_Vars.stagenum) == STAGEINDEX_MAIANSOS) {
						if (distance < closestdist) {
							closestdist = distance;
							newtarget = chr->chrnum;
						}
					}
				}
			}

			chrnums++;
		} else {
			chrnums++;
			team++;
		}
	}

	if (newtarget != -1) {
		g_Vars.chrdata->target = propGetIndexByChrId(g_Vars.chrdata, newtarget);
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset = g_Vars.aioffset + 3;
	}

	return false;
}

/**
 * @cmd 0128
 */
bool aiDetectEnemy(void)
{
	s16 *chrnums;
	s32 team = 0;
	u32 stack[4];
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 closestdist = 10000000;
	f32 maxdist = (s32)cmd[2] * 10.0f;
	s16 closesttarg = -1;

	chrnums = teamGetChrIds(1);

	if (!g_Vars.chrdata) {
		g_Vars.aioffset = g_Vars.aioffset + 4;
		return false;
	}

	/**
	 * Iterate all the chrs in team order. Generally the outer loop iterates the
	 * teams and the inner loop iterates the chrs within that team, but for the
	 * chr's current team the inner loop won't iterate so the outer loop handles
	 * it.
	 *
	 * The chrnums list returned by teamGetChrIds is an array of chrnums in
	 * team 1 terminated by a -2, then an array of chrnums in team 2 terminated
	 * by -2, and so on.
	 */
	do {
		u8 teamvalue = (1 << team);

		while (*chrnums != -2 && g_Vars.chrdata->team != teamvalue) {
			struct chrdata *chr = chrFindByLiteralId(*chrnums);

			if (chr && chr->prop
					&& !chrIsDead(chr)
					&& chr->actiontype != ACT_DEAD
					&& chr->actiontype != ACT_DIE
					&& chr->actiontype != ACT_DRUGGEDKO
					&& chr->actiontype != ACT_DRUGGEDDROP
					&& chr->actiontype != ACT_DRUGGEDCOMINGUP
					&& chrCompareTeams(g_Vars.chrdata, chr, COMPARE_ENEMIES)
					&& chr != g_Vars.chrdata
					&& (chr->hidden & CHRHFLAG_CLOAKED) == 0
					&& (chr->chrflags & CHRCFLAG_HIDDEN) == 0
					&& (chr->hidden & CHRHFLAG_DISGUISED) == 0
					&& chr->team != TEAM_NONCOMBAT
					&& (
						(g_Vars.chrdata->hidden & CHRHFLAG_PSYCHOSISED) == 0
						|| (chr->hidden & CHRHFLAG_ANTINONINTERACTABLE) == 0
						|| (chr->hidden & CHRHFLAG_DONTSHOOTME))) {
				f32 distance = chrGetDistanceToChr(g_Vars.chrdata, chr->chrnum);

				if (distance < maxdist && distance != 0 && distance < closestdist
						&& chrHasLosToProp(g_Vars.chrdata, chr->prop)
						&& (chr->chrflags & CHRCFLAG_HIDDEN) == 0) {
					if (g_Vars.chrdata->yvisang == 0) {
						closestdist = distance;
						closesttarg = chr->chrnum;
					} else {
						s16 prevtarget = g_Vars.chrdata->target;
						g_Vars.chrdata->target = propGetIndexByChrId(g_Vars.chrdata, chr->chrnum);

						if (chrIsVerticalAngleToTargetWithin(g_Vars.chrdata, g_Vars.chrdata->yvisang)) {
							closestdist = distance;
							closesttarg = chr->chrnum;
						}

						g_Vars.chrdata->target = prevtarget;
					}
				}
			}

			chrnums++;
		}

		// If the inner loop was at the end of the current team,
		// move to the next team
		if (*chrnums == -2) {
			team++;
		}

		// And progress chrnums past the -2 terminator
		chrnums++;
	} while (team < 8);

	if (closesttarg != -1) {
		g_Vars.chrdata->target = propGetIndexByChrId(g_Vars.chrdata, closesttarg);
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset = g_Vars.aioffset + 4;
	}

	return false;
}

/**
 * @cmd 0129
 */
bool aiIfSafetyLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s16 *chrnums = teamGetChrIds(g_Vars.chrdata->team);
	u8 safety = 6;
	u8 numnearby = 0;

	if (chrGetNumArghs(g_Vars.chrdata) > 0) {
		safety--;
	}

	while (*chrnums != -2) {
		struct chrdata *chr = chrFindByLiteralId(*chrnums);

		if (chr && chr->model
				&& !chrIsDead(chr)
				&& chr->actiontype != ACT_DEAD
				&& g_Vars.chrdata->chrnum != chr->chrnum
				&& chrGetDistanceToChr(g_Vars.chrdata, chr->chrnum) < 3500) {
			numnearby++;
		}

		chrnums++;
	}

	if (numnearby == 0) {
		safety -= 2;
	} else if (numnearby < 3) {
		safety--;
	}

	if (safety < cmd[2]) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 012a
 */
bool aiIfTargetMovingSlowly(void)
{
	s32 delta;
	s32 absdelta;
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (cmd[2] == 0) {
		delta = chrGetDistanceLostToTargetInLastSecond(g_Vars.chrdata);
	} else {
		struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);
		delta = chrGetDistanceLostToTargetInLastSecond(chr);
	}

	absdelta = delta > 0 ? delta : -delta;

	if (absdelta < 50) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 012b
 */
bool aiIfTargetMovingCloser(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (chrGetDistanceLostToTargetInLastSecond(g_Vars.chrdata) < -50) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 012c
 */
bool aiIfTargetMovingAway(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (chrGetDistanceLostToTargetInLastSecond(g_Vars.chrdata) > 50) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 012f
 */
bool ai012f(void)
{
	if (g_Vars.chrdata->cover >= 0) {
		coverSetInUse(g_Vars.chrdata->cover, 0);
	}

	g_Vars.aioffset += 2;

	return false;
}

s16 g_GuardQuipBank[][4] = {
	// Voicebox 0
	{ QUIP_ATTACK1,            SFX_M0_CLEAR_SHOT,                SFX_M0_CLEAR_SHOT,                SFX_M0_SHES_MINE                  },
	{ QUIP_ATTACK2,            SFX_M0_OPEN_FIRE,                 SFX_M0_WIPE_HER_OUT,              SFX_M0_WASTE_HER                  },
	{ QUIP_GOTOCOVER1,         SFX_M0_COVER_ME,                  SFX_M0_WATCH_MY_BACK,             SFX_M0_TAKE_COVER                 },
	{ QUIP_GRENADE1,           SFX_M0_M1_LOOK_OUT_LOOK_OUT,      SFX_M0_M1_ITS_A_GRENADE,          SFX_M0_M1_CLEAR_THE_AREA          },
	{ QUIP_RETREAT1,           SFX_M0_GO_TO_PLAN_B,              SFX_M0_GET_THE_HELL_OUT_OF_HERE,  SFX_M0_RETREAT                    },
	{ QUIP_GRENADE2,           SFX_M0_CATCH,                     SFX_M0_EVERYBODY_DOWN,            SFX_M0_GRENADE                    },
	{ QUIP_RETREAT2,           SFX_M0_FALL_BACK,                 SFX_M0_EVERYONE_BACK_OFF,         SFX_M0_WITHDRAW                   },
	{ QUIP_FLANK,              SFX_M0_FLANK_THE_TARGET,          SFX_M0_LETS_SPLIT_UP,             SFX_M0_SURROUND_HER               },
	{ QUIP_SURRENDER,          SFX_M0_DONT_SHOOT_ME,             SFX_M0_I_GIVE_UP,                 SFX_M0_YOU_WIN_I_SURRENDER        },
	{ QUIP_HEARNOISE,          SFX_M0_HEAR_THAT,                 SFX_M0_WHATS_THAT_NOISE,          SFX_M0_HEARD_A_NOISE              },
	{ QUIP_10,                 SFX_0037,                         SFX_0037,                         SFX_0037                          },
	{ QUIP_SEEPLAYER,          SFX_M0_HEY_YOU,                   SFX_M0_INTRUDER_ALERT,            SFX_M0_GOT_A_CONTACT              },
#if VERSION >= VERSION_NTSC_1_0
	{ QUIP_SHOTUNALERT,        SFX_M0_HOLY_SHH,                  SFX_M0_HOLY_SHH,                  SFX_M0_WHAT_THE_HELL              },
#else
	{ QUIP_SHOTUNALERT,        SFX_80F6,                         SFX_80F6,                         SFX_M0_WHAT_THE_HELL              },
#endif
	{ QUIP_INJURED1,           SFX_M0_MEDIC,                     SFX_M0_OW,                        SFX_M0_YOU_SHOT_ME                },
	{ QUIP_INJURED2,           SFX_M0_IM_HIT,                    SFX_M0_IM_TAKING_FIRE,            SFX_M0_TAKING_DAMAGE              },
	{ QUIP_KILLEDPLAYER1,      SFX_M0_GRAB_A_BODY_BAG,           SFX_M0_ONE_FOR_THE_MORGUE,        SFX_M0_REST_IN_PEACE              },
	{ QUIP_WARNFRIENDS,        SFX_M0_INTRUDER_ALERT2,           SFX_M0_WEVE_GOT_TROUBLE,          SFX_M0_WEVE_GOT_PROBLEMS          },
	{ QUIP_GOFORALARM,         SFX_0037,                         SFX_0037,                         SFX_0037                          },
	{ QUIP_SURPRISED,          SFX_M0_WHAT_THE,                  SFX_M0_WHO_THE,                   SFX_M0_WHO_THE                    },
	{ QUIP_INSPECTBODY,        SFX_M0_ARE_YOU_OKAY,              SFX_M0_GOT_A_MAN_DOWN,            SFX_M0_HES_BOUGHT_IT              },
	{ QUIP_20,                 SFX_0037,                         SFX_0037,                         SFX_0037                          },
	{ QUIP_HITPLAYER,          SFX_M0_GIVE_IT_UP,                SFX_M0_SURRENDER_NOW,             SFX_M0_TAKE_THAT                  },
	{ QUIP_MISSEDPLAYER1,      SFX_M0_DAMN_IT_MISSED,            SFX_M0_DAMN_IT_MISSED,            SFX_M0_HOW_DID_I_MISS             },
	{ QUIP_MISSEDPLAYER2,      SFX_M0_GODS_SAKE_SOMEONE_HIT_HER, SFX_M0_GODS_SAKE_SOMEONE_HIT_HER, SFX_M0_SHES_A_TRICKY_ONE          },
	{ QUIP_GOTOCOVER2,         SFX_M0_TAKE_COVER_028D,           SFX_M0_ILL_COVER_YOU,             SFX_M0_GET_DOWN                   },
	{ QUIP_DIE,                SFX_M0_YOU_BITCH,                 SFX_M0_OH_MY_GOD,                 SFX_M0_SHE_GOT_ME                 },
	{ QUIP_26,                 SFX_0037,                         SFX_0037,                         SFX_0037                          },
	{ QUIP_SEARCHSUCCESS,      SFX_M0_I_SEE_HER,                 SFX_M0_THERE_SHE_IS,              SFX_M0_THERE_MOVEMENT             },
	{ QUIP_SEEEYESPY,          SFX_M0_WHAT_THE_HELL_8167,        SFX_M0_HELLO_THERE,               SFX_M0_WHATS_THIS                 },
	{ QUIP_GREETING,           SFX_M0_HOWS_THINGS,               SFX_M0_HEY_THERE,                 SFX_M0_HI_HOW_ARE_YOU             },
	{ QUIP_ASKWEAPON1,         MP3_M0_WHERE_DID_YOU_GET_THAT,    MP3_M0_SHOULD_YOU_HAVE_THAT,      MP3_M0_WHAT_ARE_YOU_DOING         },
	{ QUIP_ASKWEAPON2,         SFX_0037,                         MP3_M0_DONT_POINT_THAT_AT_ME,     MP3_M0_WATCH_WHERE_YOURE_POINTING },
	{ QUIP_UNCOVERDISGUISE1,   MP3_M0_IMPOSTER,                  MP3_M0_IMPOSTER,                  MP3_M0_ITS_A_SPY                  },
	{ QUIP_LOSTGUN,            SFX_M0_MY_GUN,                    SFX_M0_MY_GUN,                    SFX_M0_MY_GUN                     },
	{ QUIP_GOFORALARM,         SFX_M0_TRIGGER_THE_ALARM,         SFX_M0_TRIGGER_THE_ALARM,         SFX_M0_TRIGGER_THE_ALARM          },
	{ QUIP_SEARCHFAIL,         SFX_M0_IM_SURE_I_HEARD_A_NOISE,   SFX_M0_IM_SURE_I_HEARD_A_NOISE,   SFX_M0_HEARING_THINGS             },
	{ QUIP_ATTACK3,            SFX_M0_CLEAR_SHOT,                SFX_M0_CLEAR_SHOT,                SFX_M0_OPEN_FIRE                  },
	{ QUIP_ATTACK4,            SFX_M0_CLEAR_SHOT,                SFX_M0_OPEN_FIRE,                 SFX_M0_OPEN_FIRE                  },
	{ QUIP_KILLEDPLAYER2,      SFX_M0_GRAB_A_BODY_BAG,           SFX_M0_ONE_FOR_THE_MORGUE,        SFX_M0_REST_IN_PEACE              },
	{ QUIP_GUNJAMMED,          SFX_M0_MY_GUN,                    SFX_M0_MY_GUN,                    SFX_M0_MY_GUN                     },
	{ QUIP_UNCOVEREDDISGUISE2, MP3_M0_ITS_A_SPY,                 MP3_M0_ITS_A_SPY,                 MP3_M0_ITS_A_SPY                  },
	// Voicebox 1
	{ QUIP_ATTACK1,            SFX_M1_LAST_MISTAKE,            SFX_M1_WHAT_ARE_YOU_WAITING_FOR, SFX_M1_BRING_IT_ON                },
	{ QUIP_ATTACK2,            SFX_M1_TAKE_HER_DOWN,           SFX_M1_EVERYBODY_GET_HER,        SFX_M1_ATTACK                     },
	{ QUIP_GOTOCOVER1,         SFX_M1_COVER_MY_ASS,            SFX_M1_COVER_ME_NOW,             SFX_M1_IM_GOING_FOR_COVER         },
	{ QUIP_GRENADE1,           SFX_M0_M1_LOOK_OUT_LOOK_OUT,    SFX_M0_M1_ITS_A_GRENADE,         SFX_M0_M1_CLEAR_THE_AREA          },
	{ QUIP_RETREAT1,           SFX_M1_SHES_TOO_GOOD_RUN,       SFX_M1_GET_SOME_BACKUP,          SFX_M1_EVACUATE_THE_AREA          },
	{ QUIP_GRENADE2,           SFX_M1_CATCH_THIS,              SFX_M1_HERE_KEEP_IT,             SFX_M1_GRENADE                    },
	{ QUIP_RETREAT2,           SFX_M1_WITHDRAW,                SFX_M1_FALL_BACK,                SFX_M1_EVERYONE_GET_BACK          },
	{ QUIP_FLANK,              SFX_M1_SURROUND_HER,            SFX_M1_SPREAD_OUT,               SFX_M1_SPLIT_UP                   },
	{ QUIP_SURRENDER,          SFX_M1_PLEASE_DONT,             SFX_M1_DONT_SHOOT,               SFX_M1_IM_ONLY_DOING_MY_JOB       },
	{ QUIP_HEARNOISE,          SFX_M1_THAT_SOUNDED_LIKE,       SFX_M1_GUNFIRE,                  SFX_M1_SOMEONES_SHOOTING          },
	{ QUIP_10,                 SFX_0037,                       SFX_0037,                        SFX_0037                          },
	{ QUIP_SEEPLAYER,          SFX_M1_COME_HERE,               SFX_M1_THERES_SOMEONE_HERE,      SFX_M1_GET_HER                    },
	{ QUIP_SHOTUNALERT,        SFX_M1_WHOA,                    SFX_M1_MY_GOD,                   SFX_M1_IS_THAT_A_BULLET           },
	{ QUIP_INJURED1,           SFX_M1_OH_MY_GOD,               SFX_M1_IM_HIT_IM_HIT,            SFX_M1_IM_BLEEDING                },
	{ QUIP_INJURED2,           SFX_M1_OH_GOD_IM_HIT,           SFX_M1_HELP_ME_OUT,              SFX_M1_IM_IN_TROUBLE              },
	{ QUIP_KILLEDPLAYER1,      SFX_0037,                       SFX_0037,                        SFX_0037                          },
	{ QUIP_WARNFRIENDS,        SFX_M1_M2_LOOK_OUT_SHES_COMING, SFX_M1_M2_TAKE_COVER,            SFX_M1_M2_LOOK_OUT_LOOK_OUT       },
	{ QUIP_GOFORALARM,         SFX_0037,                       SFX_0037,                        SFX_0037                          },
	{ QUIP_SURPRISED,          SFX_M1_HOLY,                    SFX_M1_WHAT_THE_HELL,            SFX_M1_WHA                        },
#if VERSION >= VERSION_NTSC_1_0
	{ QUIP_INSPECTBODY,        SFX_M1_NOOO,                    SFX_M1_OH_GOD_HES_DEAD,          SFX_M1_HES_GONE                   },
#else
	{ QUIP_INSPECTBODY,        SFX_0313,                       SFX_M1_NOOO,                     SFX_M1_HES_GONE                   },
#endif
	{ QUIP_20,                 SFX_M1_M2_LOOK_OUT_SHES_COMING, SFX_M1_M2_TAKE_COVER,            SFX_M1_M2_LOOK_OUT_LOOK_OUT       },
	{ QUIP_HITPLAYER,          SFX_M1_IM_JUST_TOO_GOOD,        SFX_M1_YEAH_BABY,                SFX_M1_YEAH_BABY                  },
	{ QUIP_MISSEDPLAYER1,      SFX_M1_BLOODY_STUPID_GUN,       SFX_M1_MY_GUN_ITS_USELESS,       SFX_M1_DAMN_IT                    },
	{ QUIP_MISSEDPLAYER2,      SFX_M1_STOP_DODGING,            SFX_M1_SOMEONE_HIT_HER,          SFX_M1_DAMN_SHES_GOOD             },
	{ QUIP_GOTOCOVER2,         SFX_M1_GO_FOR_IT,               SFX_M1_GO_GO_GO,                 SFX_M1_RUN                        },
#if VERSION >= VERSION_NTSC_1_0
	{ QUIP_DIE,                SFX_M1_SCREAM,                  SFX_M1_WHY_ME,                   SFX_M1_CHOKING                    },
#else
	{ QUIP_DIE,                SFX_034C,                       SFX_M1_WHY_ME,                   SFX_M1_CHOKING                    },
#endif
	{ QUIP_26,                 SFX_M1_OUTSTANDING,             SFX_M1_IM_JUST_TOO_GOOD,         SFX_M1_YEEHAH_GOT_ONE             },
	{ QUIP_SEARCHSUCCESS,      SFX_M1_OVER_THERE,              SFX_M1_HALT,                     SFX_M1_FREEZE                     },
	{ QUIP_SEEEYESPY,          SFX_M1_WHAT_IS_IT,              SFX_M1_HOW_DID_THAT_GET_HERE,    SFX_M1_DONT_TOUCH_IT              },
	{ QUIP_GREETING,           SFX_M1_HI_THERE,                SFX_M1_HI_THERE,                 SFX_M1_HOWS_THINGS                },
	{ QUIP_ASKWEAPON1,         MP3_M1_WHERE_ARE_YOU_TAKING,    MP3_M1_WHERE_ARE_YOU_TAKING,     MP3_M1_GET_THAT_FIREARM_APPROVED  },
	{ QUIP_ASKWEAPON2,         MP3_M1_WATCH_WHAT_YOURE_DOING,  MP3_M1_WATCH_WHAT_YOURE_DOING,   MP3_M1_BE_CAREFUL                 },
	{ QUIP_UNCOVERDISGUISE1,   MP3_M1_STOP_RIGHT_THERE,        MP3_M1_STOP_RIGHT_THERE,         MP3_M1_DROP_THE_GUN               },
	{ QUIP_LOSTGUN,            SFX_M1_M2_GEEZ,                 SFX_M1_M2_GEEZ,                  SFX_M1_M2_GEEZ                    },
	{ QUIP_GOFORALARM,         SFX_M1_WARN_THE_OTHERS,         SFX_M1_WARN_THE_OTHERS,          SFX_M1_WARN_THE_OTHERS            },
	{ QUIP_SEARCHFAIL,         SFX_M1_I_CANT_SEE_ANYBODY,      SFX_M1_THERES_NO_ONE_HERE,       SFX_M1_THERES_NO_ONE_HERE         },
	{ QUIP_ATTACK3,            SFX_M1_LAST_MISTAKE,            SFX_M1_WHAT_ARE_YOU_WAITING_FOR, SFX_M1_BRING_IT_ON                },
	{ QUIP_ATTACK4,            SFX_M1_BRING_IT_ON,             SFX_M1_ATTACK,                   SFX_M1_ATTACK                     },
	{ QUIP_KILLEDPLAYER2,      SFX_M1_IM_JUST_TOO_GOOD,        SFX_M1_YEEHAH_GOT_ONE,           SFX_M1_ANOTHER_ONE_BITES_THE_DUST },
	{ QUIP_GUNJAMMED,          SFX_M1_BLOODY_STUPID_GUN,       SFX_M1_MY_GUN_ITS_USELESS,       SFX_M1_DAMN_IT                    },
	{ QUIP_UNCOVEREDDISGUISE2, MP3_M1_STOP_RIGHT_THERE,        MP3_M1_STOP_RIGHT_THERE,         MP3_M1_STOP_RIGHT_THERE           },
	// Voicebox 2
	{ QUIP_ATTACK1,            SFX_M2_COME_ON_MAN2,                    SFX_M2_DIE,                             SFX_M2_TAKE_THIS                        },
	{ QUIP_ATTACK2,            SFX_M2_MOVE_IN,                         SFX_M2_YOURE_OUT_OF_YOUR_LEAGUE,        SFX_M2_LET_HER_HAVE_IT                  },
	{ QUIP_GOTOCOVER1,         SFX_M2_HELP_ME_OUT_HERE,                SFX_M2_HEY_DISTRACT_HER,                SFX_M2_KEEP_HER_OCCUPIED                },
	{ QUIP_GRENADE1,           SFX_M2_GET_BACK_QUICK,                  SFX_M2_WERE_GONNA_DIE,                  SFX_M2_GOD_RUN                          },
	{ QUIP_RETREAT1,           SFX_M2_RETREAT,                         SFX_M2_LETS_GET_THE_HELL_OUT_OF_HERE,   SFX_M2_GET_BACK_GET_BACK                },
	{ QUIP_GRENADE2,           SFX_M2_FIRE_IN_THE_HOLE,                SFX_M2_HERES_A_LITTLE_PRESENT_FOR_YA,   SFX_M2_TRY_THIS_FOR_SIZE                },
	{ QUIP_RETREAT2,           SFX_M2_GET_OUT_OF_THE_WAY,              SFX_M2_FALL_BACK,                       SFX_M2_MOVE_OUT                         },
	{ QUIP_FLANK,              SFX_M2_TEAM_UP_GUYS,                    SFX_M2_COME_ON_AROUND_THE_SIDE,         SFX_M2_SCATTER                          },
	{ QUIP_SURRENDER,          SFX_M2_I_DONT_LIKE_THIS_ANY_MORE,       SFX_M2_DONT_HURT_ME,                    SFX_M2_YOU_WIN_I_GIVE_UP                },
	{ QUIP_HEARNOISE,          SFX_M2_LISTEN_GUNSHOTS,                 SFX_M2_LISTEN_GUNSHOTS,                 SFX_M2_SOMEONES_NEARBY                  },
	{ QUIP_10,                 SFX_0037,                               SFX_0037,                               SFX_0037                                },
	{ QUIP_SEEPLAYER,          SFX_M2_TARGET_SIGHTED,                  SFX_M2_COME_ON_MAN,                     SFX_M2_GOD_DAMN_IT                      },
	{ QUIP_SHOTUNALERT,        SFX_M2_THAT_WAS_CLOSE,                  SFX_M2_HOLY_MOLY,                       SFX_M2_AY_CARAMBA                       },
	{ QUIP_INJURED1,           SFX_M2_GEEZ_THAT_HURT,                  SFX_M2_WHY_YOU,                         SFX_M2_WHY_YOU                          },
	{ QUIP_INJURED2,           SFX_M2_IM_INJURED,                      SFX_M2_IM_HIT_IM_HIT,                   SFX_M2_DAMN_IT_IM_TAKING_FIRE           },
	{ QUIP_KILLEDPLAYER1,      SFX_0037,                               SFX_0037,                               SFX_0037                                },
	{ QUIP_WARNFRIENDS,        SFX_M2_WATCH_OUT,                       SFX_M2_HELP_ME_OUT,                     SFX_M2_WEVE_GOT_AN_INTRUDER             },
	{ QUIP_GOFORALARM,         SFX_0037,                               SFX_0037,                               SFX_0037                                },
	{ QUIP_SURPRISED,          SFX_M2_HOW_THE,                         SFX_M2_HEY,                             SFX_M2_STOP                             },
	{ QUIP_INSPECTBODY,        SFX_M2_BODY_COUNTS_TOO_HIGH,            SFX_M2_I_NEVER_LIKED_HIM_ANYWAY,        SFX_M2_THAT_WAS_MY_BEST_FRIEND          },
	{ QUIP_20,                 SFX_M1_M2_LOOK_OUT_SHES_COMING,         SFX_M1_M2_TAKE_COVER,                   SFX_M1_M2_LOOK_OUT_LOOK_OUT             },
	{ QUIP_HITPLAYER,          SFX_M2_SURRENDER_OR_DIE,                SFX_M2_I_HAVE_YOU_NOW,                  SFX_M2_YOU_WANT_BEAT_ME                 },
	{ QUIP_MISSEDPLAYER1,      SFX_M2_DAMN_MISSED_AGAIN,               SFX_M2_I_DONT_BELIEVE_IT,               SFX_M2_DAMN_YOU                         },
	{ QUIP_MISSEDPLAYER2,      SFX_M2_HELL_SHES_GOOD,                  SFX_M2_STOP_MOVING,                     SFX_M2_NO_ESCAPE_FOR_YOU                },
	{ QUIP_GOTOCOVER2,         SFX_M2_MOVE_IT_MOVE_IT,                 SFX_M2_GET_TO_COVER_NOW,                SFX_M2_RUN_FOR_IT                       },
#if VERSION >= VERSION_NTSC_1_0
	{ QUIP_DIE,                SFX_M2_NOOO,                            SFX_M2_OH_GOD_IM_DYING,                 SFX_M2_I_DONT_WANT_TO_DIE               },
#else
	{ QUIP_DIE,                SFX_0411,                               SFX_M2_OH_GOD_IM_DYING,                 SFX_M2_I_DONT_WANT_TO_DIE               },
#endif
	{ QUIP_26,                 SFX_M2_IM_THE_MAN,                      SFX_M2_BOY_THAT_WAS_CLOSE,              SFX_M2_DID_YOU_SEE_THAT                 },
	{ QUIP_SEARCHSUCCESS,      SFX_M2_GET_HER,                         SFX_M2_THERE_ATTACK,                    SFX_M2_HEY_YOU_STOP                     },
	{ QUIP_SEEEYESPY,          SFX_M2_IS_IT_DANGEROUS,                 SFX_M2_DONT_MOVE,                       SFX_M2_STAY_BACK                        },
	{ QUIP_GREETING,           SFX_M2_HELLO,                           SFX_M2_HELLO,                           SFX_M2_HEY_WHATS_UP                     },
	{ QUIP_ASKWEAPON1,         MP3_M2_DO_YOU_HAVE_PERMISSION_FOR_THAT, MP3_M2_DO_YOU_HAVE_PERMISSION_FOR_THAT, MP3_M2_WHAT_ARE_YOU_DOING               },
	{ QUIP_ASKWEAPON2,         MP3_M2_HEY_THATS_DANGEROUS,             MP3_M2_HEY_THATS_DANGEROUS,             MP3_M2_WATCH_IT_THAT_THING_COULD_GO_OFF },
	{ QUIP_UNCOVERDISGUISE1,   MP3_M2_PUT_YOUR_HANDS_UP,               MP3_M2_PUT_YOUR_HANDS_UP,               MP3_M2_ITS_A_TERRORIST                  },
	{ QUIP_LOSTGUN,            SFX_M1_M2_GEEZ,                         SFX_M1_M2_GEEZ,                         SFX_M1_M2_GEEZ                          },
	{ QUIP_GOFORALARM,         SFX_M2_ACTIVATE_THE_ALARM,              SFX_M2_ACTIVATE_THE_ALARM,              SFX_M2_ACTIVATE_THE_ALARM               },
	{ QUIP_SEARCHFAIL,         SFX_M2_I_BET_THIS_IS_ANOTHER_DRILL,     SFX_M2_I_BET_THIS_IS_ANOTHER_DRILL,     SFX_M2_ANOTHER_FALSE_ALARM              },
	{ QUIP_ATTACK3,            SFX_M2_COME_ON_MAN2,                    SFX_M2_DIE,                             SFX_M2_TAKE_THIS                        },
	{ QUIP_ATTACK4,            SFX_M2_DIE,                             SFX_M2_YOURE_OUT_OF_YOUR_LEAGUE,        SFX_M2_YOURE_OUT_OF_YOUR_LEAGUE         },
	{ QUIP_KILLEDPLAYER2,      SFX_M2_IM_THE_MAN,                      SFX_M2_ITS_ALL_OVER_FOR_THIS_ONE,       SFX_M2_DID_YOU_SEE_THAT                 },
	{ QUIP_GUNJAMMED,          SFX_M2_GOD_DAMN_IT,                     SFX_M2_I_DONT_BELIEVE_IT,               SFX_M2_GOD_DAMN_IT                      },
	{ QUIP_UNCOVEREDDISGUISE2, MP3_M2_ITS_A_TERRORIST,                 MP3_M2_ITS_A_TERRORIST,                 MP3_M2_ITS_A_TERRORIST                  },
	// Voicebox 3 (female)
	{ QUIP_ATTACK1,            SFX_F_COME_ON,            SFX_F_COME_ON,            SFX_F_COME_ON           },
	{ QUIP_ATTACK2,            SFX_0037,                 SFX_F_EVERYONE_GET_HER,   SFX_F_ATTACK            },
	{ QUIP_GOTOCOVER1,         SFX_F_COVER_ME,           SFX_0037,                 SFX_F_TAKE_COVER        },
	{ QUIP_GRENADE1,           SFX_F_LOOK_OUT,           SFX_F_ITS_A_GRENADE,      SFX_F_ITS_A_GRENADE     },
	{ QUIP_RETREAT1,           SFX_F_GET_REINFORCEMENTS, SFX_F_EVACUATE_THE_AREA,  SFX_F_RETREAT           },
	{ QUIP_GRENADE2,           SFX_F_CATCH_THIS,         SFX_F_TIME_TO_DIE,        SFX_0037                },
	{ QUIP_RETREAT2,           SFX_F_WITHDRAW,           SFX_F_WITHDRAW,           SFX_F_FALL_BACK         },
	{ QUIP_FLANK,              SFX_0037,                 SFX_F_SPREAD_OUT,         SFX_F_SPLIT_UP          },
	{ QUIP_SURRENDER,          SFX_F_PLEASE_DONT,        SFX_F_PLEASE_DONT,        SFX_F_DONT_SHOOT        },
	{ QUIP_HEARNOISE,          SFX_0037,                 SFX_F_0389,               SFX_F_SOMEONES_SHOOTING },
	{ QUIP_10,                 0,                        0,                        0                       },
	{ QUIP_SEEPLAYER,          SFX_F_GET_HER,            SFX_F_HEY_YOU_COME_HERE,  SFX_0037                },
	{ QUIP_SHOTUNALERT,        SFX_F_UNDER_FIRE,         SFX_0037,                 SFX_F_WERE_UNDER_FIRE   },
	{ QUIP_INJURED1,           SFX_0037,                 SFX_F_IM_WOUNDED,         SFX_F_HELP_ME_OUT       },
	{ QUIP_INJURED2,           SFX_F_IM_WOUNDED,         SFX_F_HELP_ME_OUT,        SFX_F_IM_IN_TROUBLE     },
	{ QUIP_KILLEDPLAYER1,      0,                        0,                        0                       },
	{ QUIP_WARNFRIENDS,        SFX_F_TARGET_ATTACKING,   SFX_F_UNDER_FIRE,         SFX_0037                },
	{ QUIP_GOFORALARM,         SFX_F_GET_REINFORCEMENTS, SFX_F_EVACUATE_THE_AREA,  SFX_F_RETREAT           },
	{ QUIP_SURPRISED,          SFX_0037,                 SFX_F_HEY,                SFX_F_HUH               },
	{ QUIP_INSPECTBODY,        SFX_F_UNIT_DOWN,          SFX_F_UNIT_DOWN,          SFX_F_UNIT_DOWN         },
	{ QUIP_20,                 SFX_F_TARGET_ATTACKING,   SFX_F_UNDER_FIRE,         SFX_F_WERE_UNDER_FIRE   },
	{ QUIP_HITPLAYER,          SFX_F_DID_THAT_HURT,      SFX_F_YOU_WANT_SOME_MORE, SFX_0037                },
	{ QUIP_MISSEDPLAYER1,      SFX_F_THIS_GUNS_USELESS,  SFX_0037,                 SFX_F_STAND_STILL       },
#if VERSION >= VERSION_NTSC_1_0
	{ QUIP_MISSEDPLAYER2,      SFX_F_STAND_STILL,        SFX_F_SOMEONE_HIT_HER,    SFX_F_DAMN_SHES_GOOD    },
#else
	{ QUIP_MISSEDPLAYER2,      SFX_F_STAND_STILL,        SFX_F_SOMEONE_HIT_HER,    SFX_M1_SCREAM           },
#endif
	{ QUIP_GOTOCOVER2,         SFX_F_GO_FOR_IT,          SFX_0037,                 SFX_F_RUN               },
	{ QUIP_DIE,                SFX_F_WHY_ME,             SFX_F_NOO,                SFX_F_MY_GOD            },
	{ QUIP_26,                 SFX_F_IM_JUST_TOO_GOOD,   SFX_0037,                 SFX_F_SUCH_A_WASTE      },
	{ QUIP_SEARCHSUCCESS,      SFX_F_GET_HER,            SFX_F_HEY_YOU_COME_HERE,  SFX_0037                },
	{ QUIP_SEEEYESPY,          0,                        0,                        0                       },
	{ QUIP_GREETING,           SFX_0037,                 SFX_F_COME_ON,            SFX_0037                },
	{ QUIP_ASKWEAPON1,         SFX_0037,                 SFX_F_COME_ON,            SFX_0037                },
	{ QUIP_ASKWEAPON2,         SFX_0037,                 SFX_F_COME_ON,            SFX_0037                },
	{ QUIP_UNCOVERDISGUISE1,   SFX_0037,                 SFX_F_COME_ON,            SFX_0037                },
	{ QUIP_LOSTGUN,            SFX_F_HEY,                SFX_F_HUH,                SFX_F_HEY               },
	{ QUIP_GOFORALARM,         SFX_F_GET_REINFORCEMENTS, SFX_F_FALL_BACK,          SFX_F_EVACUATE_THE_AREA },
	{ QUIP_SEARCHFAIL,         0,                        0,                        0                       },
	{ QUIP_ATTACK3,            SFX_F_COME_ON,            SFX_F_COME_ON,            SFX_F_ATTACK            },
	{ QUIP_ATTACK4,            SFX_F_ATTACK,             SFX_F_ATTACK,             SFX_F_COME_ON           },
	{ QUIP_KILLEDPLAYER2,      SFX_F_GET_A_CLEANER,      SFX_F_IM_JUST_TOO_GOOD,   SFX_F_SUCH_A_WASTE      },
	{ QUIP_GUNJAMMED,          SFX_F_THIS_GUNS_USELESS,  SFX_F_THIS_GUNS_USELESS,  SFX_F_THIS_GUNS_USELESS },
	{ QUIP_UNCOVEREDDISGUISE2, 0,                        0,                        0                       },
};

s16 g_SpecialQuipBank[][4] = {
#if VERSION >= VERSION_NTSC_1_0
	{ 0,  MP3_CASS_HOW_DARE_YOU_DISTURB_ME,     MP3_CASS_YOU_WILL_REGRET,               MP3_CASS_LEAVE_NOW                 },
#else
	{ 0,  0x12f2,                               0x12f3,                                 0x12f8                             },
#endif
	{ 1,  MP3_SEC_PLEASE_DONT_KILL_ME,          MP3_SEC_DONT_SHOOT,                     MP3_SEC_PLEASE_DONT_KILL_ME        },
	{ 2,  SFX_DRCAROLL_COME_ON,                 SFX_DRCAROLL_TAKING_YOUR_TIME,          SFX_DRCAROLL_GET_OUT_OF_HERE       },
	{ 3,  SFX_DRCAROLL_KNOW_WHAT_YOURE_DOING,   SFX_DRCAROLL_0249,                      SFX_DRCAROLL_GOING_TO_THE_HELIPAD  },
	{ 4,  SFX_DRCAROLL_STOP_THAT,               SFX_DRCAROLL_WHAT,                      SFX_ARGH_DRCAROLL_0240             },
	{ 5,  SFX_ARGH_DRCAROLL_024C,               SFX_DRCAROLL_QUITE_ENOUGH,              SFX_ARGH_DRCAROLL_0251             },
	{ 6,  SFX_DRCAROLL_I_CANT_MAKE_IT,          SFX_DRCAROLL_YOU_WERE_SUPPOSED,         SFX_DRCAROLL_SYSTEMS_FAILURE       },
	{ 7,  SFX_DRCAROLL_OH_CRIKEY,               SFX_DRCAROLL_GOODNESS_GRACIOUS,         SFX_DRCAROLL_DONT_THEY_KNOW        },
	{ 8,  MP3_K7_IM_NOT_SURE,                   MP3_K7_ONE_MORE_TIME,                   MP3_K7_I_DUNNO                     },
	{ 9,  MP3_SCI_AND_AGAIN,                    MP3_SCI_TRY_IT_NOW,                     MP3_SCI_AND_AGAIN                  },
	{ 10, MP3_SCI_WHAT_ARE_YOU_DOING,           MP3_SCI_HAVENT_SEEN_YOU,                SFX_SCI_WHO_THE_HELL_ARE_YOU       },
	{ 11, MP3_JO_SHUT_DOWN_EXPERIMENTS,         MP3_JO_PULL_THE_PLUG,                   MP3_JO_SWITCH_THIS_THING_OFF       },
	{ 12, MP3_SCI_ILL_SHUT_IT_DOWN,             MP3_SCI_PLEASE_DONT_HURT_ME,            MP3_SCI_ALLOW_ME_TO_ASSIST_YOU     },
	{ 13, MP3_SCI_MY_EXPERIMENTS,               MP3_SCI_EXPERIMENT_IS_DOWN,             MP3_SCI_LEAVE_THIS_AREA            },
	{ 14, MP3_SCI_SECURITY,                     MP3_SCI_IM_CALLING_SECURITY,            MP3_SCI_ACCIDENTS_WILL_HAPPEN      },
	{ 15, MP3_SCI_HOW_DID_THAT_HAPPEN,          MP3_SCI_OFF_ALREADY,                    MP3_SCI_HAVE_YOU_BEEN_TAMPERING    },
	{ 16, MP3_SCI_SOMEONES_BROKEN_MY_EQUIPMENT, MP3_SCI_WHATS_HAPPENED_TO_THE_TERMINAL, MP3_SCI_YOU_VANDAL                 },
	{ 17, SFX_ARGH_FEMALE_000D,                 SFX_ARGH_FEMALE_000D,                   SFX_ARGH_FEMALE_000D               },
	{ 18, SFX_M0_HOWS_THINGS,                   SFX_M0_HEY_THERE,                       SFX_M0_HI_HOW_ARE_YOU              },
	{ 19, MP3_M0_WHERE_DID_YOU_GET_THAT,        MP3_M0_SHOULD_YOU_HAVE_THAT,            MP3_M0_WHAT_ARE_YOU_DOING          },
	{ 20, SFX_0037,                             MP3_M0_DONT_POINT_THAT_AT_ME,           MP3_M0_WATCH_WHERE_YOURE_POINTING  },
	{ 21, MP3_M0_IMPOSTER,                      MP3_M0_IMPOSTER,                        MP3_M0_ITS_A_SPY                   },
	{ 22, SFX_CIV_GREETINGS_CITIZEN,            SFX_CIV_HEY_SUGAR_WANNA_PARTY,          SFX_CIV_HEY_BABY                   },
	{ 23, SFX_CIV_HOWS_IT_GOING,                SFX_CIV_TAKE_IT_EASY,                   SFX_CIV_WHISTLE                    },
	{ 24, SFX_CIV_OH_MY_GOD,                    SFX_CIV_I_DONT_WANT_ANY_TROUBLE,        SFX_CIV_TAKE_THE_WALLET            },
	{ 25, SFX_CIV_THERES_A_MANIAC,              SFX_CIV_QUICK_DOWN_THERE,               SFX_CIV_GET_ME_OUT_OF_HERE         },
	{ 26, SFX_CIV_GUNS_DONT_SCARE_ME,           SFX_CIV_KEEP_AWAY_FROM_THIS_CAR,        SFX_CIV_KEEP_AWAY_FROM_THIS_CAR    },
	{ 27, SFX_FBI_WE_HAVE_AN_INTRUDER,          SFX_FBI_CODE_2_SITUATION,               SFX_FBI_REQUEST_BACKUP_IMMEDIATELY },
	{ 28, SFX_SHOULD_HAVE_COME_HERE_GIRL,       SFX_SECURE_THE_PERIMETER,               SFX_WERE_TAKING_OVER               },
	{ 29, SFX_FBI_WE_HAVE_AN_INTRUDER,          SFX_FBI_CODE_2_SITUATION,               SFX_FBI_REQUEST_BACKUP_IMMEDIATELY },
	{ 30, SFX_FBI_WE_HAVE_AN_INTRUDER,          SFX_FBI_CODE_2_SITUATION,               SFX_FBI_REQUEST_BACKUP_IMMEDIATELY },
	{ 31, MP3_ROBOT_STOP_WHERE_YOU_ARE,         MP3_ROBOT_STOP_WHERE_YOU_ARE,           MP3_ROBOT_STOP_WHERE_YOU_ARE       },
	{ 32, MP3_COME_BACK_LATER_IM_BUSY,          MP3_GO_AWAY,                            MP3_LOOK_I_CANT_HELP_YOU           },
	{ 33, MP3_PRES_STOP_SHOOTING,               MP3_PRES_YOU_SURE_THIS_IS_THE_WAY,      MP3_PRES_WHY_ARE_THEY_SHOOTING     },
	{ 34, SFX_CIV_GREETINGS_CITIZEN,            SFX_CIV_HEY_SUGAR_WANNA_PARTY,          0                                  },
	{ 35, SFX_CIV_HOWS_IT_GOING,                SFX_CIV_TAKE_IT_EASY,                   0                                  },
	{ 36, SFX_CIV_OH_MY_GOD,                    SFX_CIV_I_DONT_WANT_ANY_TROUBLE,        MP3_SEC_PLEASE_DONT_KILL_ME        },
	{ 37, SFX_CIV_THERES_A_MANIAC,              SFX_CIV_QUICK_DOWN_THERE,               0                                  },
	{ 38, SFX_ELVIS_INTERGALACTIC_PEACE,        SFX_ELVIS_EAT_HOT_LEAD_WEIRDOS,         SFX_ELVIS_KISS_MY_ALIEN_BUTT       },
	{ 39, SFX_ELVIS_ILL_KICK_YOUR_ASS,          SFX_ELVIS_FOR_YOUR_OWN_GOOD,            SFX_ELVIS_YOU_DARE_SHOOT_AT_ME     },
	{ 40, SFX_ELVIS_DONT_MESS_WITH_THE_MAIAN,   SFX_ELVIS_IM_BAD,                       SFX_ELVIS_HEHE                     },
	{ 41, SFX_ELVIS_ALL_GOING_WRONG,            SFX_ELVIS_ALL_GOING_WRONG,              SFX_ELVIS_WATCH_THE_SUIT           },
	{ 42, MP3_M2_ITS_A_TERRORIST,               MP3_M0_ITS_A_SPY,                       MP3_M1_STOP_RIGHT_THERE            },
#if VERSION >= VERSION_NTSC_1_0
	{ 43, MP3_CASS_HOW_DARE_YOU_DISTURB_ME,     MP3_CASS_LEAVE_NOW,                     MP3_CASS_LEAVE_NOW                 },
#else
	{ 43, 0x12f2,                               0x12f8,                                 0x12f8,                            },
#endif
	{ 44, SFX_ELVIS_HEHE,                       SFX_ELVIS_HEHE,                         SFX_ELVIS_HEHE                     },
	{ 45, MP3_ROBOT_ALERT_UNDER_ATTACK,         MP3_ROBOT_ALERT_UNDER_ATTACK,           MP3_ROBOT_ALERT_UNDER_ATTACK       },
};

s16 g_QuipTexts[][4] = {
	{  1, L_AME_083, L_AME_084, L_AME_085 }, // "How dare you disturb me!", "You will regret this intrusion, girl!", "If I were you, I'd leave...NOW!"
	{  2, L_AME_086, L_AME_087, L_AME_086 }, // "Please don't kill me!", "Don't shoot!"
	{  3, L_EAR_064, L_EAR_065, L_EAR_066 }, // "What are you doing in my lab?", "I haven't seen you before...", "Who the hell are you?"
	{  4, L_EAR_067, L_EAR_068, L_EAR_069 }, // "Shut down the experiment.", "Pull the plug on that, NOW.", "Switch this thing off."
	{  5, L_EAR_070, L_EAR_071, L_EAR_072 }, // "I'll shut it down.", "Please don't hurt me.", "Allow me to assist you."
	{  6, L_EAR_073, L_EAR_074, L_EAR_075 }, // "My experiments!", "There the experiment is down.", "Leave this area NOW!"
	{  7, L_EAR_076, L_EAR_077, L_EAR_078 }, // "Security!", "I'm calling security.", "Accidents will happen."
	{  8, L_EAR_079, L_EAR_080, L_EAR_081 }, // "How did that happen?", "Looks like it's off already.", "Have you been tampering with this?"
	{  9, L_EAR_082, L_EAR_083, L_EAR_084 }, // "Someone's broken my equipment.", "What's happened to the terminal?", "You vandal - you've broken it."
	{ 10, L_WAX_010, L_WAX_012, L_WAX_012 }, // "How dare you disturb me!", "If I were you, I'd leave... NOW!"
};

s16 g_SkedarQuipBank[][4] = {
	{ 1, SFX_SKEDAR_ROAR_0529, SFX_SKEDAR_ROAR_052A, SFX_SKEDAR_ROAR_052B },
	{ 2, SFX_SKEDAR_ROAR_052D, SFX_SKEDAR_ROAR_052E, SFX_SKEDAR_ROAR_052F },
	{ 3, SFX_SKEDAR_ROAR_0530, SFX_SKEDAR_ROAR_0530, SFX_SKEDAR_ROAR_0531 },
	{ 1, SFX_SKEDAR_ROAR_0532, SFX_SKEDAR_ROAR_0533, SFX_SKEDAR_ROAR_0534 },
	{ 2, SFX_SKEDAR_ROAR_0536, SFX_SKEDAR_ROAR_0537, SFX_SKEDAR_ROAR_0538 },
	{ 3, SFX_SKEDAR_ROAR_0539, SFX_SKEDAR_ROAR_0539, SFX_SKEDAR_ROAR_053A },
	{ 0 },
};

s16 g_MaianQuipBank[][4] = {
	{ 1, SFX_MAIAN_05E2,      SFX_MAIAN_05E3,      SFX_MAIAN_05E4      },
	{ 2, SFX_MAIAN_05E5,      SFX_MAIAN_05E6,      SFX_MAIAN_05E7      },
	{ 3, SFX_ARGH_MAIAN_05DF, SFX_ARGH_MAIAN_05E0, SFX_ARGH_MAIAN_05E1 },
	{ 0 },
};

/**
 * @cmd 0130
 */
#if MATCHING
#if PAL
GLOBAL_ASM(
glabel aiSayQuip
.late_rodata
glabel var7f1a9d64
.word 0x45dac000
.text
/*  f05a454:	27bdff58 */ 	addiu	$sp,$sp,-168
/*  f05a458:	afb00048 */ 	sw	$s0,0x48($sp)
/*  f05a45c:	3c10800a */ 	lui	$s0,%hi(g_Vars)
/*  f05a460:	2610a510 */ 	addiu	$s0,$s0,%lo(g_Vars)
/*  f05a464:	8e0e0434 */ 	lw	$t6,0x434($s0)
/*  f05a468:	8e0f0438 */ 	lw	$t7,0x438($s0)
/*  f05a46c:	afbf004c */ 	sw	$ra,0x4c($sp)
/*  f05a470:	8e040424 */ 	lw	$a0,0x424($s0)
/*  f05a474:	01cf1021 */ 	addu	$v0,$t6,$t7
/*  f05a478:	90450002 */ 	lbu	$a1,0x2($v0)
/*  f05a47c:	0fc12737 */ 	jal	chrFindById
/*  f05a480:	afa2009c */ 	sw	$v0,0x9c($sp)
/*  f05a484:	8e18028c */ 	lw	$t8,0x28c($s0)
/*  f05a488:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f05a48c:	8fa8009c */ 	lw	$t0,0x9c($sp)
/*  f05a490:	afb80078 */ 	sw	$t8,0x78($sp)
/*  f05a494:	94f90192 */ 	lhu	$t9,0x192($a3)
/*  f05a498:	91060003 */ 	lbu	$a2,0x3($t0)
/*  f05a49c:	00402825 */ 	move	$a1,$v0
/*  f05a4a0:	33291000 */ 	andi	$t1,$t9,0x1000
/*  f05a4a4:	10e00003 */ 	beqz	$a3,.PF0f05a4b4
/*  f05a4a8:	a3a9006b */ 	sb	$t1,0x6b($sp)
/*  f05a4ac:	10000002 */ 	b	.PF0f05a4b8
/*  f05a4b0:	90e302fe */ 	lbu	$v1,0x2fe($a3)
.PF0f05a4b4:
/*  f05a4b4:	00001825 */ 	move	$v1,$zero
.PF0f05a4b8:
/*  f05a4b8:	24010001 */ 	li	$at,0x1
/*  f05a4bc:	14610007 */ 	bne	$v1,$at,.PF0f05a4dc
/*  f05a4c0:	3c0a8007 */ 	lui	$t2,%hi(g_SkedarQuipBank)
/*  f05a4c4:	254a93b8 */ 	addiu	$t2,$t2,%lo(g_SkedarQuipBank)
/*  f05a4c8:	28c10006 */ 	slti	$at,$a2,0x6
/*  f05a4cc:	1420002a */ 	bnez	$at,.PF0f05a578
/*  f05a4d0:	afaa0084 */ 	sw	$t2,0x84($sp)
/*  f05a4d4:	10000028 */ 	b	.PF0f05a578
/*  f05a4d8:	00003025 */ 	move	$a2,$zero
.PF0f05a4dc:
/*  f05a4dc:	80eb0006 */ 	lb	$t3,0x6($a3)
/*  f05a4e0:	24030029 */ 	li	$v1,0x29
/*  f05a4e4:	3c0c8007 */ 	lui	$t4,%hi(g_MaianQuipBank)
/*  f05a4e8:	146b000d */ 	bne	$v1,$t3,.PF0f05a520
/*  f05a4ec:	258c93f0 */ 	addiu	$t4,$t4,%lo(g_MaianQuipBank)
/*  f05a4f0:	28c10003 */ 	slti	$at,$a2,0x3
/*  f05a4f4:	14200020 */ 	bnez	$at,.PF0f05a578
/*  f05a4f8:	afac0084 */ 	sw	$t4,0x84($sp)
/*  f05a4fc:	afa60070 */ 	sw	$a2,0x70($sp)
/*  f05a500:	0c004ad4 */ 	jal	random
/*  f05a504:	afa5007c */ 	sw	$a1,0x7c($sp)
/*  f05a508:	30460001 */ 	andi	$a2,$v0,0x1
/*  f05a50c:	afa60070 */ 	sw	$a2,0x70($sp)
/*  f05a510:	8fa5007c */ 	lw	$a1,0x7c($sp)
/*  f05a514:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f05a518:	10000017 */ 	b	.PF0f05a578
/*  f05a51c:	8fa8009c */ 	lw	$t0,0x9c($sp)
.PF0f05a520:
/*  f05a520:	910e0007 */ 	lbu	$t6,0x7($t0)
/*  f05a524:	3c0b8007 */ 	lui	$t3,%hi(g_SpecialQuipBank)
/*  f05a528:	256b91f8 */ 	addiu	$t3,$t3,%lo(g_SpecialQuipBank)
/*  f05a52c:	55c00012 */ 	bnezl	$t6,.PF0f05a578
/*  f05a530:	afab0084 */ 	sw	$t3,0x84($sp)
/*  f05a534:	90e202b1 */ 	lbu	$v0,0x2b1($a3)
/*  f05a538:	240f0003 */ 	li	$t7,0x3
/*  f05a53c:	28410004 */ 	slti	$at,$v0,0x4
/*  f05a540:	14200004 */ 	bnez	$at,.PF0f05a554
/*  f05a544:	00000000 */ 	nop
/*  f05a548:	a0ef02b1 */ 	sb	$t7,0x2b1($a3)
/*  f05a54c:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f05a550:	90e202b1 */ 	lbu	$v0,0x2b1($a3)
.PF0f05a554:
/*  f05a554:	00430019 */ 	multu	$v0,$v1
/*  f05a558:	3c098007 */ 	lui	$t1,%hi(g_GuardQuipBank)
/*  f05a55c:	25298cd8 */ 	addiu	$t1,$t1,%lo(g_GuardQuipBank)
/*  f05a560:	0000c012 */ 	mflo	$t8
/*  f05a564:	0018c8c0 */ 	sll	$t9,$t8,0x3
/*  f05a568:	03295021 */ 	addu	$t2,$t9,$t1
/*  f05a56c:	10000002 */ 	b	.PF0f05a578
/*  f05a570:	afaa0084 */ 	sw	$t2,0x84($sp)
/*  f05a574:	afab0084 */ 	sw	$t3,0x84($sp)
.PF0f05a578:
/*  f05a578:	54c0000e */ 	bnezl	$a2,.PF0f05a5b4
/*  f05a57c:	90e40125 */ 	lbu	$a0,0x125($a3)
/*  f05a580:	910c0004 */ 	lbu	$t4,0x4($t0)
/*  f05a584:	5580000b */ 	bnezl	$t4,.PF0f05a5b4
/*  f05a588:	90e40125 */ 	lbu	$a0,0x125($a3)
/*  f05a58c:	910d0006 */ 	lbu	$t5,0x6($t0)
/*  f05a590:	55a00008 */ 	bnezl	$t5,.PF0f05a5b4
/*  f05a594:	90e40125 */ 	lbu	$a0,0x125($a3)
/*  f05a598:	ace00120 */ 	sw	$zero,0x120($a3)
/*  f05a59c:	8e0e0438 */ 	lw	$t6,0x438($s0)
/*  f05a5a0:	00001025 */ 	move	$v0,$zero
/*  f05a5a4:	25cf000a */ 	addiu	$t7,$t6,0xa
/*  f05a5a8:	10000208 */ 	b	.PF0f05adcc
/*  f05a5ac:	ae0f0438 */ 	sw	$t7,0x438($s0)
/*  f05a5b0:	90e40125 */ 	lbu	$a0,0x125($a3)
.PF0f05a5b4:
/*  f05a5b4:	afa60070 */ 	sw	$a2,0x70($sp)
/*  f05a5b8:	0fc133e6 */ 	jal	teamGetChrIds
/*  f05a5bc:	afa5007c */ 	sw	$a1,0x7c($sp)
/*  f05a5c0:	8fb8009c */ 	lw	$t8,0x9c($sp)
/*  f05a5c4:	afa20088 */ 	sw	$v0,0x88($sp)
/*  f05a5c8:	afa00098 */ 	sw	$zero,0x98($sp)
/*  f05a5cc:	afa00094 */ 	sw	$zero,0x94($sp)
/*  f05a5d0:	93190004 */ 	lbu	$t9,0x4($t8)
/*  f05a5d4:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f05a5d8:	24010005 */ 	li	$at,0x5
/*  f05a5dc:	afb90090 */ 	sw	$t9,0x90($sp)
/*  f05a5e0:	80e30006 */ 	lb	$v1,0x6($a3)
/*  f05a5e4:	8fa5007c */ 	lw	$a1,0x7c($sp)
/*  f05a5e8:	8fa90084 */ 	lw	$t1,0x84($sp)
/*  f05a5ec:	10610007 */ 	beq	$v1,$at,.PF0f05a60c
/*  f05a5f0:	3c0a8007 */ 	lui	$t2,%hi(g_SpecialQuipBank)
/*  f05a5f4:	2401000f */ 	li	$at,0xf
/*  f05a5f8:	10610004 */ 	beq	$v1,$at,.PF0f05a60c
/*  f05a5fc:	2401003a */ 	li	$at,0x3a
/*  f05a600:	10610002 */ 	beq	$v1,$at,.PF0f05a60c
/*  f05a604:	24010028 */ 	li	$at,0x28
/*  f05a608:	14610004 */ 	bne	$v1,$at,.PF0f05a61c
.PF0f05a60c:
/*  f05a60c:	254a91f8 */ 	addiu	$t2,$t2,%lo(g_SpecialQuipBank)
/*  f05a610:	112a0002 */ 	beq	$t1,$t2,.PF0f05a61c
/*  f05a614:	00000000 */ 	nop
/*  f05a618:	afa00090 */ 	sw	$zero,0x90($sp)
.PF0f05a61c:
/*  f05a61c:	50a0001c */ 	beqzl	$a1,.PF0f05a690
/*  f05a620:	90e20126 */ 	lbu	$v0,0x126($a3)
/*  f05a624:	8ca4001c */ 	lw	$a0,0x1c($a1)
/*  f05a628:	50800019 */ 	beqzl	$a0,.PF0f05a690
/*  f05a62c:	90e20126 */ 	lbu	$v0,0x126($a3)
/*  f05a630:	908b0000 */ 	lbu	$t3,0x0($a0)
/*  f05a634:	24010006 */ 	li	$at,0x6
/*  f05a638:	55610015 */ 	bnel	$t3,$at,.PF0f05a690
/*  f05a63c:	90e20126 */ 	lbu	$v0,0x126($a3)
/*  f05a640:	0fc4a513 */ 	jal	playermgrGetPlayerNumByProp
/*  f05a644:	00000000 */ 	nop
/*  f05a648:	8e030298 */ 	lw	$v1,0x298($s0)
/*  f05a64c:	00026080 */ 	sll	$t4,$v0,0x2
/*  f05a650:	00402025 */ 	move	$a0,$v0
/*  f05a654:	0460000a */ 	bltz	$v1,.PF0f05a680
/*  f05a658:	020c6821 */ 	addu	$t5,$s0,$t4
/*  f05a65c:	8dae0064 */ 	lw	$t6,0x64($t5)
/*  f05a660:	8dcf00d8 */ 	lw	$t7,0xd8($t6)
/*  f05a664:	11e00006 */ 	beqz	$t7,.PF0f05a680
/*  f05a668:	00000000 */ 	nop
/*  f05a66c:	8e050294 */ 	lw	$a1,0x294($s0)
/*  f05a670:	14450003 */ 	bne	$v0,$a1,.PF0f05a680
/*  f05a674:	00a02025 */ 	move	$a0,$a1
/*  f05a678:	10000001 */ 	b	.PF0f05a680
/*  f05a67c:	00602025 */ 	move	$a0,$v1
.PF0f05a680:
/*  f05a680:	0fc4a4ff */ 	jal	setCurrentPlayerNum
/*  f05a684:	00000000 */ 	nop
/*  f05a688:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f05a68c:	90e20126 */ 	lbu	$v0,0x126($a3)
.PF0f05a690:
/*  f05a690:	10400009 */ 	beqz	$v0,.PF0f05a6b8
/*  f05a694:	0002c080 */ 	sll	$t8,$v0,0x2
/*  f05a698:	0302c023 */ 	subu	$t8,$t8,$v0
/*  f05a69c:	8cf90120 */ 	lw	$t9,0x120($a3)
/*  f05a6a0:	0018c0c0 */ 	sll	$t8,$t8,0x3
/*  f05a6a4:	0302c021 */ 	addu	$t8,$t8,$v0
/*  f05a6a8:	0018c040 */ 	sll	$t8,$t8,0x1
/*  f05a6ac:	0319082a */ 	slt	$at,$t8,$t9
/*  f05a6b0:	102001c0 */ 	beqz	$at,.PF0f05adb4
/*  f05a6b4:	00000000 */ 	nop
.PF0f05a6b8:
/*  f05a6b8:	0c004ad4 */ 	jal	random
/*  f05a6bc:	00000000 */ 	nop
/*  f05a6c0:	8faa0090 */ 	lw	$t2,0x90($sp)
/*  f05a6c4:	304900ff */ 	andi	$t1,$v0,0xff
/*  f05a6c8:	8fab0088 */ 	lw	$t3,0x88($sp)
/*  f05a6cc:	012a082a */ 	slt	$at,$t1,$t2
/*  f05a6d0:	102001b8 */ 	beqz	$at,.PF0f05adb4
/*  f05a6d4:	00000000 */ 	nop
/*  f05a6d8:	85640000 */ 	lh	$a0,0x0($t3)
/*  f05a6dc:	2401fffe */ 	li	$at,-2
/*  f05a6e0:	5081003e */ 	beql	$a0,$at,.PF0f05a7dc
/*  f05a6e4:	8fae0094 */ 	lw	$t6,0x94($sp)
.PF0f05a6e8:
/*  f05a6e8:	0fc0a226 */ 	jal	chrFindByLiteralId
/*  f05a6ec:	00000000 */ 	nop
/*  f05a6f0:	50400033 */ 	beqzl	$v0,.PF0f05a7c0
/*  f05a6f4:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05a6f8:	8c4c0020 */ 	lw	$t4,0x20($v0)
/*  f05a6fc:	00402025 */ 	move	$a0,$v0
/*  f05a700:	5180002f */ 	beqzl	$t4,.PF0f05a7c0
/*  f05a704:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05a708:	0fc0e706 */ 	jal	chrIsDead
/*  f05a70c:	afa20064 */ 	sw	$v0,0x64($sp)
/*  f05a710:	1440002a */ 	bnez	$v0,.PF0f05a7bc
/*  f05a714:	8fa60064 */ 	lw	$a2,0x64($sp)
/*  f05a718:	80cd0007 */ 	lb	$t5,0x7($a2)
/*  f05a71c:	24010005 */ 	li	$at,0x5
/*  f05a720:	51a10027 */ 	beql	$t5,$at,.PF0f05a7c0
/*  f05a724:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05a728:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f05a72c:	90ce02a2 */ 	lbu	$t6,0x2a2($a2)
/*  f05a730:	90ef02a2 */ 	lbu	$t7,0x2a2($a3)
/*  f05a734:	55cf0022 */ 	bnel	$t6,$t7,.PF0f05a7c0
/*  f05a738:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05a73c:	90d80113 */ 	lbu	$t8,0x113($a2)
/*  f05a740:	2b010064 */ 	slti	$at,$t8,0x64
/*  f05a744:	5420001e */ 	bnezl	$at,.PF0f05a7c0
/*  f05a748:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05a74c:	84c50000 */ 	lh	$a1,0x0($a2)
/*  f05a750:	84f90000 */ 	lh	$t9,0x0($a3)
/*  f05a754:	00e02025 */ 	move	$a0,$a3
/*  f05a758:	50b90019 */ 	beql	$a1,$t9,.PF0f05a7c0
/*  f05a75c:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05a760:	0fc12792 */ 	jal	chrGetDistanceToChr
/*  f05a764:	afa60064 */ 	sw	$a2,0x64($sp)
/*  f05a768:	3c017f1b */ 	lui	$at,%hi(var7f1a9d64)
/*  f05a76c:	c424afd4 */ 	lwc1	$f4,%lo(var7f1a9d64)($at)
/*  f05a770:	8fa90098 */ 	lw	$t1,0x98($sp)
/*  f05a774:	8fa60064 */ 	lw	$a2,0x64($sp)
/*  f05a778:	4604003c */ 	c.lt.s	$f0,$f4
/*  f05a77c:	252a0001 */ 	addiu	$t2,$t1,0x1
/*  f05a780:	4502000f */ 	bc1fl	.PF0f05a7c0
/*  f05a784:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05a788:	afaa0098 */ 	sw	$t2,0x98($sp)
/*  f05a78c:	8ccb0120 */ 	lw	$t3,0x120($a2)
/*  f05a790:	8fac009c */ 	lw	$t4,0x9c($sp)
/*  f05a794:	29610032 */ 	slti	$at,$t3,0x32
/*  f05a798:	50200009 */ 	beqzl	$at,.PF0f05a7c0
/*  f05a79c:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05a7a0:	91820006 */ 	lbu	$v0,0x6($t4)
/*  f05a7a4:	240100ff */ 	li	$at,0xff
/*  f05a7a8:	50400005 */ 	beqzl	$v0,.PF0f05a7c0
/*  f05a7ac:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05a7b0:	10410002 */ 	beq	$v0,$at,.PF0f05a7bc
/*  f05a7b4:	240d0001 */ 	li	$t5,0x1
/*  f05a7b8:	afad0094 */ 	sw	$t5,0x94($sp)
.PF0f05a7bc:
/*  f05a7bc:	8fa20088 */ 	lw	$v0,0x88($sp)
.PF0f05a7c0:
/*  f05a7c0:	2401fffe */ 	li	$at,-2
/*  f05a7c4:	84440002 */ 	lh	$a0,0x2($v0)
/*  f05a7c8:	24420002 */ 	addiu	$v0,$v0,0x2
/*  f05a7cc:	afa20088 */ 	sw	$v0,0x88($sp)
/*  f05a7d0:	1481ffc5 */ 	bne	$a0,$at,.PF0f05a6e8
/*  f05a7d4:	00000000 */ 	nop
/*  f05a7d8:	8fae0094 */ 	lw	$t6,0x94($sp)
.PF0f05a7dc:
/*  f05a7dc:	8faf0098 */ 	lw	$t7,0x98($sp)
/*  f05a7e0:	15c00174 */ 	bnez	$t6,.PF0f05adb4
/*  f05a7e4:	00000000 */ 	nop
/*  f05a7e8:	15e00007 */ 	bnez	$t7,.PF0f05a808
/*  f05a7ec:	8fb8009c */ 	lw	$t8,0x9c($sp)
/*  f05a7f0:	93020006 */ 	lbu	$v0,0x6($t8)
/*  f05a7f4:	240100ff */ 	li	$at,0xff
/*  f05a7f8:	1040000a */ 	beqz	$v0,.PF0f05a824
/*  f05a7fc:	00000000 */ 	nop
/*  f05a800:	10410008 */ 	beq	$v0,$at,.PF0f05a824
/*  f05a804:	00000000 */ 	nop
.PF0f05a808:
/*  f05a808:	8fb90098 */ 	lw	$t9,0x98($sp)
/*  f05a80c:	8fa9009c */ 	lw	$t1,0x9c($sp)
/*  f05a810:	1b200168 */ 	blez	$t9,.PF0f05adb4
/*  f05a814:	00000000 */ 	nop
/*  f05a818:	912a0006 */ 	lbu	$t2,0x6($t1)
/*  f05a81c:	19400165 */ 	blez	$t2,.PF0f05adb4
/*  f05a820:	00000000 */ 	nop
.PF0f05a824:
/*  f05a824:	0c004ad4 */ 	jal	random
/*  f05a828:	00000000 */ 	nop
/*  f05a82c:	24010003 */ 	li	$at,0x3
/*  f05a830:	0041001b */ 	divu	$zero,$v0,$at
/*  f05a834:	8fab009c */ 	lw	$t3,0x9c($sp)
/*  f05a838:	00001810 */ 	mfhi	$v1
/*  f05a83c:	a3a300a7 */ 	sb	$v1,0xa7($sp)
/*  f05a840:	916c0007 */ 	lbu	$t4,0x7($t3)
/*  f05a844:	8fb80070 */ 	lw	$t8,0x70($sp)
/*  f05a848:	8fae0084 */ 	lw	$t6,0x84($sp)
/*  f05a84c:	318d0080 */ 	andi	$t5,$t4,0x80
/*  f05a850:	15a00009 */ 	bnez	$t5,.PF0f05a878
/*  f05a854:	0018c8c0 */ 	sll	$t9,$t8,0x3
/*  f05a858:	01d94821 */ 	addu	$t1,$t6,$t9
/*  f05a85c:	306a00ff */ 	andi	$t2,$v1,0xff
/*  f05a860:	000a5840 */ 	sll	$t3,$t2,0x1
/*  f05a864:	012b6021 */ 	addu	$t4,$t1,$t3
/*  f05a868:	afa90058 */ 	sw	$t1,0x58($sp)
/*  f05a86c:	858d0002 */ 	lh	$t5,0x2($t4)
/*  f05a870:	1000000c */ 	b	.PF0f05a8a4
/*  f05a874:	a7ad00a4 */ 	sh	$t5,0xa4($sp)
.PF0f05a878:
/*  f05a878:	8e0a0424 */ 	lw	$t2,0x424($s0)
/*  f05a87c:	8fae0070 */ 	lw	$t6,0x70($sp)
/*  f05a880:	8faf0084 */ 	lw	$t7,0x84($sp)
/*  f05a884:	914902b0 */ 	lbu	$t1,0x2b0($t2)
/*  f05a888:	000ec8c0 */ 	sll	$t9,$t6,0x3
/*  f05a88c:	01f91021 */ 	addu	$v0,$t7,$t9
/*  f05a890:	00095840 */ 	sll	$t3,$t1,0x1
/*  f05a894:	004b6021 */ 	addu	$t4,$v0,$t3
/*  f05a898:	858d0002 */ 	lh	$t5,0x2($t4)
/*  f05a89c:	afa20058 */ 	sw	$v0,0x58($sp)
/*  f05a8a0:	a7ad00a4 */ 	sh	$t5,0xa4($sp)
.PF0f05a8a4:
/*  f05a8a4:	0fc13433 */ 	jal	audioWasNotPlayedRecently
/*  f05a8a8:	87a400a4 */ 	lh	$a0,0xa4($sp)
/*  f05a8ac:	1440000a */ 	bnez	$v0,.PF0f05a8d8
/*  f05a8b0:	24010001 */ 	li	$at,0x1
/*  f05a8b4:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f05a8b8:	00001825 */ 	move	$v1,$zero
/*  f05a8bc:	24080001 */ 	li	$t0,0x1
/*  f05a8c0:	10e00003 */ 	beqz	$a3,.PF0f05a8d0
/*  f05a8c4:	3c0e8007 */ 	lui	$t6,%hi(g_GuardQuipBank)
/*  f05a8c8:	10000001 */ 	b	.PF0f05a8d0
/*  f05a8cc:	90e302fe */ 	lbu	$v1,0x2fe($a3)
.PF0f05a8d0:
/*  f05a8d0:	54610092 */ 	bnel	$v1,$at,.PF0f05ab1c
/*  f05a8d4:	8fad0070 */ 	lw	$t5,0x70($sp)
.PF0f05a8d8:
/*  f05a8d8:	0fc13421 */ 	jal	audioMarkAsRecentlyPlayed
/*  f05a8dc:	87a400a4 */ 	lh	$a0,0xa4($sp)
/*  f05a8e0:	87b800a4 */ 	lh	$t8,0xa4($sp)
/*  f05a8e4:	2401034e */ 	li	$at,0x34e
/*  f05a8e8:	93ae006b */ 	lbu	$t6,0x6b($sp)
/*  f05a8ec:	57010005 */ 	bnel	$t8,$at,.PF0f05a904
/*  f05a8f0:	8e190424 */ 	lw	$t9,0x424($s0)
/*  f05a8f4:	15c00002 */ 	bnez	$t6,.PF0f05a900
/*  f05a8f8:	240f034d */ 	li	$t7,0x34d
/*  f05a8fc:	a7af00a4 */ 	sh	$t7,0xa4($sp)
.PF0f05a900:
/*  f05a900:	8e190424 */ 	lw	$t9,0x424($s0)
.PF0f05a904:
/*  f05a904:	240103f7 */ 	li	$at,0x3f7
/*  f05a908:	af200120 */ 	sw	$zero,0x120($t9)
/*  f05a90c:	8faa009c */ 	lw	$t2,0x9c($sp)
/*  f05a910:	8e0b0424 */ 	lw	$t3,0x424($s0)
/*  f05a914:	91490005 */ 	lbu	$t1,0x5($t2)
/*  f05a918:	a1690126 */ 	sb	$t1,0x126($t3)
/*  f05a91c:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f05a920:	90ec0332 */ 	lbu	$t4,0x332($a3)
/*  f05a924:	258d0001 */ 	addiu	$t5,$t4,0x1
/*  f05a928:	a0ed0332 */ 	sb	$t5,0x332($a3)
/*  f05a92c:	87b800a4 */ 	lh	$t8,0xa4($sp)
/*  f05a930:	13010022 */ 	beq	$t8,$at,.PF0f05a9bc
/*  f05a934:	24010331 */ 	li	$at,0x331
/*  f05a938:	13010020 */ 	beq	$t8,$at,.PF0f05a9bc
/*  f05a93c:	240103a1 */ 	li	$at,0x3a1
/*  f05a940:	1301001e */ 	beq	$t8,$at,.PF0f05a9bc
/*  f05a944:	24050009 */ 	li	$a1,0x9
/*  f05a948:	8e0e0424 */ 	lw	$t6,0x424($s0)
/*  f05a94c:	3406ffff */ 	li	$a2,0xffff
/*  f05a950:	0fc2496f */ 	jal	func0f0926bc
/*  f05a954:	8dc4001c */ 	lw	$a0,0x1c($t6)
/*  f05a958:	8e0f0424 */ 	lw	$t7,0x424($s0)
/*  f05a95c:	3c01bf80 */ 	lui	$at,0xbf80
/*  f05a960:	44810000 */ 	mtc1	$at,$f0
/*  f05a964:	8de5001c */ 	lw	$a1,0x1c($t7)
/*  f05a968:	2419ffff */ 	li	$t9,-1
/*  f05a96c:	240a0008 */ 	li	$t2,0x8
/*  f05a970:	24090009 */ 	li	$t1,0x9
/*  f05a974:	240bffff */ 	li	$t3,-1
/*  f05a978:	afab002c */ 	sw	$t3,0x2c($sp)
/*  f05a97c:	afa9001c */ 	sw	$t1,0x1c($sp)
/*  f05a980:	afaa0014 */ 	sw	$t2,0x14($sp)
/*  f05a984:	afb90010 */ 	sw	$t9,0x10($sp)
/*  f05a988:	afa00028 */ 	sw	$zero,0x28($sp)
/*  f05a98c:	afa00020 */ 	sw	$zero,0x20($sp)
/*  f05a990:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f05a994:	00002025 */ 	move	$a0,$zero
/*  f05a998:	87a600a4 */ 	lh	$a2,0xa4($sp)
/*  f05a99c:	2407ffff */ 	li	$a3,-1
/*  f05a9a0:	e7a00038 */ 	swc1	$f0,0x38($sp)
/*  f05a9a4:	e7a00034 */ 	swc1	$f0,0x34($sp)
/*  f05a9a8:	e7a00030 */ 	swc1	$f0,0x30($sp)
/*  f05a9ac:	0fc24e3a */ 	jal	propsnd0f0939f8
/*  f05a9b0:	e7a00024 */ 	swc1	$f0,0x24($sp)
/*  f05a9b4:	10000026 */ 	b	.PF0f05aa50
/*  f05a9b8:	8faa009c */ 	lw	$t2,0x9c($sp)
.PF0f05a9bc:
/*  f05a9bc:	0fc131c6 */ 	jal	chrGetDistanceLostToTargetInLastSecond
/*  f05a9c0:	8e040424 */ 	lw	$a0,0x424($s0)
/*  f05a9c4:	18400003 */ 	blez	$v0,.PF0f05a9d4
/*  f05a9c8:	00021823 */ 	negu	$v1,$v0
/*  f05a9cc:	10000001 */ 	b	.PF0f05a9d4
/*  f05a9d0:	00401825 */ 	move	$v1,$v0
.PF0f05a9d4:
/*  f05a9d4:	28610033 */ 	slti	$at,$v1,0x33
/*  f05a9d8:	1420001c */ 	bnez	$at,.PF0f05aa4c
/*  f05a9dc:	24050009 */ 	li	$a1,0x9
/*  f05a9e0:	8e0c0424 */ 	lw	$t4,0x424($s0)
/*  f05a9e4:	3406ffff */ 	li	$a2,0xffff
/*  f05a9e8:	0fc2496f */ 	jal	func0f0926bc
/*  f05a9ec:	8d84001c */ 	lw	$a0,0x1c($t4)
/*  f05a9f0:	8e0d0424 */ 	lw	$t5,0x424($s0)
/*  f05a9f4:	3c01bf80 */ 	lui	$at,0xbf80
/*  f05a9f8:	44810000 */ 	mtc1	$at,$f0
/*  f05a9fc:	8da5001c */ 	lw	$a1,0x1c($t5)
/*  f05aa00:	2418ffff */ 	li	$t8,-1
/*  f05aa04:	240e0008 */ 	li	$t6,0x8
/*  f05aa08:	240f0009 */ 	li	$t7,0x9
/*  f05aa0c:	2419ffff */ 	li	$t9,-1
/*  f05aa10:	afb9002c */ 	sw	$t9,0x2c($sp)
/*  f05aa14:	afaf001c */ 	sw	$t7,0x1c($sp)
/*  f05aa18:	afae0014 */ 	sw	$t6,0x14($sp)
/*  f05aa1c:	afb80010 */ 	sw	$t8,0x10($sp)
/*  f05aa20:	afa00028 */ 	sw	$zero,0x28($sp)
/*  f05aa24:	afa00020 */ 	sw	$zero,0x20($sp)
/*  f05aa28:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f05aa2c:	00002025 */ 	move	$a0,$zero
/*  f05aa30:	87a600a4 */ 	lh	$a2,0xa4($sp)
/*  f05aa34:	2407ffff */ 	li	$a3,-1
/*  f05aa38:	e7a00038 */ 	swc1	$f0,0x38($sp)
/*  f05aa3c:	e7a00034 */ 	swc1	$f0,0x34($sp)
/*  f05aa40:	e7a00030 */ 	swc1	$f0,0x30($sp)
/*  f05aa44:	0fc24e3a */ 	jal	propsnd0f0939f8
/*  f05aa48:	e7a00024 */ 	swc1	$f0,0x24($sp)
.PF0f05aa4c:
/*  f05aa4c:	8faa009c */ 	lw	$t2,0x9c($sp)
.PF0f05aa50:
/*  f05aa50:	91420008 */ 	lbu	$v0,0x8($t2)
/*  f05aa54:	1040001a */ 	beqz	$v0,.PF0f05aac0
/*  f05aa58:	00000000 */ 	nop
/*  f05aa5c:	91490007 */ 	lbu	$t1,0x7($t2)
/*  f05aa60:	93a300a7 */ 	lbu	$v1,0xa7($sp)
/*  f05aa64:	000260c0 */ 	sll	$t4,$v0,0x3
/*  f05aa68:	312b0080 */ 	andi	$t3,$t1,0x80
/*  f05aa6c:	15600014 */ 	bnez	$t3,.PF0f05aac0
/*  f05aa70:	28610003 */ 	slti	$at,$v1,0x3
/*  f05aa74:	14200002 */ 	bnez	$at,.PF0f05aa80
/*  f05aa78:	3c048007 */ 	lui	$a0,%hi(g_SpecialQuipBank+0x16a)
/*  f05aa7c:	24030002 */ 	li	$v1,0x2
.PF0f05aa80:
/*  f05aa80:	00036840 */ 	sll	$t5,$v1,0x1
/*  f05aa84:	018dc021 */ 	addu	$t8,$t4,$t5
/*  f05aa88:	00982021 */ 	addu	$a0,$a0,$t8
/*  f05aa8c:	0fc5bdaa */ 	jal	langGet
/*  f05aa90:	84849362 */ 	lh	$a0,%lo(g_SpecialQuipBank+0x16a)($a0)
/*  f05aa94:	afa20080 */ 	sw	$v0,0x80($sp)
/*  f05aa98:	0c004062 */ 	jal	sndIsFiltered
/*  f05aa9c:	87a400a4 */ 	lh	$a0,0xa4($sp)
/*  f05aaa0:	144000c4 */ 	bnez	$v0,.PF0f05adb4
/*  f05aaa4:	8fa40080 */ 	lw	$a0,0x80($sp)
/*  f05aaa8:	8fae009c */ 	lw	$t6,0x9c($sp)
/*  f05aaac:	24050006 */ 	li	$a1,0x6
/*  f05aab0:	0fc3799a */ 	jal	hudmsgCreateWithColour
/*  f05aab4:	91c60009 */ 	lbu	$a2,0x9($t6)
/*  f05aab8:	100000be */ 	b	.PF0f05adb4
/*  f05aabc:	00000000 */ 	nop
.PF0f05aac0:
/*  f05aac0:	104000bc */ 	beqz	$v0,.PF0f05adb4
/*  f05aac4:	00000000 */ 	nop
/*  f05aac8:	8e190424 */ 	lw	$t9,0x424($s0)
/*  f05aacc:	000278c0 */ 	sll	$t7,$v0,0x3
/*  f05aad0:	3c048007 */ 	lui	$a0,%hi(g_SpecialQuipBank+0x16a)
/*  f05aad4:	932a02b0 */ 	lbu	$t2,0x2b0($t9)
/*  f05aad8:	000a4840 */ 	sll	$t1,$t2,0x1
/*  f05aadc:	01e95821 */ 	addu	$t3,$t7,$t1
/*  f05aae0:	008b2021 */ 	addu	$a0,$a0,$t3
/*  f05aae4:	0fc5bdaa */ 	jal	langGet
/*  f05aae8:	84849362 */ 	lh	$a0,%lo(g_SpecialQuipBank+0x16a)($a0)
/*  f05aaec:	afa20080 */ 	sw	$v0,0x80($sp)
/*  f05aaf0:	0c004062 */ 	jal	sndIsFiltered
/*  f05aaf4:	87a400a4 */ 	lh	$a0,0xa4($sp)
/*  f05aaf8:	144000ae */ 	bnez	$v0,.PF0f05adb4
/*  f05aafc:	8fa40080 */ 	lw	$a0,0x80($sp)
/*  f05ab00:	8fac009c */ 	lw	$t4,0x9c($sp)
/*  f05ab04:	24050006 */ 	li	$a1,0x6
/*  f05ab08:	0fc3799a */ 	jal	hudmsgCreateWithColour
/*  f05ab0c:	91860009 */ 	lbu	$a2,0x9($t4)
/*  f05ab10:	100000a8 */ 	b	.PF0f05adb4
/*  f05ab14:	00000000 */ 	nop
/*  f05ab18:	8fad0070 */ 	lw	$t5,0x70($sp)
.PF0f05ab1c:
/*  f05ab1c:	25ce8cd8 */ 	addiu	$t6,$t6,%lo(g_GuardQuipBank)
/*  f05ab20:	a7a000a4 */ 	sh	$zero,0xa4($sp)
/*  f05ab24:	000dc0c0 */ 	sll	$t8,$t5,0x3
/*  f05ab28:	030e1021 */ 	addu	$v0,$t8,$t6
/*  f05ab2c:	afa2005c */ 	sw	$v0,0x5c($sp)
.PF0f05ab30:
/*  f05ab30:	8fa2005c */ 	lw	$v0,0x5c($sp)
/*  f05ab34:	00081840 */ 	sll	$v1,$t0,0x1
/*  f05ab38:	0043c821 */ 	addu	$t9,$v0,$v1
/*  f05ab3c:	87240000 */ 	lh	$a0,0x0($t9)
/*  f05ab40:	a3a800a3 */ 	sb	$t0,0xa3($sp)
/*  f05ab44:	0fc13433 */ 	jal	audioWasNotPlayedRecently
/*  f05ab48:	afa30054 */ 	sw	$v1,0x54($sp)
/*  f05ab4c:	8fa30054 */ 	lw	$v1,0x54($sp)
/*  f05ab50:	1040000d */ 	beqz	$v0,.PF0f05ab88
/*  f05ab54:	93a800a3 */ 	lbu	$t0,0xa3($sp)
/*  f05ab58:	8faa0058 */ 	lw	$t2,0x58($sp)
/*  f05ab5c:	01432821 */ 	addu	$a1,$t2,$v1
/*  f05ab60:	84a40000 */ 	lh	$a0,0x0($a1)
/*  f05ab64:	a3a800a3 */ 	sb	$t0,0xa3($sp)
/*  f05ab68:	0fc13433 */ 	jal	audioWasNotPlayedRecently
/*  f05ab6c:	afa50050 */ 	sw	$a1,0x50($sp)
/*  f05ab70:	8fa50050 */ 	lw	$a1,0x50($sp)
/*  f05ab74:	10400004 */ 	beqz	$v0,.PF0f05ab88
/*  f05ab78:	93a800a3 */ 	lbu	$t0,0xa3($sp)
/*  f05ab7c:	84af0000 */ 	lh	$t7,0x0($a1)
/*  f05ab80:	10000006 */ 	b	.PF0f05ab9c
/*  f05ab84:	a7af00a4 */ 	sh	$t7,0xa4($sp)
.PF0f05ab88:
/*  f05ab88:	25080001 */ 	addiu	$t0,$t0,0x1
/*  f05ab8c:	310900ff */ 	andi	$t1,$t0,0xff
/*  f05ab90:	29210004 */ 	slti	$at,$t1,0x4
/*  f05ab94:	1420ffe6 */ 	bnez	$at,.PF0f05ab30
/*  f05ab98:	01204025 */ 	move	$t0,$t1
.PF0f05ab9c:
/*  f05ab9c:	87ab00a4 */ 	lh	$t3,0xa4($sp)
/*  f05aba0:	1160007a */ 	beqz	$t3,.PF0f05ad8c
/*  f05aba4:	01602025 */ 	move	$a0,$t3
/*  f05aba8:	0fc13421 */ 	jal	audioMarkAsRecentlyPlayed
/*  f05abac:	a3a800a3 */ 	sb	$t0,0xa3($sp)
/*  f05abb0:	87ac00a4 */ 	lh	$t4,0xa4($sp)
/*  f05abb4:	2401034e */ 	li	$at,0x34e
/*  f05abb8:	93a800a3 */ 	lbu	$t0,0xa3($sp)
/*  f05abbc:	15810004 */ 	bne	$t4,$at,.PF0f05abd0
/*  f05abc0:	93ad006b */ 	lbu	$t5,0x6b($sp)
/*  f05abc4:	15a00002 */ 	bnez	$t5,.PF0f05abd0
/*  f05abc8:	2418034d */ 	li	$t8,0x34d
/*  f05abcc:	a7b800a4 */ 	sh	$t8,0xa4($sp)
.PF0f05abd0:
/*  f05abd0:	8e0e0424 */ 	lw	$t6,0x424($s0)
/*  f05abd4:	240103f7 */ 	li	$at,0x3f7
/*  f05abd8:	adc00120 */ 	sw	$zero,0x120($t6)
/*  f05abdc:	8fb9009c */ 	lw	$t9,0x9c($sp)
/*  f05abe0:	8e0f0424 */ 	lw	$t7,0x424($s0)
/*  f05abe4:	932a0005 */ 	lbu	$t2,0x5($t9)
/*  f05abe8:	a1ea0126 */ 	sb	$t2,0x126($t7)
/*  f05abec:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f05abf0:	90e90332 */ 	lbu	$t1,0x332($a3)
/*  f05abf4:	252b0001 */ 	addiu	$t3,$t1,0x1
/*  f05abf8:	a0eb0332 */ 	sb	$t3,0x332($a3)
/*  f05abfc:	87ac00a4 */ 	lh	$t4,0xa4($sp)
/*  f05ac00:	11810024 */ 	beq	$t4,$at,.PF0f05ac94
/*  f05ac04:	24010331 */ 	li	$at,0x331
/*  f05ac08:	11810022 */ 	beq	$t4,$at,.PF0f05ac94
/*  f05ac0c:	240103a1 */ 	li	$at,0x3a1
/*  f05ac10:	51810021 */ 	beql	$t4,$at,.PF0f05ac98
/*  f05ac14:	8e040424 */ 	lw	$a0,0x424($s0)
/*  f05ac18:	8e0d0424 */ 	lw	$t5,0x424($s0)
/*  f05ac1c:	24050009 */ 	li	$a1,0x9
/*  f05ac20:	3406ffff */ 	li	$a2,0xffff
/*  f05ac24:	8da4001c */ 	lw	$a0,0x1c($t5)
/*  f05ac28:	0fc2496f */ 	jal	func0f0926bc
/*  f05ac2c:	a3a800a3 */ 	sb	$t0,0xa3($sp)
/*  f05ac30:	8e180424 */ 	lw	$t8,0x424($s0)
/*  f05ac34:	3c01bf80 */ 	lui	$at,0xbf80
/*  f05ac38:	44810000 */ 	mtc1	$at,$f0
/*  f05ac3c:	8f05001c */ 	lw	$a1,0x1c($t8)
/*  f05ac40:	240effff */ 	li	$t6,-1
/*  f05ac44:	24190008 */ 	li	$t9,0x8
/*  f05ac48:	240a0009 */ 	li	$t2,0x9
/*  f05ac4c:	240fffff */ 	li	$t7,-1
/*  f05ac50:	afaf002c */ 	sw	$t7,0x2c($sp)
/*  f05ac54:	afaa001c */ 	sw	$t2,0x1c($sp)
/*  f05ac58:	afb90014 */ 	sw	$t9,0x14($sp)
/*  f05ac5c:	afae0010 */ 	sw	$t6,0x10($sp)
/*  f05ac60:	afa00028 */ 	sw	$zero,0x28($sp)
/*  f05ac64:	afa00020 */ 	sw	$zero,0x20($sp)
/*  f05ac68:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f05ac6c:	00002025 */ 	move	$a0,$zero
/*  f05ac70:	87a600a4 */ 	lh	$a2,0xa4($sp)
/*  f05ac74:	2407ffff */ 	li	$a3,-1
/*  f05ac78:	e7a00038 */ 	swc1	$f0,0x38($sp)
/*  f05ac7c:	e7a00034 */ 	swc1	$f0,0x34($sp)
/*  f05ac80:	e7a00030 */ 	swc1	$f0,0x30($sp)
/*  f05ac84:	0fc24e3a */ 	jal	propsnd0f0939f8
/*  f05ac88:	e7a00024 */ 	swc1	$f0,0x24($sp)
/*  f05ac8c:	1000002a */ 	b	.PF0f05ad38
/*  f05ac90:	93a800a3 */ 	lbu	$t0,0xa3($sp)
.PF0f05ac94:
/*  f05ac94:	8e040424 */ 	lw	$a0,0x424($s0)
.PF0f05ac98:
/*  f05ac98:	0fc131c6 */ 	jal	chrGetDistanceLostToTargetInLastSecond
/*  f05ac9c:	a3a800a3 */ 	sb	$t0,0xa3($sp)
/*  f05aca0:	18400003 */ 	blez	$v0,.PF0f05acb0
/*  f05aca4:	93a800a3 */ 	lbu	$t0,0xa3($sp)
/*  f05aca8:	10000002 */ 	b	.PF0f05acb4
/*  f05acac:	00401825 */ 	move	$v1,$v0
.PF0f05acb0:
/*  f05acb0:	00021823 */ 	negu	$v1,$v0
.PF0f05acb4:
/*  f05acb4:	28610033 */ 	slti	$at,$v1,0x33
/*  f05acb8:	54200020 */ 	bnezl	$at,.PF0f05ad3c
/*  f05acbc:	8fb9009c */ 	lw	$t9,0x9c($sp)
/*  f05acc0:	8e090424 */ 	lw	$t1,0x424($s0)
/*  f05acc4:	24050009 */ 	li	$a1,0x9
/*  f05acc8:	3406ffff */ 	li	$a2,0xffff
/*  f05accc:	8d24001c */ 	lw	$a0,0x1c($t1)
/*  f05acd0:	0fc2496f */ 	jal	func0f0926bc
/*  f05acd4:	a3a800a3 */ 	sb	$t0,0xa3($sp)
/*  f05acd8:	8e0b0424 */ 	lw	$t3,0x424($s0)
/*  f05acdc:	3c01bf80 */ 	lui	$at,0xbf80
/*  f05ace0:	44810000 */ 	mtc1	$at,$f0
/*  f05ace4:	8d65001c */ 	lw	$a1,0x1c($t3)
/*  f05ace8:	240cffff */ 	li	$t4,-1
/*  f05acec:	240d0008 */ 	li	$t5,0x8
/*  f05acf0:	24180009 */ 	li	$t8,0x9
/*  f05acf4:	240effff */ 	li	$t6,-1
/*  f05acf8:	afae002c */ 	sw	$t6,0x2c($sp)
/*  f05acfc:	afb8001c */ 	sw	$t8,0x1c($sp)
/*  f05ad00:	afad0014 */ 	sw	$t5,0x14($sp)
/*  f05ad04:	afac0010 */ 	sw	$t4,0x10($sp)
/*  f05ad08:	afa00028 */ 	sw	$zero,0x28($sp)
/*  f05ad0c:	afa00020 */ 	sw	$zero,0x20($sp)
/*  f05ad10:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f05ad14:	00002025 */ 	move	$a0,$zero
/*  f05ad18:	87a600a4 */ 	lh	$a2,0xa4($sp)
/*  f05ad1c:	2407ffff */ 	li	$a3,-1
/*  f05ad20:	e7a00038 */ 	swc1	$f0,0x38($sp)
/*  f05ad24:	e7a00034 */ 	swc1	$f0,0x34($sp)
/*  f05ad28:	e7a00030 */ 	swc1	$f0,0x30($sp)
/*  f05ad2c:	0fc24e3a */ 	jal	propsnd0f0939f8
/*  f05ad30:	e7a00024 */ 	swc1	$f0,0x24($sp)
/*  f05ad34:	93a800a3 */ 	lbu	$t0,0xa3($sp)
.PF0f05ad38:
/*  f05ad38:	8fb9009c */ 	lw	$t9,0x9c($sp)
.PF0f05ad3c:
/*  f05ad3c:	00087840 */ 	sll	$t7,$t0,0x1
/*  f05ad40:	3c048007 */ 	lui	$a0,%hi(g_SpecialQuipBank+0x168)
/*  f05ad44:	93220008 */ 	lbu	$v0,0x8($t9)
/*  f05ad48:	1040001a */ 	beqz	$v0,.PF0f05adb4
/*  f05ad4c:	000250c0 */ 	sll	$t2,$v0,0x3
/*  f05ad50:	014f4821 */ 	addu	$t1,$t2,$t7
/*  f05ad54:	00892021 */ 	addu	$a0,$a0,$t1
/*  f05ad58:	0fc5bdaa */ 	jal	langGet
/*  f05ad5c:	84849360 */ 	lh	$a0,%lo(g_SpecialQuipBank+0x168)($a0)
/*  f05ad60:	afa20080 */ 	sw	$v0,0x80($sp)
/*  f05ad64:	0c004062 */ 	jal	sndIsFiltered
/*  f05ad68:	87a400a4 */ 	lh	$a0,0xa4($sp)
/*  f05ad6c:	14400011 */ 	bnez	$v0,.PF0f05adb4
/*  f05ad70:	8fa40080 */ 	lw	$a0,0x80($sp)
/*  f05ad74:	8fab009c */ 	lw	$t3,0x9c($sp)
/*  f05ad78:	24050006 */ 	li	$a1,0x6
/*  f05ad7c:	0fc3799a */ 	jal	hudmsgCreateWithColour
/*  f05ad80:	91660009 */ 	lbu	$a2,0x9($t3)
/*  f05ad84:	1000000b */ 	b	.PF0f05adb4
/*  f05ad88:	00000000 */ 	nop
.PF0f05ad8c:
/*  f05ad8c:	8e0c0424 */ 	lw	$t4,0x424($s0)
/*  f05ad90:	3c050800 */ 	lui	$a1,0x800
/*  f05ad94:	24060001 */ 	li	$a2,0x1
/*  f05ad98:	ad800120 */ 	sw	$zero,0x120($t4)
/*  f05ad9c:	8fad009c */ 	lw	$t5,0x9c($sp)
/*  f05ada0:	8e0e0424 */ 	lw	$t6,0x424($s0)
/*  f05ada4:	91b80005 */ 	lbu	$t8,0x5($t5)
/*  f05ada8:	a1d80126 */ 	sb	$t8,0x126($t6)
/*  f05adac:	0fc127e8 */ 	jal	chrUnsetFlags
/*  f05adb0:	8e040424 */ 	lw	$a0,0x424($s0)
.PF0f05adb4:
/*  f05adb4:	0fc4a4ff */ 	jal	setCurrentPlayerNum
/*  f05adb8:	8fa40078 */ 	lw	$a0,0x78($sp)
/*  f05adbc:	8e190438 */ 	lw	$t9,0x438($s0)
/*  f05adc0:	00001025 */ 	move	$v0,$zero
/*  f05adc4:	272a000a */ 	addiu	$t2,$t9,0xa
/*  f05adc8:	ae0a0438 */ 	sw	$t2,0x438($s0)
.PF0f05adcc:
/*  f05adcc:	8fbf004c */ 	lw	$ra,0x4c($sp)
/*  f05add0:	8fb00048 */ 	lw	$s0,0x48($sp)
/*  f05add4:	27bd00a8 */ 	addiu	$sp,$sp,0xa8
/*  f05add8:	03e00008 */ 	jr	$ra
/*  f05addc:	00000000 */ 	nop
);
#elif VERSION >= VERSION_NTSC_1_0
GLOBAL_ASM(
glabel aiSayQuip
.late_rodata
glabel var7f1a9d64
.word 0x45dac000
.text
/*  f05a258:	27bdff58 */ 	addiu	$sp,$sp,-168
/*  f05a25c:	afb00048 */ 	sw	$s0,0x48($sp)
/*  f05a260:	3c10800a */ 	lui	$s0,%hi(g_Vars)
/*  f05a264:	26109fc0 */ 	addiu	$s0,$s0,%lo(g_Vars)
/*  f05a268:	8e0e0434 */ 	lw	$t6,0x434($s0)
/*  f05a26c:	8e0f0438 */ 	lw	$t7,0x438($s0)
/*  f05a270:	afbf004c */ 	sw	$ra,0x4c($sp)
/*  f05a274:	8e040424 */ 	lw	$a0,0x424($s0)
/*  f05a278:	01cf1021 */ 	addu	$v0,$t6,$t7
/*  f05a27c:	90450002 */ 	lbu	$a1,0x2($v0)
/*  f05a280:	0fc126d1 */ 	jal	chrFindById
/*  f05a284:	afa2009c */ 	sw	$v0,0x9c($sp)
/*  f05a288:	8e18028c */ 	lw	$t8,0x28c($s0)
/*  f05a28c:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f05a290:	8fa8009c */ 	lw	$t0,0x9c($sp)
/*  f05a294:	afb80078 */ 	sw	$t8,0x78($sp)
/*  f05a298:	94f90192 */ 	lhu	$t9,0x192($a3)
/*  f05a29c:	91060003 */ 	lbu	$a2,0x3($t0)
/*  f05a2a0:	00402825 */ 	or	$a1,$v0,$zero
/*  f05a2a4:	33291000 */ 	andi	$t1,$t9,0x1000
/*  f05a2a8:	10e00003 */ 	beqz	$a3,.L0f05a2b8
/*  f05a2ac:	a3a9006b */ 	sb	$t1,0x6b($sp)
/*  f05a2b0:	10000002 */ 	b	.L0f05a2bc
/*  f05a2b4:	90e302fe */ 	lbu	$v1,0x2fe($a3)
.L0f05a2b8:
/*  f05a2b8:	00001825 */ 	or	$v1,$zero,$zero
.L0f05a2bc:
/*  f05a2bc:	24010001 */ 	addiu	$at,$zero,0x1
/*  f05a2c0:	14610007 */ 	bne	$v1,$at,.L0f05a2e0
/*  f05a2c4:	3c0a8007 */ 	lui	$t2,%hi(g_SkedarQuipBank)
/*  f05a2c8:	254a96d8 */ 	addiu	$t2,$t2,%lo(g_SkedarQuipBank)
/*  f05a2cc:	28c10006 */ 	slti	$at,$a2,0x6
/*  f05a2d0:	1420002a */ 	bnez	$at,.L0f05a37c
/*  f05a2d4:	afaa0084 */ 	sw	$t2,0x84($sp)
/*  f05a2d8:	10000028 */ 	b	.L0f05a37c
/*  f05a2dc:	00003025 */ 	or	$a2,$zero,$zero
.L0f05a2e0:
/*  f05a2e0:	80eb0006 */ 	lb	$t3,0x6($a3)
/*  f05a2e4:	24030029 */ 	addiu	$v1,$zero,0x29
/*  f05a2e8:	3c0c8007 */ 	lui	$t4,%hi(g_MaianQuipBank)
/*  f05a2ec:	146b000d */ 	bne	$v1,$t3,.L0f05a324
/*  f05a2f0:	258c9710 */ 	addiu	$t4,$t4,%lo(g_MaianQuipBank)
/*  f05a2f4:	28c10003 */ 	slti	$at,$a2,0x3
/*  f05a2f8:	14200020 */ 	bnez	$at,.L0f05a37c
/*  f05a2fc:	afac0084 */ 	sw	$t4,0x84($sp)
/*  f05a300:	afa60070 */ 	sw	$a2,0x70($sp)
/*  f05a304:	0c004b70 */ 	jal	random
/*  f05a308:	afa5007c */ 	sw	$a1,0x7c($sp)
/*  f05a30c:	30460001 */ 	andi	$a2,$v0,0x1
/*  f05a310:	afa60070 */ 	sw	$a2,0x70($sp)
/*  f05a314:	8fa5007c */ 	lw	$a1,0x7c($sp)
/*  f05a318:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f05a31c:	10000017 */ 	b	.L0f05a37c
/*  f05a320:	8fa8009c */ 	lw	$t0,0x9c($sp)
.L0f05a324:
/*  f05a324:	910e0007 */ 	lbu	$t6,0x7($t0)
/*  f05a328:	3c0b8007 */ 	lui	$t3,%hi(g_SpecialQuipBank)
/*  f05a32c:	256b9518 */ 	addiu	$t3,$t3,%lo(g_SpecialQuipBank)
/*  f05a330:	55c00012 */ 	bnezl	$t6,.L0f05a37c
/*  f05a334:	afab0084 */ 	sw	$t3,0x84($sp)
/*  f05a338:	90e202b1 */ 	lbu	$v0,0x2b1($a3)
/*  f05a33c:	240f0003 */ 	addiu	$t7,$zero,0x3
/*  f05a340:	28410004 */ 	slti	$at,$v0,0x4
/*  f05a344:	14200004 */ 	bnez	$at,.L0f05a358
/*  f05a348:	00000000 */ 	nop
/*  f05a34c:	a0ef02b1 */ 	sb	$t7,0x2b1($a3)
/*  f05a350:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f05a354:	90e202b1 */ 	lbu	$v0,0x2b1($a3)
.L0f05a358:
/*  f05a358:	00430019 */ 	multu	$v0,$v1
/*  f05a35c:	3c098007 */ 	lui	$t1,%hi(g_GuardQuipBank)
/*  f05a360:	25298ff8 */ 	addiu	$t1,$t1,%lo(g_GuardQuipBank)
/*  f05a364:	0000c012 */ 	mflo	$t8
/*  f05a368:	0018c8c0 */ 	sll	$t9,$t8,0x3
/*  f05a36c:	03295021 */ 	addu	$t2,$t9,$t1
/*  f05a370:	10000002 */ 	b	.L0f05a37c
/*  f05a374:	afaa0084 */ 	sw	$t2,0x84($sp)
/*  f05a378:	afab0084 */ 	sw	$t3,0x84($sp)
.L0f05a37c:
/*  f05a37c:	54c0000e */ 	bnezl	$a2,.L0f05a3b8
/*  f05a380:	90e40125 */ 	lbu	$a0,0x125($a3)
/*  f05a384:	910c0004 */ 	lbu	$t4,0x4($t0)
/*  f05a388:	5580000b */ 	bnezl	$t4,.L0f05a3b8
/*  f05a38c:	90e40125 */ 	lbu	$a0,0x125($a3)
/*  f05a390:	910d0006 */ 	lbu	$t5,0x6($t0)
/*  f05a394:	55a00008 */ 	bnezl	$t5,.L0f05a3b8
/*  f05a398:	90e40125 */ 	lbu	$a0,0x125($a3)
/*  f05a39c:	ace00120 */ 	sw	$zero,0x120($a3)
/*  f05a3a0:	8e0e0438 */ 	lw	$t6,0x438($s0)
/*  f05a3a4:	00001025 */ 	or	$v0,$zero,$zero
/*  f05a3a8:	25cf000a */ 	addiu	$t7,$t6,0xa
/*  f05a3ac:	10000206 */ 	b	.L0f05abc8
/*  f05a3b0:	ae0f0438 */ 	sw	$t7,0x438($s0)
/*  f05a3b4:	90e40125 */ 	lbu	$a0,0x125($a3)
.L0f05a3b8:
/*  f05a3b8:	afa60070 */ 	sw	$a2,0x70($sp)
/*  f05a3bc:	0fc13380 */ 	jal	teamGetChrIds
/*  f05a3c0:	afa5007c */ 	sw	$a1,0x7c($sp)
/*  f05a3c4:	8fb8009c */ 	lw	$t8,0x9c($sp)
/*  f05a3c8:	afa20088 */ 	sw	$v0,0x88($sp)
/*  f05a3cc:	afa00098 */ 	sw	$zero,0x98($sp)
/*  f05a3d0:	afa00094 */ 	sw	$zero,0x94($sp)
/*  f05a3d4:	93190004 */ 	lbu	$t9,0x4($t8)
/*  f05a3d8:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f05a3dc:	24010005 */ 	addiu	$at,$zero,0x5
/*  f05a3e0:	afb90090 */ 	sw	$t9,0x90($sp)
/*  f05a3e4:	80e30006 */ 	lb	$v1,0x6($a3)
/*  f05a3e8:	8fa5007c */ 	lw	$a1,0x7c($sp)
/*  f05a3ec:	8fa90084 */ 	lw	$t1,0x84($sp)
/*  f05a3f0:	10610007 */ 	beq	$v1,$at,.L0f05a410
/*  f05a3f4:	3c0a8007 */ 	lui	$t2,%hi(g_SpecialQuipBank)
/*  f05a3f8:	2401000f */ 	addiu	$at,$zero,0xf
/*  f05a3fc:	10610004 */ 	beq	$v1,$at,.L0f05a410
/*  f05a400:	2401003a */ 	addiu	$at,$zero,0x3a
/*  f05a404:	10610002 */ 	beq	$v1,$at,.L0f05a410
/*  f05a408:	24010028 */ 	addiu	$at,$zero,0x28
/*  f05a40c:	14610004 */ 	bne	$v1,$at,.L0f05a420
.L0f05a410:
/*  f05a410:	254a9518 */ 	addiu	$t2,$t2,%lo(g_SpecialQuipBank)
/*  f05a414:	112a0002 */ 	beq	$t1,$t2,.L0f05a420
/*  f05a418:	00000000 */ 	nop
/*  f05a41c:	afa00090 */ 	sw	$zero,0x90($sp)
.L0f05a420:
/*  f05a420:	50a0001c */ 	beqzl	$a1,.L0f05a494
/*  f05a424:	90e20126 */ 	lbu	$v0,0x126($a3)
/*  f05a428:	8ca4001c */ 	lw	$a0,0x1c($a1)
/*  f05a42c:	50800019 */ 	beqzl	$a0,.L0f05a494
/*  f05a430:	90e20126 */ 	lbu	$v0,0x126($a3)
/*  f05a434:	908b0000 */ 	lbu	$t3,0x0($a0)
/*  f05a438:	24010006 */ 	addiu	$at,$zero,0x6
/*  f05a43c:	55610015 */ 	bnel	$t3,$at,.L0f05a494
/*  f05a440:	90e20126 */ 	lbu	$v0,0x126($a3)
/*  f05a444:	0fc4a25f */ 	jal	playermgrGetPlayerNumByProp
/*  f05a448:	00000000 */ 	nop
/*  f05a44c:	8e030298 */ 	lw	$v1,0x298($s0)
/*  f05a450:	00026080 */ 	sll	$t4,$v0,0x2
/*  f05a454:	00402025 */ 	or	$a0,$v0,$zero
/*  f05a458:	0460000a */ 	bltz	$v1,.L0f05a484
/*  f05a45c:	020c6821 */ 	addu	$t5,$s0,$t4
/*  f05a460:	8dae0064 */ 	lw	$t6,0x64($t5)
/*  f05a464:	8dcf00d8 */ 	lw	$t7,0xd8($t6)
/*  f05a468:	11e00006 */ 	beqz	$t7,.L0f05a484
/*  f05a46c:	00000000 */ 	nop
/*  f05a470:	8e050294 */ 	lw	$a1,0x294($s0)
/*  f05a474:	14450003 */ 	bne	$v0,$a1,.L0f05a484
/*  f05a478:	00a02025 */ 	or	$a0,$a1,$zero
/*  f05a47c:	10000001 */ 	b	.L0f05a484
/*  f05a480:	00602025 */ 	or	$a0,$v1,$zero
.L0f05a484:
/*  f05a484:	0fc4a24b */ 	jal	setCurrentPlayerNum
/*  f05a488:	00000000 */ 	nop
/*  f05a48c:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f05a490:	90e20126 */ 	lbu	$v0,0x126($a3)
.L0f05a494:
/*  f05a494:	10400007 */ 	beqz	$v0,.L0f05a4b4
/*  f05a498:	0002c100 */ 	sll	$t8,$v0,0x4
/*  f05a49c:	8cf90120 */ 	lw	$t9,0x120($a3)
/*  f05a4a0:	0302c023 */ 	subu	$t8,$t8,$v0
/*  f05a4a4:	0018c080 */ 	sll	$t8,$t8,0x2
/*  f05a4a8:	0319082a */ 	slt	$at,$t8,$t9
/*  f05a4ac:	102001c0 */ 	beqz	$at,.L0f05abb0
/*  f05a4b0:	00000000 */ 	nop
.L0f05a4b4:
/*  f05a4b4:	0c004b70 */ 	jal	random
/*  f05a4b8:	00000000 */ 	nop
/*  f05a4bc:	8faa0090 */ 	lw	$t2,0x90($sp)
/*  f05a4c0:	304900ff */ 	andi	$t1,$v0,0xff
/*  f05a4c4:	8fab0088 */ 	lw	$t3,0x88($sp)
/*  f05a4c8:	012a082a */ 	slt	$at,$t1,$t2
/*  f05a4cc:	102001b8 */ 	beqz	$at,.L0f05abb0
/*  f05a4d0:	00000000 */ 	nop
/*  f05a4d4:	85640000 */ 	lh	$a0,0x0($t3)
/*  f05a4d8:	2401fffe */ 	addiu	$at,$zero,-2
/*  f05a4dc:	5081003e */ 	beql	$a0,$at,.L0f05a5d8
/*  f05a4e0:	8fae0094 */ 	lw	$t6,0x94($sp)
.L0f05a4e4:
/*  f05a4e4:	0fc0a1dd */ 	jal	chrFindByLiteralId
/*  f05a4e8:	00000000 */ 	nop
/*  f05a4ec:	50400033 */ 	beqzl	$v0,.L0f05a5bc
/*  f05a4f0:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05a4f4:	8c4c0020 */ 	lw	$t4,0x20($v0)
/*  f05a4f8:	00402025 */ 	or	$a0,$v0,$zero
/*  f05a4fc:	5180002f */ 	beqzl	$t4,.L0f05a5bc
/*  f05a500:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05a504:	0fc0e6a5 */ 	jal	chrIsDead
/*  f05a508:	afa20064 */ 	sw	$v0,0x64($sp)
/*  f05a50c:	1440002a */ 	bnez	$v0,.L0f05a5b8
/*  f05a510:	8fa60064 */ 	lw	$a2,0x64($sp)
/*  f05a514:	80cd0007 */ 	lb	$t5,0x7($a2)
/*  f05a518:	24010005 */ 	addiu	$at,$zero,0x5
/*  f05a51c:	51a10027 */ 	beql	$t5,$at,.L0f05a5bc
/*  f05a520:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05a524:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f05a528:	90ce02a2 */ 	lbu	$t6,0x2a2($a2)
/*  f05a52c:	90ef02a2 */ 	lbu	$t7,0x2a2($a3)
/*  f05a530:	55cf0022 */ 	bnel	$t6,$t7,.L0f05a5bc
/*  f05a534:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05a538:	90d80113 */ 	lbu	$t8,0x113($a2)
/*  f05a53c:	2b010064 */ 	slti	$at,$t8,0x64
/*  f05a540:	5420001e */ 	bnezl	$at,.L0f05a5bc
/*  f05a544:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05a548:	84c50000 */ 	lh	$a1,0x0($a2)
/*  f05a54c:	84f90000 */ 	lh	$t9,0x0($a3)
/*  f05a550:	00e02025 */ 	or	$a0,$a3,$zero
/*  f05a554:	50b90019 */ 	beql	$a1,$t9,.L0f05a5bc
/*  f05a558:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05a55c:	0fc1272c */ 	jal	chrGetDistanceToChr
/*  f05a560:	afa60064 */ 	sw	$a2,0x64($sp)
/*  f05a564:	3c017f1b */ 	lui	$at,%hi(var7f1a9d64)
/*  f05a568:	c4249d64 */ 	lwc1	$f4,%lo(var7f1a9d64)($at)
/*  f05a56c:	8fa90098 */ 	lw	$t1,0x98($sp)
/*  f05a570:	8fa60064 */ 	lw	$a2,0x64($sp)
/*  f05a574:	4604003c */ 	c.lt.s	$f0,$f4
/*  f05a578:	252a0001 */ 	addiu	$t2,$t1,0x1
/*  f05a57c:	4502000f */ 	bc1fl	.L0f05a5bc
/*  f05a580:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05a584:	afaa0098 */ 	sw	$t2,0x98($sp)
/*  f05a588:	8ccb0120 */ 	lw	$t3,0x120($a2)
/*  f05a58c:	8fac009c */ 	lw	$t4,0x9c($sp)
/*  f05a590:	2961003c */ 	slti	$at,$t3,0x3c
/*  f05a594:	50200009 */ 	beqzl	$at,.L0f05a5bc
/*  f05a598:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05a59c:	91820006 */ 	lbu	$v0,0x6($t4)
/*  f05a5a0:	240100ff */ 	addiu	$at,$zero,0xff
/*  f05a5a4:	50400005 */ 	beqzl	$v0,.L0f05a5bc
/*  f05a5a8:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05a5ac:	10410002 */ 	beq	$v0,$at,.L0f05a5b8
/*  f05a5b0:	240d0001 */ 	addiu	$t5,$zero,0x1
/*  f05a5b4:	afad0094 */ 	sw	$t5,0x94($sp)
.L0f05a5b8:
/*  f05a5b8:	8fa20088 */ 	lw	$v0,0x88($sp)
.L0f05a5bc:
/*  f05a5bc:	2401fffe */ 	addiu	$at,$zero,-2
/*  f05a5c0:	84440002 */ 	lh	$a0,0x2($v0)
/*  f05a5c4:	24420002 */ 	addiu	$v0,$v0,0x2
/*  f05a5c8:	afa20088 */ 	sw	$v0,0x88($sp)
/*  f05a5cc:	1481ffc5 */ 	bne	$a0,$at,.L0f05a4e4
/*  f05a5d0:	00000000 */ 	nop
/*  f05a5d4:	8fae0094 */ 	lw	$t6,0x94($sp)
.L0f05a5d8:
/*  f05a5d8:	8faf0098 */ 	lw	$t7,0x98($sp)
/*  f05a5dc:	15c00174 */ 	bnez	$t6,.L0f05abb0
/*  f05a5e0:	00000000 */ 	nop
/*  f05a5e4:	15e00007 */ 	bnez	$t7,.L0f05a604
/*  f05a5e8:	8fb8009c */ 	lw	$t8,0x9c($sp)
/*  f05a5ec:	93020006 */ 	lbu	$v0,0x6($t8)
/*  f05a5f0:	240100ff */ 	addiu	$at,$zero,0xff
/*  f05a5f4:	1040000a */ 	beqz	$v0,.L0f05a620
/*  f05a5f8:	00000000 */ 	nop
/*  f05a5fc:	10410008 */ 	beq	$v0,$at,.L0f05a620
/*  f05a600:	00000000 */ 	nop
.L0f05a604:
/*  f05a604:	8fb90098 */ 	lw	$t9,0x98($sp)
/*  f05a608:	8fa9009c */ 	lw	$t1,0x9c($sp)
/*  f05a60c:	1b200168 */ 	blez	$t9,.L0f05abb0
/*  f05a610:	00000000 */ 	nop
/*  f05a614:	912a0006 */ 	lbu	$t2,0x6($t1)
/*  f05a618:	19400165 */ 	blez	$t2,.L0f05abb0
/*  f05a61c:	00000000 */ 	nop
.L0f05a620:
/*  f05a620:	0c004b70 */ 	jal	random
/*  f05a624:	00000000 */ 	nop
/*  f05a628:	24010003 */ 	addiu	$at,$zero,0x3
/*  f05a62c:	0041001b */ 	divu	$zero,$v0,$at
/*  f05a630:	8fab009c */ 	lw	$t3,0x9c($sp)
/*  f05a634:	00001810 */ 	mfhi	$v1
/*  f05a638:	a3a300a7 */ 	sb	$v1,0xa7($sp)
/*  f05a63c:	916c0007 */ 	lbu	$t4,0x7($t3)
/*  f05a640:	8fb80070 */ 	lw	$t8,0x70($sp)
/*  f05a644:	8fae0084 */ 	lw	$t6,0x84($sp)
/*  f05a648:	318d0080 */ 	andi	$t5,$t4,0x80
/*  f05a64c:	15a00009 */ 	bnez	$t5,.L0f05a674
/*  f05a650:	0018c8c0 */ 	sll	$t9,$t8,0x3
/*  f05a654:	01d94821 */ 	addu	$t1,$t6,$t9
/*  f05a658:	306a00ff */ 	andi	$t2,$v1,0xff
/*  f05a65c:	000a5840 */ 	sll	$t3,$t2,0x1
/*  f05a660:	012b6021 */ 	addu	$t4,$t1,$t3
/*  f05a664:	afa90058 */ 	sw	$t1,0x58($sp)
/*  f05a668:	858d0002 */ 	lh	$t5,0x2($t4)
/*  f05a66c:	1000000c */ 	b	.L0f05a6a0
/*  f05a670:	a7ad00a4 */ 	sh	$t5,0xa4($sp)
.L0f05a674:
/*  f05a674:	8e0a0424 */ 	lw	$t2,0x424($s0)
/*  f05a678:	8fae0070 */ 	lw	$t6,0x70($sp)
/*  f05a67c:	8faf0084 */ 	lw	$t7,0x84($sp)
/*  f05a680:	914902b0 */ 	lbu	$t1,0x2b0($t2)
/*  f05a684:	000ec8c0 */ 	sll	$t9,$t6,0x3
/*  f05a688:	01f91021 */ 	addu	$v0,$t7,$t9
/*  f05a68c:	00095840 */ 	sll	$t3,$t1,0x1
/*  f05a690:	004b6021 */ 	addu	$t4,$v0,$t3
/*  f05a694:	858d0002 */ 	lh	$t5,0x2($t4)
/*  f05a698:	afa20058 */ 	sw	$v0,0x58($sp)
/*  f05a69c:	a7ad00a4 */ 	sh	$t5,0xa4($sp)
.L0f05a6a0:
/*  f05a6a0:	0fc133cd */ 	jal	audioWasNotPlayedRecently
/*  f05a6a4:	87a400a4 */ 	lh	$a0,0xa4($sp)
/*  f05a6a8:	1440000a */ 	bnez	$v0,.L0f05a6d4
/*  f05a6ac:	24010001 */ 	addiu	$at,$zero,0x1
/*  f05a6b0:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f05a6b4:	00001825 */ 	or	$v1,$zero,$zero
/*  f05a6b8:	24080001 */ 	addiu	$t0,$zero,0x1
/*  f05a6bc:	10e00003 */ 	beqz	$a3,.L0f05a6cc
/*  f05a6c0:	3c0e8007 */ 	lui	$t6,%hi(g_GuardQuipBank)
/*  f05a6c4:	10000001 */ 	b	.L0f05a6cc
/*  f05a6c8:	90e302fe */ 	lbu	$v1,0x2fe($a3)
.L0f05a6cc:
/*  f05a6cc:	54610092 */ 	bnel	$v1,$at,.L0f05a918
/*  f05a6d0:	8fad0070 */ 	lw	$t5,0x70($sp)
.L0f05a6d4:
/*  f05a6d4:	0fc133bb */ 	jal	audioMarkAsRecentlyPlayed
/*  f05a6d8:	87a400a4 */ 	lh	$a0,0xa4($sp)
/*  f05a6dc:	87b800a4 */ 	lh	$t8,0xa4($sp)
/*  f05a6e0:	2401034e */ 	addiu	$at,$zero,0x34e
/*  f05a6e4:	93ae006b */ 	lbu	$t6,0x6b($sp)
/*  f05a6e8:	57010005 */ 	bnel	$t8,$at,.L0f05a700
/*  f05a6ec:	8e190424 */ 	lw	$t9,0x424($s0)
/*  f05a6f0:	15c00002 */ 	bnez	$t6,.L0f05a6fc
/*  f05a6f4:	240f034d */ 	addiu	$t7,$zero,0x34d
/*  f05a6f8:	a7af00a4 */ 	sh	$t7,0xa4($sp)
.L0f05a6fc:
/*  f05a6fc:	8e190424 */ 	lw	$t9,0x424($s0)
.L0f05a700:
/*  f05a700:	240103f7 */ 	addiu	$at,$zero,0x3f7
/*  f05a704:	af200120 */ 	sw	$zero,0x120($t9)
/*  f05a708:	8faa009c */ 	lw	$t2,0x9c($sp)
/*  f05a70c:	8e0b0424 */ 	lw	$t3,0x424($s0)
/*  f05a710:	91490005 */ 	lbu	$t1,0x5($t2)
/*  f05a714:	a1690126 */ 	sb	$t1,0x126($t3)
/*  f05a718:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f05a71c:	90ec0332 */ 	lbu	$t4,0x332($a3)
/*  f05a720:	258d0001 */ 	addiu	$t5,$t4,0x1
/*  f05a724:	a0ed0332 */ 	sb	$t5,0x332($a3)
/*  f05a728:	87b800a4 */ 	lh	$t8,0xa4($sp)
/*  f05a72c:	13010022 */ 	beq	$t8,$at,.L0f05a7b8
/*  f05a730:	24010331 */ 	addiu	$at,$zero,0x331
/*  f05a734:	13010020 */ 	beq	$t8,$at,.L0f05a7b8
/*  f05a738:	240103a1 */ 	addiu	$at,$zero,0x3a1
/*  f05a73c:	1301001e */ 	beq	$t8,$at,.L0f05a7b8
/*  f05a740:	24050009 */ 	addiu	$a1,$zero,0x9
/*  f05a744:	8e0e0424 */ 	lw	$t6,0x424($s0)
/*  f05a748:	3406ffff */ 	dli	$a2,0xffff
/*  f05a74c:	0fc249af */ 	jal	func0f0926bc
/*  f05a750:	8dc4001c */ 	lw	$a0,0x1c($t6)
/*  f05a754:	8e0f0424 */ 	lw	$t7,0x424($s0)
/*  f05a758:	3c01bf80 */ 	lui	$at,0xbf80
/*  f05a75c:	44810000 */ 	mtc1	$at,$f0
/*  f05a760:	8de5001c */ 	lw	$a1,0x1c($t7)
/*  f05a764:	2419ffff */ 	addiu	$t9,$zero,-1
/*  f05a768:	240a0008 */ 	addiu	$t2,$zero,0x8
/*  f05a76c:	24090009 */ 	addiu	$t1,$zero,0x9
/*  f05a770:	240bffff */ 	addiu	$t3,$zero,-1
/*  f05a774:	afab002c */ 	sw	$t3,0x2c($sp)
/*  f05a778:	afa9001c */ 	sw	$t1,0x1c($sp)
/*  f05a77c:	afaa0014 */ 	sw	$t2,0x14($sp)
/*  f05a780:	afb90010 */ 	sw	$t9,0x10($sp)
/*  f05a784:	afa00028 */ 	sw	$zero,0x28($sp)
/*  f05a788:	afa00020 */ 	sw	$zero,0x20($sp)
/*  f05a78c:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f05a790:	00002025 */ 	or	$a0,$zero,$zero
/*  f05a794:	87a600a4 */ 	lh	$a2,0xa4($sp)
/*  f05a798:	2407ffff */ 	addiu	$a3,$zero,-1
/*  f05a79c:	e7a00038 */ 	swc1	$f0,0x38($sp)
/*  f05a7a0:	e7a00034 */ 	swc1	$f0,0x34($sp)
/*  f05a7a4:	e7a00030 */ 	swc1	$f0,0x30($sp)
/*  f05a7a8:	0fc24e7e */ 	jal	propsnd0f0939f8
/*  f05a7ac:	e7a00024 */ 	swc1	$f0,0x24($sp)
/*  f05a7b0:	10000026 */ 	b	.L0f05a84c
/*  f05a7b4:	8faa009c */ 	lw	$t2,0x9c($sp)
.L0f05a7b8:
/*  f05a7b8:	0fc13160 */ 	jal	chrGetDistanceLostToTargetInLastSecond
/*  f05a7bc:	8e040424 */ 	lw	$a0,0x424($s0)
/*  f05a7c0:	18400003 */ 	blez	$v0,.L0f05a7d0
/*  f05a7c4:	00021823 */ 	negu	$v1,$v0
/*  f05a7c8:	10000001 */ 	b	.L0f05a7d0
/*  f05a7cc:	00401825 */ 	or	$v1,$v0,$zero
.L0f05a7d0:
/*  f05a7d0:	28610033 */ 	slti	$at,$v1,0x33
/*  f05a7d4:	1420001c */ 	bnez	$at,.L0f05a848
/*  f05a7d8:	24050009 */ 	addiu	$a1,$zero,0x9
/*  f05a7dc:	8e0c0424 */ 	lw	$t4,0x424($s0)
/*  f05a7e0:	3406ffff */ 	dli	$a2,0xffff
/*  f05a7e4:	0fc249af */ 	jal	func0f0926bc
/*  f05a7e8:	8d84001c */ 	lw	$a0,0x1c($t4)
/*  f05a7ec:	8e0d0424 */ 	lw	$t5,0x424($s0)
/*  f05a7f0:	3c01bf80 */ 	lui	$at,0xbf80
/*  f05a7f4:	44810000 */ 	mtc1	$at,$f0
/*  f05a7f8:	8da5001c */ 	lw	$a1,0x1c($t5)
/*  f05a7fc:	2418ffff */ 	addiu	$t8,$zero,-1
/*  f05a800:	240e0008 */ 	addiu	$t6,$zero,0x8
/*  f05a804:	240f0009 */ 	addiu	$t7,$zero,0x9
/*  f05a808:	2419ffff */ 	addiu	$t9,$zero,-1
/*  f05a80c:	afb9002c */ 	sw	$t9,0x2c($sp)
/*  f05a810:	afaf001c */ 	sw	$t7,0x1c($sp)
/*  f05a814:	afae0014 */ 	sw	$t6,0x14($sp)
/*  f05a818:	afb80010 */ 	sw	$t8,0x10($sp)
/*  f05a81c:	afa00028 */ 	sw	$zero,0x28($sp)
/*  f05a820:	afa00020 */ 	sw	$zero,0x20($sp)
/*  f05a824:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f05a828:	00002025 */ 	or	$a0,$zero,$zero
/*  f05a82c:	87a600a4 */ 	lh	$a2,0xa4($sp)
/*  f05a830:	2407ffff */ 	addiu	$a3,$zero,-1
/*  f05a834:	e7a00038 */ 	swc1	$f0,0x38($sp)
/*  f05a838:	e7a00034 */ 	swc1	$f0,0x34($sp)
/*  f05a83c:	e7a00030 */ 	swc1	$f0,0x30($sp)
/*  f05a840:	0fc24e7e */ 	jal	propsnd0f0939f8
/*  f05a844:	e7a00024 */ 	swc1	$f0,0x24($sp)
.L0f05a848:
/*  f05a848:	8faa009c */ 	lw	$t2,0x9c($sp)
.L0f05a84c:
/*  f05a84c:	91420008 */ 	lbu	$v0,0x8($t2)
/*  f05a850:	1040001a */ 	beqz	$v0,.L0f05a8bc
/*  f05a854:	00000000 */ 	nop
/*  f05a858:	91490007 */ 	lbu	$t1,0x7($t2)
/*  f05a85c:	93a300a7 */ 	lbu	$v1,0xa7($sp)
/*  f05a860:	000260c0 */ 	sll	$t4,$v0,0x3
/*  f05a864:	312b0080 */ 	andi	$t3,$t1,0x80
/*  f05a868:	15600014 */ 	bnez	$t3,.L0f05a8bc
/*  f05a86c:	28610003 */ 	slti	$at,$v1,0x3
/*  f05a870:	14200002 */ 	bnez	$at,.L0f05a87c
/*  f05a874:	3c048007 */ 	lui	$a0,%hi(g_SpecialQuipBank+0x16a)
/*  f05a878:	24030002 */ 	addiu	$v1,$zero,0x2
.L0f05a87c:
/*  f05a87c:	00036840 */ 	sll	$t5,$v1,0x1
/*  f05a880:	018dc021 */ 	addu	$t8,$t4,$t5
/*  f05a884:	00982021 */ 	addu	$a0,$a0,$t8
/*  f05a888:	0fc5b9f1 */ 	jal	langGet
/*  f05a88c:	84849682 */ 	lh	$a0,%lo(g_SpecialQuipBank+0x16a)($a0)
/*  f05a890:	afa20080 */ 	sw	$v0,0x80($sp)
/*  f05a894:	0c004103 */ 	jal	sndIsFiltered
/*  f05a898:	87a400a4 */ 	lh	$a0,0xa4($sp)
/*  f05a89c:	144000c4 */ 	bnez	$v0,.L0f05abb0
/*  f05a8a0:	8fa40080 */ 	lw	$a0,0x80($sp)
/*  f05a8a4:	8fae009c */ 	lw	$t6,0x9c($sp)
/*  f05a8a8:	24050006 */ 	addiu	$a1,$zero,0x6
/*  f05a8ac:	0fc3780d */ 	jal	hudmsgCreateWithColour
/*  f05a8b0:	91c60009 */ 	lbu	$a2,0x9($t6)
/*  f05a8b4:	100000be */ 	b	.L0f05abb0
/*  f05a8b8:	00000000 */ 	nop
.L0f05a8bc:
/*  f05a8bc:	104000bc */ 	beqz	$v0,.L0f05abb0
/*  f05a8c0:	00000000 */ 	nop
/*  f05a8c4:	8e190424 */ 	lw	$t9,0x424($s0)
/*  f05a8c8:	000278c0 */ 	sll	$t7,$v0,0x3
/*  f05a8cc:	3c048007 */ 	lui	$a0,%hi(g_SpecialQuipBank+0x16a)
/*  f05a8d0:	932a02b0 */ 	lbu	$t2,0x2b0($t9)
/*  f05a8d4:	000a4840 */ 	sll	$t1,$t2,0x1
/*  f05a8d8:	01e95821 */ 	addu	$t3,$t7,$t1
/*  f05a8dc:	008b2021 */ 	addu	$a0,$a0,$t3
/*  f05a8e0:	0fc5b9f1 */ 	jal	langGet
/*  f05a8e4:	84849682 */ 	lh	$a0,%lo(g_SpecialQuipBank+0x16a)($a0)
/*  f05a8e8:	afa20080 */ 	sw	$v0,0x80($sp)
/*  f05a8ec:	0c004103 */ 	jal	sndIsFiltered
/*  f05a8f0:	87a400a4 */ 	lh	$a0,0xa4($sp)
/*  f05a8f4:	144000ae */ 	bnez	$v0,.L0f05abb0
/*  f05a8f8:	8fa40080 */ 	lw	$a0,0x80($sp)
/*  f05a8fc:	8fac009c */ 	lw	$t4,0x9c($sp)
/*  f05a900:	24050006 */ 	addiu	$a1,$zero,0x6
/*  f05a904:	0fc3780d */ 	jal	hudmsgCreateWithColour
/*  f05a908:	91860009 */ 	lbu	$a2,0x9($t4)
/*  f05a90c:	100000a8 */ 	b	.L0f05abb0
/*  f05a910:	00000000 */ 	nop
/*  f05a914:	8fad0070 */ 	lw	$t5,0x70($sp)
.L0f05a918:
/*  f05a918:	25ce8ff8 */ 	addiu	$t6,$t6,%lo(g_GuardQuipBank)
/*  f05a91c:	a7a000a4 */ 	sh	$zero,0xa4($sp)
/*  f05a920:	000dc0c0 */ 	sll	$t8,$t5,0x3
/*  f05a924:	030e1021 */ 	addu	$v0,$t8,$t6
/*  f05a928:	afa2005c */ 	sw	$v0,0x5c($sp)
.L0f05a92c:
/*  f05a92c:	8fa2005c */ 	lw	$v0,0x5c($sp)
/*  f05a930:	00081840 */ 	sll	$v1,$t0,0x1
/*  f05a934:	0043c821 */ 	addu	$t9,$v0,$v1
/*  f05a938:	87240000 */ 	lh	$a0,0x0($t9)
/*  f05a93c:	a3a800a3 */ 	sb	$t0,0xa3($sp)
/*  f05a940:	0fc133cd */ 	jal	audioWasNotPlayedRecently
/*  f05a944:	afa30054 */ 	sw	$v1,0x54($sp)
/*  f05a948:	8fa30054 */ 	lw	$v1,0x54($sp)
/*  f05a94c:	1040000d */ 	beqz	$v0,.L0f05a984
/*  f05a950:	93a800a3 */ 	lbu	$t0,0xa3($sp)
/*  f05a954:	8faa0058 */ 	lw	$t2,0x58($sp)
/*  f05a958:	01432821 */ 	addu	$a1,$t2,$v1
/*  f05a95c:	84a40000 */ 	lh	$a0,0x0($a1)
/*  f05a960:	a3a800a3 */ 	sb	$t0,0xa3($sp)
/*  f05a964:	0fc133cd */ 	jal	audioWasNotPlayedRecently
/*  f05a968:	afa50050 */ 	sw	$a1,0x50($sp)
/*  f05a96c:	8fa50050 */ 	lw	$a1,0x50($sp)
/*  f05a970:	10400004 */ 	beqz	$v0,.L0f05a984
/*  f05a974:	93a800a3 */ 	lbu	$t0,0xa3($sp)
/*  f05a978:	84af0000 */ 	lh	$t7,0x0($a1)
/*  f05a97c:	10000006 */ 	b	.L0f05a998
/*  f05a980:	a7af00a4 */ 	sh	$t7,0xa4($sp)
.L0f05a984:
/*  f05a984:	25080001 */ 	addiu	$t0,$t0,0x1
/*  f05a988:	310900ff */ 	andi	$t1,$t0,0xff
/*  f05a98c:	29210004 */ 	slti	$at,$t1,0x4
/*  f05a990:	1420ffe6 */ 	bnez	$at,.L0f05a92c
/*  f05a994:	01204025 */ 	or	$t0,$t1,$zero
.L0f05a998:
/*  f05a998:	87ab00a4 */ 	lh	$t3,0xa4($sp)
/*  f05a99c:	1160007a */ 	beqz	$t3,.L0f05ab88
/*  f05a9a0:	01602025 */ 	or	$a0,$t3,$zero
/*  f05a9a4:	0fc133bb */ 	jal	audioMarkAsRecentlyPlayed
/*  f05a9a8:	a3a800a3 */ 	sb	$t0,0xa3($sp)
/*  f05a9ac:	87ac00a4 */ 	lh	$t4,0xa4($sp)
/*  f05a9b0:	2401034e */ 	addiu	$at,$zero,0x34e
/*  f05a9b4:	93a800a3 */ 	lbu	$t0,0xa3($sp)
/*  f05a9b8:	15810004 */ 	bne	$t4,$at,.L0f05a9cc
/*  f05a9bc:	93ad006b */ 	lbu	$t5,0x6b($sp)
/*  f05a9c0:	15a00002 */ 	bnez	$t5,.L0f05a9cc
/*  f05a9c4:	2418034d */ 	addiu	$t8,$zero,0x34d
/*  f05a9c8:	a7b800a4 */ 	sh	$t8,0xa4($sp)
.L0f05a9cc:
/*  f05a9cc:	8e0e0424 */ 	lw	$t6,0x424($s0)
/*  f05a9d0:	240103f7 */ 	addiu	$at,$zero,0x3f7
/*  f05a9d4:	adc00120 */ 	sw	$zero,0x120($t6)
/*  f05a9d8:	8fb9009c */ 	lw	$t9,0x9c($sp)
/*  f05a9dc:	8e0f0424 */ 	lw	$t7,0x424($s0)
/*  f05a9e0:	932a0005 */ 	lbu	$t2,0x5($t9)
/*  f05a9e4:	a1ea0126 */ 	sb	$t2,0x126($t7)
/*  f05a9e8:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f05a9ec:	90e90332 */ 	lbu	$t1,0x332($a3)
/*  f05a9f0:	252b0001 */ 	addiu	$t3,$t1,0x1
/*  f05a9f4:	a0eb0332 */ 	sb	$t3,0x332($a3)
/*  f05a9f8:	87ac00a4 */ 	lh	$t4,0xa4($sp)
/*  f05a9fc:	11810024 */ 	beq	$t4,$at,.L0f05aa90
/*  f05aa00:	24010331 */ 	addiu	$at,$zero,0x331
/*  f05aa04:	11810022 */ 	beq	$t4,$at,.L0f05aa90
/*  f05aa08:	240103a1 */ 	addiu	$at,$zero,0x3a1
/*  f05aa0c:	51810021 */ 	beql	$t4,$at,.L0f05aa94
/*  f05aa10:	8e040424 */ 	lw	$a0,0x424($s0)
/*  f05aa14:	8e0d0424 */ 	lw	$t5,0x424($s0)
/*  f05aa18:	24050009 */ 	addiu	$a1,$zero,0x9
/*  f05aa1c:	3406ffff */ 	dli	$a2,0xffff
/*  f05aa20:	8da4001c */ 	lw	$a0,0x1c($t5)
/*  f05aa24:	0fc249af */ 	jal	func0f0926bc
/*  f05aa28:	a3a800a3 */ 	sb	$t0,0xa3($sp)
/*  f05aa2c:	8e180424 */ 	lw	$t8,0x424($s0)
/*  f05aa30:	3c01bf80 */ 	lui	$at,0xbf80
/*  f05aa34:	44810000 */ 	mtc1	$at,$f0
/*  f05aa38:	8f05001c */ 	lw	$a1,0x1c($t8)
/*  f05aa3c:	240effff */ 	addiu	$t6,$zero,-1
/*  f05aa40:	24190008 */ 	addiu	$t9,$zero,0x8
/*  f05aa44:	240a0009 */ 	addiu	$t2,$zero,0x9
/*  f05aa48:	240fffff */ 	addiu	$t7,$zero,-1
/*  f05aa4c:	afaf002c */ 	sw	$t7,0x2c($sp)
/*  f05aa50:	afaa001c */ 	sw	$t2,0x1c($sp)
/*  f05aa54:	afb90014 */ 	sw	$t9,0x14($sp)
/*  f05aa58:	afae0010 */ 	sw	$t6,0x10($sp)
/*  f05aa5c:	afa00028 */ 	sw	$zero,0x28($sp)
/*  f05aa60:	afa00020 */ 	sw	$zero,0x20($sp)
/*  f05aa64:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f05aa68:	00002025 */ 	or	$a0,$zero,$zero
/*  f05aa6c:	87a600a4 */ 	lh	$a2,0xa4($sp)
/*  f05aa70:	2407ffff */ 	addiu	$a3,$zero,-1
/*  f05aa74:	e7a00038 */ 	swc1	$f0,0x38($sp)
/*  f05aa78:	e7a00034 */ 	swc1	$f0,0x34($sp)
/*  f05aa7c:	e7a00030 */ 	swc1	$f0,0x30($sp)
/*  f05aa80:	0fc24e7e */ 	jal	propsnd0f0939f8
/*  f05aa84:	e7a00024 */ 	swc1	$f0,0x24($sp)
/*  f05aa88:	1000002a */ 	b	.L0f05ab34
/*  f05aa8c:	93a800a3 */ 	lbu	$t0,0xa3($sp)
.L0f05aa90:
/*  f05aa90:	8e040424 */ 	lw	$a0,0x424($s0)
.L0f05aa94:
/*  f05aa94:	0fc13160 */ 	jal	chrGetDistanceLostToTargetInLastSecond
/*  f05aa98:	a3a800a3 */ 	sb	$t0,0xa3($sp)
/*  f05aa9c:	18400003 */ 	blez	$v0,.L0f05aaac
/*  f05aaa0:	93a800a3 */ 	lbu	$t0,0xa3($sp)
/*  f05aaa4:	10000002 */ 	b	.L0f05aab0
/*  f05aaa8:	00401825 */ 	or	$v1,$v0,$zero
.L0f05aaac:
/*  f05aaac:	00021823 */ 	negu	$v1,$v0
.L0f05aab0:
/*  f05aab0:	28610033 */ 	slti	$at,$v1,0x33
/*  f05aab4:	54200020 */ 	bnezl	$at,.L0f05ab38
/*  f05aab8:	8fb9009c */ 	lw	$t9,0x9c($sp)
/*  f05aabc:	8e090424 */ 	lw	$t1,0x424($s0)
/*  f05aac0:	24050009 */ 	addiu	$a1,$zero,0x9
/*  f05aac4:	3406ffff */ 	dli	$a2,0xffff
/*  f05aac8:	8d24001c */ 	lw	$a0,0x1c($t1)
/*  f05aacc:	0fc249af */ 	jal	func0f0926bc
/*  f05aad0:	a3a800a3 */ 	sb	$t0,0xa3($sp)
/*  f05aad4:	8e0b0424 */ 	lw	$t3,0x424($s0)
/*  f05aad8:	3c01bf80 */ 	lui	$at,0xbf80
/*  f05aadc:	44810000 */ 	mtc1	$at,$f0
/*  f05aae0:	8d65001c */ 	lw	$a1,0x1c($t3)
/*  f05aae4:	240cffff */ 	addiu	$t4,$zero,-1
/*  f05aae8:	240d0008 */ 	addiu	$t5,$zero,0x8
/*  f05aaec:	24180009 */ 	addiu	$t8,$zero,0x9
/*  f05aaf0:	240effff */ 	addiu	$t6,$zero,-1
/*  f05aaf4:	afae002c */ 	sw	$t6,0x2c($sp)
/*  f05aaf8:	afb8001c */ 	sw	$t8,0x1c($sp)
/*  f05aafc:	afad0014 */ 	sw	$t5,0x14($sp)
/*  f05ab00:	afac0010 */ 	sw	$t4,0x10($sp)
/*  f05ab04:	afa00028 */ 	sw	$zero,0x28($sp)
/*  f05ab08:	afa00020 */ 	sw	$zero,0x20($sp)
/*  f05ab0c:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f05ab10:	00002025 */ 	or	$a0,$zero,$zero
/*  f05ab14:	87a600a4 */ 	lh	$a2,0xa4($sp)
/*  f05ab18:	2407ffff */ 	addiu	$a3,$zero,-1
/*  f05ab1c:	e7a00038 */ 	swc1	$f0,0x38($sp)
/*  f05ab20:	e7a00034 */ 	swc1	$f0,0x34($sp)
/*  f05ab24:	e7a00030 */ 	swc1	$f0,0x30($sp)
/*  f05ab28:	0fc24e7e */ 	jal	propsnd0f0939f8
/*  f05ab2c:	e7a00024 */ 	swc1	$f0,0x24($sp)
/*  f05ab30:	93a800a3 */ 	lbu	$t0,0xa3($sp)
.L0f05ab34:
/*  f05ab34:	8fb9009c */ 	lw	$t9,0x9c($sp)
.L0f05ab38:
/*  f05ab38:	00087840 */ 	sll	$t7,$t0,0x1
/*  f05ab3c:	3c048007 */ 	lui	$a0,%hi(g_SpecialQuipBank+0x168)
/*  f05ab40:	93220008 */ 	lbu	$v0,0x8($t9)
/*  f05ab44:	1040001a */ 	beqz	$v0,.L0f05abb0
/*  f05ab48:	000250c0 */ 	sll	$t2,$v0,0x3
/*  f05ab4c:	014f4821 */ 	addu	$t1,$t2,$t7
/*  f05ab50:	00892021 */ 	addu	$a0,$a0,$t1
/*  f05ab54:	0fc5b9f1 */ 	jal	langGet
/*  f05ab58:	84849680 */ 	lh	$a0,%lo(g_SpecialQuipBank+0x168)($a0)
/*  f05ab5c:	afa20080 */ 	sw	$v0,0x80($sp)
/*  f05ab60:	0c004103 */ 	jal	sndIsFiltered
/*  f05ab64:	87a400a4 */ 	lh	$a0,0xa4($sp)
/*  f05ab68:	14400011 */ 	bnez	$v0,.L0f05abb0
/*  f05ab6c:	8fa40080 */ 	lw	$a0,0x80($sp)
/*  f05ab70:	8fab009c */ 	lw	$t3,0x9c($sp)
/*  f05ab74:	24050006 */ 	addiu	$a1,$zero,0x6
/*  f05ab78:	0fc3780d */ 	jal	hudmsgCreateWithColour
/*  f05ab7c:	91660009 */ 	lbu	$a2,0x9($t3)
/*  f05ab80:	1000000b */ 	b	.L0f05abb0
/*  f05ab84:	00000000 */ 	nop
.L0f05ab88:
/*  f05ab88:	8e0c0424 */ 	lw	$t4,0x424($s0)
/*  f05ab8c:	3c050800 */ 	lui	$a1,0x800
/*  f05ab90:	24060001 */ 	addiu	$a2,$zero,0x1
/*  f05ab94:	ad800120 */ 	sw	$zero,0x120($t4)
/*  f05ab98:	8fad009c */ 	lw	$t5,0x9c($sp)
/*  f05ab9c:	8e0e0424 */ 	lw	$t6,0x424($s0)
/*  f05aba0:	91b80005 */ 	lbu	$t8,0x5($t5)
/*  f05aba4:	a1d80126 */ 	sb	$t8,0x126($t6)
/*  f05aba8:	0fc12782 */ 	jal	chrUnsetFlags
/*  f05abac:	8e040424 */ 	lw	$a0,0x424($s0)
.L0f05abb0:
/*  f05abb0:	0fc4a24b */ 	jal	setCurrentPlayerNum
/*  f05abb4:	8fa40078 */ 	lw	$a0,0x78($sp)
/*  f05abb8:	8e190438 */ 	lw	$t9,0x438($s0)
/*  f05abbc:	00001025 */ 	or	$v0,$zero,$zero
/*  f05abc0:	272a000a */ 	addiu	$t2,$t9,0xa
/*  f05abc4:	ae0a0438 */ 	sw	$t2,0x438($s0)
.L0f05abc8:
/*  f05abc8:	8fbf004c */ 	lw	$ra,0x4c($sp)
/*  f05abcc:	8fb00048 */ 	lw	$s0,0x48($sp)
/*  f05abd0:	27bd00a8 */ 	addiu	$sp,$sp,0xa8
/*  f05abd4:	03e00008 */ 	jr	$ra
/*  f05abd8:	00000000 */ 	nop
);
#else
GLOBAL_ASM(
glabel aiSayQuip
.late_rodata
glabel var7f1a9d64
.word 0x45dac000
.text
/*  f0596c0:	27bdff58 */ 	addiu	$sp,$sp,-168
/*  f0596c4:	afb00048 */ 	sw	$s0,0x48($sp)
/*  f0596c8:	3c10800a */ 	lui	$s0,%hi(g_Vars)
/*  f0596cc:	2610e6c0 */ 	addiu	$s0,$s0,%lo(g_Vars)
/*  f0596d0:	8e0e0434 */ 	lw	$t6,0x434($s0)
/*  f0596d4:	8e0f0438 */ 	lw	$t7,0x438($s0)
/*  f0596d8:	afbf004c */ 	sw	$ra,0x4c($sp)
/*  f0596dc:	8e040424 */ 	lw	$a0,0x424($s0)
/*  f0596e0:	01cf1021 */ 	addu	$v0,$t6,$t7
/*  f0596e4:	90450002 */ 	lbu	$a1,0x2($v0)
/*  f0596e8:	0fc123d5 */ 	jal	chrFindById
/*  f0596ec:	afa2009c */ 	sw	$v0,0x9c($sp)
/*  f0596f0:	8e18028c */ 	lw	$t8,0x28c($s0)
/*  f0596f4:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f0596f8:	8fa8009c */ 	lw	$t0,0x9c($sp)
/*  f0596fc:	afb80078 */ 	sw	$t8,0x78($sp)
/*  f059700:	94f90192 */ 	lhu	$t9,0x192($a3)
/*  f059704:	91060003 */ 	lbu	$a2,0x3($t0)
/*  f059708:	00402825 */ 	or	$a1,$v0,$zero
/*  f05970c:	33291000 */ 	andi	$t1,$t9,0x1000
/*  f059710:	10e00003 */ 	beqz	$a3,.NB0f059720
/*  f059714:	a3a9006b */ 	sb	$t1,0x6b($sp)
/*  f059718:	10000002 */ 	beqz	$zero,.NB0f059724
/*  f05971c:	90e302fe */ 	lbu	$v1,0x2fe($a3)
.NB0f059720:
/*  f059720:	00001825 */ 	or	$v1,$zero,$zero
.NB0f059724:
/*  f059724:	24010001 */ 	addiu	$at,$zero,0x1
/*  f059728:	14610007 */ 	bne	$v1,$at,.NB0f059748
/*  f05972c:	3c0a8007 */ 	lui	$t2,%hi(g_SkedarQuipBank)
/*  f059730:	254abdd8 */ 	addiu	$t2,$t2,%lo(g_SkedarQuipBank)
/*  f059734:	28c10006 */ 	slti	$at,$a2,0x6
/*  f059738:	1420002a */ 	bnez	$at,.NB0f0597e4
/*  f05973c:	afaa0084 */ 	sw	$t2,0x84($sp)
/*  f059740:	10000028 */ 	beqz	$zero,.NB0f0597e4
/*  f059744:	00003025 */ 	or	$a2,$zero,$zero
.NB0f059748:
/*  f059748:	80eb0006 */ 	lb	$t3,0x6($a3)
/*  f05974c:	24030029 */ 	addiu	$v1,$zero,0x29
/*  f059750:	3c0c8007 */ 	lui	$t4,%hi(g_MaianQuipBank)
/*  f059754:	146b000d */ 	bne	$v1,$t3,.NB0f05978c
/*  f059758:	258cbe10 */ 	addiu	$t4,$t4,%lo(g_MaianQuipBank)
/*  f05975c:	28c10003 */ 	slti	$at,$a2,0x3
/*  f059760:	14200020 */ 	bnez	$at,.NB0f0597e4
/*  f059764:	afac0084 */ 	sw	$t4,0x84($sp)
/*  f059768:	afa60070 */ 	sw	$a2,0x70($sp)
/*  f05976c:	0c004d84 */ 	jal	random
/*  f059770:	afa5007c */ 	sw	$a1,0x7c($sp)
/*  f059774:	30460001 */ 	andi	$a2,$v0,0x1
/*  f059778:	afa60070 */ 	sw	$a2,0x70($sp)
/*  f05977c:	8fa5007c */ 	lw	$a1,0x7c($sp)
/*  f059780:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f059784:	10000017 */ 	beqz	$zero,.NB0f0597e4
/*  f059788:	8fa8009c */ 	lw	$t0,0x9c($sp)
.NB0f05978c:
/*  f05978c:	910e0007 */ 	lbu	$t6,0x7($t0)
/*  f059790:	3c0b8007 */ 	lui	$t3,%hi(g_SpecialQuipBank)
/*  f059794:	256bbc18 */ 	addiu	$t3,$t3,%lo(g_SpecialQuipBank)
/*  f059798:	55c00012 */ 	bnezl	$t6,.NB0f0597e4
/*  f05979c:	afab0084 */ 	sw	$t3,0x84($sp)
/*  f0597a0:	90e202b1 */ 	lbu	$v0,0x2b1($a3)
/*  f0597a4:	240f0003 */ 	addiu	$t7,$zero,0x3
/*  f0597a8:	28410004 */ 	slti	$at,$v0,0x4
/*  f0597ac:	14200004 */ 	bnez	$at,.NB0f0597c0
/*  f0597b0:	00000000 */ 	sll	$zero,$zero,0x0
/*  f0597b4:	a0ef02b1 */ 	sb	$t7,0x2b1($a3)
/*  f0597b8:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f0597bc:	90e202b1 */ 	lbu	$v0,0x2b1($a3)
.NB0f0597c0:
/*  f0597c0:	00430019 */ 	multu	$v0,$v1
/*  f0597c4:	3c098007 */ 	lui	$t1,%hi(g_GuardQuipBank)
/*  f0597c8:	2529b6f8 */ 	addiu	$t1,$t1,%lo(g_GuardQuipBank)
/*  f0597cc:	0000c012 */ 	mflo	$t8
/*  f0597d0:	0018c8c0 */ 	sll	$t9,$t8,0x3
/*  f0597d4:	03295021 */ 	addu	$t2,$t9,$t1
/*  f0597d8:	10000002 */ 	beqz	$zero,.NB0f0597e4
/*  f0597dc:	afaa0084 */ 	sw	$t2,0x84($sp)
/*  f0597e0:	afab0084 */ 	sw	$t3,0x84($sp)
.NB0f0597e4:
/*  f0597e4:	54c0000e */ 	bnezl	$a2,.NB0f059820
/*  f0597e8:	90e40125 */ 	lbu	$a0,0x125($a3)
/*  f0597ec:	910c0004 */ 	lbu	$t4,0x4($t0)
/*  f0597f0:	5580000b */ 	bnezl	$t4,.NB0f059820
/*  f0597f4:	90e40125 */ 	lbu	$a0,0x125($a3)
/*  f0597f8:	910d0006 */ 	lbu	$t5,0x6($t0)
/*  f0597fc:	55a00008 */ 	bnezl	$t5,.NB0f059820
/*  f059800:	90e40125 */ 	lbu	$a0,0x125($a3)
/*  f059804:	ace00120 */ 	sw	$zero,0x120($a3)
/*  f059808:	8e0e0438 */ 	lw	$t6,0x438($s0)
/*  f05980c:	00001025 */ 	or	$v0,$zero,$zero
/*  f059810:	25cf000a */ 	addiu	$t7,$t6,0xa
/*  f059814:	100001fa */ 	beqz	$zero,.NB0f05a000
/*  f059818:	ae0f0438 */ 	sw	$t7,0x438($s0)
/*  f05981c:	90e40125 */ 	lbu	$a0,0x125($a3)
.NB0f059820:
/*  f059820:	afa60070 */ 	sw	$a2,0x70($sp)
/*  f059824:	0fc13019 */ 	jal	teamGetChrIds
/*  f059828:	afa5007c */ 	sw	$a1,0x7c($sp)
/*  f05982c:	8fb8009c */ 	lw	$t8,0x9c($sp)
/*  f059830:	afa20088 */ 	sw	$v0,0x88($sp)
/*  f059834:	afa00098 */ 	sw	$zero,0x98($sp)
/*  f059838:	afa00094 */ 	sw	$zero,0x94($sp)
/*  f05983c:	93190004 */ 	lbu	$t9,0x4($t8)
/*  f059840:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f059844:	24010005 */ 	addiu	$at,$zero,0x5
/*  f059848:	afb90090 */ 	sw	$t9,0x90($sp)
/*  f05984c:	80e30006 */ 	lb	$v1,0x6($a3)
/*  f059850:	8fa5007c */ 	lw	$a1,0x7c($sp)
/*  f059854:	8fa90084 */ 	lw	$t1,0x84($sp)
/*  f059858:	10610007 */ 	beq	$v1,$at,.NB0f059878
/*  f05985c:	3c0a8007 */ 	lui	$t2,%hi(g_SpecialQuipBank)
/*  f059860:	2401000f */ 	addiu	$at,$zero,0xf
/*  f059864:	10610004 */ 	beq	$v1,$at,.NB0f059878
/*  f059868:	2401003a */ 	addiu	$at,$zero,0x3a
/*  f05986c:	10610002 */ 	beq	$v1,$at,.NB0f059878
/*  f059870:	24010028 */ 	addiu	$at,$zero,0x28
/*  f059874:	14610004 */ 	bne	$v1,$at,.NB0f059888
.NB0f059878:
/*  f059878:	254abc18 */ 	addiu	$t2,$t2,%lo(g_SpecialQuipBank)
/*  f05987c:	112a0002 */ 	beq	$t1,$t2,.NB0f059888
/*  f059880:	00000000 */ 	sll	$zero,$zero,0x0
/*  f059884:	afa00090 */ 	sw	$zero,0x90($sp)
.NB0f059888:
/*  f059888:	50a0001c */ 	beqzl	$a1,.NB0f0598fc
/*  f05988c:	90e20126 */ 	lbu	$v0,0x126($a3)
/*  f059890:	8ca4001c */ 	lw	$a0,0x1c($a1)
/*  f059894:	50800019 */ 	beqzl	$a0,.NB0f0598fc
/*  f059898:	90e20126 */ 	lbu	$v0,0x126($a3)
/*  f05989c:	908b0000 */ 	lbu	$t3,0x0($a0)
/*  f0598a0:	24010006 */ 	addiu	$at,$zero,0x6
/*  f0598a4:	55610015 */ 	bnel	$t3,$at,.NB0f0598fc
/*  f0598a8:	90e20126 */ 	lbu	$v0,0x126($a3)
/*  f0598ac:	0fc48d6f */ 	jal	playermgrGetPlayerNumByProp
/*  f0598b0:	00000000 */ 	sll	$zero,$zero,0x0
/*  f0598b4:	8e030298 */ 	lw	$v1,0x298($s0)
/*  f0598b8:	00026080 */ 	sll	$t4,$v0,0x2
/*  f0598bc:	00402025 */ 	or	$a0,$v0,$zero
/*  f0598c0:	0460000a */ 	bltz	$v1,.NB0f0598ec
/*  f0598c4:	020c6821 */ 	addu	$t5,$s0,$t4
/*  f0598c8:	8dae0064 */ 	lw	$t6,0x64($t5)
/*  f0598cc:	8dcf00d8 */ 	lw	$t7,0xd8($t6)
/*  f0598d0:	11e00006 */ 	beqz	$t7,.NB0f0598ec
/*  f0598d4:	00000000 */ 	sll	$zero,$zero,0x0
/*  f0598d8:	8e050294 */ 	lw	$a1,0x294($s0)
/*  f0598dc:	14450003 */ 	bne	$v0,$a1,.NB0f0598ec
/*  f0598e0:	00a02025 */ 	or	$a0,$a1,$zero
/*  f0598e4:	10000001 */ 	beqz	$zero,.NB0f0598ec
/*  f0598e8:	00602025 */ 	or	$a0,$v1,$zero
.NB0f0598ec:
/*  f0598ec:	0fc48d5b */ 	jal	setCurrentPlayerNum
/*  f0598f0:	00000000 */ 	sll	$zero,$zero,0x0
/*  f0598f4:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f0598f8:	90e20126 */ 	lbu	$v0,0x126($a3)
.NB0f0598fc:
/*  f0598fc:	10400007 */ 	beqz	$v0,.NB0f05991c
/*  f059900:	0002c100 */ 	sll	$t8,$v0,0x4
/*  f059904:	8cf90120 */ 	lw	$t9,0x120($a3)
/*  f059908:	0302c023 */ 	subu	$t8,$t8,$v0
/*  f05990c:	0018c080 */ 	sll	$t8,$t8,0x2
/*  f059910:	0319082a */ 	slt	$at,$t8,$t9
/*  f059914:	102001b4 */ 	beqz	$at,.NB0f059fe8
/*  f059918:	00000000 */ 	sll	$zero,$zero,0x0
.NB0f05991c:
/*  f05991c:	0c004d84 */ 	jal	random
/*  f059920:	00000000 */ 	sll	$zero,$zero,0x0
/*  f059924:	8faa0090 */ 	lw	$t2,0x90($sp)
/*  f059928:	304900ff */ 	andi	$t1,$v0,0xff
/*  f05992c:	8fab0088 */ 	lw	$t3,0x88($sp)
/*  f059930:	012a082a */ 	slt	$at,$t1,$t2
/*  f059934:	102001ac */ 	beqz	$at,.NB0f059fe8
/*  f059938:	00000000 */ 	sll	$zero,$zero,0x0
/*  f05993c:	85640000 */ 	lh	$a0,0x0($t3)
/*  f059940:	2401fffe */ 	addiu	$at,$zero,-2
/*  f059944:	5081003e */ 	beql	$a0,$at,.NB0f059a40
/*  f059948:	8fae0094 */ 	lw	$t6,0x94($sp)
.NB0f05994c:
/*  f05994c:	0fc0a048 */ 	jal	chrFindByLiteralId
/*  f059950:	00000000 */ 	sll	$zero,$zero,0x0
/*  f059954:	50400033 */ 	beqzl	$v0,.NB0f059a24
/*  f059958:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05995c:	8c4c0020 */ 	lw	$t4,0x20($v0)
/*  f059960:	00402025 */ 	or	$a0,$v0,$zero
/*  f059964:	5180002f */ 	beqzl	$t4,.NB0f059a24
/*  f059968:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05996c:	0fc0e4ec */ 	jal	chrIsDead
/*  f059970:	afa20064 */ 	sw	$v0,0x64($sp)
/*  f059974:	1440002a */ 	bnez	$v0,.NB0f059a20
/*  f059978:	8fa60064 */ 	lw	$a2,0x64($sp)
/*  f05997c:	80cd0007 */ 	lb	$t5,0x7($a2)
/*  f059980:	24010005 */ 	addiu	$at,$zero,0x5
/*  f059984:	51a10027 */ 	beql	$t5,$at,.NB0f059a24
/*  f059988:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f05998c:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f059990:	90ce02a2 */ 	lbu	$t6,0x2a2($a2)
/*  f059994:	90ef02a2 */ 	lbu	$t7,0x2a2($a3)
/*  f059998:	55cf0022 */ 	bnel	$t6,$t7,.NB0f059a24
/*  f05999c:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f0599a0:	90d80113 */ 	lbu	$t8,0x113($a2)
/*  f0599a4:	2b010064 */ 	slti	$at,$t8,0x64
/*  f0599a8:	5420001e */ 	bnezl	$at,.NB0f059a24
/*  f0599ac:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f0599b0:	84c50000 */ 	lh	$a1,0x0($a2)
/*  f0599b4:	84f90000 */ 	lh	$t9,0x0($a3)
/*  f0599b8:	00e02025 */ 	or	$a0,$a3,$zero
/*  f0599bc:	50b90019 */ 	beql	$a1,$t9,.NB0f059a24
/*  f0599c0:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f0599c4:	0fc12430 */ 	jal	chrGetDistanceToChr
/*  f0599c8:	afa60064 */ 	sw	$a2,0x64($sp)
/*  f0599cc:	3c017f1a */ 	lui	$at,%hi(var7f1a9d64)
/*  f0599d0:	c4244084 */ 	lwc1	$f4,%lo(var7f1a9d64)($at)
/*  f0599d4:	8fa90098 */ 	lw	$t1,0x98($sp)
/*  f0599d8:	8fa60064 */ 	lw	$a2,0x64($sp)
/*  f0599dc:	4604003c */ 	c.lt.s	$f0,$f4
/*  f0599e0:	252a0001 */ 	addiu	$t2,$t1,0x1
/*  f0599e4:	4502000f */ 	bc1fl	.NB0f059a24
/*  f0599e8:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f0599ec:	afaa0098 */ 	sw	$t2,0x98($sp)
/*  f0599f0:	8ccb0120 */ 	lw	$t3,0x120($a2)
/*  f0599f4:	8fac009c */ 	lw	$t4,0x9c($sp)
/*  f0599f8:	2961003c */ 	slti	$at,$t3,0x3c
/*  f0599fc:	50200009 */ 	beqzl	$at,.NB0f059a24
/*  f059a00:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f059a04:	91820006 */ 	lbu	$v0,0x6($t4)
/*  f059a08:	240100ff */ 	addiu	$at,$zero,0xff
/*  f059a0c:	50400005 */ 	beqzl	$v0,.NB0f059a24
/*  f059a10:	8fa20088 */ 	lw	$v0,0x88($sp)
/*  f059a14:	10410002 */ 	beq	$v0,$at,.NB0f059a20
/*  f059a18:	240d0001 */ 	addiu	$t5,$zero,0x1
/*  f059a1c:	afad0094 */ 	sw	$t5,0x94($sp)
.NB0f059a20:
/*  f059a20:	8fa20088 */ 	lw	$v0,0x88($sp)
.NB0f059a24:
/*  f059a24:	2401fffe */ 	addiu	$at,$zero,-2
/*  f059a28:	84440002 */ 	lh	$a0,0x2($v0)
/*  f059a2c:	24420002 */ 	addiu	$v0,$v0,0x2
/*  f059a30:	afa20088 */ 	sw	$v0,0x88($sp)
/*  f059a34:	1481ffc5 */ 	bne	$a0,$at,.NB0f05994c
/*  f059a38:	00000000 */ 	sll	$zero,$zero,0x0
/*  f059a3c:	8fae0094 */ 	lw	$t6,0x94($sp)
.NB0f059a40:
/*  f059a40:	8faf0098 */ 	lw	$t7,0x98($sp)
/*  f059a44:	15c00168 */ 	bnez	$t6,.NB0f059fe8
/*  f059a48:	00000000 */ 	sll	$zero,$zero,0x0
/*  f059a4c:	15e00007 */ 	bnez	$t7,.NB0f059a6c
/*  f059a50:	8fb8009c */ 	lw	$t8,0x9c($sp)
/*  f059a54:	93020006 */ 	lbu	$v0,0x6($t8)
/*  f059a58:	240100ff */ 	addiu	$at,$zero,0xff
/*  f059a5c:	1040000a */ 	beqz	$v0,.NB0f059a88
/*  f059a60:	00000000 */ 	sll	$zero,$zero,0x0
/*  f059a64:	10410008 */ 	beq	$v0,$at,.NB0f059a88
/*  f059a68:	00000000 */ 	sll	$zero,$zero,0x0
.NB0f059a6c:
/*  f059a6c:	8fb90098 */ 	lw	$t9,0x98($sp)
/*  f059a70:	8fa9009c */ 	lw	$t1,0x9c($sp)
/*  f059a74:	1b20015c */ 	blez	$t9,.NB0f059fe8
/*  f059a78:	00000000 */ 	sll	$zero,$zero,0x0
/*  f059a7c:	912a0006 */ 	lbu	$t2,0x6($t1)
/*  f059a80:	19400159 */ 	blez	$t2,.NB0f059fe8
/*  f059a84:	00000000 */ 	sll	$zero,$zero,0x0
.NB0f059a88:
/*  f059a88:	0c004d84 */ 	jal	random
/*  f059a8c:	00000000 */ 	sll	$zero,$zero,0x0
/*  f059a90:	24010003 */ 	addiu	$at,$zero,0x3
/*  f059a94:	0041001b */ 	divu	$zero,$v0,$at
/*  f059a98:	8fab009c */ 	lw	$t3,0x9c($sp)
/*  f059a9c:	00001810 */ 	mfhi	$v1
/*  f059aa0:	a3a300a7 */ 	sb	$v1,0xa7($sp)
/*  f059aa4:	916c0007 */ 	lbu	$t4,0x7($t3)
/*  f059aa8:	8fb80070 */ 	lw	$t8,0x70($sp)
/*  f059aac:	8fae0084 */ 	lw	$t6,0x84($sp)
/*  f059ab0:	318d0080 */ 	andi	$t5,$t4,0x80
/*  f059ab4:	15a00009 */ 	bnez	$t5,.NB0f059adc
/*  f059ab8:	0018c8c0 */ 	sll	$t9,$t8,0x3
/*  f059abc:	01d94821 */ 	addu	$t1,$t6,$t9
/*  f059ac0:	306a00ff */ 	andi	$t2,$v1,0xff
/*  f059ac4:	000a5840 */ 	sll	$t3,$t2,0x1
/*  f059ac8:	012b6021 */ 	addu	$t4,$t1,$t3
/*  f059acc:	afa90058 */ 	sw	$t1,0x58($sp)
/*  f059ad0:	858d0002 */ 	lh	$t5,0x2($t4)
/*  f059ad4:	1000000c */ 	beqz	$zero,.NB0f059b08
/*  f059ad8:	a7ad00a4 */ 	sh	$t5,0xa4($sp)
.NB0f059adc:
/*  f059adc:	8e0a0424 */ 	lw	$t2,0x424($s0)
/*  f059ae0:	8fae0070 */ 	lw	$t6,0x70($sp)
/*  f059ae4:	8faf0084 */ 	lw	$t7,0x84($sp)
/*  f059ae8:	914902b0 */ 	lbu	$t1,0x2b0($t2)
/*  f059aec:	000ec8c0 */ 	sll	$t9,$t6,0x3
/*  f059af0:	01f91021 */ 	addu	$v0,$t7,$t9
/*  f059af4:	00095840 */ 	sll	$t3,$t1,0x1
/*  f059af8:	004b6021 */ 	addu	$t4,$v0,$t3
/*  f059afc:	858d0002 */ 	lh	$t5,0x2($t4)
/*  f059b00:	afa20058 */ 	sw	$v0,0x58($sp)
/*  f059b04:	a7ad00a4 */ 	sh	$t5,0xa4($sp)
.NB0f059b08:
/*  f059b08:	0fc13066 */ 	jal	audioWasNotPlayedRecently
/*  f059b0c:	87a400a4 */ 	lh	$a0,0xa4($sp)
/*  f059b10:	1440000a */ 	bnez	$v0,.NB0f059b3c
/*  f059b14:	24010001 */ 	addiu	$at,$zero,0x1
/*  f059b18:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f059b1c:	00001825 */ 	or	$v1,$zero,$zero
/*  f059b20:	24080001 */ 	addiu	$t0,$zero,0x1
/*  f059b24:	10e00003 */ 	beqz	$a3,.NB0f059b34
/*  f059b28:	3c0e8007 */ 	lui	$t6,%hi(g_GuardQuipBank)
/*  f059b2c:	10000001 */ 	beqz	$zero,.NB0f059b34
/*  f059b30:	90e302fe */ 	lbu	$v1,0x2fe($a3)
.NB0f059b34:
/*  f059b34:	5461008a */ 	bnel	$v1,$at,.NB0f059d60
/*  f059b38:	8fad0070 */ 	lw	$t5,0x70($sp)
.NB0f059b3c:
/*  f059b3c:	0fc13054 */ 	jal	audioMarkAsRecentlyPlayed
/*  f059b40:	87a400a4 */ 	lh	$a0,0xa4($sp)
/*  f059b44:	87b800a4 */ 	lh	$t8,0xa4($sp)
/*  f059b48:	2401034e */ 	addiu	$at,$zero,0x34e
/*  f059b4c:	93ae006b */ 	lbu	$t6,0x6b($sp)
/*  f059b50:	57010005 */ 	bnel	$t8,$at,.NB0f059b68
/*  f059b54:	8e190424 */ 	lw	$t9,0x424($s0)
/*  f059b58:	15c00002 */ 	bnez	$t6,.NB0f059b64
/*  f059b5c:	240f034d */ 	addiu	$t7,$zero,0x34d
/*  f059b60:	a7af00a4 */ 	sh	$t7,0xa4($sp)
.NB0f059b64:
/*  f059b64:	8e190424 */ 	lw	$t9,0x424($s0)
.NB0f059b68:
/*  f059b68:	240103f7 */ 	addiu	$at,$zero,0x3f7
/*  f059b6c:	af200120 */ 	sw	$zero,0x120($t9)
/*  f059b70:	8faa009c */ 	lw	$t2,0x9c($sp)
/*  f059b74:	8e0b0424 */ 	lw	$t3,0x424($s0)
/*  f059b78:	91490005 */ 	lbu	$t1,0x5($t2)
/*  f059b7c:	a1690126 */ 	sb	$t1,0x126($t3)
/*  f059b80:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f059b84:	90ec0332 */ 	lbu	$t4,0x332($a3)
/*  f059b88:	258d0001 */ 	addiu	$t5,$t4,0x1
/*  f059b8c:	a0ed0332 */ 	sb	$t5,0x332($a3)
/*  f059b90:	87b800a4 */ 	lh	$t8,0xa4($sp)
/*  f059b94:	13010022 */ 	beq	$t8,$at,.NB0f059c20
/*  f059b98:	24010331 */ 	addiu	$at,$zero,0x331
/*  f059b9c:	13010020 */ 	beq	$t8,$at,.NB0f059c20
/*  f059ba0:	240103a1 */ 	addiu	$at,$zero,0x3a1
/*  f059ba4:	1301001e */ 	beq	$t8,$at,.NB0f059c20
/*  f059ba8:	24050009 */ 	addiu	$a1,$zero,0x9
/*  f059bac:	8e0e0424 */ 	lw	$t6,0x424($s0)
/*  f059bb0:	3406ffff */ 	dli	$a2,0xffff
/*  f059bb4:	0fc2433d */ 	jal	func0f0926bc
/*  f059bb8:	8dc4001c */ 	lw	$a0,0x1c($t6)
/*  f059bbc:	8e0f0424 */ 	lw	$t7,0x424($s0)
/*  f059bc0:	3c01bf80 */ 	lui	$at,0xbf80
/*  f059bc4:	44810000 */ 	mtc1	$at,$f0
/*  f059bc8:	8de5001c */ 	lw	$a1,0x1c($t7)
/*  f059bcc:	2419ffff */ 	addiu	$t9,$zero,-1
/*  f059bd0:	240a0008 */ 	addiu	$t2,$zero,0x8
/*  f059bd4:	24090009 */ 	addiu	$t1,$zero,0x9
/*  f059bd8:	240bffff */ 	addiu	$t3,$zero,-1
/*  f059bdc:	afab002c */ 	sw	$t3,0x2c($sp)
/*  f059be0:	afa9001c */ 	sw	$t1,0x1c($sp)
/*  f059be4:	afaa0014 */ 	sw	$t2,0x14($sp)
/*  f059be8:	afb90010 */ 	sw	$t9,0x10($sp)
/*  f059bec:	afa00028 */ 	sw	$zero,0x28($sp)
/*  f059bf0:	afa00020 */ 	sw	$zero,0x20($sp)
/*  f059bf4:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f059bf8:	00002025 */ 	or	$a0,$zero,$zero
/*  f059bfc:	87a600a4 */ 	lh	$a2,0xa4($sp)
/*  f059c00:	2407ffff */ 	addiu	$a3,$zero,-1
/*  f059c04:	e7a00038 */ 	swc1	$f0,0x38($sp)
/*  f059c08:	e7a00034 */ 	swc1	$f0,0x34($sp)
/*  f059c0c:	e7a00030 */ 	swc1	$f0,0x30($sp)
/*  f059c10:	0fc24762 */ 	jal	propsnd0f0939f8
/*  f059c14:	e7a00024 */ 	swc1	$f0,0x24($sp)
/*  f059c18:	10000026 */ 	beqz	$zero,.NB0f059cb4
/*  f059c1c:	8faa009c */ 	lw	$t2,0x9c($sp)
.NB0f059c20:
/*  f059c20:	0fc12df9 */ 	jal	chrGetDistanceLostToTargetInLastSecond
/*  f059c24:	8e040424 */ 	lw	$a0,0x424($s0)
/*  f059c28:	18400003 */ 	blez	$v0,.NB0f059c38
/*  f059c2c:	00021823 */ 	negu	$v1,$v0
/*  f059c30:	10000001 */ 	beqz	$zero,.NB0f059c38
/*  f059c34:	00401825 */ 	or	$v1,$v0,$zero
.NB0f059c38:
/*  f059c38:	28610033 */ 	slti	$at,$v1,0x33
/*  f059c3c:	1420001c */ 	bnez	$at,.NB0f059cb0
/*  f059c40:	24050009 */ 	addiu	$a1,$zero,0x9
/*  f059c44:	8e0c0424 */ 	lw	$t4,0x424($s0)
/*  f059c48:	3406ffff */ 	dli	$a2,0xffff
/*  f059c4c:	0fc2433d */ 	jal	func0f0926bc
/*  f059c50:	8d84001c */ 	lw	$a0,0x1c($t4)
/*  f059c54:	8e0d0424 */ 	lw	$t5,0x424($s0)
/*  f059c58:	3c01bf80 */ 	lui	$at,0xbf80
/*  f059c5c:	44810000 */ 	mtc1	$at,$f0
/*  f059c60:	8da5001c */ 	lw	$a1,0x1c($t5)
/*  f059c64:	2418ffff */ 	addiu	$t8,$zero,-1
/*  f059c68:	240e0008 */ 	addiu	$t6,$zero,0x8
/*  f059c6c:	240f0009 */ 	addiu	$t7,$zero,0x9
/*  f059c70:	2419ffff */ 	addiu	$t9,$zero,-1
/*  f059c74:	afb9002c */ 	sw	$t9,0x2c($sp)
/*  f059c78:	afaf001c */ 	sw	$t7,0x1c($sp)
/*  f059c7c:	afae0014 */ 	sw	$t6,0x14($sp)
/*  f059c80:	afb80010 */ 	sw	$t8,0x10($sp)
/*  f059c84:	afa00028 */ 	sw	$zero,0x28($sp)
/*  f059c88:	afa00020 */ 	sw	$zero,0x20($sp)
/*  f059c8c:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f059c90:	00002025 */ 	or	$a0,$zero,$zero
/*  f059c94:	87a600a4 */ 	lh	$a2,0xa4($sp)
/*  f059c98:	2407ffff */ 	addiu	$a3,$zero,-1
/*  f059c9c:	e7a00038 */ 	swc1	$f0,0x38($sp)
/*  f059ca0:	e7a00034 */ 	swc1	$f0,0x34($sp)
/*  f059ca4:	e7a00030 */ 	swc1	$f0,0x30($sp)
/*  f059ca8:	0fc24762 */ 	jal	propsnd0f0939f8
/*  f059cac:	e7a00024 */ 	swc1	$f0,0x24($sp)
.NB0f059cb0:
/*  f059cb0:	8faa009c */ 	lw	$t2,0x9c($sp)
.NB0f059cb4:
/*  f059cb4:	91420008 */ 	lbu	$v0,0x8($t2)
/*  f059cb8:	10400016 */ 	beqz	$v0,.NB0f059d14
/*  f059cbc:	00000000 */ 	sll	$zero,$zero,0x0
/*  f059cc0:	91490007 */ 	lbu	$t1,0x7($t2)
/*  f059cc4:	93a300a7 */ 	lbu	$v1,0xa7($sp)
/*  f059cc8:	000260c0 */ 	sll	$t4,$v0,0x3
/*  f059ccc:	312b0080 */ 	andi	$t3,$t1,0x80
/*  f059cd0:	15600010 */ 	bnez	$t3,.NB0f059d14
/*  f059cd4:	28610003 */ 	slti	$at,$v1,0x3
/*  f059cd8:	14200002 */ 	bnez	$at,.NB0f059ce4
/*  f059cdc:	3c048007 */ 	lui	$a0,%hi(g_SpecialQuipBank+0x16a)
/*  f059ce0:	24030002 */ 	addiu	$v1,$zero,0x2
.NB0f059ce4:
/*  f059ce4:	00036840 */ 	sll	$t5,$v1,0x1
/*  f059ce8:	018dc021 */ 	addu	$t8,$t4,$t5
/*  f059cec:	00982021 */ 	addu	$a0,$a0,$t8
/*  f059cf0:	0fc5a4dd */ 	jal	langGet
/*  f059cf4:	8484bd82 */ 	lh	$a0,%lo(g_SpecialQuipBank+0x16a)($a0)
/*  f059cf8:	8fae009c */ 	lw	$t6,0x9c($sp)
/*  f059cfc:	00402025 */ 	or	$a0,$v0,$zero
/*  f059d00:	24050006 */ 	addiu	$a1,$zero,0x6
/*  f059d04:	0fc36d93 */ 	jal	hudmsgCreateWithColour
/*  f059d08:	91c60009 */ 	lbu	$a2,0x9($t6)
/*  f059d0c:	100000b6 */ 	beqz	$zero,.NB0f059fe8
/*  f059d10:	00000000 */ 	sll	$zero,$zero,0x0
.NB0f059d14:
/*  f059d14:	104000b4 */ 	beqz	$v0,.NB0f059fe8
/*  f059d18:	00000000 */ 	sll	$zero,$zero,0x0
/*  f059d1c:	8e190424 */ 	lw	$t9,0x424($s0)
/*  f059d20:	000278c0 */ 	sll	$t7,$v0,0x3
/*  f059d24:	3c048007 */ 	lui	$a0,%hi(g_SpecialQuipBank+0x16a)
/*  f059d28:	932a02b0 */ 	lbu	$t2,0x2b0($t9)
/*  f059d2c:	000a4840 */ 	sll	$t1,$t2,0x1
/*  f059d30:	01e95821 */ 	addu	$t3,$t7,$t1
/*  f059d34:	008b2021 */ 	addu	$a0,$a0,$t3
/*  f059d38:	0fc5a4dd */ 	jal	langGet
/*  f059d3c:	8484bd82 */ 	lh	$a0,%lo(g_SpecialQuipBank+0x16a)($a0)
/*  f059d40:	8fac009c */ 	lw	$t4,0x9c($sp)
/*  f059d44:	00402025 */ 	or	$a0,$v0,$zero
/*  f059d48:	24050006 */ 	addiu	$a1,$zero,0x6
/*  f059d4c:	0fc36d93 */ 	jal	hudmsgCreateWithColour
/*  f059d50:	91860009 */ 	lbu	$a2,0x9($t4)
/*  f059d54:	100000a4 */ 	beqz	$zero,.NB0f059fe8
/*  f059d58:	00000000 */ 	sll	$zero,$zero,0x0
/*  f059d5c:	8fad0070 */ 	lw	$t5,0x70($sp)
.NB0f059d60:
/*  f059d60:	25ceb6f8 */ 	addiu	$t6,$t6,%lo(g_GuardQuipBank)
/*  f059d64:	a7a000a4 */ 	sh	$zero,0xa4($sp)
/*  f059d68:	000dc0c0 */ 	sll	$t8,$t5,0x3
/*  f059d6c:	030e1021 */ 	addu	$v0,$t8,$t6
/*  f059d70:	afa2005c */ 	sw	$v0,0x5c($sp)
.NB0f059d74:
/*  f059d74:	8fa2005c */ 	lw	$v0,0x5c($sp)
/*  f059d78:	00081840 */ 	sll	$v1,$t0,0x1
/*  f059d7c:	0043c821 */ 	addu	$t9,$v0,$v1
/*  f059d80:	87240000 */ 	lh	$a0,0x0($t9)
/*  f059d84:	a3a800a3 */ 	sb	$t0,0xa3($sp)
/*  f059d88:	0fc13066 */ 	jal	audioWasNotPlayedRecently
/*  f059d8c:	afa30054 */ 	sw	$v1,0x54($sp)
/*  f059d90:	8fa30054 */ 	lw	$v1,0x54($sp)
/*  f059d94:	1040000d */ 	beqz	$v0,.NB0f059dcc
/*  f059d98:	93a800a3 */ 	lbu	$t0,0xa3($sp)
/*  f059d9c:	8faa0058 */ 	lw	$t2,0x58($sp)
/*  f059da0:	01432821 */ 	addu	$a1,$t2,$v1
/*  f059da4:	84a40000 */ 	lh	$a0,0x0($a1)
/*  f059da8:	a3a800a3 */ 	sb	$t0,0xa3($sp)
/*  f059dac:	0fc13066 */ 	jal	audioWasNotPlayedRecently
/*  f059db0:	afa50050 */ 	sw	$a1,0x50($sp)
/*  f059db4:	8fa50050 */ 	lw	$a1,0x50($sp)
/*  f059db8:	10400004 */ 	beqz	$v0,.NB0f059dcc
/*  f059dbc:	93a800a3 */ 	lbu	$t0,0xa3($sp)
/*  f059dc0:	84af0000 */ 	lh	$t7,0x0($a1)
/*  f059dc4:	10000006 */ 	beqz	$zero,.NB0f059de0
/*  f059dc8:	a7af00a4 */ 	sh	$t7,0xa4($sp)
.NB0f059dcc:
/*  f059dcc:	25080001 */ 	addiu	$t0,$t0,0x1
/*  f059dd0:	310900ff */ 	andi	$t1,$t0,0xff
/*  f059dd4:	29210004 */ 	slti	$at,$t1,0x4
/*  f059dd8:	1420ffe6 */ 	bnez	$at,.NB0f059d74
/*  f059ddc:	01204025 */ 	or	$t0,$t1,$zero
.NB0f059de0:
/*  f059de0:	87ab00a4 */ 	lh	$t3,0xa4($sp)
/*  f059de4:	11600076 */ 	beqz	$t3,.NB0f059fc0
/*  f059de8:	01602025 */ 	or	$a0,$t3,$zero
/*  f059dec:	0fc13054 */ 	jal	audioMarkAsRecentlyPlayed
/*  f059df0:	a3a800a3 */ 	sb	$t0,0xa3($sp)
/*  f059df4:	87ac00a4 */ 	lh	$t4,0xa4($sp)
/*  f059df8:	2401034e */ 	addiu	$at,$zero,0x34e
/*  f059dfc:	93a800a3 */ 	lbu	$t0,0xa3($sp)
/*  f059e00:	15810004 */ 	bne	$t4,$at,.NB0f059e14
/*  f059e04:	93ad006b */ 	lbu	$t5,0x6b($sp)
/*  f059e08:	15a00002 */ 	bnez	$t5,.NB0f059e14
/*  f059e0c:	2418034d */ 	addiu	$t8,$zero,0x34d
/*  f059e10:	a7b800a4 */ 	sh	$t8,0xa4($sp)
.NB0f059e14:
/*  f059e14:	8e0e0424 */ 	lw	$t6,0x424($s0)
/*  f059e18:	240103f7 */ 	addiu	$at,$zero,0x3f7
/*  f059e1c:	adc00120 */ 	sw	$zero,0x120($t6)
/*  f059e20:	8fb9009c */ 	lw	$t9,0x9c($sp)
/*  f059e24:	8e0f0424 */ 	lw	$t7,0x424($s0)
/*  f059e28:	932a0005 */ 	lbu	$t2,0x5($t9)
/*  f059e2c:	a1ea0126 */ 	sb	$t2,0x126($t7)
/*  f059e30:	8e070424 */ 	lw	$a3,0x424($s0)
/*  f059e34:	90e90332 */ 	lbu	$t1,0x332($a3)
/*  f059e38:	252b0001 */ 	addiu	$t3,$t1,0x1
/*  f059e3c:	a0eb0332 */ 	sb	$t3,0x332($a3)
/*  f059e40:	87ac00a4 */ 	lh	$t4,0xa4($sp)
/*  f059e44:	11810024 */ 	beq	$t4,$at,.NB0f059ed8
/*  f059e48:	24010331 */ 	addiu	$at,$zero,0x331
/*  f059e4c:	11810022 */ 	beq	$t4,$at,.NB0f059ed8
/*  f059e50:	240103a1 */ 	addiu	$at,$zero,0x3a1
/*  f059e54:	51810021 */ 	beql	$t4,$at,.NB0f059edc
/*  f059e58:	8e040424 */ 	lw	$a0,0x424($s0)
/*  f059e5c:	8e0d0424 */ 	lw	$t5,0x424($s0)
/*  f059e60:	24050009 */ 	addiu	$a1,$zero,0x9
/*  f059e64:	3406ffff */ 	dli	$a2,0xffff
/*  f059e68:	8da4001c */ 	lw	$a0,0x1c($t5)
/*  f059e6c:	0fc2433d */ 	jal	func0f0926bc
/*  f059e70:	a3a800a3 */ 	sb	$t0,0xa3($sp)
/*  f059e74:	8e180424 */ 	lw	$t8,0x424($s0)
/*  f059e78:	3c01bf80 */ 	lui	$at,0xbf80
/*  f059e7c:	44810000 */ 	mtc1	$at,$f0
/*  f059e80:	8f05001c */ 	lw	$a1,0x1c($t8)
/*  f059e84:	240effff */ 	addiu	$t6,$zero,-1
/*  f059e88:	24190008 */ 	addiu	$t9,$zero,0x8
/*  f059e8c:	240a0009 */ 	addiu	$t2,$zero,0x9
/*  f059e90:	240fffff */ 	addiu	$t7,$zero,-1
/*  f059e94:	afaf002c */ 	sw	$t7,0x2c($sp)
/*  f059e98:	afaa001c */ 	sw	$t2,0x1c($sp)
/*  f059e9c:	afb90014 */ 	sw	$t9,0x14($sp)
/*  f059ea0:	afae0010 */ 	sw	$t6,0x10($sp)
/*  f059ea4:	afa00028 */ 	sw	$zero,0x28($sp)
/*  f059ea8:	afa00020 */ 	sw	$zero,0x20($sp)
/*  f059eac:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f059eb0:	00002025 */ 	or	$a0,$zero,$zero
/*  f059eb4:	87a600a4 */ 	lh	$a2,0xa4($sp)
/*  f059eb8:	2407ffff */ 	addiu	$a3,$zero,-1
/*  f059ebc:	e7a00038 */ 	swc1	$f0,0x38($sp)
/*  f059ec0:	e7a00034 */ 	swc1	$f0,0x34($sp)
/*  f059ec4:	e7a00030 */ 	swc1	$f0,0x30($sp)
/*  f059ec8:	0fc24762 */ 	jal	propsnd0f0939f8
/*  f059ecc:	e7a00024 */ 	swc1	$f0,0x24($sp)
/*  f059ed0:	1000002a */ 	beqz	$zero,.NB0f059f7c
/*  f059ed4:	93a800a3 */ 	lbu	$t0,0xa3($sp)
.NB0f059ed8:
/*  f059ed8:	8e040424 */ 	lw	$a0,0x424($s0)
.NB0f059edc:
/*  f059edc:	0fc12df9 */ 	jal	chrGetDistanceLostToTargetInLastSecond
/*  f059ee0:	a3a800a3 */ 	sb	$t0,0xa3($sp)
/*  f059ee4:	18400003 */ 	blez	$v0,.NB0f059ef4
/*  f059ee8:	93a800a3 */ 	lbu	$t0,0xa3($sp)
/*  f059eec:	10000002 */ 	beqz	$zero,.NB0f059ef8
/*  f059ef0:	00401825 */ 	or	$v1,$v0,$zero
.NB0f059ef4:
/*  f059ef4:	00021823 */ 	negu	$v1,$v0
.NB0f059ef8:
/*  f059ef8:	28610033 */ 	slti	$at,$v1,0x33
/*  f059efc:	54200020 */ 	bnezl	$at,.NB0f059f80
/*  f059f00:	8fb9009c */ 	lw	$t9,0x9c($sp)
/*  f059f04:	8e090424 */ 	lw	$t1,0x424($s0)
/*  f059f08:	24050009 */ 	addiu	$a1,$zero,0x9
/*  f059f0c:	3406ffff */ 	dli	$a2,0xffff
/*  f059f10:	8d24001c */ 	lw	$a0,0x1c($t1)
/*  f059f14:	0fc2433d */ 	jal	func0f0926bc
/*  f059f18:	a3a800a3 */ 	sb	$t0,0xa3($sp)
/*  f059f1c:	8e0b0424 */ 	lw	$t3,0x424($s0)
/*  f059f20:	3c01bf80 */ 	lui	$at,0xbf80
/*  f059f24:	44810000 */ 	mtc1	$at,$f0
/*  f059f28:	8d65001c */ 	lw	$a1,0x1c($t3)
/*  f059f2c:	240cffff */ 	addiu	$t4,$zero,-1
/*  f059f30:	240d0008 */ 	addiu	$t5,$zero,0x8
/*  f059f34:	24180009 */ 	addiu	$t8,$zero,0x9
/*  f059f38:	240effff */ 	addiu	$t6,$zero,-1
/*  f059f3c:	afae002c */ 	sw	$t6,0x2c($sp)
/*  f059f40:	afb8001c */ 	sw	$t8,0x1c($sp)
/*  f059f44:	afad0014 */ 	sw	$t5,0x14($sp)
/*  f059f48:	afac0010 */ 	sw	$t4,0x10($sp)
/*  f059f4c:	afa00028 */ 	sw	$zero,0x28($sp)
/*  f059f50:	afa00020 */ 	sw	$zero,0x20($sp)
/*  f059f54:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f059f58:	00002025 */ 	or	$a0,$zero,$zero
/*  f059f5c:	87a600a4 */ 	lh	$a2,0xa4($sp)
/*  f059f60:	2407ffff */ 	addiu	$a3,$zero,-1
/*  f059f64:	e7a00038 */ 	swc1	$f0,0x38($sp)
/*  f059f68:	e7a00034 */ 	swc1	$f0,0x34($sp)
/*  f059f6c:	e7a00030 */ 	swc1	$f0,0x30($sp)
/*  f059f70:	0fc24762 */ 	jal	propsnd0f0939f8
/*  f059f74:	e7a00024 */ 	swc1	$f0,0x24($sp)
/*  f059f78:	93a800a3 */ 	lbu	$t0,0xa3($sp)
.NB0f059f7c:
/*  f059f7c:	8fb9009c */ 	lw	$t9,0x9c($sp)
.NB0f059f80:
/*  f059f80:	00087840 */ 	sll	$t7,$t0,0x1
/*  f059f84:	3c048007 */ 	lui	$a0,%hi(g_SpecialQuipBank+0x168)
/*  f059f88:	93220008 */ 	lbu	$v0,0x8($t9)
/*  f059f8c:	10400016 */ 	beqz	$v0,.NB0f059fe8
/*  f059f90:	000250c0 */ 	sll	$t2,$v0,0x3
/*  f059f94:	014f4821 */ 	addu	$t1,$t2,$t7
/*  f059f98:	00892021 */ 	addu	$a0,$a0,$t1
/*  f059f9c:	0fc5a4dd */ 	jal	langGet
/*  f059fa0:	8484bd80 */ 	lh	$a0,%lo(g_SpecialQuipBank+0x168)($a0)
/*  f059fa4:	8fab009c */ 	lw	$t3,0x9c($sp)
/*  f059fa8:	00402025 */ 	or	$a0,$v0,$zero
/*  f059fac:	24050006 */ 	addiu	$a1,$zero,0x6
/*  f059fb0:	0fc36d93 */ 	jal	hudmsgCreateWithColour
/*  f059fb4:	91660009 */ 	lbu	$a2,0x9($t3)
/*  f059fb8:	1000000b */ 	beqz	$zero,.NB0f059fe8
/*  f059fbc:	00000000 */ 	sll	$zero,$zero,0x0
.NB0f059fc0:
/*  f059fc0:	8e0c0424 */ 	lw	$t4,0x424($s0)
/*  f059fc4:	3c050800 */ 	lui	$a1,0x800
/*  f059fc8:	24060001 */ 	addiu	$a2,$zero,0x1
/*  f059fcc:	ad800120 */ 	sw	$zero,0x120($t4)
/*  f059fd0:	8fad009c */ 	lw	$t5,0x9c($sp)
/*  f059fd4:	8e0e0424 */ 	lw	$t6,0x424($s0)
/*  f059fd8:	91b80005 */ 	lbu	$t8,0x5($t5)
/*  f059fdc:	a1d80126 */ 	sb	$t8,0x126($t6)
/*  f059fe0:	0fc1247e */ 	jal	chrUnsetFlags
/*  f059fe4:	8e040424 */ 	lw	$a0,0x424($s0)
.NB0f059fe8:
/*  f059fe8:	0fc48d5b */ 	jal	setCurrentPlayerNum
/*  f059fec:	8fa40078 */ 	lw	$a0,0x78($sp)
/*  f059ff0:	8e190438 */ 	lw	$t9,0x438($s0)
/*  f059ff4:	00001025 */ 	or	$v0,$zero,$zero
/*  f059ff8:	272a000a */ 	addiu	$t2,$t9,0xa
/*  f059ffc:	ae0a0438 */ 	sw	$t2,0x438($s0)
.NB0f05a000:
/*  f05a000:	8fbf004c */ 	lw	$ra,0x4c($sp)
/*  f05a004:	8fb00048 */ 	lw	$s0,0x48($sp)
/*  f05a008:	27bd00a8 */ 	addiu	$sp,$sp,0xa8
/*  f05a00c:	03e00008 */ 	jr	$ra
/*  f05a010:	00000000 */ 	sll	$zero,$zero,0x0
);
#endif
#else
// regalloc difference near 64c
bool aiSayQuip(void)
{
	u8 column; // 167
	s16 audioid; // 164
	u8 i; // 163
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset; // 156
	s32 numnearbychrs; // 152
	bool issomeonetalking; // 148
	s32 probability; // 144
	u32 stack; // 140 - not referenced
	s16 *chrnums; // 136
	s16 (*bank)[4]; // 132
	char *text; // 128
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]); // 124
	u32 prevplayernum = g_Vars.currentplayernum; // 120
	s32 distance; // 116 - not referenced
	s32 row = cmd[3]; // 112
	u32 playernum; // 108 - not referenced
#ifdef __sgi
	u8 headshotted = (g_Vars.chrdata->hidden2 & CHRH2FLAG_HEADSHOTTED); // 107
#else
	u8 headshotted = (g_Vars.chrdata->hidden2 & CHRH2FLAG_HEADSHOTTED) & 0xff; // 107
#endif
	struct chrdata *loopchr; // 100

	// Choose bank
	// 2c0
	if (CHRRACE(g_Vars.chrdata) == RACE_SKEDAR) {
		bank = g_SkedarQuipBank;

		if (row > 5) {
			row = 0;
		}
		// 2e0
	} else if (g_Vars.chrdata->headnum == HEAD_MAIAN_S) {
		bank = g_MaianQuipBank;

		if (row > 2) {
			row = random() & 1;
		}
		// 324
	} else if (cmd[7] == 0) {
		if (g_Vars.chrdata->voicebox > 3) {
			g_Vars.chrdata->voicebox = 3;
		}

		bank = &g_GuardQuipBank[g_Vars.chrdata->voicebox * 41];
	} else {
		// 37c
		bank = g_SpecialQuipBank;
	}

	// 37c
	if (!row && !cmd[4] && !cmd[6]) {
		g_Vars.chrdata->soundtimer = 0;
		g_Vars.aioffset += 10;
		return false;
	}

	// 3bc
	chrnums = teamGetChrIds(g_Vars.chrdata->team);
	numnearbychrs = 0;
	issomeonetalking = false;
	probability = cmd[4];

	// Make it impossible for Elvis and Jon to use anything but special phrases
	// 3f0
	if ((g_Vars.chrdata->headnum == HEAD_ELVIS
				|| g_Vars.chrdata->headnum == HEAD_THEKING
				|| g_Vars.chrdata->headnum == HEAD_ELVIS_GOGS
				|| g_Vars.chrdata->headnum == HEAD_JONATHAN) &&
			bank != g_SpecialQuipBank) {
		probability = 0;
	}

	// If the person talking is a player, and they've just died,
	// try using the other coop player.
	// 420
	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		// 444
		playernum = playermgrGetPlayerNumByProp(chr->prop);

		if (g_Vars.coopplayernum >= 0 && g_Vars.players[playernum]->isdead) {
			// 470
			if (playernum == g_Vars.bondplayernum) {
				playernum = g_Vars.coopplayernum;
			} else {
				playernum = g_Vars.bondplayernum;
			}
		}

		setCurrentPlayerNum(playernum);
	}

	// If soundgap permits talking at this time and probability passes
	// 494
	if ((g_Vars.chrdata->soundgap == 0 || g_Vars.chrdata->soundgap * TICKS(60) < g_Vars.chrdata->soundtimer)
			&& probability > (u8)random()) {
		// Try and find a chr in the same squadron who is currently talking
		// 4dc
		while (*chrnums != -2) {
			loopchr = chrFindByLiteralId(*chrnums);

			if (loopchr && loopchr->model
					&& !chrIsDead(loopchr)
					&& loopchr->actiontype != ACT_DEAD
					&& g_Vars.chrdata->squadron == loopchr->squadron
					&& loopchr->alertness >= 100
					&& g_Vars.chrdata->chrnum != loopchr->chrnum
					&& chrGetDistanceToChr(g_Vars.chrdata, loopchr->chrnum) < 7000) {
				// 584
				numnearbychrs++;

				// 594
				if (loopchr->soundtimer < TICKS(60) && cmd[6] != 0 && cmd[6] != 255) {
					issomeonetalking = true;
				}
			}

			chrnums++;
		}

		// 5dc
		if (!issomeonetalking &&
				((numnearbychrs == 0 && (cmd[6] == 0 || cmd[6] == 255)) ||
				 (numnearbychrs > 0 && cmd[6] > 0))) {
			column = random() % 3;

			// 64c
			if ((cmd[7] & 0x80) == 0) {
				audioid = bank[row][1 + column];
			} else {
				audioid = bank[row][1 + g_Vars.chrdata->tude];
			}

			// 6a0
			if (audioWasNotPlayedRecently(audioid) || CHRRACE(g_Vars.chrdata) == RACE_SKEDAR) {
				// 6d4
				audioMarkAsRecentlyPlayed(audioid);

				// 6e8
				// Replace gurgle with "why me"
				if (audioid == 0x34e && !headshotted) {
					audioid = 0x34d;
				}

				// 700
				g_Vars.chrdata->soundtimer = 0;
				g_Vars.chrdata->soundgap = cmd[5];
				g_Vars.chrdata->propsoundcount++;

				// 72c
				if (audioid != 0x3f7 && audioid != 0x331 && audioid != 0x3a1) {
					func0f0926bc(g_Vars.chrdata->prop, 9, 0xffff);
					// 7a8
					propsnd0f0939f8(0, g_Vars.chrdata->prop, audioid, -1,
							-1, 8, 0, 9, 0, -1, 0, -1, -1, -1, -1);
				} else {
					// Audio is "Stop moving", "Stop dodging" or "Stand still"
					distance = chrGetDistanceLostToTargetInLastSecond(g_Vars.chrdata);

					if (ABS(distance) > 50) {
						func0f0926bc(g_Vars.chrdata->prop, 9, 0xffff);
						// 840
						propsnd0f0939f8(0, g_Vars.chrdata->prop, audioid, -1,
								-1, 8, 0, 9, 0, -1, 0, -1, -1, -1, -1);
					}
				}

				// Consider putting text on screen
				// Note: if cmd[8] is 0 then it means no text, so the value
				// needs to be be decremented by one so it's 0-indexed.
				// 850
				if (cmd[8] && (cmd[7] & 0x80) == 0) {
					if (column > 2) {
						column = 2;
					}

					text = langGet(g_QuipTexts[cmd[8] - 1][1 + column]);

#if VERSION >= VERSION_NTSC_1_0
					if (!sndIsFiltered(audioid))
#endif
					{
						// 8ac
						hudmsgCreateWithColour(text, HUDMSGTYPE_INGAMESUBTITLE, cmd[9]);
					}
				} else if (cmd[8]) {
					text = langGet(g_QuipTexts[cmd[8] - 1][1 + g_Vars.chrdata->tude]);

#if VERSION >= VERSION_NTSC_1_0
					if (!sndIsFiltered(audioid))
#endif
					{
						// 904
						hudmsgCreateWithColour(text, HUDMSGTYPE_INGAMESUBTITLE, cmd[9]);
					}
				}
			} else {
				// Audio was played recently - try and find a different one
				audioid = 0;

				// 92c
				for (i = 1; i < 4; i++) {
					if (audioWasNotPlayedRecently(g_GuardQuipBank[row][i])
							&& audioWasNotPlayedRecently(bank[row][i])) {
						audioid = bank[row][i];
						break;
					}
				}

				// 99c
				if (audioid) {
					audioMarkAsRecentlyPlayed(audioid);

					// Replace gurgle with "why me"
					if (audioid == 0x34e && !headshotted) {
						audioid = 0x34d;
					}

					g_Vars.chrdata->soundtimer = 0;
					g_Vars.chrdata->soundgap = cmd[5];
					g_Vars.chrdata->propsoundcount++;

					// 9fc
					if (audioid != 0x3f7 && audioid != 0x331 && audioid != 0x3a1) {
						func0f0926bc(g_Vars.chrdata->prop, 9, 0xffff);
						// a80
						propsnd0f0939f8(0, g_Vars.chrdata->prop, audioid, -1,
								-1, 8, 0, 9, 0, -1, 0, -1, -1, -1, -1);
					} else {
						// Audio is "Stop moving", "Stop dodging" or "Stand still"
						// a90
						distance = chrGetDistanceLostToTargetInLastSecond(g_Vars.chrdata);

						if (ABS(distance) > 50) {
							func0f0926bc(g_Vars.chrdata->prop, 9, 0xffff);
							// b28
							propsnd0f0939f8(0, g_Vars.chrdata->prop, audioid, -1,
									-1, 8, 0, 9, 0, -1, 0, -1, -1, -1, -1);
						}
					}

					// b44
					if (cmd[8]) {
						text = langGet(g_QuipTexts[cmd[8] - 1][i]);

#if VERSION >= VERSION_NTSC_1_0
						if (!sndIsFiltered(audioid))
#endif
						{
							// b78
							hudmsgCreateWithColour(text, HUDMSGTYPE_INGAMESUBTITLE, cmd[9]);
						}
					}
				} else {
					g_Vars.chrdata->soundtimer = 0;
					g_Vars.chrdata->soundgap = cmd[5];
					chrUnsetFlags(g_Vars.chrdata, CHRFLAG1_TALKINGTODISGUISE, BANK_1);
				}
			}
		}
	}

	setCurrentPlayerNum(prevplayernum);

	g_Vars.aioffset += 10;

	return false;
}
#endif

void propDecrementSoundCount(struct prop *prop)
{
	if (prop && prop->chr && prop->chr->model && prop->chr->propsoundcount > 0) {
		prop->chr->propsoundcount--;
	}
}

/**
 * @cmd 01a7
 */
bool aiIfChrNotTalking(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindByLiteralId(cmd[2]);

	if (chr && chr->propsoundcount == 0) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset = g_Vars.aioffset + 4;
	}

	return false;
}

/**
 * @cmd 0131
 */
bool aiIncreaseSquadronAlertness(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s16 *chrnums = teamGetChrIds(g_Vars.chrdata->team);

	for (; *chrnums != -2; chrnums++) {
		struct chrdata *chr = chrFindByLiteralId(*chrnums);

		if (chr &&
				chr->model &&
				!chrIsDead(chr) &&
				chr->actiontype != ACT_DEAD &&
				(g_Vars.chrdata->squadron == chr->squadron || g_Vars.chrdata->squadron == 255) &&
				g_Vars.chrdata->chrnum != chr->chrnum &&
				(chrGetDistanceToChr(g_Vars.chrdata, chr->chrnum) < 1000 || chrHasFlag(g_Vars.chrdata, CHRFLAG0_SQUADALERTANYDIST, BANK_0))) {
			incrementByte(&chr->alertness, cmd[2]);
		}
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0132
 */
bool aiSetAction(void)
{
	struct bytelist *cmd = (struct bytelist *)(g_Vars.ailist + g_Vars.aioffset);
	g_Vars.chrdata->myaction = cmd->b2;

	if (cmd->b3 == 0) {
		g_Vars.chrdata->orders = 0;
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 0133
 */
bool aiSetTeamOrders(void)
{
	struct chrnumaction *chraction;
	s32 chrcount = 1;
	s16 *chrnums;
	struct chrnumaction chractions[50];
	s32 num;
	u32 stack;
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	// Get list of chrs in the current chr's squadron
	chrnums = squadronGetChrIds(g_Vars.chrdata->squadron);

	// Iterate chrs in squadron and build list of their actions.
	// Put the current chr's action first.
	chraction = chractions;
	chraction->chrnum = g_Vars.chrdata->chrnum;
	chraction->myaction = g_Vars.chrdata->myaction;
	chraction++;

	if (chrnums) {
		while (*chrnums != -2) {
			struct chrdata *chr = chrFindByLiteralId(*chrnums);

			if (chr && chr->model
					&& !chrIsDead(chr)
					&& chr->actiontype != ACT_DEAD
					&& chrCompareTeams(g_Vars.chrdata, chr, COMPARE_FRIENDS)
					&& g_Vars.chrdata->chrnum != chr->chrnum) {
				if (chr->myaction == MA_COVERWAIT
						|| chr->myaction == MA_NORMAL
						|| chr->myaction == MA_WAITING
						|| chr->myaction == MA_SHOOTING) {
					if (chrGetDistanceToChr(g_Vars.chrdata, chr->chrnum) < 3500) {
						chrcount++;
						chraction->chrnum = chr->chrnum;
						chraction->myaction = chr->myaction;
						chraction++;
					}
				}
			}

			chrnums++;
		}
	}

	chraction->myaction = MA_END;

	// Iterate the list of chrs and decide how to reassign orders to them
	// based on the current chr's action
	if (chrcount != 1) {
		chraction = &chractions[1];
		num = 1;

		while (chraction->myaction != MA_END) {
			struct chrdata *chr = chrFindByLiteralId(chraction->chrnum);

			switch (chractions[0].myaction) {
			case MA_COVERGOTO:
				if (!chrIsInTargetsFovX(chr, 45)) {
					chr->orders = MA_SHOOTING;
				}
				break;
			case MA_COVERBREAK:
				if (!chrIsInTargetsFovX(chr, 30)) {
					chr->orders = MA_SHOOTING;
				}
				num++;
				break;
			case MA_COVERSEEN:
				if (!chrIsInTargetsFovX(chr, 30)) {
					chr->orders = MA_SHOOTING;
					g_Vars.chrdata->orders = MA_COVERGOTO;
				}
				num++;
				break;
			case MA_FLANKLEFT:
				if (chrIsInTargetsFovX(chr, 50)) {
					chr->orders = MA_FLANKRIGHT;
				} else {
					chr->orders = MA_SHOOTING;
				}
				num++;
				g_Vars.chrdata->orders = MA_FLANKLEFT;
				break;
			case MA_FLANKRIGHT:
				if (chrIsInTargetsFovX(chr, 50)) {
					chr->orders = MA_FLANKLEFT;
				} else {
					chr->orders = MA_SHOOTING;
				}
				num++;
				g_Vars.chrdata->orders = MA_FLANKRIGHT;
				break;
			case MA_DODGE:
				if (!chrIsInTargetsFovX(chr, 30) &&
						chrHasFlagById(chr, CHR_SELF, CHRFLAG0_CAN_BACKOFF, BANK_0)) {
					chr->orders = MA_WITHDRAW;
				} else {
					chr->orders = MA_SHOOTING;
				}
				num++;
				break;
			case MA_GRENADE:
				if (num < 2) {
					chr->orders = MA_WAITING;
				} else if (chrHasFlagById(chr, CHR_SELF, CHRFLAG0_CAN_BACKOFF, BANK_0)) {
					chr->orders = MA_WITHDRAW;
				}
				num++;
				break;
			case MA_WAITSEEN:
				if (chrIsInTargetsFovX(chr, 30) &&
						chrHasFlagById(chr, CHR_SELF, CHRFLAG0_CAN_BACKOFF, BANK_0)) {
					chr->orders = MA_WITHDRAW;
				} else {
					chr->orders = MA_SHOOTING;
				}
				num++;
				break;
			case MA_WITHDRAW:
				if (chrHasFlagById(chr, CHR_SELF, CHRFLAG0_CAN_BACKOFF, BANK_0)) {
					chr->orders = MA_WITHDRAW;
				}
				break;
			}

			chraction++;
		}

		if (num != 1) {
			g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
		} else {
			g_Vars.aioffset += 4;
		}
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0134
 */
bool aiIfOrders(void)
{
	struct bytelist *cmd = (struct bytelist *)(g_Vars.ailist + g_Vars.aioffset);

	if (g_Vars.chrdata->orders == cmd->b3) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd->b4);

		if (g_Vars.chrdata->orders == MA_WITHDRAW) {
			// empty
		}
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 0135
 */
bool aiIfHasOrders(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (g_Vars.chrdata->orders) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0136
 */
bool aiRetreat(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (cmd[3] == 0) {
		chrRunFromPos(g_Vars.chrdata, cmd[2], (cmd[2] & 0x10) ? 400.0f : 10000.0f, &g_Vars.chrdata->runfrompos);
	} else if (cmd[3] == 1) {
		struct prop *target = chrGetTargetProp(g_Vars.chrdata);
		chrRunFromPos(g_Vars.chrdata, cmd[2], 10000, &target->pos);
	} else {
		chrAssignCoverByCriteria(g_Vars.chrdata,
				COVERCRITERIA_FURTHEREST
				| COVERCRITERIA_DISTTOTARGET
				| COVERCRITERIA_ONLYNEIGHBOURINGROOMS
				| COVERCRITERIA_ROOMSFROMME, 0);
		chrGoToCover(g_Vars.chrdata, cmd[2]);
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 0137
 */
bool aiIfChrInSquadronDoingAction(void)
{
	s32 ret;
	s16 *chrnums = squadronGetChrIds(g_Vars.chrdata->squadron);
	struct bytelist *cmd = (struct bytelist *)(g_Vars.ailist + g_Vars.aioffset);
	ret = 1;

	if (chrnums) {
		for (; *chrnums != -2; chrnums++) {
			struct chrdata *chr = chrFindByLiteralId(*chrnums);

			if (chr && chr->model && chrIsDead(chr) == false &&
					chr->actiontype != ACT_DEAD &&
					chrCompareTeams(g_Vars.chrdata, chr, COMPARE_FRIENDS) &&
					g_Vars.chrdata->chrnum != chr->chrnum &&
					chrGetDistanceToChr(g_Vars.chrdata, chr->chrnum) < 3500 &&
					chr->myaction == cmd->b2) {
				ret = 2;
				break;
			}
		}
	}

	if (ret != 1) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd->b3);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0139
 */
bool ai0139(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 angle = (cmd[3] << 16) | (cmd[4] << 8) | cmd[5] | (cmd[2] << 24);
	struct coord pos;

	chr0f04c874(g_Vars.chrdata, angle, &pos, cmd[7], cmd[6]);

	g_Vars.aioffset += 8;

	return false;
}

/**
 * @cmd 013a
 */
bool aiSetChrPresetToUnalertedTeammate(void)
{
	f32 closest_distance = 30999.9;
	s16 candidate_chrnum = -1;
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s16 *chrnums = teamGetChrIds(g_Vars.chrdata->team);

	if (g_Vars.chrdata->talktimer > TICKS(480) && g_Vars.chrdata->listening) {
		g_Vars.chrdata->listening = 0;
	}

	for (; *chrnums != -2; chrnums++) {
		struct chrdata *chr = chrFindByLiteralId(*chrnums);

		if (cmd[3] == 0 && chr && chr->model &&
				chrIsDead(chr) == false &&
				chr->actiontype != ACT_DEAD &&
				chr->actiontype != ACT_DIE &&
				chr->actiontype != ACT_DRUGGEDKO &&
				chr->actiontype != ACT_DRUGGEDDROP &&
				chr->actiontype != ACT_DRUGGEDCOMINGUP &&
				chr->alertness < 100 &&
				(g_Vars.chrdata->squadron == chr->squadron || g_Vars.chrdata->squadron == 0xff) &&
				g_Vars.chrdata->chrnum != chr->chrnum) {

			f32 distance = chrGetDistanceToChr(g_Vars.chrdata, chr->chrnum);

			if (distance < closest_distance &&
					(distance < 100.0f * (s32)cmd[2] || cmd[2] == 0) &&
					distance < closest_distance) {
				closest_distance = distance;

				if (cmd[3] != 1) {
					candidate_chrnum = chr->chrnum;
				} else if (distance > 200) {
					candidate_chrnum = chr->chrnum;
				}
			}
		}
	}

	if (candidate_chrnum != -1) {
		chrSetChrPreset(g_Vars.chrdata, candidate_chrnum);
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 013b
 */
bool aiSetSquadron(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	g_Vars.chrdata->squadron = cmd[2];
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 013c
 */
bool aiFaceCover(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (chrFaceCover(g_Vars.chrdata)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 013d
 */
bool aiIfDangerousObjectNearby(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (chrDetectDangerousObject(g_Vars.chrdata, cmd[2])) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 013e
 */
bool ai013e(void)
{
	if (func0f03aca0(g_Vars.chrdata, 400, true) == 0 && chrAssignCoverAwayFromDanger(g_Vars.chrdata, 1000, 12000) != -1) {
		chrGoToCover(g_Vars.chrdata, GOPOSFLAG_RUN);
	}

	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 013f
 */
bool aiIfHeliWeaponsArmed(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (g_Vars.hovercar) {
		if (g_Vars.hovercar->weaponsarmed) {
			g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
		} else {
			g_Vars.aioffset += 3;
		}
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0140
 */
bool aiIfHoverbotNextStep(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (g_Vars.hovercar) {
		if ((g_Vars.hovercar->nextstep > cmd[3] && cmd[2] == 1) ||
				(g_Vars.hovercar->nextstep < cmd[3] && cmd[2] == 0)) {
			g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
		} else {
			g_Vars.aioffset += 5;
		}
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 0141
 */
bool aiShuffleInvestigationTerminals(void)
{
	// 2 = goodtag
	// 3 = badtag
	// 4 = pc1
	// 5 = pc2
	// 6 = pc3
	// 7 = pc4
	// 8 = always 00

	u8 rand1;
	u8 rand2;
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct tag *goodtag = tagFindById(cmd[2]);
	struct tag *badtag = tagFindById(cmd[3]);
	struct tag *pc;

	/**
	 * @bug: These should be using modulus 4. Because it's 1 short, the final
	 * else conditions are unreachable so only 3 of the 4 terminals can be
	 * selected as the destination.
	 */
	rand1 = random() % 3;
	rand2 = random() % 3;

	if (cmd[8] == 0) {
		// Place the good terminal
		if (rand1 == 0) {
			pc = tagFindById(cmd[4]);
			goodtag->cmdoffset = pc->cmdoffset;
			goodtag->obj = pc->obj;
		} else if (rand1 == 1) {
			pc = tagFindById(cmd[5]);
			goodtag->cmdoffset = pc->cmdoffset;
			goodtag->obj = pc->obj;
		} else if (rand1 == 2) {
			pc = tagFindById(cmd[6]);
			goodtag->cmdoffset = pc->cmdoffset;
			goodtag->obj = pc->obj;
		} else {
			pc = tagFindById(cmd[7]);
			goodtag->cmdoffset = pc->cmdoffset;
			goodtag->obj = pc->obj;
		}

		// Handle both good and alarm terminals having the same destination
		if (rand2 == rand1 && rand2 > 0) {
			rand2--;
		} else if (rand2 == rand1 && rand2 < 3) {
			rand2++;
		}

		// Place the alarm terminal
		if (rand2 == 0) {
			pc = tagFindById(cmd[4]);
			badtag->cmdoffset = pc->cmdoffset;
			badtag->obj = pc->obj;
		} else if (rand2 == 1) {
			pc = tagFindById(cmd[5]);
			badtag->cmdoffset = pc->cmdoffset;
			badtag->obj = pc->obj;
		} else if (rand2 == 2) {
			pc = tagFindById(cmd[6]);
			badtag->cmdoffset = pc->cmdoffset;
			badtag->obj = pc->obj;
		} else {
			pc = tagFindById(cmd[7]);
			badtag->cmdoffset = pc->cmdoffset;
			badtag->obj = pc->obj;
		}
	}

	g_Vars.aioffset += 9;

	return false;
}

/**
 * Used by the blow command, which makes Investigation scientists run to the
 * computer terminals. The number of the left is the pad that the terminal sits
 * on, and the number on the right is the pad that the scientist will run to
 * when running to the terminal.
 */
u16 g_InvestigationPadMap[] = {
	0x0246, 0x007b,
	0x0247, 0x007d,
	0x0248, 0x007e,
	0x0249, 0x007c,
	0x024a, 0x0093,
	0x024b, 0x0091,
	0x024c, 0x0092,
	0x024d, 0x0093,
	0x024e, 0x00ae,
	0x024f, 0x00ac,
	0x0250, 0x00ab,
	0x0251, 0x00ad,
	0x0252, 0x00a1,
	0x0253, 0x009f,
	0x0254, 0x00a0,
	0x0255, 0x009e,
	0x0256, 0x00bb,
	0x0257, 0x00b8,
	0x0258, 0x00b9,
	0x0259, 0x00ba,
};

/**
 * @cmd 0142
 */
bool aiSetPadPresetToInvestigationTerminal(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj) {
		s16 objpad = obj->pad;
		s32 i;

		for (i = 0; i < sizeof(g_InvestigationPadMap) / sizeof(g_InvestigationPadMap[0]); i += 2) {
			if (objpad == g_InvestigationPadMap[i]) {
				chrSetPadPreset(g_Vars.chrdata, g_InvestigationPadMap[i + 1]);
			}
		}
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 0143
 */
bool aiHeliArmWeapons(void)
{
	if (g_Vars.hovercar) {
		chopperSetArmed(g_Vars.hovercar, true);
	}

	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 0144
 */
bool aiHeliUnarmWeapons(void)
{
	if (g_Vars.hovercar) {
		chopperSetArmed(g_Vars.hovercar, false);
	}

	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 0145
 */
bool aiRebuildTeams(void)
{
	rebuildTeams();
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 0146
 */
bool aiRebuildSquadrons(void)
{
	rebuildSquadrons();
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 0147
 */
bool aiIfSquadronIsDead(void)
{
	/**
	 * @bug: anyalive is initialised to true here, and reset to false in each
	 * loop iteration. This causes it to use the last chr's status only.
	 */
	u32 stack[2];
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	bool anyalive = true;
	s16 *chrnums = squadronGetChrIds(cmd[2]);

	if (chrnums) {
		while (*chrnums != -2) {
			struct chrdata *chr = chrFindByLiteralId(*chrnums);

			if (chr && chr->model) {
				anyalive = false;

				if (!chrIsDead(chr) && chr->actiontype != ACT_DEAD) {
					anyalive = true;
				}
			}

			chrnums++;
		}
	}

	if (!anyalive) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0148
 */
bool aiChrSetListening(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->listening == 0) {
		chr->listening = cmd[3];
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 0149
 */
bool aiIfChrListening(void)
{
	struct bytelist *cmd = (struct bytelist *)(g_Vars.ailist + g_Vars.aioffset);
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd->b2);

	if (cmd->b4 == 0) {
		if (chr->listening == cmd->b3) {
			g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd->b5);
		} else {
			g_Vars.aioffset += 6;
		}
	} else {
		if (g_Vars.chrdata->convtalk == 0) {
			g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd->b5);
		} else {
			g_Vars.aioffset += 6;
		}
	}

	return false;
}

/**
 * @cmd 014a
 */
bool aiIfTrue(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[5]);

	return false;
}

/**
 * @cmd 014b
 */
bool aiIfNotListening(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (g_Vars.chrdata->listening == 0) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0157
 */
bool aiSetTintedGlassEnabled(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	g_TintedGlassEnabled = cmd[2];
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0152
 */
bool aiIfNumChrsInSquadronGreaterThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s32 count = 0;
	s16 *chrnums = squadronGetChrIds(cmd[3]);

	if (chrnums) {
		while (*chrnums != -2) {
			struct chrdata *chr = chrFindByLiteralId(*chrnums);

			if (chr && chr->prop && chrIsDead(chr) == false
					&& chr->actiontype != ACT_DEAD
					&& chr->actiontype != ACT_DRUGGEDKO
					&& chr->actiontype != ACT_DRUGGEDDROP
					&& chr->actiontype != ACT_DRUGGEDCOMINGUP) {
				count++;
			}

			chrnums++;
		}
	}

	if (count > cmd[2]) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 0165
 */
bool aiIfChrInjured(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && (chr->chrflags & CHRCFLAG_INJUREDTARGET)) {
		chr->chrflags &= ~CHRCFLAG_INJUREDTARGET;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0166
 */
bool aiIfAction(void)
{
	struct bytelist *cmd = (struct bytelist *)(g_Vars.ailist + g_Vars.aioffset);

	if (g_Vars.chrdata->myaction == cmd->b2) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd->b3);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0167
 */
bool aiHovercopterFireRocket(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	chopperFireRocket(g_Vars.hovercar, cmd[2]);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0169
 */
bool aiIfNaturalAnim(void)
{
	struct bytelist *cmd = (struct bytelist *)(g_Vars.ailist + g_Vars.aioffset);

	if (g_Vars.chrdata->naturalanim == cmd->b2) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd->b3);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 016a
 */
bool aiIfY(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = NULL;
	f32 cutoff_y = ((cmd[4] | (cmd[3] << 8)) << 16) >> 16;

	if (cmd[2] == CHR_TARGET && g_Vars.hovercar) {
		struct chopperobj *chopper = chopperFromHovercar(g_Vars.hovercar);

		if (chopper) {
			struct prop *target = chopperGetTargetProp(chopper);

			if (target && (target->type == PROPTYPE_CHR || target->type == PROPTYPE_PLAYER)) {
				chr = target->chr;
			}
		}
	} else {
		chr = chrFindById(g_Vars.chrdata, cmd[2]);
	}

	if (chr && chr->prop && (
				(chr->prop->pos.y < cutoff_y && cmd[5] == 0) ||
				(chr->prop->pos.y > cutoff_y && cmd[5] == 1))) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[6]);
	} else {
		g_Vars.aioffset += 7;
	}

	return false;
}

/**
 * @cmd 016c
 */
bool aiNoOp016c(void)
{
	g_Vars.aioffset += 2;
	return false;
}

/**
 * @cmd 016d
 */
bool aiChrAdjustMotionBlur(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr) {
		if (cmd[4] == 0) {
			chr->blurdrugamount -= TICKS(cmd[3]);
		} else {
			chr->blurdrugamount += TICKS(cmd[3]);
		}
	}

	g_Vars.aioffset += 5;

	return false;
}

/**
 * @cmd 016e
 */
bool aiDamageChrByAmount(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct coord coord = {0, 0, 0};
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop) {
		if (cmd[4] == 2) {
			struct gset gset = {WEAPON_COMBATKNIFE, 0, 0, FUNC_POISON};
			chrDamageByMisc(chr, (s32)cmd[3] * 0.03125f, &coord, &gset, NULL);
		} else if (cmd[4] == 0) {
			chrDamageByMisc(chr, (s32)cmd[3] * 0.03125f, &coord, NULL, NULL);
		} else {
			chrDamageByMisc(chr, (s32)cmd[3] * -0.03125f, &coord, NULL, NULL);
		}
	}

	g_Vars.aioffset += 5;

	return false;
}

/**
 * @cmd 016f
 */
bool aiIfChrHasGun(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->model && chr->gunprop == NULL) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 0170
 */
bool aiDoGunCommand(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct weaponobj *weapon = g_Vars.chrdata->gunprop->weapon;

	if (cmd[2] == 0 || ((weapon->base.hidden & OBJHFLAG_PROJECTILE) == 0 && cmd[2] == 1)) {
		if (cmd[2] == 0) {
			chrGoToProp(g_Vars.chrdata, g_Vars.chrdata->gunprop, GOPOSFLAG_JOG);
		}

		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0171
 */
bool aiIfDistanceToGunLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 distance = (cmd[3] | (cmd[2] << 8)) * 10.0f;
	f32 xdiff = 0;
	f32 ydiff = 0;
	f32 zdiff = 0;

	if (g_Vars.chrdata->gunprop) {
		xdiff = g_Vars.chrdata->prop->pos.x - g_Vars.chrdata->gunprop->pos.x;
		ydiff = g_Vars.chrdata->prop->pos.y - g_Vars.chrdata->gunprop->pos.y;
		zdiff = g_Vars.chrdata->prop->pos.z - g_Vars.chrdata->gunprop->pos.z;
	}

	if (ydiff < 200 && ydiff > -200 &&
			xdiff < distance && xdiff > -distance &&
			zdiff < distance && zdiff > -distance) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 0172
 */
bool aiRecoverGun(void)
{ \
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct prop *prop = g_Vars.chrdata->gunprop;
	g_Vars.chrdata->gunprop = NULL;

	if (prop && prop->obj && prop->parent == NULL && prop->type == PROPTYPE_WEAPON) {
		propDeregisterRooms(prop);
		propDelist(prop);
		propDisable(prop);
		chrEquipWeapon(prop->weapon, g_Vars.chrdata);
	}

	g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);

	if (1);

	return false;
}

/**
 * @cmd 0173
 */
bool aiChrCopyProperties(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->model) {
		g_Vars.chrdata->hearingscale = chr->hearingscale;
		g_Vars.chrdata->visionrange = chr->visionrange;
		g_Vars.chrdata->padpreset1 = chr->padpreset1;
		g_Vars.chrdata->chrpreset1 = chr->chrpreset1;
		g_Vars.chrdata->flags = chr->flags;
		g_Vars.chrdata->flags2 = chr->flags2;
		g_Vars.chrdata->team = chr->team;
		g_Vars.chrdata->squadron = chr->squadron;

		// @bug? Copying from self to self
		g_Vars.chrdata->accuracyrating = g_Vars.chrdata->accuracyrating;
		g_Vars.chrdata->speedrating = g_Vars.chrdata->speedrating;

		g_Vars.chrdata->naturalanim = chr->naturalanim;
		g_Vars.chrdata->myspecial = chr->myspecial;
		g_Vars.chrdata->yvisang = chr->yvisang;
		g_Vars.chrdata->teamscandist = chr->teamscandist;

		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0177
 */
bool aiPlayerAutoWalk(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s16 pad_id = cmd[4] | (cmd[3] << 8);
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		u32 prevplayernum = g_Vars.currentplayernum;
		u32 playernum = playermgrGetPlayerNumByProp(chr->prop);
		setCurrentPlayerNum(playernum);
		playerAutoWalk(pad_id, cmd[5], cmd[6], cmd[7], cmd[8]);
		setCurrentPlayerNum(prevplayernum);
	}

	g_Vars.aioffset += 9;

	return false;
}

/**
 * @cmd 0178
 */
bool aiIfPlayerAutoWalkFinished(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	bool walking = false;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		u32 prevplayernum = g_Vars.currentplayernum;
		u32 playernum = playermgrGetPlayerNumByProp(chr->prop);
		setCurrentPlayerNum(playernum);

		if (g_Vars.tickmode == TICKMODE_AUTOWALK) {
			walking = true;
		}

		setCurrentPlayerNum(prevplayernum);
	}

	if (walking) {
		g_Vars.aioffset += 4;
	} else {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	}

	return false;
}

/**
 * @cmd 0181
 */
bool aiIfPlayerLookingAtObject(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[3]);
	bool pass = false;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		u32 prevplayernum = g_Vars.currentplayernum;
		u32 playernum = playermgrGetPlayerNumByProp(chr->prop);
		setCurrentPlayerNum(playernum);

		if (g_Vars.currentplayer->lookingatprop.prop == obj->prop) {
			pass = true;
		}

		setCurrentPlayerNum(prevplayernum);
	}

	if (pass) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 0182
 */
bool aiPunchOrKick(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (g_Vars.chrdata && chrTryPunch(g_Vars.chrdata, cmd[2])) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0183
 */
bool aiIfTargetIsPlayer(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct prop *target = chrGetTargetProp(g_Vars.chrdata);

	if (target->type == PROPTYPE_EYESPY || target->type == PROPTYPE_PLAYER) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0184
 */
bool ai0184(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	chrTryAttackAmount(g_Vars.chrdata, 512, 0, cmd[2], cmd[3]);
	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 0186
 */
bool aiIfSoundTimer(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s32 value = TICKS(cmd[3] | (cmd[2] << 8));

	if ((g_Vars.chrdata->soundtimer > value && cmd[4] == 0) ||
			(g_Vars.chrdata->soundtimer < value && cmd[4] == 1)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[5]);
	} else {
		g_Vars.aioffset += 6;
	}

	return false;
}

/**
 * @cmd 0187
 */
bool aiSetTargetToEyespyIfInSight(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s16 prevtarget = g_Vars.chrdata->target;
	struct eyespy *eyespy = g_Vars.players[g_Vars.chrdata->p1p2]->eyespy;

	if (eyespy) {
		struct chrdata *chr = eyespy->prop->chr;
		g_Vars.chrdata->target = propGetIndexByChrId(g_Vars.chrdata, chr->chrnum);

		if (chrCheckCanSeeTarget(g_Vars.chrdata)) {
			g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
		} else {
			g_Vars.aioffset += 3;
			g_Vars.chrdata->target = prevtarget;
		}
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 0188
 */
bool aiIfLiftStationary(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);
	bool pass = false;

	if (obj && obj->prop && obj->type == OBJTYPE_LIFT) {
		struct liftobj *lift = (struct liftobj *)obj;

		if ((obj->flags & OBJFLAG_DEACTIVATED) || lift->dist == 0) {
			pass = true;
		}
	}

	if (pass) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0189
 */
bool aiLiftGoToStop(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop && obj->type == OBJTYPE_LIFT) {
		struct liftobj *lift = (struct liftobj *)obj;
		liftGoToStop(lift, cmd[3]);
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 018a
 */
bool aiIfLiftAtStop(void)
{
	struct bytelist *cmd = (struct bytelist *)(g_Vars.ailist + g_Vars.aioffset);
	struct defaultobj *obj = objFindByTagId(cmd->b2);
	bool pass = false;

	if (obj && obj->prop && obj->type == OBJTYPE_LIFT) {
		struct liftobj *lift = (struct liftobj *)obj;

		if (lift->levelcur == cmd->b3 && lift->dist == 0) {
			pass = true;
		}
	}

	if (pass) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd->b4);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 018b
 */
bool aiConfigureRain(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	weatherConfigureRain(cmd[2]);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 01b6
 */
bool aiConfigureSnow(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	weatherConfigureSnow(cmd[2]);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 018c
 */
bool aiChrToggleModelPart(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr) {
		chrToggleModelPart(chr, cmd[3]);
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 018d
 */
bool aiActivateLift(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[3]);

	if (obj && obj->prop) {
		liftActivate(obj->prop, cmd[2]);
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 018e
 */
bool aiMiniSkedarTryPounce(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 thing = cmd[4] | (cmd[3] << 8);

	if (chrTrySkJump(g_Vars.chrdata, g_Vars.chrdata->pouncebits, cmd[2], thing, cmd[5])) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[6]);
	} else {
		g_Vars.aioffset += 7;

	}

	return false;
}

/**
 * @cmd 018f
 */
bool aiIfObjectDistanceToPadLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 distance = (f32)(cmd[4] | (cmd[3] << 8)) * 10;
	f32 xdiff;
	f32 ydiff;
	f32 zdiff;
#if VERSION >= VERSION_NTSC_1_0
	s32 pad_id = (cmd[6] | (cmd[5] << 8)) & 0xffff;
#else
	u16 pad_id = cmd[6] | (cmd[5] << 8);
#endif
	struct defaultobj *obj = objFindByTagId(cmd[2]);
	struct pad pad;
	bool pass = false;

	if (obj && obj->prop) {
		pad_id = chrResolvePadId(g_Vars.chrdata, pad_id);

#if VERSION >= VERSION_NTSC_1_0
		if (pad_id >= 0)
#endif
		{
			padUnpack(pad_id, PADFIELD_POS, &pad);
			xdiff = obj->prop->pos.x - pad.pos.x;
			ydiff = obj->prop->pos.y - pad.pos.y;
			zdiff = obj->prop->pos.z - pad.pos.z;

			if (ydiff < 200 && ydiff > -200 &&
					xdiff < distance && xdiff > -distance &&
					zdiff < distance && zdiff > -distance) {
				pass = true;
			}
		}
	}

	if (pass) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[7]);
	} else {
		g_Vars.aioffset += 8;
	}

	return false;
}

/**
 * @cmd 0190
 */
bool aiSetSavefileFlag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	gamefileSetFlag(cmd[2]);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0191
 */
bool aiUnsetSavefileFlag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	gamefileUnsetFlag(cmd[2]);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 0192
 */
bool aiIfSavefileFlagIsSet(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (gamefileHasFlag(cmd[2])) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 0193
 */
bool aiIfSavefileFlagIsUnset(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (!gamefileHasFlag(cmd[2])) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 019e
 */
bool aiIfObjHealthLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s32 damage = cmd[4] | (cmd[3] << 8);
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	bool condition_passes = false;

	if (obj && obj->prop && obj->damage < damage) {
		condition_passes = true;
	}

	if (condition_passes) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[5]);
	} else {
		g_Vars.aioffset += 6;
	}

	return false;
}

/**
 * @cmd 019f
 */
bool aiSetObjHealth(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s32 damage = cmd[4] | (cmd[3] << 8);
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop) {
		obj->damage = damage;
	}

	g_Vars.aioffset += 5;

	return false;
}

/**
 * @cmd 01a0
 */
bool aiSetChrSpecialDeathAnimation(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr) {
		chr->specialdie = cmd[3];
	}

	g_Vars.aioffset += 4;

	return 0;
}

/**
 * @cmd 01a1
 */
bool aiSetRoomToSearch(void)
{
	struct chrdata *target = chrFindById(g_Vars.chrdata, CHR_TARGET);

	if (target && target->prop) {
		g_Vars.chrdata->roomtosearch = target->prop->rooms[0];
	}

	g_Vars.aioffset += 2;

	return false;
}

s16 g_CiMainQuips[][3] = {
	{ MP3_CARR_WHAT_IS_IT,           MP3_CARR_A_LITTLE_BUSY,          MP3_CARR_PROUD_OF_YOU            },
	{ MP3_CIFEM_FOSTER,              MP3_CIFEM_COME_TO_SEE_ME,        MP3_CIFEM_PRACTICE_MAKES_PERFECT },
	{ MP3_GRIMSHAW_KNOW_YOUR_ENEMY,  MP3_GRIMSHAW_ALL_EARS,           MP3_GRIMSHAW_GAP_IN_RECORDS      },
	{ MP3_HOLO_LAST_MINUTE_TRAINING, MP3_HOLO_PRACTICE_MAKES_PERFECT, MP3_HOLO_HAVE_A_LIFE             },
	{ MP3_HANGERGUY_DONT_SCREW_UP,   MP3_HANGERGUY_DONT_MAKE_A_MESS,  MP3_HANGERGUY_HALF_HOUR_AGO      },
	{ MP3_FOSTER_WHAT_TO_SHOOT_AT,   MP3_FOSTER_ZEROED_THE_SIGHTS,    MP3_FOSTER_GOOD_SHOW             },
};

s16 g_CiGreetingQuips[][3] = {
	{ SFX_CARR_HELLO_JOANNA,       SFX_CARR_HELLO_JOANNA,       SFX_CARR_HELLO_JOANNA       },
	{ SFX_CIFEM_HI_THERE,          SFX_CIFEM_HI_THERE,          SFX_CIFEM_HI_THERE          },
	{ SFX_GRIMSHAW_WELCOME,        SFX_GRIMSHAW_HI_THERE,       SFX_GRIMSHAW_UMM_ERR_HI     },
	{ SFX_HOLO_HI,                 SFX_HOLO_HI,                 SFX_HOLO_HI                 },
	{ SFX_HANGAR_WHAT_DO_YOU_WANT, SFX_HANGAR_WHAT_DO_YOU_WANT, SFX_HANGAR_WHAT_DO_YOU_WANT },
	{ SFX_FOSTER_STAR_AGENT,       SFX_FOSTER_STAR_AGENT,       SFX_FOSTER_STAR_AGENT       },
	{ SFX_CIM_HEY_THERE,           SFX_CIM_HEY_THERE,           SFX_CIM_HI                  },
	{ SFX_CIM_HI,                  SFX_CIM_HOWS_IT_GOING,       SFX_CIM_HOWS_IT_GOING       },
	{ SFX_CIFEM_HELLO,             SFX_CIFEM_HELLO,             SFX_CIFEM_HI_JO             },
	{ SFX_CIFEM_HI_JO,             SFX_CIFEM_HOWS_IT_GOING,     SFX_CIFEM_HOWS_IT_GOING     },
};

s16 g_CiAnnoyedQuips[][3] = {
	{ MP3_CARR_WORRIES_ME,           MP3_CARR_WORRIES_ME,           MP3_CARR_WORRIES_ME           },
	{ MP3_CIFEM_PATIENCE,            MP3_CIFEM_PATIENCE,            MP3_CIFEM_PATIENCE            },
	{ MP3_GRIMSHAW_BUG_OFF,          MP3_GRIMSHAW_CRIMINAL_RECORD,  MP3_GRIMSHAW_LOSER            },
	{ MP3_HOLO_IRRITATING,           MP3_HOLO_IRRITATING,           MP3_HOLO_IRRITATING           },
	{ MP3_HANGAR_LIVED_THIS_LONG,    MP3_HANGAR_LIVED_THIS_LONG,    MP3_HANGAR_LIVED_THIS_LONG    },
	{ MP3_FOSTER_LEAVE_ME_ALONE,     MP3_FOSTER_LEAVE_ME_ALONE,     MP3_FOSTER_LEAVE_ME_ALONE     },
	{ MP3_CIM_BE_SOMEWHERE,          MP3_CIM_BE_SOMEWHERE,          MP3_CIM_BE_SOMEWHERE          },
	{ MP3_CIM_BE_SOMEWHERE,          MP3_CIM_BE_SOMEWHERE,          MP3_CIM_BE_SOMEWHERE          },
	{ MP3_CIFEM_BE_SOMEWHERE,        MP3_CIFEM_BE_SOMEWHERE,        MP3_CIFEM_BE_SOMEWHERE        },
	{ MP3_CIFEM_BOTHER_SOMEONE_ELSE, MP3_CIFEM_BOTHER_SOMEONE_ELSE, MP3_CIFEM_BOTHER_SOMEONE_ELSE },
};

s16 g_CiThanksQuips[] = {
	0,
	MP3_CIFEM_THANKS1,
	MP3_GRIMSHAW_TOO_CLOSE,
	MP3_CIFEM_THANKS2,
	0,
	MP3_FOSTER_RELY_ON_YOU,
	MP3_CIM_THANKS_JO,
	MP3_CIM_THANKS_JO,
	MP3_CIFEM_THANKS3,
	MP3_CIFEM_THANKS3,
};

/**
 * @cmd 01a2
 */
bool aiSayCiStaffQuip(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s16 quip;

	if (cmd[2] == CIQUIP_GREETING) {
		quip = g_CiGreetingQuips[g_Vars.chrdata->morale][random() % 3];
		audioPlayFromProp((s8)cmd[3], quip, 0, g_Vars.chrdata->prop, 9, 0);
	}

	if (cmd[2] == CIQUIP_MAIN) {
		quip = g_CiMainQuips[g_Vars.chrdata->morale][random() % 3];
		audioPlayFromProp((s8)cmd[3], quip, 0, g_Vars.chrdata->prop, 9, 0);
	}

	if (cmd[2] == CIQUIP_ANNOYED) {
		quip = g_CiAnnoyedQuips[g_Vars.chrdata->morale][random() % 3];
		audioPlayFromProp((s8)cmd[3], quip, 0, g_Vars.chrdata->prop, 9, 0);
	}

	if (cmd[2] == CIQUIP_THANKS) {
		quip = g_CiThanksQuips[g_Vars.chrdata->morale];
		audioPlayFromProp((s8)cmd[3], quip, 0, g_Vars.chrdata->prop, 9, 0);
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 01a3
 */
bool aiDoPresetAnimation(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	// These all appear to be talking animations
	u16 anims[] = {
		/* 0*/ ANIM_0296,
		/* 1*/ ANIM_0297,
		/* 2*/ ANIM_0298,
		/* 3*/ ANIM_028A, // when value is 3 (1/2 chance)
		/* 4*/ ANIM_028C, // when value is 3 (1/2 chance)
		/* 5*/ ANIM_0290,
		/* 6*/ ANIM_0291,
		/* 7*/ ANIM_TALKING_00A3, // when value is 255 (1/8 chance)
		/* 8*/ ANIM_028E, // when value is 255 (1/8 chance)
		/* 9*/ ANIM_028F, // when value is 255 (1/8 chance)
		/*10*/ ANIM_TALKING_0231, // when value is 255 (1/8 chance)
		/*11*/ ANIM_TALKING_0232, // when value is 255 (1/8 chance)
		/*12*/ ANIM_TALKING_0233, // when value is 255 (1/8 chance)
		/*13*/ ANIM_TALKING_0234, // when value is 255 (1/8 chance)
		/*14*/ ANIM_028D, // when value is 255 (1/8 chance)
	};

	if (cmd[2] == 255) {
		chrTryStartAnim(g_Vars.chrdata, anims[7 + (random() % 8)], 0, -1, 0, 15, 0.5);
	} else if (cmd[2] == 254) {
		struct prop *prop0 = chrGetHeldProp(g_Vars.chrdata, 1);
		struct prop *prop1 = chrGetHeldProp(g_Vars.chrdata, 0);

		if (weaponIsOneHanded(prop0) || weaponIsOneHanded(prop1)) {
			chrTryStartAnim(g_Vars.chrdata, ANIM_FIX_GUN_JAM_EASY, 0, -1, 0, 5, 0.5);
		} else {
			chrTryStartAnim(g_Vars.chrdata, ANIM_FIX_GUN_JAM_HARD, 0, -1, 0, 5, 0.5);
		}
	} else if (cmd[2] == 3) {
		chrTryStartAnim(g_Vars.chrdata, anims[3 + (random() & 1)], 0, -1, 0, 15, 0.5);
	} else {
		chrTryStartAnim(g_Vars.chrdata, anims[cmd[2]], 0, -1, 0, 15, 0.5);
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 01a5
 */
bool aiIfUsingLift(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (chrIsUsingLift(g_Vars.chrdata)) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 01a6
 */
bool aiIfTargetYDifferenceLessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct prop *prop = chrGetTargetProp(g_Vars.chrdata);
	f32 diff = prop->pos.y - g_Vars.chrdata->prop->pos.y;

	if (diff < 0) {
		diff = 0 - diff;
	}

	if (diff < (s32)cmd[2] * 10.0f) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 01aa
 */
bool ai01aa(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 a = 3000;

	func0f0056f4(
			g_Vars.currentplayer->prop->rooms[0],
			&g_Vars.currentplayer->prop->pos,
			g_Vars.chrdata->prop->rooms[0],
			&g_Vars.chrdata->prop->pos,
			0, &a, 0);

	if (a < 3000) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 01ae
 */
bool aiClearInventory(void)
{
	u32 stackpadding[2];
	u32 prevplayernum = g_Vars.currentplayernum;
	s32 playernum;

	for (playernum = 0; playernum < PLAYERCOUNT(); playernum++) {
		setCurrentPlayerNum(playernum);

		if (g_Vars.currentplayer == g_Vars.bond || g_Vars.currentplayer == g_Vars.coop) {
			invClear();
#if VERSION >= VERSION_NTSC_1_0
			g_Vars.currentplayer->devicesactive = 0;
#endif
			invGiveSingleWeapon(WEAPON_UNARMED);
			bgunEquipWeapon(WEAPON_UNARMED);
		}
	}

	setCurrentPlayerNum(prevplayernum);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 01ad
 */
bool aiReleaseObject(void)
{
	bmoveSetModeForAllPlayers(MOVEMODE_WALK);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 01af
 */
bool aiChrGrabObject(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[3]);
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER && obj && obj->prop) {
		u32 prevplayernum = g_Vars.currentplayernum;
		u32 playernum = playermgrGetPlayerNumByProp(chr->prop);
		setCurrentPlayerNum(playernum);

		if (g_Vars.currentplayer->bondmovemode == MOVEMODE_WALK
				&& bmoveGetCrouchPos() == CROUCHPOS_STAND
				&& g_Vars.currentplayer->crouchoffset == 0) {
			bmoveGrabProp(obj->prop);
		}

		setCurrentPlayerNum(prevplayernum);
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 01b1
 */
bool aiShuffleRuinsPillars(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct tag *ptr1 = tagFindById(cmd[2]);
	struct tag *ptr2 = tagFindById(cmd[3]);
	struct tag *ptr3 = tagFindById(cmd[4]);
	struct tag *src;
	u8 marked1index = random() % 5;
	u8 marked2index = random() % 5;
	u8 marked3index = random() % 5;
	u8 pillars[5];
	u8 mines[5];
	pillars[0] = cmd[5];
	pillars[1] = cmd[6];
	pillars[2] = cmd[7];
	pillars[3] = cmd[8];
	pillars[4] = cmd[9];
	mines[0] = cmd[13];
	mines[1] = cmd[14];
	mines[2] = cmd[15];
	mines[3] = cmd[16];
	mines[4] = cmd[17];

	while (marked2index == marked1index) {
		marked2index = random() % 5;
	}

	while (marked3index == marked2index || marked3index == marked1index) {
		marked3index = random() % 5;
	}

	// Pillar/mine 1
	src = tagFindById(pillars[marked1index]);
	ptr1->cmdoffset = src->cmdoffset;
	ptr1->obj = src->obj;

	ptr1 = tagFindById(cmd[10]);
	src = tagFindById(mines[marked1index]);
	ptr1->cmdoffset = src->cmdoffset;
	ptr1->obj = src->obj;

	// Pillar/mine 2
	src = tagFindById(pillars[marked2index]);
	ptr2->cmdoffset = src->cmdoffset;
	ptr2->obj = src->obj;

	ptr2 = tagFindById(cmd[11]);
	src = tagFindById(mines[marked2index]);
	ptr2->cmdoffset = src->cmdoffset;
	ptr2->obj = src->obj;

	// Pillar/mine 3
	src = tagFindById(pillars[marked3index]);
	ptr3->cmdoffset = src->cmdoffset;
	ptr3->obj = src->obj;

	ptr3 = tagFindById(cmd[12]);
	src = tagFindById(mines[marked3index]);
	ptr3->cmdoffset = src->cmdoffset;
	ptr3->obj = src->obj;

	g_Vars.aioffset += 18;

	return false;
}

/**
 * @cmd 01b2
 */
bool aiSetWindSpeed(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	g_SkyWindSpeed = 0.1f * (s32)cmd[2];
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 01b3
 */
bool aiToggleP1P2(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (g_Vars.coopplayernum >= 0) {
		struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

		if (chr) {
			if (chr->p1p2 == g_Vars.bondplayernum && !g_Vars.coop->isdead) {
				chr->p1p2 = g_Vars.coopplayernum;
			} else if (!g_Vars.bond->isdead) {
				chr->p1p2 = g_Vars.bondplayernum;
			}
		}
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 01b5
 */
bool aiChrSetP1P2(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (g_Vars.coopplayernum >= 0) {
		struct chrdata *chr1 = chrFindById(g_Vars.chrdata, cmd[2]);
		struct chrdata *chr2 = chrFindById(g_Vars.chrdata, cmd[3]);

		if (chr1 && chr2 && chr2->prop && chr2->prop->type == PROPTYPE_PLAYER) {
			u32 playernum = playermgrGetPlayerNumByProp(chr2->prop);

			if (!g_Vars.players[playernum]->isdead) {
				if (chr2->prop == g_Vars.coop->prop) {
					chr1->p1p2 = g_Vars.coopplayernum;
				} else {
					chr1->p1p2 = g_Vars.bondplayernum;
				}
			}
		}
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 01b7
 */
bool aiChrSetCloaked(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop) {
		if (!chrIsDead(chr)) {
			if (cmd[3]) {
				chrCloak(chr, cmd[4]);
			} else {
				chrUncloak(chr, cmd[4]);
			}
		}
	}

	g_Vars.aioffset += 5;

	return false;
}

/**
 * @cmd 01b8
 */
bool aiSetAutogunTargetTeam(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop && obj->type == OBJTYPE_AUTOGUN) {
		struct autogunobj *autogun = (struct autogunobj *)obj;
		autogun->targetteam = cmd[3];
		autogun->target = NULL;
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 01b9
 */
bool aiShufflePelagicSwitches(void)
{
	u8 buttonsdone[] = {0, 0, 0, 0, 0, 0, 0, 0};
	u8 i;
	u8 j;
	struct tag *tag;
	struct tag *button;
	u8 index;

	for (i = 8; i < 16; i++) {
		tag = tagFindById(i);
		index = random() & 7;

		if (buttonsdone[index] == 0) {
			// Switch has not yet been mapped
			button = tagFindById(index);
			tag->cmdoffset = button->cmdoffset;
			tag->obj = button->obj;
			buttonsdone[index] = 1;
		} else {
			// Switch is already mapped - find first unmapped switch
			for (j = 0; buttonsdone[j]; j++);

			button = tagFindById(j);
			tag->cmdoffset = button->cmdoffset;
			tag->obj = button->obj;
			buttonsdone[j] = 1;
		}
	}

	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 01bb
 */
bool aiNoOp01bb(void)
{
	g_Vars.aioffset += 4;
	return false;
}

/**
 * @cmd 01bc
 */
bool ai01bc(void)
{
	struct bytelist *cmd = (struct bytelist *)(g_Vars.ailist + g_Vars.aioffset);

	if (g_Vars.chrdata->pouncebits == cmd->b2) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd->b3);
	} else {
		g_Vars.aioffset += 4;
	}

	return false;
}

/**
 * @cmd 01bd
 */
bool aiIfTrainingPcHolographed(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct trainingdata *data = dtGetData();

	if (data->holographedpc) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 01be
 */
bool aiIfPlayerUsingDevice(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);
	struct prop *prop = chr ? chr->prop : NULL;
	u8 active = false;

	if (prop && prop->type == PROPTYPE_PLAYER) {
		u32 playernum = playermgrGetPlayerNumByProp(prop);
		u32 prevplayernum = g_Vars.currentplayernum;
		setCurrentPlayerNum(playernum);

		if (currentPlayerGetDeviceState(cmd[3]) == DEVICESTATE_ACTIVE) {
			active = true;
		}

		setCurrentPlayerNum(prevplayernum);
	}

	if (active) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 01bf
 */
bool aiChrBeginOrEndTeleport(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u16 pad_id = cmd[3] | (cmd[2] << 8);
	f32 fvalue;
	struct chrdata *chr;
#if VERSION >= VERSION_NTSC_1_0
	s32 mainpri;
#endif
	u32 playernum;
	u32 prevplayernum;
#if VERSION >= VERSION_NTSC_1_0
	s32 audiopri;
#endif
	struct sndstate *handle;
	fvalue = 0.4;
	chr = chrFindById(g_Vars.chrdata, cmd[4]);
	prevplayernum = g_Vars.currentplayernum;

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		playernum = playermgrGetPlayerNumByProp(chr->prop);
		setCurrentPlayerNum(playernum);
	}

	if (pad_id == 0) {
		g_Vars.currentplayer->teleportstate = TELEPORTSTATE_EXITING;
		g_Vars.currentplayer->teleporttime = 0;
	} else {
		g_Vars.currentplayer->teleporttime = 0;
		g_Vars.currentplayer->teleportstate = TELEPORTSTATE_PREENTER;
		g_Vars.currentplayer->teleportpad = pad_id;
		g_Vars.currentplayer->teleportcamerapad = 0;

#if VERSION >= VERSION_NTSC_1_0
		mainpri = osGetThreadPri(0);
		audiopri = osGetThreadPri(&g_AudioManager.thread);
		osSetThreadPri(0, audiopri + 1);
#endif

		handle = sndStart(var80095200, SFX_RELOAD_FARSIGHT, NULL, -1, -1, -1, -1, -1);

		if (handle) {
			audioPostEvent(handle, 16, *(u32 *)&fvalue);
		}

#if VERSION >= VERSION_NTSC_1_0
		osSetThreadPri(0, mainpri);
#endif
	}

	g_Vars.aioffset += 5;

	setCurrentPlayerNum(prevplayernum);

	return false;
}

/**
 * @cmd 01c0
 */
bool aiIfChrTeleportFullWhite(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[3]);
	u32 prevplayernum = g_Vars.currentplayernum;
#if VERSION >= VERSION_NTSC_1_0
	s32 mainpri;
#endif
	f32 fvalue;
#if VERSION >= VERSION_NTSC_1_0
	s32 audiopri;
#endif
	struct sndstate *handle;

	if (chr && chr->prop && chr->prop->type == PROPTYPE_PLAYER) {
		u32 playernum = playermgrGetPlayerNumByProp(chr->prop);
		setCurrentPlayerNum(playernum);
	}

	if (g_Vars.currentplayer->teleportstate < TELEPORTSTATE_WHITE) {
		g_Vars.aioffset += 4;
	} else {
		fvalue = 0.4;

#if VERSION >= VERSION_NTSC_1_0
		mainpri = osGetThreadPri(0);
		audiopri = osGetThreadPri(&g_AudioManager.thread);
		osSetThreadPri(0, audiopri + 1);
#endif

		handle = sndStart(var80095200, SFX_FIRE_SHOTGUN, NULL, -1, -1, -1, -1, -1);

		if (handle) {
			audioPostEvent(handle, 16, *(u32 *)&fvalue);
		}

#if VERSION >= VERSION_NTSC_1_0
		osSetThreadPri(0, mainpri);
#endif

		g_Vars.currentplayer->teleportstate = TELEPORTSTATE_WHITE;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	}

	setCurrentPlayerNum(prevplayernum);

	return false;
}

/**
 * @cmd 01c5
 */
bool aiAvoid(void)
{
	chrAvoid(g_Vars.chrdata);
	g_Vars.aioffset += 2;

	return false;
}

/**
 * @cmd 01c8
 */
bool aiTitleInitMode(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	g_Vars.aioffset += 3;
	titleInitFromAiCmd(cmd[2]);

	return false;
}

/**
 * @cmd 01c9
 */
bool aiTryExitTitle(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (titleIsChangingMode()) {
		titleExit();
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset = g_Vars.aioffset + 3;
	}

	return false;
}

/**
 * @cmd 01ca
 */
bool aiChrSetCutsceneWeapon(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);
	s32 model_id = playermgrGetModelOfWeapon(cmd[3]);
	s32 fallback_model_id = playermgrGetModelOfWeapon(cmd[4]);

	if (chr) {
		if (cmd[3] == 0xff) {
			if (cmd[4] == 0xff) {
				if (chr->weapons_held[0]) {
					struct weaponobj *weapon = chr->weapons_held[0]->weapon;
					bool valid = true;

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
						valid = false;
					}

					if (valid) {
						weaponDeleteFromChr(chr, HAND_LEFT);
						weaponDeleteFromChr(chr, HAND_RIGHT);
					}
				}
			} else {
				if (chr->weapons_held[0] == NULL && chr->weapons_held[1] == NULL && fallback_model_id >= 0) {
					weaponCreateForChr(chr, fallback_model_id, cmd[4], 0, NULL, NULL);
				}
			}
		} else {
			weaponDeleteFromChr(chr, HAND_LEFT);
			weaponDeleteFromChr(chr, HAND_RIGHT);

			if (model_id >= 0) {
				weaponCreateForChr(chr, model_id, cmd[3], 0, NULL, NULL);
			}

			if (fallback_model_id >= 0) {
				weaponCreateForChr(chr, fallback_model_id, cmd[4], OBJFLAG_WEAPON_LEFTHANDED, NULL, NULL);
			}
		}
	}

	g_Vars.aioffset += 5;

	return false;
}

/**
 * @cmd 01cb
 */
bool aiFadeScreen(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	u32 color = (cmd[3] << 16) | (cmd[4] << 8) | cmd[5] | (cmd[2] << 24);
	s16 num_frames = (cmd[7] | (cmd[6] << 8));
	lvConfigureFade(color, num_frames);
	g_Vars.aioffset += 8;

	return false;
}

/**
 * @cmd 01cc
 */
bool aiIfFadeComplete(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;

	if (lvIsFadeActive() == false) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

/**
 * @cmd 01cd
 */
bool aiSetChrHudpieceVisible(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr && chr->prop && chr->model) {
		chrSetHudpieceVisible(chr, cmd[3]);
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 01ce
 */
bool aiSetPassiveMode(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	bgunSetPassiveMode(cmd[2]);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 01cf
 */
bool aiChrSetFiringInCutscene(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);
	struct coord from = {0, 0, 0};
	struct coord to = {0, 0, 0};

	if (chr && chr->weapons_held[HAND_RIGHT]) {
		if (cmd[3]) {
			chrSetFiring(chr, HAND_RIGHT, true);
			chrUpdateFireslot(chr, HAND_RIGHT, true, false, &from, &to);
		} else {
			chrSetFiring(chr, HAND_RIGHT, false);
		}
	}

	g_Vars.aioffset += 4;

	return false;
}

/**
 * @cmd 01d0
 */
bool aiSetPortalFlag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s16 portalnum = cmd[3] | (cmd[2] << 8);

	g_BgPortals[portalnum].flags |= cmd[4];
	g_Vars.aioffset += 5;

	return false;
}

/**
 * @cmd 01d1
 */
bool aiObjSetModelPartVisible(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct defaultobj *obj = objFindByTagId(cmd[2]);

	if (obj && obj->prop) {
		objSetModelPartVisible(obj, cmd[3], cmd[4]);
	}

	g_Vars.aioffset += 5;

	return false;
}

/**
 * @cmd 01d2
 */
bool aiChrEmitSparks(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr) {
		chrDrCarollEmitSparks(chr);
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 01d3
 */
bool aiSetDrCarollImages(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *drcaroll = chrFindById(g_Vars.chrdata, cmd[2]);

	if (drcaroll) {
		if (cmd[4] == 7) {
			if ((g_Vars.lvframenum % 4) == 2) {
				drcaroll->drcarollimage_left = random() % 6;
			}
		} else if (cmd[4] == 8) {
			drcaroll->drcarollimage_left = random() % 6;
		} else {
			drcaroll->drcarollimage_left = cmd[4];
		}

		if (cmd[3] == 7) {
			if ((g_Vars.lvframenum % 4) == 2) {
				drcaroll->drcarollimage_right = random() % 6;
			}
		} else if (cmd[3] == 8) {
			drcaroll->drcarollimage_right = random() % 6;
		} else {
			drcaroll->drcarollimage_right = cmd[3];
		}
	}

	g_Vars.aioffset += 5;

	return false;
}

/**
 * @cmd 01d4
 */
bool aiSetRoomFlag(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s16 roomnum = cmd[3] | cmd[2] << 8;
	s16 flag = cmd[5] | cmd[4] << 8;

	g_Rooms[roomnum].flags |= flag;

	g_Vars.aioffset += 6;

	return false;
}

/**
 * @cmd 01d5
 */
bool aiShowCutsceneChrs(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	bool show = cmd[2];
	s32 i;

	if (show) {
		for (i = chrsGetNumSlots() - 1; i >= 0; i--) {
			if (g_ChrSlots[i].chrnum >= 0 && g_ChrSlots[i].prop && (g_ChrSlots[i].hidden2 & CHRH2FLAG_HIDDENFORCUTSCENE)) {
				g_ChrSlots[i].hidden2 &= ~CHRH2FLAG_HIDDENFORCUTSCENE;
				g_ChrSlots[i].chrflags &= ~CHRCFLAG_HIDDEN;
			}
		}
	} else {
		for (i = chrsGetNumSlots() - 1; i >= 0; i--) {
			if (g_ChrSlots[i].chrnum >= 0 && g_ChrSlots[i].prop &&
					(g_ChrSlots[i].chrflags & (CHRCFLAG_UNPLAYABLE | CHRCFLAG_HIDDEN)) == 0) {
				g_ChrSlots[i].hidden2 |= CHRH2FLAG_HIDDENFORCUTSCENE;
				g_ChrSlots[i].chrflags |= CHRCFLAG_HIDDEN;
			}
		}
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 01d6
 */
bool aiConfigureEnvironment(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s16 room_id = (cmd[3] | (cmd[2] << 8));
	s32 value = cmd[5];
	s32 i;

	switch (cmd[4]) {
	case AIENVCMD_00:
		var8006ae18 = value;
		break;
	case AIENVCMD_01:
		var8006ae1c = value;
		break;
	case AIENVCMD_02:
		var8006ae20 = value;
		break;
	case AIENVCMD_03:
		var8006ae24 = value;
		break;
	case AIENVCMD_04:
		var8006ae28 = value;
		break;
	case AIENVCMD_ROOM_SETAMBIENT:
		g_Rooms[room_id].flags &= ~ROOMFLAG_PLAYAMBIENTTRACK;
		if (value) {
			g_Rooms[room_id].flags |= ROOMFLAG_PLAYAMBIENTTRACK;
		}
		break;
	case AIENVCMD_ROOM_SETOUTDOORS:
		g_Rooms[room_id].flags &= ~ROOMFLAG_OUTDOORS;
		if (value) {
			g_Rooms[room_id].flags |= ROOMFLAG_OUTDOORS;
		}
		break;
	case AIENVCMD_07:
		g_Rooms[room_id].unk4e_04 = value;
		break;
	case AIENVCMD_08:
		g_Rooms[room_id].unk4d = value;
		break;
	case AIENVCMD_SETAMBIENT:
		for (i = 1; i < g_Vars.roomcount; i++) {
			if (value) {
				g_Rooms[i].flags |= ROOMFLAG_PLAYAMBIENTTRACK;
			} else {
				g_Rooms[i].flags &= ~ROOMFLAG_PLAYAMBIENTTRACK;
			}
		}
		break;
	case AIENVCMD_PLAYNOSEDIVE:
		sndPlayNosedive(value);
		break;
	case AIENVCMD_TICKMUSICQUEUE:
		musicTickEvents();
		break;
	case AIENVCMD_ROOM_SETFAULTYLIGHTS:
		roomSetLightsFaulty(room_id, value);
		break;
	case AIENVCMD_STOPNOSEDIVE:
		sndStopNosedive();
		break;
	case AIENVCMD_PLAYUFOHUM:
		sndPlayUfo(value);
		break;
	case AIENVCMD_STOPUFOHUM:
		sndStopUfo();
		break;
	}

	g_Vars.aioffset += 6;

	return false;
}

/**
 * @cmd 01d7
 */
bool aiIfDistanceToTarget2LessThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 distance = (cmd[3] | (cmd[2] << 8)) * 10.0f;

	if (chrGetDistanceToTarget2(g_Vars.chrdata) < distance) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 01d8
 */
bool aiIfDistanceToTarget2GreaterThan(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	f32 distance = (cmd[3] | (cmd[2] << 8)) * 10.0f;

	if (chrGetDistanceToTarget2(g_Vars.chrdata) > distance) {
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[4]);
	} else {
		g_Vars.aioffset += 5;
	}

	return false;
}

/**
 * @cmd 01d9
 */
bool aiPlaySoundFromProp(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	s16 audio_id = cmd[5] | (cmd[4] << 8);
	s32 volumemaybe = cmd[7] | (cmd[6] << 8);
	u16 unk1 = cmd[10] | (cmd[10] << 8); // @bug: Using 10 twice
	s32 channel = (s8)cmd[2];
	s16 unk2 = cmd[8];
	struct defaultobj *obj = objFindByTagId(cmd[3]);

	audioPlayFromProp(channel, audio_id, volumemaybe, obj->prop, unk2, unk1);

	g_Vars.aioffset += 11;

	return false;
}

/**
 * @cmd 01da
 */
bool aiPlayTemporaryPrimaryTrack(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	musicStartTemporaryPrimary(cmd[2]);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 01db
 */
bool aiChrKill(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	struct chrdata *chr = chrFindById(g_Vars.chrdata, cmd[2]);

	if (chr) {
		chr->actiontype = ACT_DEAD;
		chr->act_dead.fadetimer60 = -1;
		chr->act_dead.fadenow = false;
		chr->act_dead.fadewheninvis = false;
		chr->act_dead.invistimer60 = 0;
		chr->act_dead.notifychrindex = 0;
		chr->sleep = 0;
		chr->chrflags |= CHRCFLAG_KEEPCORPSEKO | CHRCFLAG_PERIMDISABLEDTMP;
	}

	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 01dc
 */
bool aiRemoveWeaponFromInventory(void)
{
	u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
	invRemoveItemByNum(cmd[2]);
	g_Vars.aioffset += 3;

	return false;
}

/**
 * @cmd 01dd
 */
bool aiIfMusicEventQueueIsEmpty(void)
{
	f32 value = (u64)osGetCount() * 64 / 3000;

	if (g_MusicEventQueueLength) {
		g_Vars.aioffset += 4;
	} else {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[3]);
	}

	return false;
}

/**
 * @cmd 01de
 */
bool aiIfCoopMode(void)
{
	if (g_Vars.normmplayerisrunning == false && g_MissionConfig.iscoop) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}

#if VERSION >= VERSION_NTSC_1_0
/**
 * @cmd 01e0
 */
bool aiRemoveReferencesToChr(void)
{
	if (g_Vars.chrdata && g_Vars.chrdata->prop) {
		u32 index = g_Vars.chrdata->prop - g_Vars.props;
		chrClearReferences(index);
	}

	g_Vars.aioffset += 2;

	return false;
}
#endif

#if VERSION >= VERSION_NTSC_1_0
/**
 * @cmd 01b4
 */
bool ai01b4(void)
{
	if (g_Vars.chrdata && g_Vars.chrdata->prop &&
			chr0f01f264(g_Vars.chrdata, &g_Vars.chrdata->prop->pos, g_Vars.chrdata->prop->rooms, 0, false)) {
		u8 *cmd = g_Vars.ailist + g_Vars.aioffset;
		g_Vars.aioffset = chraiGoToLabel(g_Vars.ailist, g_Vars.aioffset, cmd[2]);
	} else {
		g_Vars.aioffset += 3;
	}

	return false;
}
#endif
