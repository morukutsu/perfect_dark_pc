#pragma once

//#include "game/modeldef.h"

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void AssetLoadFileTable();
void AssetConvertModeldef(uint16_t fileid);

#ifdef __cplusplus
}
#endif