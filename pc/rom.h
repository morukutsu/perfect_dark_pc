#ifndef _H_PC_ROM
#define _H_PC_ROM

#include <ultra64.h>

extern char* g_romData;

void romInit();
void romEnd();
void romCopy(void* memaddr, u32 romaddr, u32 len);
void enableRomCopyLogs();
void disableRomCopyLogs();

#endif