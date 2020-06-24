#ifndef _IN_GAME_CHR_CHR_H
#define _IN_GAME_CHR_CHR_H
#include <ultra64.h>
#include "types.h"

void func0f01e250(void);
void chrSetChrnum(struct chrdata *chr, s16 chrnum);
void chrRemoveFromChrsC(s32 chrnum);
u32 func0f01e7f4(void);
u32 func0f01f264(struct chrdata *chr, struct coord *pos, s16 *room, f32 arg3, bool arg4);
void func0f01f378(void);
s32 chrsGetNumFree(void);
s16 getLowestUnusedChrId(void);
struct prop *func0f020b14(struct prop *prop, struct model *model, struct coord *pos, s16 *rooms, f32 arg3, u8 *ailist);
void func0f020d44(struct prop *prop, bool arg1);
u32 func0f021258(void);
void func0f02133c(struct chrdata *chr);
void chrDoFlinchcntThing(struct chrdata *chr, f32 arg1);
u32 func0f02143c(void);
void func0f021fa8(struct chrdata *chr, struct coord *pos, s16 *rooms);
void func0f0220ec(struct chrdata *chr, s32 arg1, s32 arg2);
u32 func0f022214(void);
void chrUpdateCloak(struct chrdata *chr);
u32 func0f022be4(void);
void chrSetPoisoned(struct chrdata *chr, void *arg1);
u32 func0f022d60(void);
bool propchrDoFireslotThing(struct prop *prop);
u32 func0f023098(void);
void chrDropItems(struct chrdata *chr);
void chrSetHudpieceVisible(struct chrdata *chr, bool visible);
void chrDropWeapons(struct chrdata *chr);
void func0f0246e4(u8 *arg0);
u32 func0f024738(void);
u32 func0f024b18(void);
u32 func0f025074(void);
void bodyGetBloodColour(s16 bodynum, u8 *colour1, u32 *colour2);
u32 func0f025288(void);
void func0f025ea4(struct chrdata *chr, struct prop *prop, s32 arg2, struct coord *coord, struct coord *coord2, struct chrdata *chr2);
u32 func0f0260c4(void);
u32 func0f0268bc(void);
u32 func0f0270f4(void);
f32 func0f0278a4(struct chrdata *chr);
u32 func0f027994(void);
u32 func0f027e1c(void);
u32 func0f028498(void);
void func0f028590(f32 arg0);
u32 func0f028a50(void);
u32 func0f028d50(void);
u32 func0f028e18(void);
u32 func0f028e6c(void);
u32 func0f028f7c(void);
u32 func0f0291d4(void);
u32 func0f0292bc(void);
u32 func0f02932c(void);
u32 func0f0293ec(void);
u32 func0f0294cc(void);
u32 func0f0295f8(void);
f32 propGetShieldThing(struct prop **propptr);
u32 func0f02983c(void);
u32 func0f02b7d4(void);
u32 func0f02bdf8(void);
u32 func0f02c6e0(void);
u32 func0f02c9b0(void);
u32 func0f02ccb4(void);
s32 getNumChrs(void);
void chrInsertToChrsC(s32 chrnum, s32 chrindex);
void func0f01e760(s32 arg0);
void setVar8006297c(u32 arg0);
u32 getVar8006297c(void);
void setVar80062980(u32 arg0);
u32 getVar80062980(void);
void chrSetOrUnsetHiddenFlag00000100(struct chrdata *chr, bool unset);
void chrSetMaxDamage(struct chrdata *chr, f32 maxdamage);
f32 chrGetMaxDamage(struct chrdata *chr);
void chrAddHealth(struct chrdata *chr, f32 health);
f32 chrGetArmor(struct chrdata *chr);
void chrInit(struct prop *prop, u8 *ailist);
struct prop *propAllocateChr(struct model *model, struct coord *pos, s16 *rooms, f32 arg3, u8 *ailist);
void propClearReferences(s32 propnum);
void func0f022084(struct chrdata *chr, s16 *room);
void func0f0220ac(struct chrdata *chr);
void chrCloak(struct chrdata *chr, bool arg1);
void chrUncloak(struct chrdata *chr, bool value);
void chrUncloakTemporarily(struct chrdata *chr);
void func0f02472c(void);
void func0f028490(f32 arg1);
struct chrdata *chrFindByLiteralId(s32 chrnum);
struct prop *chrGetEquippedWeaponProp(struct chrdata *chr, s32 hand);
struct prop *chrGetEquippedWeaponPropWithCheck(struct chrdata *chr, s32 hand);
struct prop *chrGetTargetProp(struct chrdata *chr);
bool chrUpdateGeometry(struct prop *prop, struct geo **arg1, struct geo **arg2);
void propChrGetBbox(struct prop *prop, f32 *width, f32 *ymax, f32 *ymin);

#endif
