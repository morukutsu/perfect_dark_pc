#ifndef _IN_GAME_GAME_166E40_H
#define _IN_GAME_GAME_166E40_H
#include <ultra64.h>
#include "data.h"
#include "types.h"

romptr_t fileGetRomAddress(s32 filenum);
s32 fileGetRomSize(s32 filenum);
void filesInit(void);
void fileLoadPartToAddr(u16 filenum, void *memaddr, s32 offset, u32 len);
u32 fileGetInflatedSize(s32 filenum);
void *fileLoadToNew(s32 filenum, u32 method);
void *fileLoadToAddr(s32 filenum, s32 method, u8 *ptr, u32 size);
u32 fileGetLoadedSize(s32 filenum);
u32 fileGetAllocationSize(s32 filenum);
void fileSetSize(s32 filenum, void *ptr, u32 size, bool reallocate);
void fileSetLoadedsize(s32 filenum, u32 size);
void filesStop(u8 arg0);
void func0f167330(void);
void fileLoad(u8 *dst, u32 allocationlen, u32 *romaddrptr, struct fileinfo *info);

#endif
