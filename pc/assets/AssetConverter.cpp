#include "AssetConverter.h"

#include <vector>
#include <string>
#include <utility>
#include <map>
#include <set>
#include <assert.h>

#include "game/modeldef.h"
#include "lib/dma.h"

extern "C" {
	#include "game/texdecompress.h"
	#include "game/file.h"
	#include "lib/rzip.h"
	#include "rom.h"
	#include "print.h"
	#include "byteswap.h"
};

/*
	Perfect Dark file descriptor, this structure is populated from the rom
	in AssetLoadFileTable()
*/
struct PDFile
{
	uint32_t fileRomAddress;
	size_t fileSize;
	std::string filename;
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

/*
	Store the address of the file index and data segment for each ROM version
*/
struct PDRomInfo
{
	uint32_t filesIndexAddress; // addr of the files index, relative to the inflated files data segment
	uint32_t filesDataRomAddress; // addr of the files data segment in the ROM
};

/*
	 #                 ntsc-beta  ntsc-1.0   ntsc-final pal-beta   pal-final  jpn-final
	'files':           [0x29160,   0x28080,   0x28080,   0x29b90,   0x28910,   0x28800,   ],
	'data':            [0x30850,   0x39850,   0x39850,   0x39850,   0x39850,   0x39850,   ],
*/

PDRomInfo s_PDRomInfo[PD_VERSION_COUNT];
std::vector<PDFile> s_PDFilesTable;

const size_t FILELOAD_BUFFER_SIZE = 1 * 1024 * 1024;
char s_fileLoadBuffer[FILELOAD_BUFFER_SIZE];
char s_fileInflateBuffer[FILELOAD_BUFFER_SIZE];

/*
	Loads the original file table from the ROM into s_PDFilesTable
	This file table will be used by the asset converter 
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

	// Copy temporaryBufferSize bytes from the ROM (It's larger than the actual size of the file index)
	// TODO: find the size of each segment and use that size instead
	romCopy(data, rom.filesDataRomAddress, temporaryBufferSize);
	
	// Unzip data segment into a temporary buffer
	size_t filesIndexDataLength = rzipInflate(data, inflatedData, 0);
	debugPrint(PC_DBG_FLAG_FILE, "filesIndexDataLength: %x bytes\n", filesIndexDataLength);

	// Read the file index from the inflated data segment
	size_t filesCount = 0;

	uint32_t* filesAddresses = (uint32_t*)(inflatedData + rom.filesIndexAddress);

	s_PDFilesTable.reserve(NUM_FILES);
	for (uint32_t i = 0; i <= NUM_FILES; i++) 
	{
		uint32_t fileRomOffset = swap_uint32(filesAddresses[i]);

		PDFile file = {
			fileRomOffset,
			0,
			""
		};

		s_PDFilesTable.push_back(file);

		// Set the size of the previous file
		// To calculate the size of a file, we need the address of the next one
		// Not possible to calculate the size of the last file
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

	// The address list of file names index is the last file in the table
	uint32_t filesNamesTableRomAddress = s_PDFilesTable.back().fileRomAddress;
	debugPrint(PC_DBG_FLAG_FILE, "filesNamesTableRomAddress: %x\n", filesNamesTableRomAddress);

	// This offset is relative to the begining of the ROM
	// Here we index the ROM directly
	uint32_t* filesNamesAddresses = (uint32_t*)(g_romData + filesNamesTableRomAddress);

	for (uint32_t i = 0; i <= NUM_FILES; i++) 
	{
		uint32_t filenameOffset = swap_uint32(filesNamesAddresses[i]);
		const char* filename = (const char*)(g_romData + filesNamesTableRomAddress + filenameOffset);

		size_t length = strlen(filename);
		s_PDFilesTable[i].filename = std::string(filename);
	}

	delete[] data;
	delete[] inflatedData;
}

/*
	Load a file into a new memory buffer
	The caller is responsible for destroying it
*/
std::pair<uint8_t*, size_t> AssetLoadFile(uint16_t fileid)
{
	PDFile& fileDescriptor = s_PDFilesTable[fileid];

	assert(fileDescriptor.fileSize <= FILELOAD_BUFFER_SIZE);

	// Load the file (and decompress it) into a temporary buffer
	romCopy((void*)s_fileLoadBuffer, fileDescriptor.fileRomAddress, fileDescriptor.fileSize);
	size_t inflatedSize = rzipInflate(s_fileLoadBuffer, s_fileInflateBuffer, NULL);

	assert(inflatedSize <= FILELOAD_BUFFER_SIZE);

	// Allocate a buffer of the loaded size and return it
	uint8_t* data = new uint8_t[inflatedSize];
	memcpy(data, s_fileInflateBuffer, inflatedSize);

	debugPrint(PC_DBG_FLAG_FILE, "Loaded file %s - rom size 0x%x bytes - loaded size 0x%x bytes\n", 
		fileDescriptor.filename.c_str(),
		fileDescriptor.fileSize,
		inflatedSize
	);

	return std::make_pair(data, inflatedSize);
}

void AssetLoadFileToAddr(uint16_t fileid, void* dst, size_t allocationSize)
{
	std::pair<uint8_t*, size_t> fileData = AssetLoadFile(fileid);
	assert(allocationSize > fileData.second);

	memcpy(dst, fileData.first, fileData.second);
	delete fileData.first;
}

/*
	Recursively traverse modelnodes from a modeldef file and output a list of modelnode offsets
	The modelnode offsets are used to retrieve each node in the original N64 format
*/
void _traverseModelnodesLoad(std::vector<uint32_t>& offsets, void* base, uint32_t offset)
{
	if (offset == 0) return;

	// Avoid nodes already in the list
	for (auto& it : offsets)
		if (it == offset) return;

	offsets.push_back(offset);

	struct modelnode_load* node = (struct modelnode_load*)((uintptr_t)base + offset);

	uint32_t child = swap_uint32(node->child);
	if (child) 
		_traverseModelnodesLoad(offsets, base, child);

	uint32_t next = swap_uint32(node->next);
	while (next) 
	{
		_traverseModelnodesLoad(offsets, base, next);

		node = (struct modelnode_load*)((uintptr_t)base + next);
		next = swap_uint32(node->next);
	}
}

const size_t PD_ASSET_CONVERT_BUFFER_SIZE = 1 * 1024 * 1024;
uint8_t* s_convertBuffer[PD_ASSET_CONVERT_BUFFER_SIZE];

#define MODEL_VTX_SECTION_MAX_COUNT 128

struct model_vtx_section {
	u32 start, size;
};

class FileWriteOffset
{
	public:
		FileWriteOffset(void* addr) { baseAddress = addr; }
		size_t get() { return offset; }
		void increment(size_t size) { offset += size; }

		void* getDataPtrAt(size_t off) { return (void*)((uintptr_t)baseAddress + (uintptr_t)off); }
		void* getDataPtrCurrent() { return (void*)((uintptr_t)baseAddress + (uintptr_t)offset); }

	public:
		size_t offset = 0;
		void* baseAddress;
};

/*
	Convert N64 modeldef files the PC format
*/
size_t AssetConvertModeldef(uint16_t fileid, uint8_t* src, size_t fileSize)
{
	debugPrint(PC_DBG_FLAG_FILE, "AssetConvertModeldef: fileid %x\n", fileid);
	//std::pair<uint8_t*, size_t> fileData = AssetLoadFile(fileid);
	std::pair<uint8_t*, size_t> fileData = std::make_pair(src, fileSize);

	/*
		When converting a file, it's possible that the file becomes larger than the original N64 file
		We convert the files into a large buffer, and at the end of the conversion we can calculate the final file size
	*/
	struct modeldef_load* modeldef_load = (struct modeldef_load*)fileData.first;
	struct modeldef* modeldef = (struct modeldef*)s_convertBuffer;
	FileWriteOffset fileWriteOffset(s_convertBuffer);

	// To cleanup
	u32 vtxSectionsCount = 0;
	struct model_vtx_section vtxSectionsSrc[MODEL_VTX_SECTION_MAX_COUNT];
	struct model_vtx_section vtxSectionsDst[MODEL_VTX_SECTION_MAX_COUNT];

	modeldef->rootnode = 0;
	modeldef->skel = (struct skeleton*)(uintptr_t)swap_uint32(modeldef_load->skel);
	modeldef->parts = (struct modelnode**)NULL;
	modeldef->numparts = swap_int16(modeldef_load->numparts);
	modeldef->nummatrices = swap_int16(modeldef_load->nummatrices);
	modeldef->scale = swap_f32(modeldef_load->scale);
	modeldef->rwdatalen = swap_int16(modeldef_load->rwdatalen);
	modeldef->numtexconfigs = swap_int16(modeldef_load->numtexconfigs);
	modeldef->texconfigs = 0;

	fileWriteOffset.increment(sizeof (struct modeldef));

	debugPrint(PC_DBG_FLAG_MODEL, "modeldef %d\n", fileid);
	debugPrint(PC_DBG_FLAG_MODEL, "- numparts %d\n", modeldef->numparts);
	debugPrint(PC_DBG_FLAG_MODEL, "- nummatrices %d\n", modeldef->nummatrices);
	debugPrint(PC_DBG_FLAG_MODEL, "- scale %f\n", modeldef->scale);
	debugPrint(PC_DBG_FLAG_MODEL, "- rwdatalen %d\n", modeldef->rwdatalen);
	debugPrint(PC_DBG_FLAG_MODEL, "- numtexconfigs %d\n", modeldef->numtexconfigs);
	debugPrint(PC_DBG_FLAG_MODEL, "- textconfigs offset %x\n", swap_uint32(modeldef_load->texconfigs));

	// List all the modelnodes in the file
	const uint32_t vmaSectionOffset = 0x5000000;
	uint32_t rootNodeOffset = swap_uint32(modeldef_load->rootnode);

	void* nodesBaseAddress = (void*)((uintptr_t)modeldef_load - (uintptr_t)vmaSectionOffset);

	std::vector<uint32_t> modelnodesOffsets;
	_traverseModelnodesLoad(modelnodesOffsets, nodesBaseAddress, rootNodeOffset);

	std::map<uint32_t, uint32_t> origToNewOffsetMap;
	std::set<uint32_t> gdlOffsets;

	// Traverse the list of nodes and write them into the final file
	std::vector<struct modelnode> nodes;
	for (auto& offset : modelnodesOffsets)
	{
		struct modelnode_load* nodeLoad = (struct modelnode_load*)((uintptr_t)nodesBaseAddress + (uintptr_t)offset);
		struct modelnode node = 
		{
			swap_uint16(nodeLoad->type),
			(union modelrodata*)(uintptr_t)swap_uint32(nodeLoad->rodata),
			(struct modelnode*)(uintptr_t)swap_uint32(nodeLoad->parent),
			(struct modelnode*)(uintptr_t)swap_uint32(nodeLoad->next),
			(struct modelnode*)(uintptr_t)swap_uint32(nodeLoad->prev),
			(struct modelnode*)(uintptr_t)swap_uint32(nodeLoad->child),
		};

		assert(node.type > 0);

		origToNewOffsetMap[offset] = fileWriteOffset.get();

		debugPrint(PC_DBG_FLAG_MODEL, "modelnode written: %x -> %x\n", offset, fileWriteOffset.get());

		memcpy(fileWriteOffset.getDataPtrCurrent(), &node, sizeof(struct modelnode));
		fileWriteOffset.increment(sizeof(struct modelnode));
	}

	// Traverse the nodes again and convert each one
	for (auto& offset : modelnodesOffsets)
	{
		uint32_t newOffset = origToNewOffsetMap[offset];
		struct modelnode* node = (struct modelnode*)fileWriteOffset.getDataPtrAt(newOffset);
		assert(node->type > 0);

		debugPrint(PC_DBG_FLAG_MODEL, "modelnode ptr: %llx type: %x\n", node, node->type);

		node->child = (struct modelnode*)(uintptr_t)origToNewOffsetMap[(uintptr_t)node->child];
		node->parent = (struct modelnode*)(uintptr_t)origToNewOffsetMap[(uintptr_t)node->parent];
		node->next = (struct modelnode*)(uintptr_t)origToNewOffsetMap[(uintptr_t)node->next];
		node->prev = (struct modelnode*)(uintptr_t)origToNewOffsetMap[(uintptr_t)node->prev];

		// And load the rodata, convert it if necessary
		uint8_t type = node->type & 0xff;
		switch(type)
		{
			case MODELNODETYPE_POSITION: {
				struct modelrodata_position* rodataSrc = 
					(struct modelrodata_position*)((uintptr_t)modeldef_load - vmaSectionOffset + (uintptr_t)node->rodata);
				
				struct modelrodata_position* rodataDst = (struct modelrodata_position*)fileWriteOffset.getDataPtrCurrent();

				rodataDst->pos = rodataSrc->pos;
				swap_coord(&rodataDst->pos);
				rodataDst->part = swap_uint16(rodataSrc->part);
				rodataDst->mtxindex0 = swap_int16(rodataSrc->mtxindex0);
				rodataDst->mtxindex1 = swap_int16(rodataSrc->mtxindex1);
				rodataDst->mtxindex2 = swap_int16(rodataSrc->mtxindex2);
				rodataDst->drawdist = swap_f32(rodataSrc->drawdist);

				origToNewOffsetMap[(uintptr_t)node->rodata] = fileWriteOffset.get();

				fileWriteOffset.increment(sizeof(struct modelrodata_position));
				break;
			}

			case MODELNODETYPE_BBOX: {
				struct modelrodata_bbox* rodataSrc = 
					(struct modelrodata_bbox*)((uintptr_t)modeldef_load - vmaSectionOffset + (uintptr_t)node->rodata);
				
				struct modelrodata_bbox* rodataDst = (struct modelrodata_bbox*)fileWriteOffset.getDataPtrCurrent();

				rodataDst->hitpart = swap_int32(rodataSrc->hitpart);
				rodataDst->xmin = swap_f32(rodataSrc->xmin);
				rodataDst->xmax = swap_f32(rodataSrc->xmax);
				rodataDst->ymin = swap_f32(rodataSrc->ymin);
				rodataDst->ymax = swap_f32(rodataSrc->ymax);
				rodataDst->zmin = swap_f32(rodataSrc->zmin);
				rodataDst->zmax = swap_f32(rodataSrc->zmax);

				origToNewOffsetMap[(uintptr_t)node->rodata] = fileWriteOffset.get();

				fileWriteOffset.increment(sizeof(struct modelrodata_bbox));
				break;
			}

			case MODELNODETYPE_TOGGLE: {
				struct modelrodata_toggle_load* rodataSrc = 
					(struct modelrodata_toggle_load*)((uintptr_t)modeldef_load - vmaSectionOffset + (uintptr_t)node->rodata);
				
				struct modelrodata_toggle* rodataDst = (struct modelrodata_toggle*)fileWriteOffset.getDataPtrCurrent();

				rodataDst->target = (struct modelnode*)(uintptr_t)origToNewOffsetMap[swap_uint32(rodataSrc->target)];
				rodataDst->rwdataindex = swap_uint16(rodataDst->rwdataindex);

				origToNewOffsetMap[(uintptr_t)node->rodata] = fileWriteOffset.get();

				fileWriteOffset.increment(sizeof(struct modelrodata_toggle));
				break;
			}

			case MODELNODETYPE_GUNDL: {
				struct modelrodata_gundl_load* rodataSrc = 
					(struct modelrodata_gundl_load*)((uintptr_t)modeldef_load - vmaSectionOffset + (uintptr_t)node->rodata);
				
				struct modelrodata_gundl* rodataDst = (struct modelrodata_gundl*)fileWriteOffset.getDataPtrCurrent();
				
				/*
					Note: the display lists are following each other at the end of the file
					Copy them at the end. They must be stored in the order of iteration of
					modelIterateDisplayLists
				*/

				debugPrint(PC_DBG_FLAG_MODEL, "- opagdl: %x\n", swap_uint32(rodataSrc->opagdl));
				debugPrint(PC_DBG_FLAG_MODEL, "- xlugdl: %x\n", swap_uint32(rodataSrc->xlugdl));
				debugPrint(PC_DBG_FLAG_MODEL, "- baseaddr: %x\n", swap_uint32(rodataSrc->baseaddr));
				debugPrint(PC_DBG_FLAG_MODEL, "- vertices: %x [%x:%x] \n", 
					swap_uint32(rodataSrc->vertices),
					swap_uint32(rodataSrc->vertices),
					swap_uint32(rodataSrc->vertices) + swap_int16(rodataSrc->numvertices) * sizeof(struct gfxvtx));
				debugPrint(PC_DBG_FLAG_MODEL, "- numvertices: %x\n", swap_int16(rodataSrc->numvertices));
				debugPrint(PC_DBG_FLAG_MODEL, "- unk12: %x\n", swap_int16(rodataSrc->unk12));

				gdlOffsets.insert(swap_uint32(rodataSrc->opagdl));
				gdlOffsets.insert(swap_uint32(rodataSrc->xlugdl));

				rodataDst->opagdl = (Gfx*)(uintptr_t)swap_uint32(rodataSrc->opagdl);
				rodataDst->xlugdl = (Gfx*)(uintptr_t)swap_uint32(rodataSrc->xlugdl);
				rodataDst->baseaddr = (void*)(uintptr_t)swap_uint32(rodataSrc->baseaddr);
				rodataDst->vertices = (struct gfxvtx*)(uintptr_t)swap_uint32(rodataSrc->vertices);
				rodataDst->numvertices = swap_int16(rodataSrc->numvertices);
				rodataDst->unk12 = swap_int16(rodataSrc->unk12);

				origToNewOffsetMap[(uintptr_t)node->rodata] = fileWriteOffset.get();

				fileWriteOffset.increment(sizeof(struct modelrodata_gundl));

				// We can probably append the vertices next to it here
				// Or put them in the file before the display lists, dunno if it's important or not
				// Then we can parse the display list, look for vertex calls and adjust the offsets there?
				struct gfxvtx* vtxptr = (struct gfxvtx*)((uintptr_t)modeldef_load - (uintptr_t)vmaSectionOffset + (uintptr_t)swap_uint32(rodataSrc->vertices));
				struct gfxvtx* vtxdst = (struct gfxvtx*)fileWriteOffset.getDataPtrCurrent();

				origToNewOffsetMap[swap_uint32(rodataSrc->vertices)] = fileWriteOffset.get();

				rodataDst->vertices = (struct gfxvtx*)(uintptr_t)fileWriteOffset.get();
				debugPrint(PC_DBG_FLAG_MODEL, "write vertices at: %x\n", rodataDst->vertices);
				
				vtxSectionsSrc[vtxSectionsCount].start = swap_uint32(rodataSrc->vertices);
				vtxSectionsSrc[vtxSectionsCount].size = rodataDst->numvertices * sizeof(struct gfxvtx);

				vtxSectionsDst[vtxSectionsCount].start = fileWriteOffset.get();
				vtxSectionsDst[vtxSectionsCount].size = rodataDst->numvertices * sizeof(struct gfxvtx);

				vtxSectionsCount++;

				for (int i = 0; i < rodataDst->numvertices; i++) {
					vtxdst[i].v[0] = swap_int16(vtxptr[i].v[0]);
					vtxdst[i].v[1] = swap_int16(vtxptr[i].v[1]);
					vtxdst[i].v[2] = swap_int16(vtxptr[i].v[2]);
					vtxdst[i].flags = vtxptr[i].flags;
					vtxdst[i].colour = vtxptr[i].colour;
					vtxdst[i].s = swap_int16(vtxptr[i].s);
					vtxdst[i].t = swap_int16(vtxptr[i].t);

					fileWriteOffset.increment(sizeof(struct gfxvtx));
				}

				// It seems the color table is located right after the vertices
				// We don't know the size of the color table, so let's go through the display list commands
				Gfx* cmds[2] = {
					rodataDst->opagdl != 0 ? (Gfx*)((uintptr_t)modeldef_load - (uintptr_t)vmaSectionOffset + (uintptr_t)rodataDst->opagdl) : 0,
					rodataDst->xlugdl != 0 ? (Gfx*)((uintptr_t)modeldef_load - (uintptr_t)vmaSectionOffset + (uintptr_t)rodataDst->xlugdl) : 0,
				};

				for (int i = 0; i < 2; i++) 
				{
					Gfx* cmd = cmds[i];

					while (cmds[i] != 0) {
						u8 opcode = swap_uint32(cmd->words.w0) >> 24;

						if (opcode == (u8)G_ENDDL) break;
						if (opcode == (u8)G_SETCOLOR) 
						{
							u32 colorBufferSize = ((swap_uint32(cmd->words.w0) >> 16) & 0xFF) + 0x4;
							
							// Copy the colors to the new buffer
							u32* colorSrc = (u32*)((uintptr_t)modeldef_load + swap_uint32(cmd->words.w1) - (uintptr_t)vmaSectionOffset);
							u32* colorDst = (u32*)fileWriteOffset.getDataPtrCurrent();
							for (int j = 0; j < colorBufferSize / 4; j++)
							{
								colorDst[j] = swap_uint32(colorSrc[j]);
							}

							origToNewOffsetMap[swap_uint32(cmd->words.w1)] = fileWriteOffset.get();

							fileWriteOffset.increment(colorBufferSize);
						}

						cmd++;
					}
				}

				break;
			}

			case MODELNODETYPE_REORDER: {
				struct modelrodata_reorder_load* rodataSrc = 
					(struct modelrodata_reorder_load*)((uintptr_t)modeldef_load - vmaSectionOffset + (uintptr_t)node->rodata);
				
				struct modelrodata_reorder* rodataDst = (struct modelrodata_reorder*)fileWriteOffset.getDataPtrCurrent();

				rodataDst->unk00 = swap_f32(rodataSrc->unk00);
				rodataDst->unk04 = swap_f32(rodataSrc->unk04);
				rodataDst->unk08 = swap_f32(rodataSrc->unk08);
				rodataDst->unk0c[0] = swap_f32(rodataSrc->unk0c[0]);
				rodataDst->unk0c[1] = swap_f32(rodataSrc->unk0c[1]);
				rodataDst->unk0c[2] = swap_f32(rodataSrc->unk0c[2]);
				rodataDst->side = swap_int16(rodataSrc->side);
				rodataDst->rwdataindex = swap_uint16(rodataSrc->rwdataindex);

				rodataDst->unk18 = (struct modelnode*)(uintptr_t)origToNewOffsetMap[swap_uint32(rodataSrc->unk18)];
				rodataDst->unk1c = (struct modelnode*)(uintptr_t)origToNewOffsetMap[swap_uint32(rodataSrc->unk1c)];

				origToNewOffsetMap[(uintptr_t)node->rodata] = fileWriteOffset.get();

				fileWriteOffset.increment(sizeof(struct modelrodata_reorder));
				break;
			}

			case MODELNODETYPE_DL: {
				struct modelrodata_dl_load* rodataSrc = 
					(struct modelrodata_dl_load*)((uintptr_t)modeldef_load - vmaSectionOffset + (uintptr_t)node->rodata);
				
				struct modelrodata_dl* rodataDst = (struct modelrodata_dl*)fileWriteOffset.getDataPtrCurrent();
				
				debugPrint(PC_DBG_FLAG_MODEL, "- opagdl: %x\n", swap_uint32(rodataSrc->opagdl));
				debugPrint(PC_DBG_FLAG_MODEL, "- xlugdl: %x\n", swap_uint32(rodataSrc->xlugdl));
				debugPrint(PC_DBG_FLAG_MODEL, "- colourtable: %x\n", swap_uint32(rodataSrc->colourtable));
				debugPrint(PC_DBG_FLAG_MODEL, "- vertices: %x\n", swap_uint32(rodataSrc->vertices));
				debugPrint(PC_DBG_FLAG_MODEL, "- numvertices: %x\n", swap_int16(rodataSrc->numvertices));
				debugPrint(PC_DBG_FLAG_MODEL, "- mcount: %x\n", swap_int16(rodataSrc->mcount));
				debugPrint(PC_DBG_FLAG_MODEL, "- rwdataindex: %x\n", swap_uint16(rodataSrc->rwdataindex));
				debugPrint(PC_DBG_FLAG_MODEL, "- numcolours: %x\n", swap_uint16(rodataSrc->numcolours));

				gdlOffsets.insert(swap_uint32(rodataSrc->opagdl));
				gdlOffsets.insert(swap_uint32(rodataSrc->xlugdl));

				rodataDst->opagdl = (Gfx*)(uintptr_t)swap_uint32(rodataSrc->opagdl);
				rodataDst->xlugdl = (Gfx*)(uintptr_t)swap_uint32(rodataSrc->xlugdl);
				rodataDst->vertices = (struct gfxvtx *)(uintptr_t)swap_uint32(rodataSrc->vertices);
				rodataDst->numvertices = swap_int16(rodataSrc->numvertices);
				rodataDst->mcount = swap_int16(rodataSrc->mcount);
				rodataDst->rwdataindex = swap_uint16(rodataSrc->rwdataindex);
				rodataDst->numcolours = swap_uint16(rodataSrc->numcolours);

				// The colourtable offset is right after the vertices..
				u32* colourptr = 0;

				colourptr = (u32*)((uintptr_t)rodataDst->vertices + (uintptr_t)rodataDst->numvertices * sizeof(struct gfxvtx));
				debugPrint(PC_DBG_FLAG_MODEL, "- colourptr: %llx\n", colourptr);

				origToNewOffsetMap[(uintptr_t)node->rodata] = fileWriteOffset.get();

				fileWriteOffset.increment(sizeof(struct modelrodata_dl));

				// Copy the vertices
				struct gfxvtx* vtxptr = (struct gfxvtx*)((uintptr_t)modeldef_load - (uintptr_t)vmaSectionOffset + (uintptr_t)swap_uint32(rodataSrc->vertices));
				struct gfxvtx* vtxdst = (struct gfxvtx*)fileWriteOffset.getDataPtrCurrent();

				rodataDst->vertices = (struct gfxvtx*)(uintptr_t)fileWriteOffset.get();
				debugPrint(PC_DBG_FLAG_MODEL, "write vertices at: %x\n", rodataDst->vertices);
				
				vtxSectionsSrc[vtxSectionsCount].start = swap_uint32(rodataSrc->vertices);
				vtxSectionsSrc[vtxSectionsCount].size = rodataDst->numvertices * sizeof(struct gfxvtx);

				vtxSectionsDst[vtxSectionsCount].start = fileWriteOffset.get();
				vtxSectionsDst[vtxSectionsCount].size = rodataDst->numvertices * sizeof(struct gfxvtx);

				vtxSectionsCount++;

				for (int i = 0; i < rodataDst->numvertices; i++) {
					vtxdst[i].v[0] = swap_int16(vtxptr[i].v[0]);
					vtxdst[i].v[1] = swap_int16(vtxptr[i].v[1]);
					vtxdst[i].v[2] = swap_int16(vtxptr[i].v[2]);
					vtxdst[i].flags = vtxptr[i].flags;
					vtxdst[i].colour = vtxptr[i].colour;
					vtxdst[i].s = swap_int16(vtxptr[i].s);
					vtxdst[i].t = swap_int16(vtxptr[i].t);

					fileWriteOffset.increment(sizeof(struct gfxvtx));
				}

				// Copy the colors
				u8 colorBufferSize = rodataDst->numcolours * 4;

				// Copy the colors to the new buffer
				u32* colorSrc = (u32*)((uintptr_t)modeldef_load + (uintptr_t)colourptr - (uintptr_t)vmaSectionOffset);
				u32* colorDst = (u32*)fileWriteOffset.getDataPtrCurrent();
				for (int j = 0; j < colorBufferSize / 4; j++)
				{
					colorDst[j] = swap_uint32(colorSrc[j]);
				}

				origToNewOffsetMap[(uintptr_t)colourptr] = fileWriteOffset.get();

				fileWriteOffset.increment(colorBufferSize);
				break;
			}

			case MODELNODETYPE_CHRINFO: {
				struct modelrodata_chrinfo* rodataSrc = (struct modelrodata_chrinfo*)((uintptr_t)modeldef_load - vmaSectionOffset + (uintptr_t)node->rodata);
				struct modelrodata_chrinfo* rodataDst = (struct modelrodata_chrinfo*)fileWriteOffset.getDataPtrCurrent();
				
				rodataDst->animpart = swap_uint16(rodataSrc->animpart);
				rodataDst->mtxindex = swap_int16(rodataSrc->mtxindex);
				rodataDst->unk04 = swap_f32(rodataSrc->unk04);
				rodataDst->rwdataindex = swap_uint16(rodataSrc->rwdataindex);

				origToNewOffsetMap[(uintptr_t)node->rodata] = fileWriteOffset.get();

				fileWriteOffset.increment(sizeof(struct modelrodata_chrinfo));
				break;
			}

			case MODELNODETYPE_HEADSPOT: {
				struct modelrodata_headspot* rodataSrc = (struct modelrodata_headspot*)((uintptr_t)modeldef_load - vmaSectionOffset + (uintptr_t)node->rodata);
				struct modelrodata_headspot* rodataDst = (struct modelrodata_headspot*)fileWriteOffset.getDataPtrCurrent();
				
				rodataDst->rwdataindex = swap_uint16(rodataSrc->rwdataindex);

				origToNewOffsetMap[(uintptr_t)node->rodata] = fileWriteOffset.get();

				fileWriteOffset.increment(sizeof(struct modelrodata_headspot));
				break;
			}

			case MODELNODETYPE_DISTANCE: {
				struct modelrodata_distance_load* rodataSrc = (struct modelrodata_distance_load*)((uintptr_t)modeldef_load - vmaSectionOffset + (uintptr_t)node->rodata);
				struct modelrodata_distance* rodataDst = (struct modelrodata_distance*)fileWriteOffset.getDataPtrCurrent();
				
				rodataDst->near = swap_f32(rodataSrc->near);
				rodataDst->far = swap_f32(rodataSrc->far);
				rodataDst->target = (struct modelnode*)(uintptr_t)origToNewOffsetMap[swap_uint32(rodataSrc->target)];
				rodataDst->rwdataindex = swap_uint16(rodataSrc->rwdataindex);
				
				origToNewOffsetMap[(uintptr_t)node->rodata] = fileWriteOffset.get();

				fileWriteOffset.increment(sizeof(struct modelrodata_distance));
				break;
			}

			case 0x19: {
				struct modelrodata_type19* rodataSrc = (struct modelrodata_type19*)((uintptr_t)modeldef_load - vmaSectionOffset + (uintptr_t)node->rodata);
				struct modelrodata_type19* rodataDst = (struct modelrodata_type19*)fileWriteOffset.getDataPtrCurrent();
				
				rodataDst->numvertices = swap_int32(rodataSrc->numvertices);
				for (u32 i = 0; i < 4; i++)
				{
					rodataDst->vertices[i] = rodataSrc->vertices[i];
					swap_coord(&rodataDst->vertices[i]);
				}
				
				origToNewOffsetMap[(uintptr_t)node->rodata] = fileWriteOffset.get();

				fileWriteOffset.increment(sizeof(struct modelrodata_type19));
				break;
			}

			case MODELNODETYPE_CHRGUNFIRE: {
				struct modelrodata_chrgunfire_load* rodataSrc = (struct modelrodata_chrgunfire_load*)((uintptr_t)modeldef_load - vmaSectionOffset + (uintptr_t)node->rodata);
				struct modelrodata_chrgunfire* rodataDst = (struct modelrodata_chrgunfire*)fileWriteOffset.getDataPtrCurrent();
				
				rodataDst->pos = rodataSrc->pos;
				swap_coord(&rodataDst->pos);

				rodataDst->dim = rodataSrc->dim;
				swap_coord(&rodataDst->dim);

				rodataDst->texture = (struct textureconfig*)(uintptr_t)swap_uint32(rodataSrc->texture);
				rodataDst->unk1c = swap_f32(rodataSrc->unk1c);
				rodataDst->rwdataindex = swap_uint16(rodataSrc->rwdataindex);
				rodataDst->baseaddr = (void*)(uintptr_t)swap_uint32(rodataSrc->baseaddr);

				origToNewOffsetMap[(uintptr_t)node->rodata] = fileWriteOffset.get();

				fileWriteOffset.increment(sizeof(struct modelrodata_chrgunfire));
				break;
			}

			case MODELNODETYPE_POSITIONHELD: {
				struct modelrodata_positionheld* rodataSrc = (struct modelrodata_positionheld*)((uintptr_t)modeldef_load - vmaSectionOffset + (uintptr_t)node->rodata);
				struct modelrodata_positionheld* rodataDst = (struct modelrodata_positionheld*)fileWriteOffset.getDataPtrCurrent();
				
				rodataDst->pos = rodataSrc->pos;
				swap_coord(&rodataDst->pos);

				rodataDst->mtxindex = swap_int16(rodataSrc->mtxindex);

				origToNewOffsetMap[(uintptr_t)node->rodata] = fileWriteOffset.get();

				fileWriteOffset.increment(sizeof(struct modelrodata_positionheld));
				break;
			}

			case MODELNODETYPE_11: {
				struct modelrodata_type11_load* rodataSrc = (struct modelrodata_type11_load*)((uintptr_t)modeldef_load - vmaSectionOffset + (uintptr_t)node->rodata);
				struct modelrodata_type11* rodataDst = (struct modelrodata_type11*)fileWriteOffset.getDataPtrCurrent();
				
				rodataDst->unk00 = swap_uint32(rodataSrc->unk00);
				rodataDst->unk04 = swap_uint32(rodataSrc->unk04);
				rodataDst->unk08 = swap_uint32(rodataSrc->unk08);
				rodataDst->unk0c = swap_uint32(rodataSrc->unk0c);
				rodataDst->unk10 = swap_uint32(rodataSrc->unk10);
				rodataDst->unk14 = (void*)(uintptr_t)swap_uint32(rodataSrc->unk14);

				origToNewOffsetMap[(uintptr_t)node->rodata] = fileWriteOffset.get();

				fileWriteOffset.increment(sizeof(struct modelrodata_type11));
				break;
			}
			
			default:
				debugPrint(PC_DBG_FLAG_MODEL, "ERROR: unhandled modelnode type: %x\n", type);
				fatalExit();
				break;
		}
	}

	// Adjust rodata offsets
	for (auto& offset : modelnodesOffsets)
	{
		uint32_t newOffset = origToNewOffsetMap[offset];
		struct modelnode* node = (struct modelnode*)fileWriteOffset.getDataPtrAt(newOffset);
		
		node->rodata = (union modelrodata*)(uintptr_t)origToNewOffsetMap[(uintptr_t)node->rodata];
	}
	
	// Adjust offsets in the main modeldef structure
	modeldef->rootnode = (struct modelnode*)(uintptr_t)origToNewOffsetMap[rootNodeOffset];

	/*
		Read parts
		-----------------------
		(previous file content)
		parts: ptr to the array of parts
		array of parts:
			- ptr to part1
			- ptr to part2...
	*/
	
	// Modeldef write: Copy the parts offsets
	const u32 PARTS_SECTION_START = fileWriteOffset.get();
	debugPrint(PC_DBG_FLAG_MODEL, "modeldefOffset: %x (after parts)\n", fileWriteOffset);

	for (int i = 0; i < modeldef->numparts; i++) {
		u32* parts = (u32*)(
			(uintptr_t)modeldef_load - vmaSectionOffset 
			+ swap_uint32(modeldef_load->parts)
			+ 4 * i);
		
		u32 partOffset = swap_uint32(*parts);

		// Using the N64 offset, we can fill the "parts" offsets array
		u32 newOffset = origToNewOffsetMap[partOffset];
		u64* ptr = (u64*)((uintptr_t)s_convertBuffer + (uintptr_t)PARTS_SECTION_START + i * sizeof(void*));
		*ptr = (u64)newOffset;

		fileWriteOffset.increment(sizeof(void*));
	}

	modeldef->parts = (struct modelnode**)(uintptr_t)PARTS_SECTION_START;

	// Is followed by a variable s16 array of part numbers... Write it to the file
	const u32 PARTNUM_SECTION_START = fileWriteOffset.get();

	s16* partnums = (s16*)((uintptr_t)modeldef_load - vmaSectionOffset + swap_uint32(modeldef_load->parts) + 4 * modeldef->numparts);
	for (int i = 0; i < modeldef->numparts; i++) {
		s16* ptr = (s16*)((uintptr_t)s_convertBuffer + (uintptr_t)PARTNUM_SECTION_START + i * sizeof(s16));
		*ptr = swap_int16(partnums[i]);

		fileWriteOffset.increment(sizeof(s16));
	}

	const u32 TEXCONFIGS_SECTION_START = fileWriteOffset.get();
	const u32 texconfigsOffset = swap_uint32(modeldef_load->texconfigs);

	for (int i = 0; i < modeldef->numtexconfigs; i++) {
		struct textureconfig_load* src = (struct textureconfig_load*)((uintptr_t)modeldef_load - vmaSectionOffset + texconfigsOffset);
		struct textureconfig* dst = (struct textureconfig*)fileWriteOffset.getDataPtrCurrent();
		
		debugPrint(PC_DBG_FLAG_MODEL, "src[%d].texturenum: %x\n", i, swap_uint32(src[i].texturenum));
		debugPrint(PC_DBG_FLAG_MODEL, "src[%d].width: %d\n", i, (src[i].width));
		debugPrint(PC_DBG_FLAG_MODEL, "src[%d].height: %d\n", i, (src[i].height));
		debugPrint(PC_DBG_FLAG_MODEL, "src[%d].level: %x\n", i, (src[i].level));
		debugPrint(PC_DBG_FLAG_MODEL, "src[%d].depth: %x\n", i, (src[i].depth));
		debugPrint(PC_DBG_FLAG_MODEL, "src[%d].format: %x\n", i, (src[i].format));
		debugPrint(PC_DBG_FLAG_MODEL, "src[%d].s: %x\n", i, (src[i].s));
		debugPrint(PC_DBG_FLAG_MODEL, "src[%d].t: %x\n", i, (src[i].t));

		dst->texturenum = swap_uint32(src[i].texturenum);
		dst->width = (src[i].width);
		dst->height = (src[i].height);
		dst->level = (src[i].level);
		dst->format = (src[i].format);
		dst->depth = (src[i].depth);
		dst->s = (src[i].s);
		dst->t = (src[i].t);
		dst->unk0b = (src[i].unk0b);

		//hexdump((uintptr_t)modeldef_load - vma + (uintptr_t)dst->texturenum, dst->width * dst->height * 2);

		fileWriteOffset.increment(sizeof(struct textureconfig));
	}

	modeldef->texconfigs = (struct textureconfig*)(uintptr_t)TEXCONFIGS_SECTION_START;

	/* --------------------
		TEXTURES
	   -------------------- */
	const u32 TEXDATA_SECTION_START = fileWriteOffset.get();

	for (int i = 0; i < modeldef->numtexconfigs; i++) {
		struct textureconfig* list = (struct textureconfig*)((uintptr_t)modeldef + (uintptr_t)modeldef->texconfigs);
		struct textureconfig* tex = &list[i];

		// Sometimes the texture is embedded in the modeldef file, sometimes it's not
		u32 segment = (tex->texturenum & 0xf000000) >> 24;
		if (segment == 0x5) {
			debugPrint(PC_DBG_FLAG_MODEL, "Modeldef embedded texture\n");

			u32 oldTextureOffset = tex->texturenum;
			void* textureAddr = (void*)((uintptr_t)modeldef_load - vmaSectionOffset + (uintptr_t)tex->texturenum);
			u32 len = 0;
			
			if (tex->depth == G_IM_SIZ_4b) {
				len = (tex->width * tex->height) / 2;
			} else if (tex->depth == G_IM_SIZ_16b) {
				len = tex->width * tex->height * 2;
			} else if (tex->depth == G_IM_SIZ_8b) {
				len = tex->width * tex->height;
			} else {
				debugPrint(PC_DBG_FLAG_MODEL, "[ERROR] Unhandled texture format in modeldef, implement %x.\n", tex->depth);
				fatalExit();
			}

			void* textureDst = fileWriteOffset.getDataPtrCurrent();
			memcpy(textureDst, textureAddr, len);
			
			/*
				Embedded texture are ready to be passed to the GPU so their rows are already swizzled
				Swizzle them back to normal
			*/
			s32 format = 0;
			if (tex->format == G_IM_FMT_RGBA && tex->depth == G_IM_SIZ_16b)
				format = TEXFORMAT_RGBA16;
			else if (tex->format == G_IM_FMT_IA && tex->depth == G_IM_SIZ_16b)
				format = TEXFORMAT_IA16;
			else if (tex->format == G_IM_FMT_RGBA && tex->depth == G_IM_SIZ_32b)
				format = TEXFORMAT_RGBA32;
			else if (tex->format == G_IM_FMT_RGBA && tex->depth == G_IM_SIZ_8b)
				format = TEXFORMAT_RGBA16_CI8;
			else if (tex->format == G_IM_FMT_IA && tex->depth == G_IM_SIZ_8b)
				format = TEXFORMAT_IA8;
			else if (tex->format == G_IM_FMT_CI && tex->depth == G_IM_SIZ_8b)
				format = TEXFORMAT_IA16_CI8;
			else if (tex->format == G_IM_FMT_I && tex->depth == G_IM_SIZ_8b)
				format = TEXFORMAT_I8;
			else if (tex->format == G_IM_FMT_RGBA && tex->depth == G_IM_SIZ_4b)
				format = TEXFORMAT_RGBA16_CI4;
			else if (tex->format == G_IM_FMT_IA && tex->depth == G_IM_SIZ_4b)
				format = TEXFORMAT_IA4;
			else if (tex->format == G_IM_FMT_CI && tex->depth == G_IM_SIZ_4b)
				format = TEXFORMAT_IA16_CI4;
			else if (tex->format == G_IM_FMT_I && tex->depth == G_IM_SIZ_4b)
				format = TEXFORMAT_I4;

			/*
			#define TEXFORMAT_RGBA32     0x00 // 32-bit RGBA (8/8/8/8)
			#define TEXFORMAT_RGBA16     0x01 // 16-bit RGBA (5/5/5/1)
			#define TEXFORMAT_RGB24      0x02 // 24-bit RGB (8/8/8)
			#define TEXFORMAT_RGB15      0x03 // 15-bit RGB (5/5/5)
			#define TEXFORMAT_IA16       0x04 // 16-bit grayscale+alpha
			#define TEXFORMAT_IA8        0x05 // 8-bit grayscale+alpha (4/4)
			#define TEXFORMAT_IA4        0x06 // 4-bit grayscale+alpha (3/1)
			#define TEXFORMAT_I8         0x07 // 8-bit grayscale
			#define TEXFORMAT_I4         0x08 // 4-bit grayscale
			#define TEXFORMAT_RGBA16_CI8 0x09 // 16-bit 5551 paletted colour with 8-bit palette indexes
			#define TEXFORMAT_RGBA16_CI4 0x0a // 16-bit 5551 paletted colour with 4-bit palette indexes
			#define TEXFORMAT_IA16_CI8   0x0b // 16-bit 88 paletted greyscale+alpha with 8-bit palette indexes
			#define TEXFORMAT_IA16_CI4   0x0c // 16-bit 88 paletted greyscale+alpha with 4-bit palette indexes
			*/

			texSwapAltRowBytes2((u8*)textureDst, tex->width, tex->height, format);

			tex->texturenum = fileWriteOffset.get();

			// Put the vma offset in the texturenum offset
			// texLoad will detect it and know this texture is already loaded in RAM
			tex->texturenum = fileWriteOffset.get() | vmaSectionOffset;

			debugPrint(PC_DBG_FLAG_MODEL, "old embedded texture offset %x new offset %x\n", oldTextureOffset, fileWriteOffset);

			origToNewOffsetMap[oldTextureOffset] = fileWriteOffset.get();

			fileWriteOffset.increment(len);
		} else {
			// The texture is probably not embedded
			debugPrint(PC_DBG_FLAG_MODEL, "External texture %x\n", tex->texturenum);
		}
	}

	/*
		GDLs are at the end of the original file.
		We can retrieve them iterating them by sort order.
		Make sure we have at least 1 GDL offset
	*/
	gdlOffsets.erase(0);

	if (gdlOffsets.size() > 0)
	{
		uint32_t gdlsStartOffset = *gdlOffsets.begin();
		uint32_t gdlsEndOffset = vmaSectionOffset + fileData.second;
		debugPrint(PC_DBG_FLAG_MODEL, "GDLS [%x:%x]\n", gdlsStartOffset, gdlsEndOffset);

		uint32_t gdlsSize = gdlsEndOffset - gdlsStartOffset;
		
		// Copy the GDls to the new file
		memcpy(fileWriteOffset.getDataPtrCurrent(), 
			(void*)((uintptr_t)modeldef_load + ((uintptr_t)gdlsStartOffset & 0xffffff)), 
			gdlsSize
		);

		// Save the offset of the first GDL
		origToNewOffsetMap[gdlsStartOffset] = fileWriteOffset.get();

		// Adjust the endianness of each display list element
		u32* dlptr = (u32*)fileWriteOffset.getDataPtrCurrent();
		for (u32 i = 0; i < gdlsSize / sizeof(u32); i++) {
			dlptr[i] = swap_uint32(dlptr[i]);
		}
		
		// Look for some commands such as G_VTX and adjust the adresses in them
		Gfx* dst = (Gfx*)fileWriteOffset.getDataPtrCurrent();
		for (u32 i = 0; i < gdlsSize / sizeof(Gfx); i++) {
			u8 opcode = dst[i].words.w0 >> 24;

			if (opcode == (u8)G_VTX) 
			{
				// G_VTX references offsets in memory, adjust the offsets to the new file format
				// Here we have to find in which vertex buffer this vtx command falls in
				// And adjust this offset
				u32 offset = dst[i].words.w1;

				// Skip SPSEGMENT_MODEL_VTX segments
				u32 segment = (offset & 0xF000000) >> 24;
				if (segment == SPSEGMENT_MODEL_VTX) continue;

				for (int j = 0; j < vtxSectionsCount; j++) {
					if (offset >= vtxSectionsSrc[j].start && offset < vtxSectionsSrc[j].start + vtxSectionsSrc[j].size) {
						u32 diff = dst[i].words.w1 - vtxSectionsSrc[j].start;
						u32 newOffset = vtxSectionsDst[j].start + diff;
						dst[i].words.w1 = newOffset | 0x5000000;
						break;
					}

					if (j == vtxSectionsCount - 1) {
						debugPrint(PC_DBG_FLAG_MODEL, "[ERROR] Vtx offset not found %x\n", offset);
						fatalExit();
					}
				}
			} 
			else if (opcode == (u8)G_SETCOLOR) 
			{
				u32 colorOffset = dst[i].words.w1;

				// Skip SPSEGMENT_MODEL_COL2 segments
				u32 segment = (colorOffset & 0xF000000) >> 24;
				if (segment == SPSEGMENT_MODEL_COL2) continue;

				u32 newColorOffset = origToNewOffsetMap[dst[i].words.w1] | 0x5000000;
				dst[i].words.w1 = newColorOffset;
			} 
			else if (opcode == (u8)G_SETTIMG) 
			{
				u32 oldOffset = dst[i].words.w1;
				u32 newOffset = origToNewOffsetMap[oldOffset] | 0x5000000; 
				dst[i].words.w1 = newOffset;
			}
		}

		fileWriteOffset.increment(gdlsSize);
	}

	for (auto& offset : gdlOffsets)
	{
		uint32_t gdlsStartOffset = *gdlOffsets.begin();
		uint32_t gdlOffsetToFirstGdl = offset - gdlsStartOffset;

		origToNewOffsetMap[offset] = origToNewOffsetMap[gdlsStartOffset] + gdlOffsetToFirstGdl;
	}

	// Iterates nodes again and replace gdl offsets by the new ones
	// For each GDL, we have to calculate their offset relative to the first GDL in the original file
	// Then we can use that information to calculate the offset in the new file
	for (auto& offset : modelnodesOffsets)
	{
		uint32_t newOffset = origToNewOffsetMap[offset];
		struct modelnode* node = (struct modelnode*)fileWriteOffset.getDataPtrAt(newOffset);
		assert(node->type > 0);

		uint32_t type = node->type & 0xff;
		switch(type)
		{
			case MODELNODETYPE_GUNDL: {
				union modelrodata* rodata = (union modelrodata*)fileWriteOffset.getDataPtrAt((uintptr_t)node->rodata);
				rodata->gundl.xlugdl = (Gfx*)(uintptr_t)origToNewOffsetMap[(uintptr_t)rodata->gundl.xlugdl];
				rodata->gundl.opagdl = (Gfx*)(uintptr_t)origToNewOffsetMap[(uintptr_t)rodata->gundl.opagdl];
				break;
			}

			case MODELNODETYPE_DL: {
				union modelrodata* rodata = (union modelrodata*)fileWriteOffset.getDataPtrAt((uintptr_t)node->rodata);
				rodata->dl.xlugdl = (Gfx*)(uintptr_t)origToNewOffsetMap[(uintptr_t)rodata->dl.xlugdl];
				rodata->dl.opagdl = (Gfx*)(uintptr_t)origToNewOffsetMap[(uintptr_t)rodata->dl.opagdl];
				break;
			}
		}
	}

	debugPrint(PC_DBG_FLAG_MODEL, "# Previous file size: %x\n", fileData.second);
	debugPrint(PC_DBG_FLAG_MODEL, "# New file size: %x\n", fileWriteOffset);

	// Copy the converted file to the original buffer and cleanup
	memcpy((void*)src, (void*)s_convertBuffer, fileWriteOffset.get());
	memset(s_convertBuffer, 0, PD_ASSET_CONVERT_BUFFER_SIZE);

	return fileWriteOffset.get();
}

void AssetConvertModeldefAtAddr(void* dst, size_t allocationSize, size_t originalFileSize)
{
	// Load the asset, convert it...

}
