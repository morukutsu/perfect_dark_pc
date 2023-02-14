#include "native_functions.h"
#include "rom.h"
#include "va_list.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "print.h"
#include <signal.h>

char* g_romData = NULL;
const unsigned int romSize = 32 * 1024 * 1024;

void romInit()
{
    debugPrint(PC_DBG_FLAG_ROM, "rom init\n");

    FILE* f = fopen("../pd.ntsc-final.z64", "r");
    if (f) {
        g_romData = nativeMalloc(romSize);
        size_t read = fread(g_romData, romSize, 1, f);
        fclose(f);

        debugPrint(PC_DBG_FLAG_ROM, "    Loaded %d bytes\n", read * romSize);
    } else {
        debugPrint(PC_DBG_FLAG_ROM, "    Could not open rom file. Exiting.\n");
        debugPrint(PC_DBG_FLAG_ROM, "    ROM path is hardcoded in rom.c.\n");

        exit(1);
    }
}

/*
    Notes PC
    Some calling code will perform copy from ROM directly into memory to fill data structures
    However the N64 is big endian while the PC platform is usually little endian (like the M1 mac or Intel PC)
    So we have to be careful and do endianness conversions in most cases
*/
void romCopy(void* memaddr, u32 romaddr, u32 len)
{
    debugPrint(PC_DBG_FLAG_ROM, "Read %x bytes from ROM at %08x\n", len, romaddr);

    char* mem = (char*)memaddr;
    for (u32 i = 0; i < len; i++) 
    {
        if (romaddr + i < romSize) 
        {
            mem[i] = g_romData[romaddr + i];
        }
        else
        {
            // Read ouside of rom
            debugPrint(PC_DBG_FLAG_ROM, "Error: read outside of rom %08x\n", romaddr + 1);
            break;
        }
    }
}

void romEnd()
{
    if (g_romData != NULL)
    {
        nativeFree(g_romData);
    }
}
