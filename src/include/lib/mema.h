#ifndef _IN_LIB_MEMA_H
#define _IN_LIB_MEMA_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

void memaDefrag(void);
void memaInit(void);
void memaReset(void *ptr, u32 size);
void memaPrint(void);
void *memaAlloc(u32 size);
s64 memaGrow(s64 addr, u32 amount);
void _memaFree(s64 addr, s32 size);
void memaFree(void *addr, s32 size);
s32 memaGetLongestFree(void);
bool memaRealloc(s64 addr, u32 oldsize, u32 newsize);

#endif
