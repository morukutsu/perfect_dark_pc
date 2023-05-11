#pragma once

//#include "game/modeldef.h"

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void AssetLoadFileTable();

void AssetLoadFileToAddr(uint16_t fileid, void* dst, size_t allocationSize);
//void AssetConvertModeldef(uint16_t fileid);
size_t AssetConvertModeldef(uint16_t fileid, uint8_t* src, size_t fileSize);
//void AssetConvertModeldefAtAddr(void* dst, size_t allocationSize, size_t originalFileSize);


#ifdef __cplusplus
}
#endif