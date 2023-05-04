#include "va_list.h"
#include "print.h"
#include <stdio.h>
#include <stdlib.h>

unsigned int g_pcDebugFlags = 0;

void print(const char *format, ...) 
{
    va_list ap;
	va_start(ap, format);

	vprintf(format, ap);
	fflush(stdout);
}

void hexdump(void* addr, int len)
{
	char* data = (char*)addr;

	for (int i = 0; i < len; i++) {
		if (i != 0 && i % 8 == 0) {
			print("\n");
		}

		print("%02x ", data[i] & 0xFF);
	}
	print("\n");
}

void debugSetFlags(unsigned int flags)
{
	g_pcDebugFlags = flags;
}

char* debugGetFlagName(unsigned int flag)
{
	if (flag & PC_DBG_FLAG_GFX)
		return "GFX";
	else if (flag & PC_DBG_FLAG_ROM)
		return "ROM";
	else if (flag & PC_DBG_FLAG_GAME)
		return "GAME";
	else if (flag & PC_DBG_FLAG_FILE)
		return "FILE";
	else if (flag & PC_DBG_FLAG_TEX)
		return "TEX";
	else if (flag & PC_DBG_FLAG_MODEL)
		return "MODEL";
	else if (flag & PC_DBG_FLAG_BG)
		return "BG";
		
	return "";
}

void debugPrint(unsigned int flag, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);

	if (g_pcDebugFlags & flag) {
		va_list ap;
		va_start(ap, format);

		print("[%s] ", debugGetFlagName(flag));
		vprintf(format, ap);
		fflush(stdout);
	}
}

// TODO: move this function somwhere else
void fatalExit()
{
	exit(1);
}
