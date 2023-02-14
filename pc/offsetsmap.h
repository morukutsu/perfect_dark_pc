#ifndef _H_PC_OFFSETS_MAP
#define _H_PC_OFFSETS_MAP

#include <ultra64.h>

struct offset_pc {
	u32 old, new;
	u32 size;
};

void initOffsetsContext();
void clearOffsetsContext();
void* replaceOffsetGlobal(u32 old);
void addOffsetGlobal(u32 old, u32 new, u32 size);

#endif
