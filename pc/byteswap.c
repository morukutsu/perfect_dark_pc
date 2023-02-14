#include "byteswap.h"

//! Byte swap unsigned short
u16 swap_uint16(u16 val)
{
    return (val << 8) | (val >> 8 );
}

//! Byte swap short
s16 swap_int16(s16 val) 
{
    return (val << 8) | ((val >> 8) & 0xFF);
}

//! Byte swap unsigned int
u32 swap_uint32(u32 val)
{
    val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF ); 
    return (val << 16) | (val >> 16);
}

//! Byte swap int
s32 swap_int32(s32 val)
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF ); 
    return (val << 16) | ((val >> 16) & 0xFFFF);
}

f32 swap_f32(f32 val)
{
    u8* ptr = (u8*)&val;
    u8 tmp;

    // A B C D
    // D C B A

    tmp = ptr[3];
    ptr[3] = ptr[0];
    ptr[0] = tmp;

    tmp = ptr[2];
    ptr[2] = ptr[1];
    ptr[1] = tmp;

    return val;
}

f32 swap_pf32(f32* val)
{
    u8* ptr = (u8*)val;
    u8 tmp;

    // A B C D
    // D C B A

    tmp = ptr[3];
    ptr[3] = ptr[0];
    ptr[0] = tmp;

    tmp = ptr[2];
    ptr[2] = ptr[1];
    ptr[1] = tmp;

    return *val;
}

void swap_coord(struct coord *c)
{
    c->x = swap_pf32(&c->x);
    c->y = swap_pf32(&c->y);
    c->z = swap_pf32(&c->z);
}
