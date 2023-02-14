#include "gbi_addon.h"
#include "print.h"
#include "hashmap.h"
#include "gfxpc.h"
#include "gbi.h"
#include <signal.h>

extern struct hashmap *gfxpcExtradataMap;

void gDma1p_addon(void* cmd, unsigned int op, void* dl, unsigned int a, unsigned int b, char* dbgFile, int dbgLine)
{
    struct cmd_extradata ext = {
        cmd, 
        dl,
        dbgFile,
        dbgLine
    };

    hashmap_set(gfxpcExtradataMap, &ext);
}

void gAddr_addon(void* cmd, void* addr, char* dbgFile, int dbgLine)
{
    struct cmd_extradata ext = {
        cmd, 
        addr,
        dbgFile,
        dbgLine
    };

    hashmap_set(gfxpcExtradataMap, &ext);
}