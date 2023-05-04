#ifndef _H_PC_PRINT
#define _H_PC_PRINT

#define PC_DBG_FLAG_ALL    0xffffffff
#define PC_DBG_FLAG_ROM    0x00000001
#define PC_DBG_FLAG_GFX    0x00000002
#define PC_DBG_FLAG_GAME   0x00000004
#define PC_DBG_FLAG_FILE   0x00000008
#define PC_DBG_FLAG_FILE   0x00000008
#define PC_DBG_FLAG_TEX    0x00000010
#define PC_DBG_FLAG_MODEL  0x00000020
#define PC_DBG_FLAG_BG     0x00000040

void print(const char *format, ...);
void hexdump(void* addr, int len);

void debugSetFlags(unsigned int flags);
void debugPrint(unsigned int flags, const char *format, ...);

void fatalExit();

#endif