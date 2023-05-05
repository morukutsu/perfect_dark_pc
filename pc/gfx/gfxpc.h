#ifndef _H_PC_GFX
#define _H_PC_GFX

#include <ultra64.h>
#include <stdint.h>

struct cmd_extradata {
    void* cmdPtr;
    void* memPtr;
    char* file;
    int line;
};

struct Texture {
    // Hashed fields
    void* addr;
    uint32_t fmt, siz;

    // Non hashed fields
    uint32_t width, height;
    uint32_t texture;
};

void gfx_start();
void gfx_execute_commands(Gfx* gdl, Gfx* gdlEnd);
void gfx_pc_init();
void gfx_extradata_clear();
void* debug_iter_texture_cache(size_t* iter, void** item);


#endif