#ifndef _H_PC_GBI_ADDON
#define _H_PC_GBI_ADDON

void gDma1p_addon(void* cmd, unsigned int op, void* dl, unsigned int a, unsigned int b, char* dbgFile, int dbgLine);	
void gAddr_addon(void* cmd, void* addr, char* dbgFile, int dbgLine);	

#endif