#include <ultra64.h>
#include "constants.h"
#include "game/bondhead.h"
#include "game/bg.h"
#include "game/pad.h"
#include "game/setup.h"
#include "bss.h"
#include "lib/collision.h"
#include "lib/mtx.h"
#include "lib/anim.h"
#include "lib/model.h"
#include "data.h"
#include "types.h"

#include "game/file.h"

#include "print.h"
#include "byteswap.h"
#include "gfx/hashmap.h"
#include "offsetsmap.h"
#include "native_functions.h"

/**
 * The function assumes that a pad file's data has been loaded from the ROM
 * and is pointed to by g_StageSetup.padfiledata. These pads are in a packed
 * format. During gameplay, the game uses padUnpack as needed to temporarily
 * populate pad structs from this data.
 *
 * setupPreparePads prepares the packed data by doing the following:
 * - populates the room field (if -1)
 * - multiplies each pad's bounding box by 1 (this is effectively a no op)
 * - sets the g_StageSetup pad/waygroup/waypoint/cover pointers
 * - promotes file offsets to RAM pointers
 * - does similar things for cover by calling setupPrepareCover()
 */
void setupPreparePads(void)
{
	s16 *roomsptr;
	s32 padnum;
	s32 numpads;
	s32 roomnum;
	struct pad pad;
	struct waypoint *waypoint;
	struct waygroup *waygroup;
	s16 inrooms[24];
	s16 aboverooms[22];
	s32 offset;

	print("setupPreparePads()\n");

	u32 filenum = g_Stages[g_StageIndex].padsfileid;
	u32 allocsize = fileGetAllocationSize(filenum);
	u32 loadedsize = fileGetLoadedSize(filenum);
	print("allocsize %x loadedsize %x\n", allocsize, loadedsize);

	void* srcPadsFileRaw = nativeMalloc(allocsize);
	memcpy(srcPadsFileRaw, g_StageSetup.padfiledata, allocsize);
	u32 fileWriteBuffer = 0;

	struct padsfileheader * srcPadsFile = (struct padsfileheader *)srcPadsFileRaw;

	g_PadsFile = (struct padsfileheader *)g_StageSetup.padfiledata;
	memset(g_PadsFile, 0, allocsize);

	g_PadOffsets = (u16 *)(g_StageSetup.padfiledata + 0x14);
	
	g_PadsFile->numpads = swap_int32(srcPadsFile->numpads);
	g_PadsFile->numcovers = swap_int32(srcPadsFile->numcovers);
	g_PadsFile->waypointsoffset = swap_int32(srcPadsFile->waypointsoffset);
	g_PadsFile->waygroupsoffset = swap_int32(srcPadsFile->waygroupsoffset);
	g_PadsFile->coversoffset = swap_int32(srcPadsFile->coversoffset);

	// The real size of the header is 0x14, then the pad offets starts at 0x14
	//g_PadsFile->padoffsets[0] = swap_int16(srcPadsFile->padoffsets[0]);

	print("- g_PadsFile->waypointsoffset: %x\n", g_PadsFile->waypointsoffset);
	print("- g_PadsFile->waygroupsoffset: %x\n", g_PadsFile->waygroupsoffset);
	print("- g_PadsFile->coversoffset: %x\n", g_PadsFile->coversoffset);

	padnum = 0;
	numpads = g_PadsFile->numpads;
	//fileWriteBuffer += sizeof(struct padsfileheader);
	fileWriteBuffer += 0x14;

	print("- sizeof(struct padsfileheader): %x\n", sizeof(struct padsfileheader));
	print("- numpads: %x\n", numpads);

	{
		u16* src = (u16*)((uintptr_t)srcPadsFileRaw + (uintptr_t)0x14);

		// Convert padoffsets
		for (u32 i = 0; i < numpads; i++)
		{
			g_PadOffsets[i] = swap_uint16(src[i]);
			fileWriteBuffer += sizeof(u16);
		}
	}

	print("- fileWriteBuffer: %x\n", fileWriteBuffer);

	// After the pad offets there is the actual pad content
	// We can copy the content from the first offset to waypointsoffset
	if (numpads > 0)
	{
		u32 padsSize = g_PadsFile->waypointsoffset - g_PadOffsets[0];
		void* src = (void*)((uintptr_t)srcPadsFileRaw + (uintptr_t)fileWriteBuffer); // NOTE: at this point, N64 and PC structures are the same size
		void* dst = (void*)((uintptr_t)g_StageSetup.padfiledata + (uintptr_t)fileWriteBuffer);
		memcpy(dst, src, padsSize);
		fileWriteBuffer += padsSize;
	}

	for (; padnum < numpads; padnum++) {
		offset = g_PadOffsets[padnum];

		// Convert the pad data, assuming the offsets are never traversed twice, we can convert the header here...
		u32* headerPtr = (u32*)&g_StageSetup.padfiledata[offset];
		*headerPtr = swap_uint32(*headerPtr);

		padUnpack(padnum, PADFIELD_POS | PADFIELD_BBOX, &pad);

		// If room is negative (ie. not specified)
		s32 room = ((s32)(*headerPtr) << 18) >> 22;

		if (room < 0) {
			roomsptr = NULL;
			bgFindRoomsByPos(&pad.pos, inrooms, aboverooms, 20, NULL);

			if (inrooms[0] != -1) {
				roomsptr = inrooms;
			} else if (aboverooms[0] != -1) {
				roomsptr = aboverooms;
			}

			if (roomsptr != NULL) {
				roomnum = cdFindFloorRoomAtPos(&pad.pos, roomsptr);

				if (roomnum > 0) {
					room = roomnum;
				} else {
					room = roomsptr[0];
				}
			}

			// flags, room and liftnum
			// ffffffff ffffffff ffrrrrrr rrrrllll
			u32 mask = 0x3FF0;
			u32 h = (*headerPtr) & (~mask);

			// Then update the room bitfield in the header
			*headerPtr = h | (room << 4);
		}

		// Scale the bbox by 1 and save it back into the packed pad data.
		// Yeah, this is effectively doing nothing.
		if ((*(u32 *) headerPtr >> 14) & PADFLAG_HASBBOXDATA) {
			f32 scale = 1;

			pad.bbox.xmin *= scale;
			pad.bbox.xmax *= scale;
			pad.bbox.ymin *= scale;
			pad.bbox.ymax *= scale;
			pad.bbox.zmin *= scale;
			pad.bbox.zmax *= scale;

			padCopyBboxFromPad(padnum, &pad);
		}
	}

	// Copy the waypoints	
	{
		struct waypoint_load* waypointsSrc = (struct waypoint_load*)((uintptr_t)srcPadsFileRaw + g_PadsFile->waypointsoffset);
		struct waypoint* waypointsDst = (struct waypoint*)((uintptr_t)g_StageSetup.padfiledata + (uintptr_t)fileWriteBuffer);

		struct waypoint_load* waypointsSrcStart = waypointsSrc;
		struct waypoint* waypointsDstStart = waypointsDst;

		g_StageSetup.waypoints = waypointsDst;

		while (waypointsSrc->padnum >= 0) {
			waypointsDst->padnum = swap_int32(waypointsSrc->padnum);
			waypointsDst->neighbours = (s32*)(uintptr_t)swap_uint32(waypointsSrc->neighbours);
			waypointsDst->groupnum = swap_int32(waypointsSrc->groupnum);
			waypointsDst->unk0c = swap_int32(waypointsSrc->unk0c);

			u32 oldWaypointOffset = g_PadsFile->waypointsoffset + (u32)((uintptr_t)waypointsSrc - (uintptr_t)waypointsSrcStart);

			/*print("waypoint: %x\n", oldWaypointOffset);
			print("- padnum: %x\n", waypointsDst->padnum);
			print("- neighbours: %x\n", waypointsDst->neighbours);
			print("- groupnum: %x\n", waypointsDst->groupnum);
			print("- unk0c: %x\n", waypointsDst->unk0c);*/

			waypointsSrc++;
			waypointsDst++;
			fileWriteBuffer += sizeof (struct waypoint);
		}

		{
			// Write last element of the waypoints list
			waypointsDst->padnum = -1;
			fileWriteBuffer += sizeof (struct waypoint);
		}

		// Traverse the list again to write the neighbours
		waypointsDst = waypointsDstStart;
		while (waypointsDst->padnum >= 0) {
			//print("padnum dst: %x\n", waypointsDst->padnum);

			// Parse neighbours list and copy it somewhere
			s32* neighbours = (s32*)((uintptr_t)srcPadsFileRaw + (uintptr_t)waypointsDst->neighbours);
			s32* neighboursDst = (s32*)((uintptr_t)g_StageSetup.padfiledata + (uintptr_t)fileWriteBuffer);

			waypointsDst->neighbours = fileWriteBuffer;
			while (swap_int32(*neighbours) >= 0) {
				//print("		- n: %x\n", swap_int32(*neighbours));
				*neighboursDst = swap_int32(*neighbours);
				neighbours++;
				neighboursDst++;
				fileWriteBuffer += sizeof(s32);
			}

			// Write last element
			*neighboursDst = -1;
			fileWriteBuffer += sizeof(s32);

			waypointsDst++;
		}
	}

	// Copy waygroups
	{
		struct waygroup_load* waygroupsSrc = (struct waygroup_load*)((uintptr_t)srcPadsFileRaw + (uintptr_t)g_PadsFile->waygroupsoffset);
		struct waygroup* waygroupsDst = (struct waygroup*)((uintptr_t)g_StageSetup.padfiledata + (uintptr_t)fileWriteBuffer);
		struct waygroup* waygroupsDstStart = waygroupsDst;

		g_StageSetup.waygroups = waygroupsDst;

		while (waygroupsSrc->neighbours != NULL) {
			waygroupsDst->neighbours = (s32*)(uintptr_t)swap_uint32(waygroupsSrc->neighbours);
			waygroupsDst->waypoints = (s32*)(uintptr_t)swap_uint32(waygroupsSrc->waypoints);
			waygroupsDst->unk08 = swap_uint32(waygroupsSrc->unk08);

			/*print("waygroup\n");
			print("- neighbours: %x\n", waygroupsDst->neighbours);
			print("- waypoints: %x\n", waygroupsDst->waypoints);
			print("- unk08: %x\n", waygroupsDst->unk08);*/

			waygroupsSrc++;
			waygroupsDst++;
			fileWriteBuffer += sizeof(struct waygroup);
		}

		waygroupsDst->neighbours = NULL;
		fileWriteBuffer += sizeof(struct waygroup);

		// Copy waypoints
		waygroupsDst = waygroupsDstStart;
		while (waygroupsDst->neighbours != NULL) {
			//print("waygroup dst\n");
			
			s32* waypointsSrc = (s32*)((uintptr_t)srcPadsFileRaw + (uintptr_t)waygroupsDst->waypoints);
			s32* waypointsDst = (s32*)((uintptr_t)g_StageSetup.padfiledata + (uintptr_t)fileWriteBuffer);

			waygroupsDst->waypoints = fileWriteBuffer;

			while (swap_int32(*waypointsSrc) >= 0) {
				//print("		- n: %x\n", swap_int32(*waypointsSrc));
				*waypointsDst = swap_int32(*waypointsSrc);
				waypointsSrc++;
				waypointsDst++;
				fileWriteBuffer += sizeof(s32);
			}

			// Write last element
			*waypointsDst = -1;
			fileWriteBuffer += sizeof(s32);

			waygroupsDst++;
		}

		// Copy neighbours
		waygroupsDst = waygroupsDstStart;
		while (waygroupsDst->neighbours != NULL) {
			//print("waygroup dst\n");
			
			s32* neighboursSrc = (s32*)((uintptr_t)srcPadsFileRaw + (uintptr_t)waygroupsDst->neighbours);
			s32* neighboursDst = (s32*)((uintptr_t)g_StageSetup.padfiledata + (uintptr_t)fileWriteBuffer);

			waygroupsDst->neighbours = fileWriteBuffer;

			while (swap_int32(*neighboursSrc) >= 0) {
				//print("		- n: %x\n", swap_int32(*neighboursSrc));
				*neighboursDst = swap_int32(*neighboursSrc);
				neighboursSrc++;
				neighboursDst++;
				fileWriteBuffer += sizeof(s32);
			}

			// Write last element
			*neighboursDst = -1;
			fileWriteBuffer += sizeof(s32);

			waygroupsDst++;
		}

	}

	{
		// Copy covers
		struct coverdefinition* coversSrc = (struct coverdefinition*)((uintptr_t)srcPadsFileRaw + (uintptr_t)g_PadsFile->coversoffset);
		struct coverdefinition* coversDst = (struct coverdefinition*)((uintptr_t)g_StageSetup.padfiledata + (uintptr_t)fileWriteBuffer);

		g_StageSetup.cover = (void *)coversDst;

		for (u32 i = 0; i < g_PadsFile->numcovers; i++) 
		{
			coversDst->pos = coversSrc->pos;
			swap_coord(&coversDst->pos);

			coversDst->look = coversSrc->look;
			swap_coord(&coversDst->look);

			coversDst->flags = swap_uint16(coversSrc->flags);

			coversSrc++;
			coversDst++;
			fileWriteBuffer += sizeof(struct coverdefinition);
		}
	}

	if (g_StageSetup.cover != NULL) {
		setupPrepareCover();
	}

	// Promote offsets to pointers in waypoints
	waypoint = g_StageSetup.waypoints;

	while (waypoint->padnum >= 0) {
		waypoint->neighbours = (s32 *)((uintptr_t)g_StageSetup.padfiledata + (uintptr_t)waypoint->neighbours);
		waypoint++;
	}

	// Promote offsets to pointers in waygroups
	waygroup = g_StageSetup.waygroups;

	while (waygroup->neighbours != NULL) {
		waygroup->neighbours = (s32 *)((uintptr_t)g_StageSetup.padfiledata + (uintptr_t)waygroup->neighbours);
		waygroup->waypoints = (s32 *)((uintptr_t)g_StageSetup.padfiledata + (uintptr_t)waygroup->waypoints);
		waygroup++;
	}

	nativeFree(srcPadsFile);
}
