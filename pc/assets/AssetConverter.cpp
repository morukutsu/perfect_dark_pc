#include "AssetConverter.h"

#include <vector>

#include "game/modeldef.h"
#include "game/file.h"
#include "lib/dma.h"

extern "C" {
    #include "lib/rzip.h"
    #include "rom.h"
    #include "print.h"
    #include "byteswap.h"
};

struct PDFile
{
    uint32_t fileRomAddress;
    size_t fileSize;
};

enum PDVersion
{
    PD_VERSION_NTSC_BETA = 0,
    PD_VERSION_NTSC_1_0,
    PD_VERSION_NTSC_FINAL,
    PD_VERSION_PAL_BETA,
    PD_VERSION_PAL_FINAL,
    PD_VERSION_JPN_FINAL,
    PD_VERSION_COUNT
};

struct PDRomInfo
{
    uint32_t filesIndexRomAddress;  // addr of the files index, relative to the files data segment
    uint32_t filesDataRomAddress;   // addr of the files data segment in the ROM
};

/*
     #                 ntsc-beta  ntsc-1.0   ntsc-final pal-beta   pal-final  jpn-final
    'files':           [0x29160,   0x28080,   0x28080,   0x29b90,   0x28910,   0x28800,   ],
    'data':            [0x30850,   0x39850,   0x39850,   0x39850,   0x39850,   0x39850,   ],
*/

PDRomInfo s_PDRomInfo[PD_VERSION_COUNT];
std::vector<PDFile> s_PDFilesTable;

/*
    Loads the original file table from the ROM into s_PDFilesTable
    This file table will be used by the asset converter to convert assets
*/
void AssetLoadFileTable()
{
	debugPrint(PC_DBG_FLAG_FILE, "AssetLoadFileTable(): loading files table from ROM\n");

    s_PDRomInfo[PD_VERSION_NTSC_BETA]  = { 0x29160, 0x30850 };
    s_PDRomInfo[PD_VERSION_NTSC_1_0]   = { 0x28080, 0x39850 };
    s_PDRomInfo[PD_VERSION_NTSC_FINAL] = { 0x28080, 0x39850 };
    s_PDRomInfo[PD_VERSION_PAL_BETA]   = { 0x29b90, 0x39850 };
    s_PDRomInfo[PD_VERSION_PAL_FINAL]  = { 0x28910, 0x39850 };
    s_PDRomInfo[PD_VERSION_JPN_FINAL]  = { 0x28800, 0x39850 };

    PDVersion romVersion = PD_VERSION_NTSC_FINAL;
    PDRomInfo& rom = s_PDRomInfo[romVersion];

    const size_t temporaryBufferSize = 1 * 1024 * 1024;
    uint8_t* data = new uint8_t[temporaryBufferSize];
    uint8_t* inflatedData = new uint8_t[temporaryBufferSize];

    // Will copy temporaryBufferSize bytes from the ROM. It's larger than the actual size of the file index
	romCopy(data, rom.filesDataRomAddress, temporaryBufferSize);
    
    // Unzip data segment
	size_t filesIndexDataLength = rzipInflate(data, inflatedData, 0);
	debugPrint(PC_DBG_FLAG_FILE, "filesIndexDataLength: %x bytes\n", filesIndexDataLength);

    // Find files in uncompressed data segment
    size_t filesCount = 0;;

    uint32_t* filesAddresses = (uint32_t*)(inflatedData + rom.filesIndexRomAddress);

    s_PDFilesTable.reserve(NUM_FILES);
	for (uint32_t i = 0; i <= NUM_FILES; i++) {
        uint32_t fileRomOffset = swap_uint32(filesAddresses[i]);

        PDFile file = {
            fileRomOffset,
            0
        };

        s_PDFilesTable.push_back(file);

        // Set the size of the previous file
        // To calculate the size of a file, we need the address of the next one
        if (i > 0)
        {
            s_PDFilesTable[i - 1].fileSize = s_PDFilesTable[i].fileRomAddress - s_PDFilesTable[i - 1].fileRomAddress;
        }

		if ((fileRomOffset == 0 && filesCount != 0) || i == NUM_FILES) {
			debugPrint(PC_DBG_FLAG_FILE, "loaded %d files\n", filesCount);
			break;
		}

		filesCount++;
	}

    delete[] data;
    delete[] inflatedData;
}


void AssetLoadFile(uint16_t fileid)
{

}

void AssetConvertModeldef(uint16_t fileid)
{
}
