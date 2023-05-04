#include <ultra64.h>
#include "constants.h"
#include "bss.h"
#include "lib/args.h"
#include "lib/str.h"
#include "lib/rmon.h"
#include "string.h"
#include "data.h"
#include "types.h"

u32 g_ArgBuffer[30];

s32 argc = 1;

char *argv[17] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

/**
 * Split the given string into words and populate the argv array.
 */
char *argParseString(char *str)
{
	argv[0] = "";
	argc = 1;

	while (*str != '\0') {
		while (*str == ' ') {
			*str = '\0';
			str++;
		}

		argv[argc] = str;
		argc++;

		while (*str > ' ') {
			*str++;
		}
	}

	return str;
}

void argSetString(char *string)
{
	strcpy((char *) g_ArgBuffer, string);
	argParseString((char *) g_ArgBuffer);
}

bool argsParseDebugArgs(void)
{
#if 0
	u32 devaddr;
	u32 stack;
	s32 i;
	bool ret = false;

	devaddr = 0x1ffff00;

	if (rmonIsDisabled()) {
		g_ArgBuffer[0] = 0;
	} else {
		for (i = 0; i < 30; i++) {
			osPiReadIo(devaddr, &g_ArgBuffer[i]);
			devaddr += 4;
		}
	}

	argParseString((char *) g_ArgBuffer);

	// I'm guessing the -d stands for debug. If set at boot, the main thread
	// stops itself immediately after creating the rmon thread.
	if (argFindByPrefix(1, "-d")) {
		ret = true;
	}

	if (argFindByPrefix(1, "-s")) {
		g_SndDisabled = true;
	}

#if VERSION < VERSION_JPN_FINAL
	if (argFindByPrefix(1, "-j")) {
		g_Jpn = true;
	}
#endif

	return ret;
#endif
	return false;
}

/**
 * Find a program argument based on its prefix and occurrence of that prefix,
 * and return a pointer to whatever comes after the prefix.
 *
 * Typically called with arguments such as 1, "-hard". In this example, if the
 * argument "-hard2" exists then it would return a pointer to "2".
 */
char *argFindByPrefix(s32 occurrence, char *str)
{
	s32 len = strlen(str);
	s32 i;

	for (i = 1; i < argc; i++) {
		if (strncmp(str, argv[i], (size_t)len) == 0) {
			occurrence--;

			if (occurrence == 0) {
				return &argv[i][len];
			}
		}
	}

	return NULL;
}

void argGetLevel(s32 *stagenum)
{
	char *ptr = argFindByPrefix(1, "-level_");

	if (ptr) {
		*stagenum = ptr[0] * 10 + ptr[1] - 528;
	}
}
