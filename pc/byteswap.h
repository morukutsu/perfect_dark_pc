#ifndef _H_PC_BYTESWAP
#define _H_PC_BYTESWAP

#include <ultra64.h>
#include "types.h"

u16 swap_uint16(u16 val);
s16 swap_int16(s16 val);
u32 swap_uint32(u32 val);
s32 swap_int32(s32 val);
f32 swap_f32(f32 val);
f32 swap_pf32(f32* val);

// other
void swap_coord(struct coord *c);


#endif
