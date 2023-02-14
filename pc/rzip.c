#include <ultra64.h>
#include "inflate/inflate.h"
#include "types.h"

// TODO: understad why rzipInflate stack smashes if the scratch is too small
// TODO: PC, this was 0x4000. Too small again?
u8 s_scratch[0x8000]; 

s32 rzipInflate(void *src, void *dst, void *scratch)
{
    // Ignore the scratch pad, only use the local one
    // TODO: Is it thread safe though?
    return inflate1173(src, dst, (void*)s_scratch);
}

bool rzipIs1173(void *buffer)
{
    u8* buf = (u8*)buffer;
    
    if (buf[0] == 0x11 && buf[1] == 0x73)
    {
        return true;
    }

    return false;
}
