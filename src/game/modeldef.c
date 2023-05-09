#include <ultra64.h>
#include "constants.h"
#include "game/chraction.h"
//#include "game/ceil.h"
#include "game/bondgun.h"
#include "game/game_0b0fd0.h"
#include "game/tex.h"
#include "game/menugfx.h"
#include "game/menu.h"
#include "game/mainmenu.h"
#include "game/inv.h"
#include "game/game_1531a0.h"
#include "game/file.h"
#include "game/texdecompress.h"
#include "game/tex.h"
#include "game/modeldef.h"
#include "game/lang.h"
#include "game/mplayer/mplayer.h"
#include "game/options.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/main.h"
#include "lib/model.h"
#include "data.h"
#include "types.h"
#include "string.h"

#include "print.h"
#include "byteswap.h"
#include "gfx/hashmap.h"
#include "native_functions.h"
#include <stdlib.h>

#include "assets/AssetConverter.h"

struct skeleton *g_Skeletons[] = {
	&g_SkelChr,
	&g_SkelClassicGun,
	&g_Skel06,
	&g_SkelUzi,
	&g_SkelBasic,
	&g_SkelCctv,
	&g_SkelWindowedDoor,
	&g_Skel11,
	&g_Skel12,
	&g_Skel13,
	&g_SkelTerminal,
	&g_SkelCiHub,
	&g_SkelAutogun,
	&g_Skel17,
	&g_Skel18,
	&g_Skel19,
	&g_Skel0A,
	&g_Skel0B,
	&g_SkelCasing,
	&g_SkelChrGun,
	&g_Skel0C,
	&g_SkelJoypad,
	&g_SkelLift,
	&g_SkelSkedar,
	&g_SkelLogo,
	&g_SkelPdLogo,
	&g_SkelHoverbike,
	&g_SkelJumpship,
	&g_Skel20,
	&g_Skel21,
	&g_Skel22,
	&g_SkelLaptopGun,
	&g_SkelK7Avenger,
	&g_SkelChopper,
	&g_SkelFalcon2,
	&g_SkelKnife,
	&g_SkelDrCaroll,
	&g_SkelRope,
	&g_SkelCmp150,
	&g_SkelBanner,
	&g_SkelDragon,
	&g_SkelSuperDragon,
	&g_SkelRocket,
	&g_Skel4A,
	&g_SkelShotgun,
	&g_SkelFarsight,
	&g_Skel4D,
	&g_SkelReaper,
	&g_SkelDropship,
	&g_SkelMauler,
	&g_SkelDevastator,
	&g_SkelRobot,
	&g_SkelPistol,
	&g_SkelAr34,
	&g_SkelMagnum,
	&g_SkelSlayerRocket,
	&g_SkelCyclone,
	&g_SkelSniperRifle,
	&g_SkelTranquilizer,
	&g_SkelCrossbow,
	&g_SkelHudPiece,
	&g_SkelTimedProxyMine,
	&g_SkelPhoenix,
	&g_SkelCallisto,
	&g_SkelHand,
	&g_SkelRcp120,
	&g_SkelSkShuttle,
	&g_SkelLaser,
	&g_SkelMaianUfo,
	&g_SkelGrenade,
	&g_SkelCableCar,
	&g_SkelSubmarine,
	&g_SkelTarget,
	&g_SkelEcmMine,
	&g_SkelUplink,
	&g_SkelRareLogo,
	&g_SkelWireFence,
	&g_SkelRemoteMine,
	&g_SkelBB,
};

/*
void modeldefDebugGdl(Gfx* gdl)
{
	print("DBG GDL: %llx\n", gdl);
	while (true)
	{
		u8 opcode = gdl->words.w0 >> 24;
		if (opcode == (u8)G_ENDDL) break;

		debug_opcode(gdl, 0);
		gdl++;
	}
}*/

void modeldef0f1a7560(struct modeldef *modeldef, u16 filenum, u32 arg2, struct modeldef *modeldef2, struct texpool *texpool, u32 arg5)
{
	s32 allocsize;
	s32 loadedsize;
	s32 sp84;
	u32 previousGdlOffset;
	u32 gdlSizeBytes;
	uintptr_t newGdlOffset;
	struct modelnode *node;
	struct modelnode *prevnode;
	uintptr_t gdl;
	struct gfxvtx *vertices;

	allocsize = fileGetAllocationSize(filenum);
	loadedsize = fileGetLoadedSize(filenum);
	node = NULL;

	modelIterateDisplayLists(modeldef, &node, (Gfx **)&gdl);

	newGdlOffset = gdl;

	if (gdl) {
		/*
			v1 is the position where the first gdl will be copied, at the end of the allocated buffer
			texCopyGdls copies all the GDLs to the end
			I think the GDLs are at the very end of the file, so if we subtract the first gdl address from the loadedsize
			we can get the size of all GDLs.
		*/
		s32 v1 = allocsize - (loadedsize - (s32)(((uintptr_t)modeldef + (gdl & 0xffffff)) - (uintptr_t)modeldef));

		// sp84 is where the GDLs at the end of the file are located minus the first DL
		// this is used to locate where are located the DLs at the end of the buffer
		// in texLoadFromGdl
		sp84 = (s32)v1 + (s32)((uintptr_t)modeldef - ((uintptr_t)modeldef + (gdl & 0xffffff)));

		texCopyGdls((Gfx *)((uintptr_t)modeldef + (gdl & 0xffffff)),
				(Gfx *)(v1 + (uintptr_t)modeldef),
				loadedsize - (s32)(((uintptr_t)modeldef + (gdl & 0xffffff)) - (uintptr_t)modeldef));
				
		texLoadFromConfigs(modeldef->texconfigs, modeldef->numtexconfigs, texpool, (uintptr_t)modeldef2 - arg2);

		while (node) {
			prevnode = node;
			previousGdlOffset = gdl;

			modelIterateDisplayLists(modeldef, &node, (Gfx **) &gdl);

			if (gdl) {
				// s4 is the size of the previous display list
				gdlSizeBytes = gdl - previousGdlOffset;
			} else {
				// at the end of the file, we can just use the size of the file loaded minus the offset of the last gdl we've seen
				gdlSizeBytes = loadedsize + (uintptr_t)modeldef - (uintptr_t)modeldef - (previousGdlOffset & 0xffffff);
			}

			// The first run doesn't change anything because the start offset of the first GDL wont change
			// However, texLoadFromGdl might ouput a gdl of a different size
			// so we adjust the offsets of them as we go
			modelNodeReplaceGdl(modeldef, prevnode, (Gfx*)(uintptr_t)previousGdlOffset, (Gfx *) newGdlOffset);

			if (prevnode->type == MODELNODETYPE_DL) {
				struct modelrodata_dl *rodata = &prevnode->rodata->dl;
				vertices = rodata->vertices;
			} else {
				vertices = NULL;
			}

			// This will copy from the gdls at the end of the file, to the ones we first loaded
			Gfx* gdlAtEnd = (Gfx *)((uintptr_t)modeldef + (previousGdlOffset & 0xffffff) + sp84);
			Gfx* gdlAtStart = (Gfx*)((uintptr_t)modeldef + (newGdlOffset & 0xffffff));

			newGdlOffset += texLoadFromGdl(gdlAtEnd, 
				gdlSizeBytes, 
				gdlAtStart, 
				texpool, 
				(u8*)vertices
			);
		}

		fileSetSize(filenum, modeldef, (((uintptr_t)modeldef + (newGdlOffset & 0xffffff)) - (uintptr_t)modeldef + 0xf) & ~0xf, arg5);
	}

}

void modelPromoteTypeToPointer(struct modeldef *modeldef)
{
	s32 i;

	if ((u32)(uintptr_t)modeldef->skel < 0x10000) {
		// Note PC: this crashes because there is no zero after the end of the skeleton array
		//for (i = 0; g_Skeletons[i] != NULL; i++) {
		for (i = 0; g_Skeletons[i] != NULL && i < 79; i++) {
			if ((s16)(uintptr_t)modeldef->skel == g_Skeletons[i]->skel) {
				modeldef->skel = g_Skeletons[i];
				return;
			}
		}
	}
}

#define MDF_TO_PTR(base) ()

void printModelNode(struct modelnode* node)
{
	debugPrint(PC_DBG_FLAG_MODEL, "- node addr: %llx\n", node);
	debugPrint(PC_DBG_FLAG_MODEL, "    type %x rodata %llx parent %llx next %llx prev %llx child %llx\n", 
		node->type,  node->rodata, node->parent, node->next, node->prev,  node->child);
}

struct modelnode readModelnodeAtOffset(void* base, u32 offset)
{
	const u32 vma = 0x5000000;

	struct modelnode_load* node = (struct modelnode_load*)((uintptr_t)base - vma + offset);
	struct modelnode newNode = {
		swap_uint16(node->type),
		(union modelrodata*)(uintptr_t)swap_uint32(node->rodata),
		(struct modelnode*)(uintptr_t)swap_uint32(node->parent),
		(struct modelnode*)(uintptr_t)swap_uint32(node->next),
		(struct modelnode*)(uintptr_t)swap_uint32(node->prev),
		(struct modelnode*)(uintptr_t)swap_uint32(node->child),
	};

	return newNode;
}

u32 g_modeldefWriteOffset;
struct hashmap *offsetMap = NULL;
struct hashmap *rodataOffsetMap = NULL;
struct hashmap *gdlOffsetMap = NULL;
struct hashmap *vtxOffsetMap = NULL;
struct hashmap *colOffsetMap = NULL;
struct hashmap *texOffsetMap = NULL;

struct modeloffset_pc {
	u32 old, new;
	u32 size;
};

uint64_t modeloffset_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct modeloffset_pc *data = item;
    return hashmap_sip(item, sizeof(u32), seed0, seed1);
}

int modeloffset_compare(const void *a, const void *b, void *udata) {
    const struct modeloffset_pc *ua = a;
    const struct modeloffset_pc *ub = b;
    return ua->old - ub->old;
}

void writeModelnode(void* dst, u32 offset, struct modelnode* node)
{
	memcpy(dst + offset, node, sizeof(struct modelnode));
}

void traverseModelnodes(void* dst, void* base, u32 offset, u32 depth)
{
	// Only traverse nodes which are not already in the hashmap
	{
		struct modeloffset_pc moff = { offset };
		void* item = hashmap_get(offsetMap, &moff);
		if (item) return;
	}

	debugPrint(PC_DBG_FLAG_MODEL, "traverseModelnodes offset: %x depth %d\n", offset, depth);

	if (!offset) return;

	struct modelnode node = readModelnodeAtOffset(base, offset);
	printModelNode(&node);

	writeModelnode(dst, g_modeldefWriteOffset, &node);
	struct modeloffset_pc moff = {offset, g_modeldefWriteOffset};
	hashmap_set(offsetMap, &moff);
	g_modeldefWriteOffset += sizeof(struct modelnode);

	u32 child = (u32)(uintptr_t)node.child;
	if (child) {
		traverseModelnodes(dst, base, child, depth + 1);
	}

	u32 next = (u32)(uintptr_t)node.next;
	while (next) {
		traverseModelnodes(dst, base, next, depth);

		struct modelnode n = readModelnodeAtOffset(base, next);
		next = (u32)(uintptr_t)n.next;
	}

	
}

void* replaceOffset(struct hashmap* map, u32 old) 
{
	struct modeloffset_pc moff = { old };
	struct modeloffset_pc* item = hashmap_get(map, &moff);
	if (item) {
		return (void*)(uintptr_t)item->new;
	} else {
		// For now, ignore replacements for DL COL2 segment addrs
		if (old != 0 && old != 0x6000000) {
			debugPrint(PC_DBG_FLAG_MODEL, "ERROR, offset not found: %x\n", old);
			fatalExit();
		}
	}

	return (void*)(uintptr_t)old;
}

#define MODEL_VTX_SECTION_MAX_COUNT 128

struct model_vtx_section {
	u32 start, size;
};


struct modeldef *convertModeldef(struct modeldef* modeldef, u16 fileid, u8 *dst)
{
	struct modeldef_load *modeldef_load;
	u8* rawBuffer;
	u32 vtxSectionsCount = 0;
	struct model_vtx_section vtxSectionsSrc[MODEL_VTX_SECTION_MAX_COUNT];
	struct model_vtx_section vtxSectionsDst[MODEL_VTX_SECTION_MAX_COUNT];

	u32 allocsize = fileGetAllocationSize(fileid);
	u32 loadedsize = fileGetLoadedSize(fileid);

	debugPrint(PC_DBG_FLAG_MODEL, "allocsize %x loadedsize %x\n", allocsize, loadedsize);

	rawBuffer = nativeMalloc(loadedsize);
	memcpy(rawBuffer, modeldef, loadedsize); // probably not necessary?

	modeldef_load = (struct modeldef_load*)rawBuffer;

	u32 modeldefOffset = 0;
	g_modeldefWriteOffset = 0;
	const u32 vma = 0x5000000;

	// Modeldef write: copy the first structure
	modeldef->rootnode = 0;
	modeldef->skel = (struct skeleton *)(uintptr_t)swap_uint32(modeldef_load->skel);
	modeldef->parts = (struct modelnode**)NULL;
	modeldef->numparts = swap_int16(modeldef_load->numparts);
	modeldef->nummatrices = swap_int16(modeldef_load->nummatrices);
	modeldef->scale = swap_f32(modeldef_load->scale);
	modeldef->rwdatalen = swap_int16(modeldef_load->rwdatalen);
	modeldef->numtexconfigs = swap_int16(modeldef_load->numtexconfigs);
	modeldef->texconfigs = 0;

	modeldefOffset += sizeof (struct modeldef);
	debugPrint(PC_DBG_FLAG_MODEL, "modeldefOffset: %x (after modeldef)\n", modeldefOffset);

	debugPrint(PC_DBG_FLAG_MODEL, "modeldef %d\n", fileid);
	debugPrint(PC_DBG_FLAG_MODEL, "- numparts %d\n", modeldef->numparts);
	debugPrint(PC_DBG_FLAG_MODEL, "- nummatrices %d\n", modeldef->nummatrices);
	debugPrint(PC_DBG_FLAG_MODEL, "- scale %f\n", modeldef->scale);
	debugPrint(PC_DBG_FLAG_MODEL, "- rwdatalen %d\n", modeldef->rwdatalen);
	debugPrint(PC_DBG_FLAG_MODEL, "- numtexconfigs %d\n", modeldef->numtexconfigs);
	debugPrint(PC_DBG_FLAG_MODEL, "- textconfigs offset %x\n", swap_uint32(modeldef_load->texconfigs));

	// Modeldef write: copy the nodes
	// Starting with the rootnode
	struct modelnode_load* rootNode = (struct modelnode_load*)(
		(uintptr_t)modeldef_load - vma
		+ swap_uint32(modeldef_load->rootnode)
	);

	g_modeldefWriteOffset = modeldefOffset;
	const u32 NODES_SECTION_START = modeldefOffset;

	/*
		This function will rewrite a lot of offsets from the N64 format to the PC format
		Create hashmaps to keep track of old offsets and new offsets
		TODO: store all of this within a single hashmap?
	*/
    offsetMap = hashmap_new(sizeof(struct modeloffset_pc), 0, 0, 0, modeloffset_hash, modeloffset_compare, NULL, NULL);
    rodataOffsetMap = hashmap_new(sizeof(struct modeloffset_pc), 0, 0, 0, modeloffset_hash, modeloffset_compare, NULL, NULL);
    gdlOffsetMap = hashmap_new(sizeof(struct modeloffset_pc), 0, 0, 0, modeloffset_hash, modeloffset_compare, NULL, NULL);
    vtxOffsetMap = hashmap_new(sizeof(struct modeloffset_pc), 0, 0, 0, modeloffset_hash, modeloffset_compare, NULL, NULL);
    colOffsetMap = hashmap_new(sizeof(struct modeloffset_pc), 0, 0, 0, modeloffset_hash, modeloffset_compare, NULL, NULL);
    texOffsetMap = hashmap_new(sizeof(struct modeloffset_pc), 0, 0, 0, modeloffset_hash, modeloffset_compare, NULL, NULL);
	
	traverseModelnodes(modeldef, modeldef_load, swap_uint32(modeldef_load->rootnode), 0);
	modeldefOffset = g_modeldefWriteOffset;
	debugPrint(PC_DBG_FLAG_MODEL, "modeldefOffset: %x (after nodes)\n", modeldefOffset);

	const u32 RODATA_SECTION_START = modeldefOffset;

	// Once model nodes have been traversed once, we can replace the N64 structure offsets by the PC ones
	u32 modelNodesCount = hashmap_count(offsetMap);
	for (int i = 0; i < modelNodesCount; i++) {
		struct modelnode* node = (struct modelnode*)(
			((uintptr_t)modeldef) + NODES_SECTION_START + i * sizeof(struct modelnode)
		);

		debugPrint(PC_DBG_FLAG_MODEL, "modelnode ptr: %llx type: %x\n", node, node->type);

		node->child = replaceOffset(offsetMap, (u32)(uintptr_t)node->child);
		node->parent = replaceOffset(offsetMap, (u32)(uintptr_t)node->parent);
		node->next = replaceOffset(offsetMap, (u32)(uintptr_t)node->next);
		node->prev = replaceOffset(offsetMap, (u32)(uintptr_t)node->prev);

		// And load the rodata, convert it if necessary
		u32 type = node->type & 0xff;
		switch(type)
		{
			case MODELNODETYPE_POSITION: {
				struct modelrodata_position* rodataSrc = 
					(struct modelrodata_position*)((uintptr_t)modeldef_load - vma + (uintptr_t)node->rodata);
				
				struct modelrodata_position* rodataDst = (struct modelrodata_position*)((uintptr_t)modeldef + (uintptr_t)modeldefOffset);

				rodataDst->pos = rodataSrc->pos;
				swap_coord(&rodataDst->pos);
				rodataDst->part = swap_uint16(rodataSrc->part);
				rodataDst->mtxindex0 = swap_int16(rodataSrc->mtxindex0);
				rodataDst->mtxindex1 = swap_int16(rodataSrc->mtxindex1);
				rodataDst->mtxindex2 = swap_int16(rodataSrc->mtxindex2);
				rodataDst->drawdist = swap_f32(rodataSrc->drawdist);

				struct modeloffset_pc moff = { (u32)(uintptr_t)node->rodata, modeldefOffset};
				hashmap_set(rodataOffsetMap, &moff);

				modeldefOffset += sizeof(struct modelrodata_position);
				break;
			}

			case MODELNODETYPE_BBOX: {
				struct modelrodata_bbox* rodataSrc = 
					(struct modelrodata_bbox*)((uintptr_t)modeldef_load - vma + (uintptr_t)node->rodata);
				
				struct modelrodata_bbox* rodataDst = (struct modelrodata_bbox*)((uintptr_t)modeldef + (uintptr_t)modeldefOffset);

				rodataDst->hitpart = swap_int32(rodataSrc->hitpart);
				rodataDst->xmin = swap_f32(rodataSrc->xmin);
				rodataDst->xmax = swap_f32(rodataSrc->xmax);
				rodataDst->ymin = swap_f32(rodataSrc->ymin);
				rodataDst->ymax = swap_f32(rodataSrc->ymax);
				rodataDst->zmin = swap_f32(rodataSrc->zmin);
				rodataDst->zmax = swap_f32(rodataSrc->zmax);

				struct modeloffset_pc moff = { (u32)(uintptr_t)node->rodata, modeldefOffset };
				hashmap_set(rodataOffsetMap, &moff);

				modeldefOffset += sizeof(struct modelrodata_bbox);
				break;
			}

			case MODELNODETYPE_TOGGLE: {
				struct modelrodata_toggle_load* rodataSrc = 
					(struct modelrodata_toggle_load*)((uintptr_t)modeldef_load - vma + (uintptr_t)node->rodata);
				
				struct modelrodata_toggle* rodataDst = (struct modelrodata_toggle*)((uintptr_t)modeldef + (uintptr_t)modeldefOffset);

				rodataDst->target = replaceOffset(offsetMap, swap_uint32(rodataSrc->target));
				rodataDst->rwdataindex = swap_uint16(rodataDst->rwdataindex);

				struct modeloffset_pc moff = { (u32)(uintptr_t)node->rodata, modeldefOffset };
				hashmap_set(rodataOffsetMap, &moff);

				modeldefOffset += sizeof(struct modelrodata_toggle);

				break;
			}

			case MODELNODETYPE_GUNDL: {
				struct modelrodata_gundl_load* rodataSrc = 
					(struct modelrodata_gundl_load*)((uintptr_t)modeldef_load - vma + (uintptr_t)node->rodata);
				
				struct modelrodata_gundl* rodataDst = (struct modelrodata_gundl*)((uintptr_t)modeldef + (uintptr_t)modeldefOffset);
				
				// TODO: copy vertices

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

				rodataDst->opagdl = (Gfx*)(uintptr_t)swap_uint32(rodataSrc->opagdl);
				rodataDst->xlugdl = (Gfx*)(uintptr_t)swap_uint32(rodataSrc->xlugdl);
				rodataDst->baseaddr = (void*)(uintptr_t)swap_uint32(rodataSrc->baseaddr);
				rodataDst->vertices = (struct gfxvtx*)(uintptr_t)swap_uint32(rodataSrc->vertices);
				rodataDst->numvertices = swap_int16(rodataSrc->numvertices);
				rodataDst->unk12 = swap_int16(rodataSrc->unk12);

				struct modeloffset_pc moff = { (u32)(uintptr_t)node->rodata, modeldefOffset};
				hashmap_set(rodataOffsetMap, &moff);

				modeldefOffset += sizeof(struct modelrodata_gundl);

				// We can probably append the vertices next to it here
				// Or put them in the file before the display lists, dunno if it's important or not
				// Then we can parse the display list, look for vertex calls and adjust the offsets there?
				struct gfxvtx* vtxptr = (struct gfxvtx*)((uintptr_t)modeldef_load - (uintptr_t)vma + (uintptr_t)swap_uint32(rodataSrc->vertices));
				struct gfxvtx* vtxdst = (struct gfxvtx*)((uintptr_t)modeldef + (uintptr_t)modeldefOffset);

				struct modeloffset_pc moff2 = { swap_uint32(rodataSrc->vertices), modeldefOffset, sizeof(struct gfxvtx) * rodataDst->numvertices };
				hashmap_set(vtxOffsetMap, &moff2);

				rodataDst->vertices = (struct gfxvtx*)(uintptr_t)modeldefOffset;
				debugPrint(PC_DBG_FLAG_MODEL, "write vertices at: %x\n", rodataDst->vertices);
				
				vtxSectionsSrc[vtxSectionsCount].start = swap_uint32(rodataSrc->vertices);
				vtxSectionsSrc[vtxSectionsCount].size = rodataDst->numvertices * sizeof(struct gfxvtx);

				vtxSectionsDst[vtxSectionsCount].start = modeldefOffset;
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

					modeldefOffset += sizeof(struct gfxvtx);
				}

				// It seems the color table is located right after the vertices
				// We don't know the size of the color table, so let's go through the display list commands
				Gfx* cmds[2] = {
					rodataDst->opagdl != 0 ? (Gfx*)((uintptr_t)modeldef_load - (uintptr_t)vma + (uintptr_t)rodataDst->opagdl) : 0,
					rodataDst->xlugdl != 0 ? (Gfx*)((uintptr_t)modeldef_load - (uintptr_t)vma + (uintptr_t)rodataDst->xlugdl) : 0,
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
							u32* colorSrc = (u32*)((uintptr_t)modeldef_load + swap_uint32(cmd->words.w1) - (uintptr_t)vma);
							u32* colorDst = (u32*)((uintptr_t)modeldef + (uintptr_t)modeldefOffset);
							for (int j = 0; j < colorBufferSize / 4; j++)
							{
								colorDst[j] = swap_uint32(colorSrc[j]);
							}

							struct modeloffset_pc moff3 = { swap_uint32(cmd->words.w1), modeldefOffset, colorBufferSize };
							hashmap_set(colOffsetMap, &moff3);

							modeldefOffset += colorBufferSize;
						}

						cmd++;
					}
				}

				break;
			}

			case MODELNODETYPE_REORDER: {
				struct modelrodata_reorder_load* rodataSrc = 
					(struct modelrodata_reorder_load*)((uintptr_t)modeldef_load - vma + (uintptr_t)node->rodata);
				
				struct modelrodata_reorder* rodataDst = (struct modelrodata_reorder*)((uintptr_t)modeldef + (uintptr_t)modeldefOffset);

				rodataDst->unk00 = swap_f32(rodataSrc->unk00);
				rodataDst->unk04 = swap_f32(rodataSrc->unk04);
				rodataDst->unk08 = swap_f32(rodataSrc->unk08);
				rodataDst->unk0c[0] = swap_f32(rodataSrc->unk0c[0]);
				rodataDst->unk0c[1] = swap_f32(rodataSrc->unk0c[1]);
				rodataDst->unk0c[2] = swap_f32(rodataSrc->unk0c[2]);
				rodataDst->side = swap_int16(rodataSrc->side);
				rodataDst->rwdataindex = swap_uint16(rodataSrc->rwdataindex);

				rodataDst->unk18 = (struct modelnode*)(uintptr_t)replaceOffset(offsetMap, swap_uint32(rodataSrc->unk18));
				rodataDst->unk1c = (struct modelnode*)(uintptr_t)replaceOffset(offsetMap, swap_uint32(rodataSrc->unk1c));

				struct modeloffset_pc moff = { (u32)(uintptr_t)node->rodata, modeldefOffset};
				hashmap_set(rodataOffsetMap, &moff);

				modeldefOffset += sizeof(struct modelrodata_reorder);
				break;
			}

			case MODELNODETYPE_DL: {
				struct modelrodata_dl_load* rodataSrc = 
					(struct modelrodata_dl_load*)((uintptr_t)modeldef_load - vma + (uintptr_t)node->rodata);
				
				struct modelrodata_dl* rodataDst = (struct modelrodata_dl*)((uintptr_t)modeldef + (uintptr_t)modeldefOffset);
				
				debugPrint(PC_DBG_FLAG_MODEL, "- opagdl: %x\n", swap_uint32(rodataSrc->opagdl));
				debugPrint(PC_DBG_FLAG_MODEL, "- xlugdl: %x\n", swap_uint32(rodataSrc->xlugdl));
				debugPrint(PC_DBG_FLAG_MODEL, "- colourtable: %x\n", swap_uint32(rodataSrc->colourtable));
				debugPrint(PC_DBG_FLAG_MODEL, "- vertices: %x\n", swap_uint32(rodataSrc->vertices));
				debugPrint(PC_DBG_FLAG_MODEL, "- numvertices: %x\n", swap_int16(rodataSrc->numvertices));
				debugPrint(PC_DBG_FLAG_MODEL, "- mcount: %x\n", swap_int16(rodataSrc->mcount));
				debugPrint(PC_DBG_FLAG_MODEL, "- rwdataindex: %x\n", swap_uint16(rodataSrc->rwdataindex));
				debugPrint(PC_DBG_FLAG_MODEL, "- numcolours: %x\n", swap_uint16(rodataSrc->numcolours));

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

				struct modeloffset_pc moff = { (u32)(uintptr_t)node->rodata, modeldefOffset };
				hashmap_set(rodataOffsetMap, &moff);

				modeldefOffset += sizeof(struct modelrodata_dl);

				// Copy the vertices
				struct gfxvtx* vtxptr = (struct gfxvtx*)((uintptr_t)modeldef_load - (uintptr_t)vma + (uintptr_t)swap_uint32(rodataSrc->vertices));
				struct gfxvtx* vtxdst = (struct gfxvtx*)((uintptr_t)modeldef + (uintptr_t)modeldefOffset);

				struct modeloffset_pc moff2 = { swap_uint32(rodataSrc->vertices), modeldefOffset, sizeof(struct gfxvtx) * rodataDst->numvertices };
				hashmap_set(vtxOffsetMap, &moff2);

				rodataDst->vertices = (struct gfxvtx*)(uintptr_t)modeldefOffset;
				debugPrint(PC_DBG_FLAG_MODEL, "write vertices at: %x\n", rodataDst->vertices);
				
				vtxSectionsSrc[vtxSectionsCount].start = swap_uint32(rodataSrc->vertices);
				vtxSectionsSrc[vtxSectionsCount].size = rodataDst->numvertices * sizeof(struct gfxvtx);

				vtxSectionsDst[vtxSectionsCount].start = modeldefOffset;
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

					modeldefOffset += sizeof(struct gfxvtx);
				}

				// Copy the colors
				u8 colorBufferSize = rodataDst->numcolours * 4;

				// Copy the colors to the new buffer
				u32* colorSrc = (u32*)((uintptr_t)modeldef_load + (uintptr_t)colourptr - (uintptr_t)vma);
				u32* colorDst = (u32*)((uintptr_t)modeldef + (uintptr_t)modeldefOffset);
				for (int j = 0; j < colorBufferSize / 4; j++)
				{
					colorDst[j] = swap_uint32(colorSrc[j]);
				}
				
				struct modeloffset_pc moff3 = { (uintptr_t)colourptr, modeldefOffset, colorBufferSize };
				hashmap_set(colOffsetMap, &moff3);

				modeldefOffset += colorBufferSize;
			
				break;
			}

			case MODELNODETYPE_CHRINFO: {
				struct modelrodata_chrinfo* rodataSrc = (struct modelrodata_chrinfo*)((uintptr_t)modeldef_load - vma + (uintptr_t)node->rodata);
				struct modelrodata_chrinfo* rodataDst = (struct modelrodata_chrinfo*)((uintptr_t)modeldef + (uintptr_t)modeldefOffset);
				
				rodataDst->animpart = swap_uint16(rodataSrc->animpart);
				rodataDst->mtxindex = swap_int16(rodataSrc->mtxindex);
				rodataDst->unk04 = swap_f32(rodataSrc->unk04);
				rodataDst->rwdataindex = swap_uint16(rodataSrc->rwdataindex);

				struct modeloffset_pc moff = { (u32)(uintptr_t)node->rodata, modeldefOffset };
				hashmap_set(rodataOffsetMap, &moff);

				modeldefOffset += sizeof(struct modelrodata_chrinfo);
				break;
			}

			case MODELNODETYPE_HEADSPOT: {
				struct modelrodata_headspot* rodataSrc = (struct modelrodata_headspot*)((uintptr_t)modeldef_load - vma + (uintptr_t)node->rodata);
				struct modelrodata_headspot* rodataDst = (struct modelrodata_headspot*)((uintptr_t)modeldef + (uintptr_t)modeldefOffset);
				
				rodataDst->rwdataindex = swap_uint16(rodataSrc->rwdataindex);

				struct modeloffset_pc moff = { (u32)(uintptr_t)node->rodata, modeldefOffset };
				hashmap_set(rodataOffsetMap, &moff);

				modeldefOffset += sizeof(struct modelrodata_headspot);
				break;
			}

			case MODELNODETYPE_DISTANCE: {
				struct modelrodata_distance_load* rodataSrc = (struct modelrodata_distance_load*)((uintptr_t)modeldef_load - vma + (uintptr_t)node->rodata);
				struct modelrodata_distance* rodataDst = (struct modelrodata_distance*)((uintptr_t)modeldef + (uintptr_t)modeldefOffset);
				
				rodataDst->near = swap_f32(rodataSrc->near);
				rodataDst->far = swap_f32(rodataSrc->far);
				rodataDst->target = replaceOffset(offsetMap, swap_uint32(rodataSrc->target));
				rodataDst->rwdataindex = swap_uint16(rodataSrc->rwdataindex);
				
				struct modeloffset_pc moff = { (u32)(uintptr_t)node->rodata, modeldefOffset };
				hashmap_set(rodataOffsetMap, &moff);

				modeldefOffset += sizeof(struct modelrodata_distance);
				break;
			}

			case 0x19: {
				struct modelrodata_type19* rodataSrc = (struct modelrodata_type19*)((uintptr_t)modeldef_load - vma + (uintptr_t)node->rodata);
				struct modelrodata_type19* rodataDst = (struct modelrodata_type19*)((uintptr_t)modeldef + (uintptr_t)modeldefOffset);
				
				rodataDst->numvertices = swap_int32(rodataSrc->numvertices);
				for (u32 i = 0; i < 4; i++)
				{
					rodataDst->vertices[i] = rodataSrc->vertices[i];
					swap_coord(&rodataDst->vertices[i]);
				}
				
				struct modeloffset_pc moff = { (u32)(uintptr_t)node->rodata, modeldefOffset };
				hashmap_set(rodataOffsetMap, &moff);

				modeldefOffset += sizeof(struct modelrodata_type19);
				break;
			}

			case MODELNODETYPE_CHRGUNFIRE: {
				struct modelrodata_chrgunfire_load* rodataSrc = (struct modelrodata_chrgunfire_load*)((uintptr_t)modeldef_load - vma + (uintptr_t)node->rodata);
				struct modelrodata_chrgunfire* rodataDst = (struct modelrodata_chrgunfire*)((uintptr_t)modeldef + (uintptr_t)modeldefOffset);
				
				rodataDst->pos = rodataSrc->pos;
				swap_coord(&rodataDst->pos);

				rodataDst->dim = rodataSrc->dim;
				swap_coord(&rodataDst->dim);

				rodataDst->texture = (struct textureconfig*)(uintptr_t)swap_uint32(rodataSrc->texture);
				rodataDst->unk1c = swap_f32(rodataSrc->unk1c);
				rodataDst->rwdataindex = swap_uint16(rodataSrc->rwdataindex);
				rodataDst->baseaddr = (void*)(uintptr_t)swap_uint32(rodataSrc->baseaddr);

				struct modeloffset_pc moff = { (u32)(uintptr_t)node->rodata, modeldefOffset };
				hashmap_set(rodataOffsetMap, &moff);

				modeldefOffset += sizeof(struct modelrodata_chrgunfire);
				break;
			}

			case MODELNODETYPE_POSITIONHELD: {
				struct modelrodata_positionheld* rodataSrc = (struct modelrodata_positionheld*)((uintptr_t)modeldef_load - vma + (uintptr_t)node->rodata);
				struct modelrodata_positionheld* rodataDst = (struct modelrodata_positionheld*)((uintptr_t)modeldef + (uintptr_t)modeldefOffset);
				
				rodataDst->pos = rodataSrc->pos;
				swap_coord(&rodataDst->pos);

				rodataDst->mtxindex = swap_int16(rodataSrc->mtxindex);

				struct modeloffset_pc moff = { (u32)(uintptr_t)node->rodata, modeldefOffset };
				hashmap_set(rodataOffsetMap, &moff);

				modeldefOffset += sizeof(struct modelrodata_positionheld);
				break;
			}

			case MODELNODETYPE_11: {
				struct modelrodata_type11_load* rodataSrc = (struct modelrodata_type11_load*)((uintptr_t)modeldef_load - vma + (uintptr_t)node->rodata);
				struct modelrodata_type11* rodataDst = (struct modelrodata_type11*)((uintptr_t)modeldef + (uintptr_t)modeldefOffset);
				
				rodataDst->unk00 = swap_uint32(rodataSrc->unk00);
				rodataDst->unk04 = swap_uint32(rodataSrc->unk04);
				rodataDst->unk08 = swap_uint32(rodataSrc->unk08);
				rodataDst->unk0c = swap_uint32(rodataSrc->unk0c);
				rodataDst->unk10 = swap_uint32(rodataSrc->unk10);
				rodataDst->unk14 = (void*)(uintptr_t)swap_uint32(rodataSrc->unk14);

				struct modeloffset_pc moff = { (u32)(uintptr_t)node->rodata, modeldefOffset };
				hashmap_set(rodataOffsetMap, &moff);

				modeldefOffset += sizeof(struct modelrodata_type11);
				break;
			}
			
			default:
				debugPrint(PC_DBG_FLAG_MODEL, "ERROR: unhandled modelnode type: %x\n", type);
				fatalExit();
				break;
		}
	}

	// Adjust rodata offsets
	for (int i = 0; i < modelNodesCount; i++) {
		struct modelnode* node = (struct modelnode*)(
			((uintptr_t)modeldef) + NODES_SECTION_START + i * sizeof(struct modelnode)
		);
		
		node->rodata = replaceOffset(rodataOffsetMap, (u32)(uintptr_t)node->rodata);
	}
	
	// Adjust offsets in the main modeldef structure
	modeldef->rootnode = (struct modelnode*)(uintptr_t)NODES_SECTION_START;

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
	const u32 PARTS_SECTION_START = modeldefOffset;
	debugPrint(PC_DBG_FLAG_MODEL, "modeldefOffset: %x (after parts)\n", modeldefOffset);

	for (int i = 0; i < modeldef->numparts; i++) {
		u32* parts = (u32*)(
			(uintptr_t)modeldef_load - vma 
			+ swap_uint32(modeldef_load->parts)
			+ 4 * i);
		
		u32 partOffset = swap_uint32(*parts);

		// Using the N64 offset, we can fill the "parts" offsets array
		u32 newOffset = (u32)(uintptr_t)replaceOffset(offsetMap, partOffset);
		u64* ptr = (u64*)((uintptr_t)modeldef + (uintptr_t)PARTS_SECTION_START + i * sizeof(void*));
		*ptr = (u64)newOffset;

		modeldefOffset += sizeof(void*);
	}

	modeldef->parts = (struct modelnode**)(uintptr_t)PARTS_SECTION_START;

	// Is followed by a variable s16 array of part numbers... Write it to the file
	const u32 PARTNUM_SECTION_START = modeldefOffset;

	s16* partnums = (s16*)((uintptr_t)modeldef_load - vma + swap_uint32(modeldef_load->parts) + 4 * modeldef->numparts);
	for (int i = 0; i < modeldef->numparts; i++) {
		s16* ptr = (s16*)((uintptr_t)modeldef + (uintptr_t)PARTNUM_SECTION_START + i * sizeof(s16));
		*ptr = swap_int16(partnums[i]);

		modeldefOffset += sizeof(s16);
	}
	
	const u32 TEXCONFIGS_SECTION_START = modeldefOffset;
	const u32 texconfigsOffset = swap_uint32(modeldef_load->texconfigs);

	for (int i = 0; i < modeldef->numtexconfigs; i++) {
		struct textureconfig_load* src = (struct textureconfig_load*)((uintptr_t)modeldef_load - vma + texconfigsOffset);
		struct textureconfig* dst = (struct textureconfig*)((uintptr_t)modeldef + modeldefOffset);
		
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

		modeldefOffset += sizeof(struct textureconfig);
	}

	modeldef->texconfigs = (struct textureconfig*)(uintptr_t)TEXCONFIGS_SECTION_START;

	/* --------------------
		TEXTURES
	   -------------------- */
	const u32 TEXDATA_SECTION_START = modeldefOffset;

	for (int i = 0; i < modeldef->numtexconfigs; i++) {
		struct textureconfig* list = (struct textureconfig*)((uintptr_t)modeldef + (uintptr_t)modeldef->texconfigs);
		struct textureconfig* tex = &list[i];

		// Sometimes the texture is embedded in the modeldef file, sometimes it's not
		u32 segment = (tex->texturenum & 0xf000000) >> 24;
		if (segment == 0x5) {
			debugPrint(PC_DBG_FLAG_MODEL, "Modeldef embedded texture\n");

			u32 oldTextureOffset = tex->texturenum;
			void* textureAddr = (void*)((uintptr_t)modeldef_load - vma + (uintptr_t)tex->texturenum);
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

			void* textureDst = (void*)((uintptr_t)modeldef + (uintptr_t)modeldefOffset);
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

			texSwapAltRowBytes2(textureDst, tex->width, tex->height, format);

			tex->texturenum = modeldefOffset;

			// Put the vma offset in the texturenum offset
			// texLoad will detect it and know this texture is already loaded in RAM
			tex->texturenum = modeldefOffset | vma;

			debugPrint(PC_DBG_FLAG_MODEL, "old embedded texture offset %x new offset %x\n", oldTextureOffset, modeldefOffset);

			struct modeloffset_pc moff = { oldTextureOffset, modeldefOffset };
			hashmap_set(texOffsetMap, &moff);

			modeldefOffset += len;
		} else {
			// The texture is probably not embedded
			debugPrint(PC_DBG_FLAG_MODEL, "External texture\n");
		}
	}
	
	modelPromoteTypeToPointer(modeldef);


	
	//modelPromoteOffsetsToPointers(modeldef, 0x5000000, (uintptr_t) modeldef);
	modelPromoteOffsetsToPointers(modeldef, 0x0, (uintptr_t) modeldef);

	// Copy the GDLs
	const u32 GDL_SECTION_START = modeldefOffset;
	{
		struct modelnode* node = NULL;
		struct modelnode* prevnode = NULL;
		Gfx* prevGdl, *gdl;
		
		modelIterateDisplayLists(modeldef, &node, (Gfx **)&gdl);
		while (gdl) {
			prevnode = node;
			prevGdl = gdl;

			modelIterateDisplayLists(modeldef, &node, (Gfx **) &gdl);

			u32 gdlSize;
			if (gdl) {
				gdlSize = ((uintptr_t)gdl & 0xffffff) - ((uintptr_t)prevGdl & 0xffffff);
			} else {
				gdlSize = loadedsize - ((uintptr_t)prevGdl & 0xffffff);
			}

			debugPrint(PC_DBG_FLAG_MODEL, "loadedsize: %x prevGdl: %x gdlSize: %x\n", loadedsize, prevGdl, gdlSize);

			memcpy((void*)((uintptr_t)modeldef + modeldefOffset), 
				(void*)((uintptr_t)modeldef_load + ((uintptr_t)prevGdl & 0xffffff)), 
				gdlSize
			);

			// Adjust the endianness of each display list element
			u32* dlptr = (u32*)((uintptr_t)modeldef + modeldefOffset);
			for (u32 i = 0; i < gdlSize / sizeof(u32); i++) {
				dlptr[i] = swap_uint32(dlptr[i]);
			}

			// Look for some commands such as G_VTX and adjust the adresses in them
			Gfx* dst = (Gfx*)((uintptr_t)modeldef + modeldefOffset);
			for (u32 i = 0; i < gdlSize / sizeof(Gfx); i++) {
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

					u32 newColorOffset = (u32)(uintptr_t)replaceOffset(colOffsetMap, dst[i].words.w1) | 0x5000000;
					dst[i].words.w1 = newColorOffset;
				} 
				else if (opcode == (u8)G_SETTIMG) 
				{
					u32 oldOffset = dst[i].words.w1;
					u32 newOffset = (u32)(uintptr_t)replaceOffset(texOffsetMap, oldOffset) | 0x5000000; 
					dst[i].words.w1 = newOffset;
				}
			}

			struct modeloffset_pc moff = {(u32)(uintptr_t)prevGdl, modeldefOffset};
			hashmap_set(gdlOffsetMap, &moff);

			modeldefOffset += gdlSize;
		}
	}

	debugPrint(PC_DBG_FLAG_MODEL, "End of GDL copy\n");

	// Iterates nodes again and replace gdl offsets by the new ones
	for (int i = 0; i < modelNodesCount; i++) {
		struct modelnode* node = (struct modelnode*)(
			((uintptr_t)modeldef) + NODES_SECTION_START + i * sizeof(struct modelnode)
		);

		u32 type = node->type & 0xff;
		switch(type)
		{
			case MODELNODETYPE_GUNDL: {
				node->rodata->gundl.xlugdl = replaceOffset(gdlOffsetMap, (u32)(uintptr_t)node->rodata->gundl.xlugdl);
				node->rodata->gundl.opagdl = replaceOffset(gdlOffsetMap, (u32)(uintptr_t)node->rodata->gundl.opagdl);
				break;
			}

			case MODELNODETYPE_DL: {
				node->rodata->dl.xlugdl = replaceOffset(gdlOffsetMap, (u32)(uintptr_t)node->rodata->dl.xlugdl);
				node->rodata->dl.opagdl = replaceOffset(gdlOffsetMap, (u32)(uintptr_t)node->rodata->dl.opagdl);
				break;
			}
		}
	}

	// Here, the file is converted, however it could be larger than the original file size
	debugPrint(PC_DBG_FLAG_MODEL, "# Previous file size: %x\n", loadedsize);
	debugPrint(PC_DBG_FLAG_MODEL, "# New file size: %x\n", modeldefOffset);

	if (modeldefOffset > loadedsize) {
		debugPrint(PC_DBG_FLAG_MODEL, "# Larger file, calling fileSetLoadedsize...\n");
		fileSetLoadedsize(fileid, modeldefOffset);
	}

	hashmap_free(offsetMap);
	hashmap_free(rodataOffsetMap);
	hashmap_free(gdlOffsetMap);
	hashmap_free(vtxOffsetMap);
	hashmap_free(colOffsetMap);
	hashmap_free(texOffsetMap);

	nativeFree(rawBuffer);

	return modeldef;
}

struct modeldef *modeldefLoad(u16 fileid, u8 *dst, s32 size, struct texpool *arg3)
{
	struct modeldef *modeldef;

	AssetConvertModeldef(fileid);
	
	g_LoadType = LOADTYPE_MODEL;

	if (dst) {
		modeldef = fileLoadToAddr(fileid, FILELOADMETHOD_EXTRAMEM, dst, size);
	} else {
		modeldef = fileLoadToNew(fileid, FILELOADMETHOD_EXTRAMEM);
	}

	modeldef = convertModeldef(modeldef, fileid, dst);

	modeldef0f1a7560(modeldef, fileid, 0x5000000, modeldef, arg3, dst == NULL);

	return modeldef;
}

struct modeldef *modeldefLoadToNew(u16 fileid)
{
	return modeldefLoad(fileid, NULL, 0, NULL);
}

struct modeldef *modeldefLoadToAddr(u16 fileid, u8 *dst, s32 size)
{
	return modeldefLoad(fileid, dst, size, NULL);
}
