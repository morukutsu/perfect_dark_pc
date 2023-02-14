#ifndef _H_PC_GFX
#define _H_PC_GFX

#include <ultra64.h>

struct cmd_extradata {
    void* cmdPtr;
    void* memPtr;
    char* file;
    int line;
};

void gfx_start();
void gfx_execute_commands(Gfx* gdl, Gfx* gdlEnd);
void gfx_pc_init();
void gfx_extradata_clear();

#endif