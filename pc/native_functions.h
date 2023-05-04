#ifndef _H_NATIVE_FUNCTIONS
#define _H_NATIVE_FUNCTIONS

#define nativeMalloc(n)   __builtin_malloc ((n))
#define nativeFree(n)     __builtin_free ((n))
//#define memset(a, b, c)   __builtin_memset((a), (b), (c))

#endif