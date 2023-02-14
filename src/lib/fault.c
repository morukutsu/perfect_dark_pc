#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "lib/crash.h"
#include "lib/rmon.h"
#include "data.h"
#include "types.h"

#define PC_PORT_FAULT_DISABLED

void faultInit(void)
{
#ifndef PC_PORT_FAULT_DISABLED
	crashCreateThread();
#endif
}

void faultAssert(char *message, char *file, s32 line)
{
#ifndef PC_PORT_FAULT_DISABLED
	rmonPrintf("ASSERTION : %s, %d: (\"%s\")\n", file, line, message);
#endif
}
