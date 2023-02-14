#ifndef _ULTRA64_STUB_H_
#define _ULTRA64_STUB_H_

#if 0
// types, to move somewhere cleaner
typedef unsigned char u8;           /* unsigned  8-bit */
typedef unsigned short int u16;     /* unsigned 16-bit */
typedef unsigned int u32;           /* unsigned 32-bit */
typedef unsigned long long int u64; /* unsigned 64-bit */

typedef signed char s8;           /* signed  8-bit */
typedef signed short int s16;     /* signed 16-bit */
typedef signed int s32;           /* signed 32-bit */
typedef signed long long int s64; /* signed 64-bit */

typedef volatile unsigned char vu8;       /* unsigned  8-bit */
typedef volatile unsigned short vu16;     /* unsigned 16-bit */
typedef volatile unsigned long vu32;      /* unsigned 32-bit */
typedef volatile unsigned long long vu64; /* unsigned 64-bit */

typedef volatile signed char vs8; /* signed  8-bit */
typedef volatile short vs16;      /* signed 16-bit */
typedef volatile long vs32;       /* signed 32-bit */
typedef volatile long long vs64;  /* signed 64-bit */

typedef float f32;  /* single prec floating point */
typedef double f64; /* double prec floating point */

#define bool s32
#define ubool u32
#define intptr_t s32
#define uintptr_t u32
#define romptr_t u32

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

#include <PR/ultratypes.h>
#include <PR/rcp.h>
#include <PR/os.h>
#include <PR/region.h>
#include <PR/rmon.h>
#include <PR/sptask.h>
#include <PR/mbi.h>
#include <PR/libaudio.h>
#include <PR/gu.h>
#include <PR/ramrom.h>
#include <PR/sp.h>
#include <PR/ucode.h>
#include <PR/ultraerror.h>
#include <PR/ultralog.h>

#endif

#endif
